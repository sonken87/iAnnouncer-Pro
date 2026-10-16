use std::collections::HashMap;
use std::fmt::Write;
use std::sync::LazyLock;

use dioxus::prelude::*;

use super::geo::{LatLon, distance_nm, initial_course_deg};
use crate::{Color, classes};

/// Leaflet + the skeuo map widget, evaluated once per page.
static MAP_SCRIPT: LazyLock<String> = LazyLock::new(|| {
    format!(
        "if(!window.L){{{}\n}}\n{}",
        include_str!("../../assets/vendor/leaflet/leaflet.js"),
        include_str!("../../assets/map.js")
    )
});

/// The page script that powers [`MapView`]: Leaflet and the skeuo map
/// widget. `MapView` loads it by itself; it is exported for hosts that
/// prefer to ship it as a static asset.
pub fn map_script() -> &'static str {
    MAP_SCRIPT.as_str()
}

// ------------------------------------------------------------------ data

/// What a waypoint is, drawn with the matching aeronautical-chart symbol.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum WaypointKind {
    /// Aerodrome: a circle with four ticks.
    Airport,
    /// VOR: a hexagon with a centre dot.
    Vor,
    /// NDB: a dotted circle.
    Ndb,
    /// Named fix / intersection: a triangle.
    Fix,
    /// User-defined point: a diamond.
    #[default]
    User,
}

impl WaypointKind {
    pub fn as_str(self) -> &'static str {
        match self {
            WaypointKind::Airport => "airport",
            WaypointKind::Vor => "vor",
            WaypointKind::Ndb => "ndb",
            WaypointKind::Fix => "fix",
            WaypointKind::User => "user",
        }
    }
}

/// A point of a flight route.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Waypoint {
    /// Identifier printed on the map (`"ZBAA"`, `"VYK"`, `"WPT3"`).
    pub ident: String,
    pub name: Option<String>,
    pub pos: LatLon,
    pub kind: WaypointKind,
    /// Planned altitude, printed under the ident.
    pub altitude_ft: Option<f64>,
}

impl Waypoint {
    pub fn new(ident: impl Into<String>, kind: WaypointKind, lat: f64, lon: f64) -> Self {
        Self {
            ident: ident.into(),
            name: None,
            pos: LatLon::new(lat, lon),
            kind,
            altitude_ft: None,
        }
    }

    pub fn named(mut self, name: impl Into<String>) -> Self {
        self.name = Some(name.into());
        self
    }

    pub fn at_altitude(mut self, feet: f64) -> Self {
        self.altitude_ft = Some(feet);
        self
    }
}

/// One leg of a route, as in a navigation log.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Leg {
    /// Index of the waypoint the leg starts at (it ends at `from + 1`).
    pub from: usize,
    /// Initial true course in degrees.
    pub course_deg: f64,
    pub distance_nm: f64,
    /// Distance flown at the end of this leg.
    pub cumulative_nm: f64,
}

/// The legs between consecutive waypoints.
pub fn route_legs(waypoints: &[Waypoint]) -> Vec<Leg> {
    let mut total = 0.0;
    waypoints
        .windows(2)
        .enumerate()
        .map(|(i, w)| {
            let distance = distance_nm(w[0].pos, w[1].pos);
            total += distance;
            Leg {
                from: i,
                course_deg: initial_course_deg(w[0].pos, w[1].pos),
                distance_nm: distance,
                cumulative_nm: total,
            }
        })
        .collect()
}

/// A flight route drawn on a [`MapView`]: great-circle legs in the palette
/// color, labelled with true course and distance.
#[derive(Clone, Debug, PartialEq)]
pub struct FlightRoute {
    /// Identifies the route in [`MapEvent`]s.
    pub id: String,
    pub waypoints: Vec<Waypoint>,
    pub color: Color,
    /// Waypoints can be dragged, inserted (drag a leg's midpoint handle) and
    /// removed (double-click); edits arrive as [`MapEvent`]s.
    pub editable: bool,
    /// Draw legs as great-circle arcs (the real shortest path) rather than
    /// straight lines on the map projection.
    pub geodesic: bool,
    /// Label each leg with its course and distance.
    pub leg_labels: bool,
    /// Waypoint highlighted with an LED glow.
    pub selected: Option<usize>,
}

impl FlightRoute {
    pub fn new(id: impl Into<String>, waypoints: Vec<Waypoint>) -> Self {
        Self {
            id: id.into(),
            waypoints,
            color: Color::Primary,
            editable: false,
            geodesic: true,
            leg_labels: true,
            selected: None,
        }
    }

    pub fn color(mut self, color: Color) -> Self {
        self.color = color;
        self
    }

    pub fn editable(mut self, editable: bool) -> Self {
        self.editable = editable;
        self
    }

    pub fn selected(mut self, index: Option<usize>) -> Self {
        self.selected = index;
        self
    }

    pub fn legs(&self) -> Vec<Leg> {
        route_legs(&self.waypoints)
    }

    pub fn total_distance_nm(&self) -> f64 {
        self.legs().last().map_or(0.0, |l| l.cumulative_nm)
    }

    /// Applies an editing event aimed at this route (add, move, insert,
    /// remove, select) and returns whether anything changed. New points are
    /// user waypoints named `WPT1`, `WPT2`, ...
    pub fn apply(&mut self, event: &MapEvent) -> bool {
        let mine = |route: &str| route == self.id;
        match event {
            MapEvent::AddWaypoint { route, pos } if mine(route) => {
                let ident = self.next_user_ident();
                self.waypoints
                    .push(Waypoint::new(ident, WaypointKind::User, pos.lat, pos.lon));
                self.selected = Some(self.waypoints.len() - 1);
                true
            }
            MapEvent::InsertWaypoint { route, index, pos } if mine(route) => {
                let ident = self.next_user_ident();
                let at = (*index).min(self.waypoints.len());
                self.waypoints.insert(
                    at,
                    Waypoint::new(ident, WaypointKind::User, pos.lat, pos.lon),
                );
                self.selected = Some(at);
                true
            }
            MapEvent::MoveWaypoint { route, index, pos } if mine(route) => {
                match self.waypoints.get_mut(*index) {
                    Some(w) => {
                        w.pos = *pos;
                        self.selected = Some(*index);
                        true
                    }
                    None => false,
                }
            }
            MapEvent::RemoveWaypoint { route, index }
                if mine(route) && *index < self.waypoints.len() =>
            {
                self.waypoints.remove(*index);
                self.selected = None;
                true
            }
            MapEvent::SelectWaypoint { route, index } if mine(route) => {
                self.selected = Some(*index);
                true
            }
            _ => false,
        }
    }

    fn next_user_ident(&self) -> String {
        let n = (1..)
            .find(|n| !self.waypoints.iter().any(|w| w.ident == format!("WPT{n}")))
            .unwrap_or(1);
        format!("WPT{n}")
    }
}

/// What a [`MapMarker`] shows.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum MapMarkerKind {
    /// An aircraft symbol rotated to its heading (degrees true).
    Aircraft { heading_deg: f64 },
    /// A waypoint symbol not attached to a route.
    Waypoint(WaypointKind),
    /// A plain dot.
    Dot,
}

/// A standalone symbol on the map (aircraft, navaid, point of interest).
#[derive(Clone, Debug, PartialEq)]
pub struct MapMarker {
    /// Identifies the marker in [`MapEvent::MarkerClick`].
    pub id: String,
    pub pos: LatLon,
    pub kind: MapMarkerKind,
    pub label: Option<String>,
    pub color: Color,
}

impl MapMarker {
    pub fn aircraft(id: impl Into<String>, lat: f64, lon: f64, heading_deg: f64) -> Self {
        Self {
            id: id.into(),
            pos: LatLon::new(lat, lon),
            kind: MapMarkerKind::Aircraft { heading_deg },
            label: None,
            color: Color::Warning,
        }
    }

    pub fn label(mut self, label: impl Into<String>) -> Self {
        self.label = Some(label.into());
        self
    }

    pub fn color(mut self, color: Color) -> Self {
        self.color = color;
        self
    }
}

/// A circular area (e.g. a restricted zone or a radio coverage radius).
#[derive(Clone, Debug, PartialEq)]
pub struct MapCircle {
    pub id: String,
    pub center: LatLon,
    pub radius_nm: f64,
    pub color: Color,
    pub label: Option<String>,
}

/// Where the base map tiles come from (an XYZ tile URL template).
#[derive(Clone, Debug, PartialEq)]
pub struct TileSource {
    /// e.g. `https://tile.openstreetmap.org/{z}/{x}/{y}.png`
    pub url: String,
    /// Shown in the corner; required by most tile providers.
    pub attribution: String,
    pub max_zoom: u8,
    /// Letters substituted for `{s}` in the URL.
    pub subdomains: String,
}

impl TileSource {
    /// OpenStreetMap's public tiles. Fine for development; their usage
    /// policy asks heavy or commercial users to run their own tile server or
    /// use a commercial provider.
    pub fn openstreetmap() -> Self {
        Self {
            url: "https://tile.openstreetmap.org/{z}/{x}/{y}.png".into(),
            attribution: "© OpenStreetMap contributors".into(),
            max_zoom: 19,
            subdomains: "abc".into(),
        }
    }

    pub fn custom(url: impl Into<String>, attribution: impl Into<String>) -> Self {
        Self {
            url: url.into(),
            attribution: attribution.into(),
            max_zoom: 19,
            subdomains: "abc".into(),
        }
    }
}

impl Default for TileSource {
    fn default() -> Self {
        Self::openstreetmap()
    }
}

/// Something the user did on a [`MapView`].
#[derive(Clone, Debug, PartialEq)]
pub enum MapEvent {
    /// The map was clicked (and `add_waypoints_to` is not set).
    Click(LatLon),
    /// The map was clicked with `add_waypoints_to` set: append a waypoint.
    AddWaypoint { route: String, pos: LatLon },
    /// A waypoint was dragged to a new position.
    MoveWaypoint {
        route: String,
        index: usize,
        pos: LatLon,
    },
    /// A leg's midpoint handle was dragged: insert a waypoint at `index`.
    InsertWaypoint {
        route: String,
        index: usize,
        pos: LatLon,
    },
    /// A waypoint was clicked.
    SelectWaypoint { route: String, index: usize },
    /// A waypoint was double-clicked (or right-clicked) on an editable route.
    RemoveWaypoint { route: String, index: usize },
    /// A standalone marker was clicked.
    MarkerClick { id: String },
    /// Panning/zooming ended (only with `report_view`).
    ViewChange { center: LatLon, zoom: f64 },
}

impl MapEvent {
    /// Parses the JSON event the map widget reports.
    pub fn parse(json: &str) -> Option<MapEvent> {
        let fields = parse_flat_object(json)?;
        let text = |k: &str| match fields.get(k) {
            Some(Json::Str(s)) => Some(s.clone()),
            _ => None,
        };
        let num = |k: &str| match fields.get(k) {
            Some(Json::Num(n)) => Some(*n),
            _ => None,
        };
        let pos = || Some(LatLon::new(num("lat")?, num("lon")?));
        let index = || num("index").filter(|n| *n >= 0.0).map(|n| n as usize);
        let route = || text("route");
        Some(match text("type")?.as_str() {
            "click" => MapEvent::Click(pos()?),
            "add" => MapEvent::AddWaypoint {
                route: route()?,
                pos: pos()?,
            },
            "move" => MapEvent::MoveWaypoint {
                route: route()?,
                index: index()?,
                pos: pos()?,
            },
            "insert" => MapEvent::InsertWaypoint {
                route: route()?,
                index: index()?,
                pos: pos()?,
            },
            "select" => MapEvent::SelectWaypoint {
                route: route()?,
                index: index()?,
            },
            "remove" => MapEvent::RemoveWaypoint {
                route: route()?,
                index: index()?,
            },
            "marker" => MapEvent::MarkerClick { id: text("id")? },
            "view" => MapEvent::ViewChange {
                center: pos()?,
                zoom: num("zoom")?,
            },
            _ => return None,
        })
    }
}

// ------------------------------------------------------------------ component

/// A slippy map set into a well, with flight routes, markers and areas.
/// Not part of MUI. Base map tiles are loaded from `tiles` (online); the
/// controls are keycaps (zoom in/out, fit route) with a nautical-mile scale.
///
/// The map keeps its own pan/zoom: `center`/`zoom` (or `fit_routes`) are
/// applied when it first appears and whenever `view_key` changes.
#[derive(Props, Clone, PartialEq)]
pub struct MapViewProps {
    #[props(default = LatLon::new(35.0, 105.0))]
    pub center: LatLon,
    #[props(default = 4.0)]
    pub zoom: f64,
    /// Change this to move the map to `center`/`zoom` (or fit) again.
    #[props(default)]
    pub view_key: u32,
    /// Fit all routes and markers instead of using `center`/`zoom`.
    #[props(default)]
    pub fit_routes: bool,
    #[props(default)]
    pub tiles: TileSource,
    #[props(default)]
    pub routes: Vec<FlightRoute>,
    #[props(default)]
    pub markers: Vec<MapMarker>,
    #[props(default)]
    pub circles: Vec<MapCircle>,
    /// Clicking the map appends a waypoint to the route with this id
    /// ([`MapEvent::AddWaypoint`]).
    #[props(into)]
    pub add_waypoints_to: Option<String>,
    /// CSS height of the map.
    #[props(default = "420px".to_string(), into)]
    pub height: String,
    /// Allow panning and zooming.
    #[props(default = true)]
    pub interactive: bool,
    /// Zoom with the mouse wheel.
    #[props(default = true)]
    pub scroll_zoom: bool,
    /// Zoom / fit keycaps.
    #[props(default = true)]
    pub controls: bool,
    /// Nautical-mile scale bar.
    #[props(default = true)]
    pub scale: bool,
    /// Report [`MapEvent::ViewChange`] after each pan/zoom.
    #[props(default)]
    pub report_view: bool,
    pub min_zoom: Option<f64>,
    pub max_zoom: Option<f64>,
    #[props(default = "Map".to_string(), into)]
    pub aria_label: String,
    #[props(default, into)]
    pub class: String,
    pub onevent: Option<EventHandler<MapEvent>>,
}

#[component]
pub fn MapView(props: MapViewProps) -> Element {
    // Load Leaflet and the widget once; it then finds this map on the page.
    use_effect(|| {
        document::eval(map_script());
    });
    let config = map_config(&props);
    let onevent = props.onevent;
    rsx! {
        div {
            class: classes(["skeuo-map skeuo-well", &props.class]),
            style: "height:{props.height}",
            role: "region",
            aria_label: props.aria_label,
            "data-skeuo-map": config,
            div { class: "skeuo-map__viewport", "data-sk-ignore": "true" }
            input {
                r#type: "hidden",
                class: "skeuo-map__events",
                oninput: move |e: FormEvent| {
                    if let (Some(h), Some(ev)) = (&onevent, MapEvent::parse(&e.value())) {
                        h.call(ev);
                    }
                },
            }
        }
    }
}

/// The widget configuration as JSON (the `data-skeuo-map` attribute).
pub fn map_config(props: &MapViewProps) -> String {
    let mut o = JsonWriter::default();
    o.begin_object();
    o.key("center");
    o.raw(&format!(
        "[{},{}]",
        num(props.center.lat),
        num(props.center.lon)
    ));
    o.field_num("zoom", props.zoom);
    o.field_num("view", props.view_key as f64);
    o.field_bool("fit_routes", props.fit_routes);
    o.key("tiles");
    o.begin_object();
    o.field_str("url", &props.tiles.url);
    o.field_str("attribution", &props.tiles.attribution);
    o.field_num("max_zoom", props.tiles.max_zoom as f64);
    o.field_str("subdomains", &props.tiles.subdomains);
    o.end_object();
    if let Some(z) = props.min_zoom {
        o.field_num("min_zoom", z);
    }
    if let Some(z) = props.max_zoom {
        o.field_num("max_zoom", z);
    }
    o.field_bool("interactive", props.interactive);
    o.field_bool("scroll_zoom", props.scroll_zoom);
    o.field_bool("controls", props.controls);
    o.field_bool("scale", props.scale);
    o.field_bool("report_view", props.report_view);
    if let Some(r) = &props.add_waypoints_to {
        o.field_str("add_to", r);
    }

    o.key("routes");
    o.begin_array();
    for r in &props.routes {
        o.begin_object();
        o.field_str("id", &r.id);
        o.field_str("color", r.color.as_str());
        o.field_bool("geodesic", r.geodesic);
        o.field_bool("labels", r.leg_labels);
        o.field_bool("editable", r.editable);
        if let Some(i) = r.selected {
            o.field_num("selected", i as f64);
        }
        o.key("waypoints");
        o.begin_array();
        for w in &r.waypoints {
            o.begin_object();
            o.field_str("ident", &w.ident);
            if let Some(n) = &w.name {
                o.field_str("name", n);
            }
            o.field_num("lat", w.pos.lat);
            o.field_num("lon", w.pos.lon);
            o.field_str("kind", w.kind.as_str());
            if let Some(a) = w.altitude_ft {
                o.field_num("alt_ft", a);
            }
            o.end_object();
        }
        o.end_array();
        o.end_object();
    }
    o.end_array();

    o.key("markers");
    o.begin_array();
    for m in &props.markers {
        o.begin_object();
        o.field_str("id", &m.id);
        o.field_num("lat", m.pos.lat);
        o.field_num("lon", m.pos.lon);
        match m.kind {
            MapMarkerKind::Aircraft { heading_deg } => {
                o.field_str("kind", "aircraft");
                o.field_num("heading", heading_deg);
            }
            MapMarkerKind::Waypoint(k) => o.field_str("kind", k.as_str()),
            MapMarkerKind::Dot => o.field_str("kind", "dot"),
        }
        if let Some(l) = &m.label {
            o.field_str("label", l);
        }
        o.field_str("color", m.color.as_str());
        o.end_object();
    }
    o.end_array();

    o.key("circles");
    o.begin_array();
    for c in &props.circles {
        o.begin_object();
        o.field_str("id", &c.id);
        o.field_num("lat", c.center.lat);
        o.field_num("lon", c.center.lon);
        o.field_num("radius_nm", c.radius_nm);
        o.field_str("color", c.color.as_str());
        if let Some(l) = &c.label {
            o.field_str("label", l);
        }
        o.end_object();
    }
    o.end_array();
    o.end_object();
    o.out
}

// ------------------------------------------------------------------ JSON helpers

fn num(n: f64) -> String {
    if n.is_finite() {
        format!("{n}")
    } else {
        "0".into()
    }
}

/// A minimal streaming JSON writer: each value either follows a key or gets
/// a comma before it when it is not the first in its container.
#[derive(Default)]
struct JsonWriter {
    out: String,
    /// Per open container: has it received a value yet?
    filled: Vec<bool>,
    after_key: bool,
}

impl JsonWriter {
    fn value_prefix(&mut self) {
        if self.after_key {
            self.after_key = false;
        } else if let Some(filled) = self.filled.last_mut() {
            if *filled {
                self.out.push(',');
            }
            *filled = true;
        }
    }
    fn begin_object(&mut self) {
        self.value_prefix();
        self.out.push('{');
        self.filled.push(false);
    }
    fn end_object(&mut self) {
        self.out.push('}');
        self.filled.pop();
    }
    fn begin_array(&mut self) {
        self.value_prefix();
        self.out.push('[');
        self.filled.push(false);
    }
    fn end_array(&mut self) {
        self.out.push(']');
        self.filled.pop();
    }
    fn key(&mut self, k: &str) {
        self.value_prefix();
        quote(&mut self.out, k);
        self.out.push(':');
        self.after_key = true;
    }
    fn raw(&mut self, v: &str) {
        self.value_prefix();
        self.out.push_str(v);
    }
    fn field_str(&mut self, k: &str, v: &str) {
        self.key(k);
        self.value_prefix();
        quote(&mut self.out, v);
    }
    fn field_num(&mut self, k: &str, v: f64) {
        self.key(k);
        self.raw(&num(v));
    }
    fn field_bool(&mut self, k: &str, v: bool) {
        self.key(k);
        self.raw(if v { "true" } else { "false" });
    }
}

fn quote(out: &mut String, s: &str) {
    out.push('"');
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => {
                let _ = write!(out, "\\u{:04x}", c as u32);
            }
            c => out.push(c),
        }
    }
    out.push('"');
}

enum Json {
    Str(String),
    Num(f64),
    Other,
}

/// Parses a flat JSON object of strings, numbers, booleans and nulls (the
/// widget's events).
fn parse_flat_object(s: &str) -> Option<HashMap<String, Json>> {
    let mut chars = s.trim().chars().peekable();
    let mut out = HashMap::new();
    let skip_ws = |it: &mut std::iter::Peekable<std::str::Chars>| {
        while it.peek().is_some_and(|c| c.is_whitespace()) {
            it.next();
        }
    };
    let read_string = |it: &mut std::iter::Peekable<std::str::Chars>| -> Option<String> {
        if it.next()? != '"' {
            return None;
        }
        let mut v = String::new();
        loop {
            match it.next()? {
                '"' => return Some(v),
                '\\' => match it.next()? {
                    'n' => v.push('\n'),
                    't' => v.push('\t'),
                    'r' => v.push('\r'),
                    'u' => {
                        let hex: String = (0..4).filter_map(|_| it.next()).collect();
                        v.push(char::from_u32(u32::from_str_radix(&hex, 16).ok()?).unwrap_or('?'));
                    }
                    c => v.push(c),
                },
                c => v.push(c),
            }
        }
    };
    if chars.next()? != '{' {
        return None;
    }
    loop {
        skip_ws(&mut chars);
        match chars.peek()? {
            '}' => return Some(out),
            ',' => {
                chars.next();
                continue;
            }
            _ => {}
        }
        let key = read_string(&mut chars)?;
        skip_ws(&mut chars);
        if chars.next()? != ':' {
            return None;
        }
        skip_ws(&mut chars);
        let value = match chars.peek()? {
            '"' => Json::Str(read_string(&mut chars)?),
            _ => {
                let mut raw = String::new();
                while chars.peek().is_some_and(|c| !matches!(c, ',' | '}')) {
                    raw.push(chars.next()?);
                }
                match raw.trim().parse::<f64>() {
                    Ok(n) => Json::Num(n),
                    Err(_) => Json::Other,
                }
            }
        };
        out.insert(key, value);
    }
}
