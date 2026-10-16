// Map & aviation page; mirrors crates/gallery/src/sections/map.rs.
#include <algorithm>
#include <cstdio>

#include "../gallery.hpp"

namespace gallery {

namespace {

/// Demo data only (airports are real, the en-route points are illustrative):
/// not for navigation.
std::vector<Waypoint> beijing_shanghai() {
    return {
        {.ident = "ZBAA", .kind = WaypointKind::Airport, .pos = {40.0801, 116.5846}, .name = "Beijing Capital"},
        {.ident = "TJ", .kind = WaypointKind::Vor, .pos = {39.12, 117.35}, .name = "Tianjin", .altitude_ft = 9800},
        {.ident = "JNN", .kind = WaypointKind::Vor, .pos = {36.86, 117.22}, .name = "Jinan", .altitude_ft = 29100},
        {.ident = "P42", .kind = WaypointKind::Fix, .pos = {34.30, 118.10}, .altitude_ft = 29100},
        {.ident = "SSN", .kind = WaypointKind::Ndb, .pos = {32.05, 120.90}, .altitude_ft = 12000},
        {.ident = "ZSPD", .kind = WaypointKind::Airport, .pos = {31.1434, 121.8052}, .name = "Shanghai Pudong"},
    };
}

FlightRoute plan_route() { return {.id = "plan", .waypoints = beijing_shanghai(), .editable = true}; }

std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, f, v);
    return buf;
}

Node RoutePlanningDemo() {
    FlightRoute& route = use_state<FlightRoute>("map.route", [] { return plan_route(); });
    unsigned& view = use_state<unsigned>("map.view", [] { return 0u; });
    std::string& gs = use_state<std::string>("map.gs", [] { return std::string("450"); });
    std::optional<double> ground_speed;
    try {
        double g = std::stod(gs);
        if (g > 0) ground_speed = g;
    } catch (...) {
    }
    double total = route.total_distance_nm();
    std::string summary = std::to_string(route.waypoints.size()) + " waypoints · " + fmt("%.0f", total) + " NM";
    if (ground_speed) summary += " · " + format_hours(total / *ground_speed) + " at " + fmt("%.0f", *ground_speed) + " kt";

    return Demo(
        "MapView · flight route planning",
        "a map set into a well · keycap zoom / fit · great-circle legs labelled with true course and NM · click to "
        "add, drag to move, drag a leg's midpoint to insert, double-click to delete",
        {
            MapView({.view_key = view,
                     .fit_routes = true,
                     .routes = {route},
                     .add_waypoints_to = "plan",
                     .height = "480px",
                     .aria_label = "Route planning map",
                     .onevent = [&route](const MapEvent& e) { route.apply(e); }}),
            el("div").style("height: 14px"),
            DemoRow("route", {
                Button({.variant = ButtonVariant::Outlined, .start_icon = Icon(IconName::Map),
                        .onclick = [&view] { ++view; }}, "Fit route"),
                Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral,
                        .onclick = [&route] {
                            std::reverse(route.waypoints.begin(), route.waypoints.end());
                            route.selected.reset();
                        }}, "Reverse"),
                Button({.variant = ButtonVariant::Text, .color = Color::Error,
                        .onclick = [&route, &view] { route = plan_route(); ++view; }}, "Reset"),
                TextField({.size = Size::Small, .label = "Ground speed", .value = gs,
                           .end_adornment = Node("kt"), .style = "min-width: 140px; width: 140px",
                           .oninput = [&gs](const std::string& v) { gs = v; }}),
                el("span").cls("skeuo-caption").child(summary),
            }),
            Navlog({.waypoints = route.waypoints,
                    .ground_speed_kt = ground_speed,
                    .selected = route.selected,
                    .onselect = [&route](size_t i) { route.selected = i; }}),
            el("p").cls("skeuo-caption").style("margin: 10px 0 0")
                .child("Demo data, not for navigation. Tiles © OpenStreetMap contributors."),
        });
}

Node RouteMapDemo() {
    std::vector<Waypoint> route = {
        {.ident = "ZBAA", .kind = WaypointKind::Airport, .pos = {40.0801, 116.5846}, .name = "Beijing Capital"},
        {.ident = "KLAX", .kind = WaypointKind::Airport, .pos = {33.9425, -118.4081}, .name = "Los Angeles"},
    };
    LatLon now = intermediate_point(route[0].pos, route[1].pos, 0.3);
    LatLon ahead = intermediate_point(route[0].pos, route[1].pos, 0.31);
    MapMarker aircraft = MapMarker::aircraft("CA983", now.lat, now.lon, initial_course_deg(now, ahead));
    aircraft.label = "CA983 · FL370";
    return Demo(
        "MapView · flight route map",
        "display only · the great-circle arc (blue) is the shortest path; the straight map line (grey) is longer · "
        "aircraft symbol rotated to its heading",
        MapView({.fit_routes = true,
                 .routes = {{.id = "straight", .waypoints = route, .color = Color::Neutral, .geodesic = false,
                             .leg_labels = false},
                            {.id = "gc", .waypoints = route, .color = Color::Primary}},
                 .markers = {aircraft},
                 .circles = {{.id = "anchorage", .center = {61.17, -149.99}, .radius_nm = 250,
                              .color = Color::Warning, .label = "Diversion area"}},
                 .height = "360px",
                 .aria_label = "Beijing to Los Angeles route map"}));
}

}  // namespace

Node MapPage() { return {RoutePlanningDemo(), RouteMapDemo()}; }

}  // namespace gallery
