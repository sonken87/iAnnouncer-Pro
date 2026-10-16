// skeuo-ui for C++ — maps and aviation: MapView (slippy map with flight
// routes, markers and areas), Navlog (leg table) and great-circle helpers.
// Not part of MUI. Same API as the Rust `map` module; the map widget itself
// (Leaflet + map.js) is shared with the Rust version.
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ------------------------------------------------------------------ geo

/// Mean earth radius in nautical miles.
inline constexpr double kEarthRadiusNm = 3440.065;

/// A position in decimal degrees (north and east positive).
struct LatLon {
    double lat = 0;
    double lon = 0;
    bool operator==(const LatLon&) const = default;
};

/// Great-circle distance in nautical miles (haversine).
double distance_nm(LatLon a, LatLon b);
/// Initial true course from a to b, degrees 0..360.
double initial_course_deg(LatLon a, LatLon b);
/// The point a fraction f (0..1) of the way from a to b along the great circle.
LatLon intermediate_point(LatLon a, LatLon b, double f);
/// Course as on a chart: three digits, "045".
std::string format_course(double deg);
/// Hours as h:mm, "1:05".
std::string format_hours(double hours);

// ------------------------------------------------------------------ data

/// What a waypoint is, drawn with the matching aeronautical-chart symbol.
enum class WaypointKind {
    Airport,  // a circle with four ticks
    Vor,      // a hexagon with a centre dot
    Ndb,      // a dotted circle
    Fix,      // a triangle
    User,     // a diamond
};
std::string_view as_str(WaypointKind k);

/// A point of a flight route.
struct Waypoint {
    /// Identifier printed on the map ("ZBAA", "VYK", "WPT3").
    std::string ident;
    WaypointKind kind = WaypointKind::User;
    LatLon pos = {};
    std::optional<std::string> name = {};
    /// Planned altitude, printed under the ident.
    std::optional<double> altitude_ft = {};
};

/// One leg of a route, as in a navigation log.
struct Leg {
    /// Index of the waypoint the leg starts at (it ends at from + 1).
    size_t from = 0;
    double course_deg = 0;
    double distance_nm = 0;
    double cumulative_nm = 0;
};

/// The legs between consecutive waypoints.
std::vector<Leg> route_legs(const std::vector<Waypoint>& waypoints);

/// Something the user did on a MapView.
struct MapEvent {
    enum class Type {
        Click,           // map clicked (add_waypoints_to not set): pos
        AddWaypoint,     // map clicked with add_waypoints_to: route, pos
        MoveWaypoint,    // waypoint dragged: route, index, pos
        InsertWaypoint,  // leg midpoint handle dragged: route, index (new), pos
        SelectWaypoint,  // waypoint clicked: route, index
        RemoveWaypoint,  // waypoint double-clicked / right-clicked: route, index
        MarkerClick,     // standalone marker clicked: id
        ViewChange,      // pan/zoom ended (report_view): pos = centre, zoom
    };
    Type type = Type::Click;
    std::string route = {};
    size_t index = 0;
    LatLon pos = {};
    std::string id = {};
    double zoom = 0;

    /// Parses the JSON event the map widget reports.
    static std::optional<MapEvent> parse(std::string_view json);
};

/// A flight route drawn on a MapView: great-circle legs in the palette color,
/// labelled with true course and distance.
struct FlightRoute {
    /// Identifies the route in MapEvents.
    std::string id;
    std::vector<Waypoint> waypoints = {};
    Color color = Color::Primary;
    /// Waypoints can be dragged, inserted and removed; edits arrive as MapEvents.
    bool editable = false;
    /// Great-circle arcs rather than straight lines on the projection.
    bool geodesic = true;
    /// Label each leg with its course and distance.
    bool leg_labels = true;
    /// Waypoint highlighted with an LED glow.
    std::optional<size_t> selected = {};

    std::vector<Leg> legs() const { return route_legs(waypoints); }
    double total_distance_nm() const;
    /// Applies an editing event aimed at this route; returns whether anything
    /// changed. New points are user waypoints named WPT1, WPT2, ...
    bool apply(const MapEvent& event);
};

/// What a MapMarker shows.
enum class MapMarkerKind { Aircraft, Waypoint, Dot };

/// A standalone symbol on the map (aircraft, navaid, point of interest).
struct MapMarker {
    /// Identifies the marker in MapEvent::MarkerClick.
    std::string id;
    LatLon pos = {};
    MapMarkerKind kind = MapMarkerKind::Dot;
    /// Aircraft heading, degrees true.
    double heading_deg = 0;
    /// Symbol when kind is Waypoint.
    WaypointKind waypoint_kind = WaypointKind::User;
    std::optional<std::string> label = {};
    Color color = Color::Warning;

    static MapMarker aircraft(std::string id, double lat, double lon, double heading_deg);
};

/// A circular area (e.g. a restricted zone or a coverage radius).
struct MapCircle {
    std::string id;
    LatLon center = {};
    double radius_nm = 0;
    Color color = Color::Info;
    std::optional<std::string> label = {};
};

/// Where the base map tiles come from (an XYZ tile URL template). Defaults to
/// OpenStreetMap's public tiles: fine for development; their usage policy asks
/// heavy or commercial users to run their own tile server or use a provider.
struct TileSource {
    std::string url = "https://tile.openstreetmap.org/{z}/{x}/{y}.png";
    std::string attribution = "© OpenStreetMap contributors";
    int max_zoom = 19;
    std::string subdomains = "abc";
};

// ------------------------------------------------------------------ MapView

struct MapViewProps {
    LatLon center = {35.0, 105.0};
    double zoom = 4;
    /// Change this to move the map to center/zoom (or fit) again.
    unsigned view_key = 0;
    /// Fit all routes and markers instead of using center/zoom.
    bool fit_routes = false;
    TileSource tiles = {};
    std::vector<FlightRoute> routes = {};
    std::vector<MapMarker> markers = {};
    std::vector<MapCircle> circles = {};
    /// Clicking the map appends a waypoint to the route with this id.
    std::optional<std::string> add_waypoints_to = {};
    /// CSS height of the map.
    std::string height = "420px";
    bool interactive = true;
    bool scroll_zoom = true;
    /// Zoom / fit keycaps.
    bool controls = true;
    /// Nautical-mile scale bar.
    bool scale = true;
    /// Report MapEvent::Type::ViewChange after each pan/zoom.
    bool report_view = false;
    std::optional<double> min_zoom = {};
    std::optional<double> max_zoom = {};
    std::string aria_label = "Map";
    std::string cls = {};
    std::function<void(const MapEvent&)> onevent = {};
};

/// A slippy map set into a well, with flight routes, markers and areas. The
/// map keeps its own pan/zoom: center/zoom (or fit_routes) are applied when it
/// first appears and whenever view_key changes.
Node MapView(const MapViewProps& props);

/// The widget configuration as JSON (the data-skeuo-map attribute).
std::string map_config(const MapViewProps& props);

// ------------------------------------------------------------------ Navlog

struct NavlogProps {
    std::vector<Waypoint> waypoints = {};
    /// Ground speed in knots; adds the ETE columns.
    std::optional<double> ground_speed_kt = {};
    /// Highlights the leg ending at this waypoint.
    std::optional<size_t> selected = {};
    Color color = Color::Primary;
    Size size = Size::Small;
    std::string cls = {};
    /// Called with the index of the waypoint a clicked leg ends at.
    std::function<void(size_t)> onselect = {};
};

/// A navigation log: one row per leg with true course, distance, cumulative
/// distance and, given a ground speed, the estimated time en route.
Node Navlog(const NavlogProps& props);

}  // namespace skeuo
