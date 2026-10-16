#include <charconv>
#include <cmath>

#include "skeuo/json.hpp"
#include "skeuo/map.hpp"

namespace skeuo {

std::string_view as_str(WaypointKind k) {
    switch (k) {
        case WaypointKind::Airport: return "airport";
        case WaypointKind::Vor: return "vor";
        case WaypointKind::Ndb: return "ndb";
        case WaypointKind::Fix: return "fix";
        case WaypointKind::User: return "user";
    }
    return "user";
}

MapMarker MapMarker::aircraft(std::string id, double lat, double lon, double heading_deg) {
    MapMarker m;
    m.id = std::move(id);
    m.pos = {lat, lon};
    m.kind = MapMarkerKind::Aircraft;
    m.heading_deg = heading_deg;
    return m;
}

// ------------------------------------------------------------------ FlightRoute

double FlightRoute::total_distance_nm() const {
    auto l = legs();
    return l.empty() ? 0.0 : l.back().cumulative_nm;
}

namespace {
std::string next_user_ident(const std::vector<Waypoint>& wps) {
    for (int n = 1;; ++n) {
        std::string ident = "WPT" + std::to_string(n);
        bool used = false;
        for (const auto& w : wps) used = used || w.ident == ident;
        if (!used) return ident;
    }
}
}  // namespace

bool FlightRoute::apply(const MapEvent& e) {
    if (e.route != id) return false;
    using T = MapEvent::Type;
    switch (e.type) {
        case T::AddWaypoint:
            waypoints.push_back({.ident = next_user_ident(waypoints), .kind = WaypointKind::User, .pos = e.pos});
            selected = waypoints.size() - 1;
            return true;
        case T::InsertWaypoint: {
            size_t at = std::min(e.index, waypoints.size());
            waypoints.insert(waypoints.begin() + long(at),
                             {.ident = next_user_ident(waypoints), .kind = WaypointKind::User, .pos = e.pos});
            selected = at;
            return true;
        }
        case T::MoveWaypoint:
            if (e.index >= waypoints.size()) return false;
            waypoints[e.index].pos = e.pos;
            selected = e.index;
            return true;
        case T::RemoveWaypoint:
            if (e.index >= waypoints.size()) return false;
            waypoints.erase(waypoints.begin() + long(e.index));
            selected.reset();
            return true;
        case T::SelectWaypoint:
            selected = e.index;
            return true;
        default: return false;
    }
}

// ------------------------------------------------------------------ events

std::optional<MapEvent> MapEvent::parse(std::string_view text) {
    json::Value v = json::parse(text);
    if (v.type != json::Value::Type::Object) return std::nullopt;
    auto has = [&](const char* k, json::Value::Type t) {
        const json::Value* x = v.get(k);
        return x && x->type == t;
    };
    using J = json::Value::Type;
    bool pos = has("lat", J::Number) && has("lon", J::Number);
    bool idx = has("index", J::Number) && v.num("index") >= 0;
    bool route = has("route", J::String);
    MapEvent e;
    e.pos = {v.num("lat"), v.num("lon")};
    e.route = v.str("route");
    e.index = idx ? size_t(v.num("index")) : 0;
    std::string type = v.str("type");
    if (type == "click" && pos) e.type = Type::Click;
    else if (type == "add" && route && pos) e.type = Type::AddWaypoint;
    else if (type == "move" && route && idx && pos) e.type = Type::MoveWaypoint;
    else if (type == "insert" && route && idx && pos) e.type = Type::InsertWaypoint;
    else if (type == "select" && route && idx) e.type = Type::SelectWaypoint;
    else if (type == "remove" && route && idx) e.type = Type::RemoveWaypoint;
    else if (type == "marker" && has("id", J::String)) {
        e.type = Type::MarkerClick;
        e.id = v.str("id");
    } else if (type == "view" && pos && has("zoom", J::Number)) {
        e.type = Type::ViewChange;
        e.zoom = v.num("zoom");
    } else {
        return std::nullopt;
    }
    return e;
}

// ------------------------------------------------------------------ config

namespace {

std::string num(double n) {
    if (!std::isfinite(n)) return "0";
    char buf[64];
    auto r = std::to_chars(buf, buf + sizeof buf, n);  // shortest round-trip, like Rust's Display
    return std::string(buf, r.ptr);
}

/// Streaming JSON writer: each value follows a key or gets a comma.
class Writer {
public:
    std::string out;
    void begin_object() { value(); out += '{'; filled_.push_back(false); }
    void end_object() { out += '}'; filled_.pop_back(); }
    void begin_array() { value(); out += '['; filled_.push_back(false); }
    void end_array() { out += ']'; filled_.pop_back(); }
    void key(std::string_view k) { value(); out += json::quote(k); out += ':'; after_key_ = true; }
    void raw(std::string_view v) { value(); out += v; }
    void str(std::string_view k, std::string_view v) { key(k); raw(json::quote(v)); }
    void number(std::string_view k, double v) { key(k); raw(num(v)); }
    void boolean(std::string_view k, bool v) { key(k); raw(v ? "true" : "false"); }

private:
    void value() {
        if (after_key_) { after_key_ = false; return; }
        if (!filled_.empty()) {
            if (filled_.back()) out += ',';
            filled_.back() = true;
        }
    }
    std::vector<bool> filled_;
    bool after_key_ = false;
};

}  // namespace

std::string map_config(const MapViewProps& p) {
    Writer o;
    o.begin_object();
    o.key("center");
    o.raw("[" + num(p.center.lat) + "," + num(p.center.lon) + "]");
    o.number("zoom", p.zoom);
    o.number("view", p.view_key);
    o.boolean("fit_routes", p.fit_routes);
    o.key("tiles");
    o.begin_object();
    o.str("url", p.tiles.url);
    o.str("attribution", p.tiles.attribution);
    o.number("max_zoom", p.tiles.max_zoom);
    o.str("subdomains", p.tiles.subdomains);
    o.end_object();
    if (p.min_zoom) o.number("min_zoom", *p.min_zoom);
    if (p.max_zoom) o.number("max_zoom", *p.max_zoom);
    o.boolean("interactive", p.interactive);
    o.boolean("scroll_zoom", p.scroll_zoom);
    o.boolean("controls", p.controls);
    o.boolean("scale", p.scale);
    o.boolean("report_view", p.report_view);
    if (p.add_waypoints_to) o.str("add_to", *p.add_waypoints_to);

    o.key("routes");
    o.begin_array();
    for (const auto& r : p.routes) {
        o.begin_object();
        o.str("id", r.id);
        o.str("color", as_str(r.color));
        o.boolean("geodesic", r.geodesic);
        o.boolean("labels", r.leg_labels);
        o.boolean("editable", r.editable);
        if (r.selected) o.number("selected", double(*r.selected));
        o.key("waypoints");
        o.begin_array();
        for (const auto& w : r.waypoints) {
            o.begin_object();
            o.str("ident", w.ident);
            if (w.name) o.str("name", *w.name);
            o.number("lat", w.pos.lat);
            o.number("lon", w.pos.lon);
            o.str("kind", as_str(w.kind));
            if (w.altitude_ft) o.number("alt_ft", *w.altitude_ft);
            o.end_object();
        }
        o.end_array();
        o.end_object();
    }
    o.end_array();

    o.key("markers");
    o.begin_array();
    for (const auto& m : p.markers) {
        o.begin_object();
        o.str("id", m.id);
        o.number("lat", m.pos.lat);
        o.number("lon", m.pos.lon);
        switch (m.kind) {
            case MapMarkerKind::Aircraft:
                o.str("kind", "aircraft");
                o.number("heading", m.heading_deg);
                break;
            case MapMarkerKind::Waypoint: o.str("kind", as_str(m.waypoint_kind)); break;
            case MapMarkerKind::Dot: o.str("kind", "dot"); break;
        }
        if (m.label) o.str("label", *m.label);
        o.str("color", as_str(m.color));
        o.end_object();
    }
    o.end_array();

    o.key("circles");
    o.begin_array();
    for (const auto& c : p.circles) {
        o.begin_object();
        o.str("id", c.id);
        o.number("lat", c.center.lat);
        o.number("lon", c.center.lon);
        o.number("radius_nm", c.radius_nm);
        o.str("color", as_str(c.color));
        if (c.label) o.str("label", *c.label);
        o.end_object();
    }
    o.end_array();
    o.end_object();
    return o.out;
}

// ------------------------------------------------------------------ MapView

Node MapView(const MapViewProps& props) {
    Node input = el("input").attr("type", "hidden").cls("skeuo-map__events");
    if (props.onevent) {
        input.on("input", [cb = props.onevent](const Event& e) {
            if (auto ev = MapEvent::parse(e.value)) cb(*ev);
        });
    }
    return el("div")
        .cls(classes({"skeuo-map skeuo-well", props.cls}))
        .attr("style", "height:" + props.height)
        .attr("role", "region")
        .attr("aria-label", props.aria_label)
        .attr("data-skeuo-map", map_config(props))
        // The map widget (map.js) owns the viewport's children.
        .child(el("div").cls("skeuo-map__viewport").attr("data-sk-ignore", "true"))
        .child(std::move(input));
}

}  // namespace skeuo
