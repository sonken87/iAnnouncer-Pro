// skeuo-ui — map widget (shared by the Rust and C++ implementations).
//
// A component renders
//
//   <div class="skeuo-map ..." data-skeuo-map='{ ...config JSON... }'>
//     <div class="skeuo-map__viewport" data-sk-ignore></div>   <- Leaflet lives here
//     <input type="hidden" class="skeuo-map__events">          <- events come out here
//   </div>
//
// This script watches the page for such elements, creates a Leaflet map for
// each, redraws it whenever `data-skeuo-map` changes, and reports what the
// user does by writing a JSON event into the hidden input and firing an
// `input` event on it (which both Dioxus and the C++ runtime deliver to the
// component's oninput handler).
//
// Config (all optional except center/zoom):
//   center [lat, lon], zoom, view (number; when it changes the view is re-applied),
//   fit_routes (fit all routes/markers instead of center/zoom when applying the view),
//   tiles {url, attribution, max_zoom, subdomains}, min_zoom, max_zoom,
//   interactive, scroll_zoom, controls, scale, add_to (route id), report_view,
//   routes [{id, color, geodesic, labels, editable, selected,
//            waypoints [{ident, name, lat, lon, kind, alt_ft}]}],
//   markers [{id, lat, lon, kind, label, color, heading}],
//   circles [{id, lat, lon, radius_nm, color, label}]
// Events:
//   {type:"click", lat, lon}                      map clicked (no add_to)
//   {type:"add", route, lat, lon}                 map clicked with add_to set
//   {type:"move", route, index, lat, lon}         waypoint dragged
//   {type:"insert", route, index, lat, lon}       leg midpoint handle dragged (new index)
//   {type:"select", route, index}                 waypoint clicked
//   {type:"remove", route, index}                 waypoint double-clicked / context-clicked
//   {type:"marker", id}                           marker clicked
//   {type:"view", lat, lon, zoom}                 pan/zoom ended (only with report_view)
(() => {
  "use strict";
  if (window.__skeuoMap) return;
  const L = window.L;
  if (!L) {
    console.error("skeuo-ui map: Leaflet (window.L) is not loaded");
    return;
  }

  const EARTH_NM = 3440.065;  // mean earth radius in nautical miles
  const RAD = Math.PI / 180;
  const maps = new Set();

  // ---------------------------------------------------------------- geodesy
  // Same formulas as the Rust/C++ `geo` modules (spherical earth).
  function toVec(lat, lon) {
    const p = lat * RAD, l = lon * RAD;
    return [Math.cos(p) * Math.cos(l), Math.cos(p) * Math.sin(l), Math.sin(p)];
  }
  function centralAngle(a, b) {
    const dLat = (b.lat - a.lat) * RAD, dLon = (b.lon - a.lon) * RAD;
    const h = Math.sin(dLat / 2) ** 2 + Math.cos(a.lat * RAD) * Math.cos(b.lat * RAD) * Math.sin(dLon / 2) ** 2;
    return 2 * Math.asin(Math.min(1, Math.sqrt(h)));
  }
  function distanceNm(a, b) { return centralAngle(a, b) * EARTH_NM; }
  function courseDeg(a, b) {
    const p1 = a.lat * RAD, p2 = b.lat * RAD, dl = (b.lon - a.lon) * RAD;
    const y = Math.sin(dl) * Math.cos(p2);
    const x = Math.cos(p1) * Math.sin(p2) - Math.sin(p1) * Math.cos(p2) * Math.cos(dl);
    return (Math.atan2(y, x) / RAD + 360) % 360;
  }
  // Points along the great circle from a to b (spherical linear interpolation).
  function greatCircle(a, b) {
    const d = centralAngle(a, b);
    const steps = Math.max(1, Math.ceil(d / RAD));  // about one point per degree of arc
    const va = toVec(a.lat, a.lon), vb = toVec(b.lat, b.lon);
    const out = [];
    for (let i = 0; i <= steps; i++) {
      const f = i / steps;
      if (d < 1e-9) { out.push([a.lat, a.lon]); continue; }
      const A = Math.sin((1 - f) * d) / Math.sin(d), B = Math.sin(f * d) / Math.sin(d);
      const x = A * va[0] + B * vb[0], y = A * va[1] + B * vb[1], z = A * va[2] + B * vb[2];
      out.push([Math.atan2(z, Math.hypot(x, y)) / RAD, Math.atan2(y, x) / RAD]);
    }
    return out;
  }
  function midpoint(a, b) {
    const pts = greatCircle(a, b);
    if (pts.length > 2) return pts[Math.floor(pts.length / 2)];
    return [(a.lat + b.lat) / 2, (a.lon + b.lon) / 2];
  }
  // Keeps longitudes continuous so a path crossing the antimeridian is drawn
  // the short way round instead of across the whole map.
  function unwrap(points) {
    const out = [];
    let offset = 0, prev = null;
    for (const [lat, lon] of points) {
      if (prev !== null) {
        const d = lon + offset - prev;
        if (d > 180) offset -= 360;
        else if (d < -180) offset += 360;
      }
      prev = lon + offset;
      out.push([lat, prev]);
    }
    return out;
  }

  // The copy of `lon` (±360°) closest to `ref`, so standalone markers show
  // on the same world copy as the routes.
  function nearLon(lon, ref) {
    return lon + 360 * Math.round((ref - lon) / 360);
  }

  // ---------------------------------------------------------------- symbols
  // Aeronautical-chart style symbols, drawn as SVG and colored by CSS
  // (currentColor = the route/marker palette color).
  const SYMBOLS = {
    airport: '<circle cx="12" cy="12" r="6.5" fill="var(--skm-fill)" stroke="currentColor" stroke-width="2"/>' +
      '<path d="M12 2.5v3.5M12 18v3.5M2.5 12H6M18 12h3.5" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>',
    vor: '<path d="M8 5h8l4 7-4 7H8l-4-7z" fill="var(--skm-fill)" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"/>' +
      '<circle cx="12" cy="12" r="1.8" fill="currentColor"/>',
    ndb: '<circle cx="12" cy="12" r="7.5" fill="none" stroke="currentColor" stroke-width="1.8" stroke-dasharray="1.2 2.4" stroke-linecap="round"/>' +
      '<circle cx="12" cy="12" r="3" fill="currentColor"/>',
    fix: '<path d="M12 4.5l7.5 13h-15z" fill="var(--skm-fill)" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"/>',
    user: '<path d="M12 4l7 8-7 8-7-8z" fill="var(--skm-fill)" stroke="currentColor" stroke-width="1.8" stroke-linejoin="round"/>',
    dot: '<circle cx="12" cy="12" r="5" fill="currentColor"/>',
    aircraft: '<path d="M12 2.5c.9 0 1.5.8 1.5 1.8v5.2l7.5 4.3v2l-7.5-2.3v4.6l2.2 1.6v1.6L12 20.4l-3.7.9v-1.6l2.2-1.6v-4.6L3 15.8v-2l7.5-4.3V4.3c0-1 .6-1.8 1.5-1.8z" fill="currentColor" stroke="var(--skm-fill)" stroke-width="1" stroke-linejoin="round"/>',
  };
  function symbolSvg(kind) {
    return `<svg viewBox="0 0 24 24" width="24" height="24" aria-hidden="true">${SYMBOLS[kind] || SYMBOLS.user}</svg>`;
  }
  function esc(s) {
    return String(s == null ? "" : s).replace(/[&<>"']/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c]));
  }
  function colorClass(c) { return "skeuo-color-" + (c || "primary"); }

  // ---------------------------------------------------------------- controls
  // Zoom / fit keycaps, top-left.
  const KeyControl = L.Control.extend({
    options: { position: "topleft" },
    onAdd(map) {
      const box = L.DomUtil.create("div", "skeuo-map__keys");
      const key = (label, title, fn, extra) => {
        const b = L.DomUtil.create("button", "skeuo-key skeuo-key--pressable skeuo-map__key " + (extra || ""), box);
        b.type = "button";
        b.title = title;
        b.setAttribute("aria-label", title);
        b.innerHTML = label;
        L.DomEvent.disableClickPropagation(b);
        L.DomEvent.on(b, "click", (e) => { L.DomEvent.preventDefault(e); fn(); });
        return b;
      };
      key('<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M12 5v14M5 12h14"/></svg>', "Zoom in", () => map.zoomIn());
      key('<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"><path d="M5 12h14"/></svg>', "Zoom out", () => map.zoomOut());
      this.fitKey = key('<svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round"><path d="M4 9V4h5M20 9V4h-5M4 15v5h5M20 15v5h-5"/></svg>', "Fit route", () => this.options.onFit(), "skeuo-map__key--fit");
      return box;
    },
  });

  // Nautical-mile scale bar, bottom-left.
  const NM_STEPS = [0.5, 1, 2, 5, 10, 20, 25, 50, 100, 200, 250, 500, 1000, 2000];
  const ScaleControl = L.Control.extend({
    options: { position: "bottomleft", maxWidth: 110 },
    onAdd(map) {
      this.el = L.DomUtil.create("div", "skeuo-map__scale");
      this.bar = L.DomUtil.create("div", "skeuo-map__scale-bar", this.el);
      this.label = L.DomUtil.create("span", "skeuo-map__scale-label", this.el);
      map.on("move zoom resize", this.update, this);
      this.update();
      return this.el;
    },
    onRemove(map) { map.off("move zoom resize", this.update, this); },
    update() {
      const map = this._map, size = map.getSize();
      const y = size.y / 2;
      const a = map.containerPointToLatLng([0, y]), b = map.containerPointToLatLng([this.options.maxWidth, y]);
      const nm = a.distanceTo(b) / 1852;
      let step = NM_STEPS[0];
      for (const s of NM_STEPS) if (s <= nm) step = s;
      this.bar.style.width = Math.round(this.options.maxWidth * step / nm) + "px";
      this.label.textContent = step + " NM";
    },
  });

  // ---------------------------------------------------------------- one map
  class SkeuoMap {
    constructor(root) {
      this.root = root;
      this.viewport = root.querySelector(".skeuo-map__viewport");
      this.input = root.querySelector(".skeuo-map__events");
      this.raw = null;
      this.cfg = null;
      this.view = undefined;
      this.tileKey = null;
      const canvas = L.DomUtil.create("div", "skeuo-map__canvas", this.viewport);
      this.map = L.map(canvas, { zoomControl: false, attributionControl: true, worldCopyJump: true, zoomSnap: 0.25 });
      this.map.attributionControl.setPrefix(false);
      this.overlay = L.layerGroup().addTo(this.map);
      this.map.on("click", (e) => this.onMapClick(e));
      this.map.on("moveend", () => {
        if (!this.cfg || !this.cfg.report_view) return;
        const c = this.map.getCenter();
        this.emit({ type: "view", lat: c.lat, lon: c.lng, zoom: this.map.getZoom() });
      });
      // Leg labels hide when their leg is too short on screen to hold them.
      this.labels = [];
      this.map.on("zoomend", () => this.cullLabels());
      this.resize = new ResizeObserver(() => this.map.invalidateSize({ pan: false }));
      this.resize.observe(this.viewport);
    }

    emit(event) {
      if (!this.input) return;
      this.input.value = JSON.stringify(event);
      this.input.dispatchEvent(new Event("input", { bubbles: true }));
    }

    onMapClick(e) {
      const ll = e.latlng.wrap();
      if (this.cfg && this.cfg.add_to) this.emit({ type: "add", route: this.cfg.add_to, lat: ll.lat, lon: ll.lng });
      else this.emit({ type: "click", lat: ll.lat, lon: ll.lng });
    }

    update() {
      const raw = this.root.getAttribute("data-skeuo-map");
      if (raw === this.raw) return;
      this.raw = raw;
      let cfg;
      try { cfg = JSON.parse(raw); } catch (err) { console.error("skeuo-ui map: bad config", err); return; }
      this.cfg = cfg;
      const map = this.map;

      // Interaction options.
      const interactive = cfg.interactive !== false;
      for (const h of ["dragging", "doubleClickZoom", "boxZoom", "keyboard", "touchZoom"]) map[h][interactive ? "enable" : "disable"]();
      map.scrollWheelZoom[interactive && cfg.scroll_zoom !== false ? "enable" : "disable"]();
      if (cfg.min_zoom != null) map.setMinZoom(cfg.min_zoom);
      if (cfg.max_zoom != null) map.setMaxZoom(cfg.max_zoom);
      // Clicking to add waypoints should not also zoom on double click.
      if (cfg.add_to) map.doubleClickZoom.disable();

      // Tiles.
      const t = cfg.tiles || {};
      const tileKey = JSON.stringify(t);
      if (tileKey !== this.tileKey) {
        this.tileKey = tileKey;
        if (this.tiles) this.tiles.remove();
        if (t.url) {
          this.tiles = L.tileLayer(t.url, {
            attribution: t.attribution || "",
            maxZoom: t.max_zoom || 19,
            subdomains: t.subdomains || "abc",
            className: "skeuo-map__tiles",
          }).addTo(map);
        }
      }

      // View: applied on the first config and whenever `view` changes. Leaflet
      // needs a view before controls and layers can be added.
      if (cfg.view !== this.view) {
        this.view = cfg.view;
        if (cfg.fit_routes && this.hasContent()) this.fit();
        else map.setView(cfg.center || [0, 0], cfg.zoom != null ? cfg.zoom : 3);
      }

      // Controls.
      const wantKeys = cfg.controls !== false, wantScale = cfg.scale !== false;
      if (wantKeys && !this.keys) this.keys = new KeyControl({ onFit: () => this.fit() }).addTo(map);
      if (!wantKeys && this.keys) { this.keys.remove(); this.keys = null; }
      if (this.keys) this.keys.fitKey.hidden = !this.hasContent();
      if (wantScale && !this.scale) this.scale = new ScaleControl().addTo(map);
      if (!wantScale && this.scale) { this.scale.remove(); this.scale = null; }

      this.draw();

    }

    hasContent() {
      const c = this.cfg || {};
      return (c.routes || []).some((r) => (r.waypoints || []).length) || (c.markers || []).length > 0;
    }

    fit() {
      const pts = [];
      for (const r of this.cfg.routes || []) {
        const wps = r.waypoints || [];
        for (let i = 0; i + 1 < wps.length; i++) pts.push(...unwrap(r.geodesic === false ? [[wps[i].lat, wps[i].lon], [wps[i + 1].lat, wps[i + 1].lon]] : greatCircle(wps[i], wps[i + 1])));
        if (wps.length === 1) pts.push([wps[0].lat, wps[0].lon]);
      }
      for (const m of this.cfg.markers || []) pts.push([m.lat, m.lon]);
      if (!pts.length) return;
      this.map.fitBounds(L.latLngBounds(pts), { padding: [28, 28], maxZoom: 10 });
    }

    draw() {
      const cfg = this.cfg, group = this.overlay;
      group.clearLayers();
      this.labels = [];
      const ref = this.map.getCenter().lng;

      for (const c of cfg.circles || []) {
        L.circle([c.lat, nearLon(c.lon, ref)], {
          radius: (c.radius_nm || 0) * 1852,
          className: "skeuo-map__area " + colorClass(c.color),
          interactive: false,
        }).addTo(group);
        if (c.label) {
          L.marker([c.lat, nearLon(c.lon, ref)], {
            interactive: false,
            icon: L.divIcon({ className: "skeuo-map__area-label " + colorClass(c.color), html: `<span>${esc(c.label)}</span>`, iconSize: null }),
          }).addTo(group);
        }
      }

      for (const route of cfg.routes || []) this.drawRoute(route, group);
      this.cullLabels();

      for (const m of cfg.markers || []) {
        const kind = m.kind || "dot";
        const rotate = kind === "aircraft" ? `style="transform: rotate(${Number(m.heading) || 0}deg)"` : "";
        const marker = L.marker([m.lat, nearLon(m.lon, ref)], {
          keyboard: false,
          zIndexOffset: kind === "aircraft" ? 1000 : 500,
          icon: L.divIcon({
            className: `skeuo-map__marker skeuo-map__marker--${kind} ${colorClass(m.color)}`,
            html: `<span class="skeuo-map__symbol" ${rotate}>${symbolSvg(kind)}</span>` +
              (m.label ? `<span class="skeuo-map__tag">${esc(m.label)}</span>` : ""),
            iconSize: [24, 24],
            iconAnchor: [12, 12],
          }),
        }).addTo(group);
        marker.on("click", () => this.emit({ type: "marker", id: m.id }));
      }
    }

    drawRoute(route, group) {
      const wps = route.waypoints || [];
      // Longitudes made continuous along the route (antimeridian crossings),
      // starting from the copy nearest the map centre.
      const lons = [];
      wps.forEach((w, i) => {
        lons.push(i === 0 ? nearLon(w.lon, this.map.getCenter().lng) : nearLon(w.lon, lons[i - 1]));
      });
      const cls = colorClass(route.color);
      const geodesic = route.geodesic !== false;
      const editable = !!route.editable;

      for (let i = 0; i + 1 < wps.length; i++) {
        const a = wps[i], b = wps[i + 1];
        const shift = (pts) => { const d = lons[i] - pts[0][1]; return pts.map(([la, lo]) => [la, lo + d]); };
        const path = shift(unwrap(geodesic ? greatCircle(a, b) : [[a.lat, a.lon], [b.lat, b.lon]]));
        // Casing under the line keeps it legible on any tile.
        L.polyline(path, { className: "skeuo-map__leg-casing", interactive: false }).addTo(group);
        L.polyline(path, { className: `skeuo-map__leg ${cls}`, interactive: false }).addTo(group);

        let mid = midpoint(a, b);
        mid = [mid[0], nearLon(mid[1], (lons[i] + lons[i + 1]) / 2)];
        if (route.labels !== false) {
          const course = String(Math.round(courseDeg(a, b)) % 360).padStart(3, "0");
          const dist = distanceNm(a, b);
          const label = L.marker(mid, {
            interactive: false,
            keyboard: false,
            icon: L.divIcon({
              className: `skeuo-map__leg-label ${cls}`,
              html: `<span>${course}° · ${dist < 10 ? dist.toFixed(1) : Math.round(dist)} NM</span>`,
              iconSize: null,
            }),
          }).addTo(group);
          this.labels.push({ marker: label, a: [a.lat, lons[i]], b: [b.lat, lons[i + 1]] });
        }
        if (editable) {
          // A small handle at mid-leg: drag it to insert a waypoint there.
          const handle = L.marker(mid, {
            draggable: true,
            keyboard: false,
            zIndexOffset: 200,
            title: "Drag to insert a waypoint",
            icon: L.divIcon({ className: `skeuo-map__insert ${cls}`, html: "<span></span>", iconSize: [14, 14], iconAnchor: [7, 7] }),
          }).addTo(group);
          handle.on("dragend", (e) => {
            const ll = e.target.getLatLng().wrap();
            this.emit({ type: "insert", route: route.id, index: i + 1, lat: ll.lat, lon: ll.lng });
          });
        }
      }

      wps.forEach((w, i) => {
        const kind = w.kind || "user";
        const selected = route.selected === i;
        const marker = L.marker([w.lat, lons[i]], {
          draggable: editable,
          keyboard: false,
          zIndexOffset: selected ? 800 : 400,
          title: w.name ? `${w.ident} — ${w.name}` : w.ident,
          icon: L.divIcon({
            className: `skeuo-map__waypoint skeuo-map__waypoint--${kind} ${cls}` + (selected ? " skeuo-map__waypoint--selected" : "") + (editable ? " skeuo-map__waypoint--editable" : ""),
            html: `<span class="skeuo-map__symbol">${symbolSvg(kind)}</span><span class="skeuo-map__tag">${esc(w.ident)}${w.alt_ft != null ? `<small>${esc(Math.round(w.alt_ft))} ft</small>` : ""}</span>`,
            iconSize: [24, 24],
            iconAnchor: [12, 12],
          }),
        }).addTo(group);
        marker.on("click", (e) => {
          L.DomEvent.stopPropagation(e);
          this.emit({ type: "select", route: route.id, index: i });
        });
        if (editable) {
          marker.on("dblclick contextmenu", (e) => {
            L.DomEvent.stopPropagation(e);
            if (e.originalEvent) L.DomEvent.preventDefault(e.originalEvent);
            this.emit({ type: "remove", route: route.id, index: i });
          });
          marker.on("dragend", (e) => {
            const ll = e.target.getLatLng().wrap();
            this.emit({ type: "move", route: route.id, index: i, lat: ll.lat, lon: ll.lng });
          });
        }
      });
    }

    cullLabels() {
      const map = this.map;
      for (const { marker, a, b } of this.labels) {
        const len = map.latLngToLayerPoint(a).distanceTo(map.latLngToLayerPoint(b));
        const el = marker.getElement();
        if (!el) continue;
        // Room for the label plus the two waypoint symbols at the leg's ends.
        el.classList.remove("skeuo-map__leg-label--hidden");
        const width = el.firstElementChild ? el.firstElementChild.offsetWidth : 90;
        el.classList.toggle("skeuo-map__leg-label--hidden", len < width + 30);
      }
    }

    destroy() {
      this.resize.disconnect();
      this.map.remove();
    }
  }

  // ---------------------------------------------------------------- page scan
  const byRoot = new WeakMap();
  function scan() {
    for (const root of document.querySelectorAll(".skeuo-map[data-skeuo-map]")) {
      let m = byRoot.get(root);
      if (!m) {
        if (!root.querySelector(".skeuo-map__viewport")) continue;
        m = new SkeuoMap(root);
        byRoot.set(root, m);
        maps.add(m);
      }
      m.update();
    }
    for (const m of Array.from(maps)) {
      if (!m.root.isConnected) {
        m.destroy();
        maps.delete(m);
        byRoot.delete(m.root);
      }
    }
  }

  let pending = false;
  function schedule() {
    if (pending) return;
    pending = true;
    requestAnimationFrame(() => { pending = false; scan(); });
  }
  new MutationObserver(schedule).observe(document.documentElement, {
    subtree: true, childList: true, attributes: true, attributeFilter: ["data-skeuo-map"],
  });
  window.__skeuoMap = { scan, maps, distanceNm, courseDeg, greatCircle };
  schedule();
})();
