#include <cmath>

#include "skeuo/json.hpp"
#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {
bool close(double a, double b, double eps) { return std::fabs(a - b) < eps; }
}  // namespace

TEST(map_geo_distance_course_and_midpoint) {
    LatLon o{0, 0};
    CHECK(close(distance_nm(o, {0, 1}), 60.04, 0.01));
    CHECK(close(initial_course_deg(o, {0, 1}), 90, 1e-9));
    CHECK(close(initial_course_deg(o, {1, 0}), 0, 1e-9));
    CHECK(close(initial_course_deg(o, {0, -1}), 270, 1e-9));
    LatLon mid = intermediate_point(o, {0, 10}, 0.5);
    CHECK(close(mid.lat, 0, 1e-9) && close(mid.lon, 5, 1e-9));
    double d = distance_nm({40.0801, 116.5846}, {31.1434, 121.8052});
    CHECK(d > 560 && d < 600) << d;
    LatLon pek{40.08, 116.58}, lax{33.94, -118.41};
    double c = initial_course_deg(pek, lax);
    CHECK(c > 30 && c < 60) << c;
    CHECK(intermediate_point(pek, lax, 0.5).lat > 50);
}

TEST(map_geo_formatting) {
    CHECK(format_course(5.4) == "005");
    CHECK(format_course(359.6) == "000");
    CHECK(format_course(-10) == "350");
    CHECK(format_hours(1 + 5.0 / 60) == "1:05");
    CHECK(format_hours(0) == "0:00");
}

TEST(map_route_legs_accumulate) {
    std::vector<Waypoint> wps = {{.ident = "A", .kind = WaypointKind::Airport, .pos = {0, 0}},
                                 {.ident = "B", .kind = WaypointKind::Vor, .pos = {0, 1}},
                                 {.ident = "C", .kind = WaypointKind::Fix, .pos = {1, 1}}};
    auto legs = route_legs(wps);
    CHECK(legs.size() == 2);
    CHECK(close(legs[0].course_deg, 90, 1e-6));
    CHECK(close(legs[1].course_deg, 0, 1e-6));
    CHECK(close(legs[1].cumulative_nm, legs[0].distance_nm + legs[1].distance_nm, 1e-9));
    CHECK(route_legs({wps[0]}).empty());
}

TEST(map_event_parsing) {
    auto m = MapEvent::parse(R"({"type":"move","route":"r","index":2,"lat":40.5,"lon":-116.25})");
    CHECK((m && m->type == MapEvent::Type::MoveWaypoint && m->route == "r" && m->index == 2 &&
           m->pos == LatLon{40.5, -116.25}));
    auto c = MapEvent::parse(R"({"type":"click","lat":1,"lon":2})");
    CHECK((c && c->type == MapEvent::Type::Click && c->pos == LatLon{1, 2}));
    auto k = MapEvent::parse(R"({"type":"marker","id":"CA\"983"})");
    CHECK(k && k->type == MapEvent::Type::MarkerClick && k->id == "CA\"983");
    auto v = MapEvent::parse(R"({"type":"view","lat":10,"lon":20,"zoom":5.5})");
    CHECK(v && v->type == MapEvent::Type::ViewChange && v->zoom == 5.5);
    CHECK(!MapEvent::parse(R"({"type":"move","route":"r"})"));
    CHECK(!MapEvent::parse("not json"));
    CHECK(!MapEvent::parse(R"({"type":"teleport"})"));
}

TEST(map_flight_route_applies_own_edits) {
    FlightRoute r{.id = "plan",
                  .waypoints = {{.ident = "A", .kind = WaypointKind::Airport, .pos = {0, 0}},
                                {.ident = "B", .kind = WaypointKind::Airport, .pos = {0, 2}}}};
    LatLon p{1, 1};
    using T = MapEvent::Type;
    CHECK(!r.apply({.type = T::AddWaypoint, .route = "other", .pos = p}));
    CHECK(r.apply({.type = T::AddWaypoint, .route = "plan", .pos = p}));
    CHECK(r.waypoints[2].ident == "WPT1" && r.selected == 2u);
    CHECK(r.apply({.type = T::InsertWaypoint, .route = "plan", .index = 1, .pos = p}));
    CHECK(r.waypoints[1].ident == "WPT2");
    CHECK(r.apply({.type = T::MoveWaypoint, .route = "plan", .index = 0, .pos = p}));
    CHECK(r.waypoints[0].pos == p);
    CHECK(!r.apply({.type = T::MoveWaypoint, .route = "plan", .index = 99, .pos = p}));
    CHECK(r.apply({.type = T::RemoveWaypoint, .route = "plan", .index = 1}));
    CHECK(r.waypoints.size() == 3 && !r.selected);
    CHECK(r.apply({.type = T::AddWaypoint, .route = "plan", .pos = p}));
    CHECK(r.waypoints[3].ident == "WPT2");
}

TEST(map_view_renders_config_viewport_and_event_input) {
    std::string html = render_static([] {
        return MapView({.routes = {{.id = "r1",
                                    .waypoints = {{.ident = "ZB\"AA", .kind = WaypointKind::Airport,
                                                   .pos = {40.08, 116.58}, .altitude_ft = 1000},
                                                  {.ident = "ZSPD", .kind = WaypointKind::Airport, .pos = {31.14, 121.81}}},
                                    .editable = true}},
                        .markers = {MapMarker::aircraft("ac", 35, 118, 135)},
                        .add_waypoints_to = "r1",
                        .height = "300px"});
    });
    for (const char* needle : {R"(class="skeuo-map skeuo-well")", "height:300px", R"(class="skeuo-map__viewport")",
                               "data-sk-ignore", R"(type="hidden")", "skeuo-map__events", "data-skeuo-map="})
        CONTAINS(html, needle);
    size_t start = html.find("data-skeuo-map=\"") + 16;
    std::string raw = html.substr(start, html.find('"', start) - start);
    auto unescape = [](std::string s) {
        for (auto [from, to] : {std::pair{"&quot;", "\""}, std::pair{"&amp;", "&"}}) {
            for (size_t p; (p = s.find(from)) != std::string::npos;) s.replace(p, std::string(from).size(), to);
        }
        return s;
    };
    std::string cfg = unescape(raw);
    for (const char* needle : {R"("add_to":"r1")", R"("ident":"ZB\"AA")", R"("kind":"airport")", R"("alt_ft":1000)",
                               R"("editable":true)", R"("kind":"aircraft","heading":135)",
                               R"("url":"https://tile.openstreetmap.org/{z}/{x}/{y}.png")"})
        CONTAINS(cfg, needle);
    CHECK(skeuo::json::parse(cfg).type == skeuo::json::Value::Type::Object) << cfg;
}

TEST(map_view_reports_events_to_cpp) {
    std::optional<MapEvent> got;
    Runtime rt([&] { return MapView({.onevent = [&](const MapEvent& e) { got = e; }}); });
    CONTAINS(rt.render(), R"(data-sk-on="input:0")");
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"input","value":"{\"type\":\"click\",\"lat\":1.5,\"lon\":2}"}})");
    CHECK((got && got->type == MapEvent::Type::Click && got->pos == LatLon{1.5, 2}));
}

TEST(map_navlog_lists_legs_totals_and_ete) {
    std::string html = render_static([] {
        return Navlog({.waypoints = {{.ident = "A", .kind = WaypointKind::Airport, .pos = {0, 0}},
                                     {.ident = "B", .kind = WaypointKind::Vor, .pos = {0, 1}},
                                     {.ident = "C", .kind = WaypointKind::Airport, .pos = {0, 2}}},
                       .ground_speed_kt = 120,
                       .selected = 2});
    });
    for (const char* needle : {"Navigation log", ">090<", ">60.0<", ">120.1<", ">0:30<", ">1:00<", "Total", "ETE",
                               R"(aria-selected="true")"})
        CONTAINS(html, needle);
    std::string empty = render_static([] { return Navlog({}); });
    CONTAINS(empty, "Add at least two waypoints");
    LACKS(empty, "ETE");
}

TEST(map_navlog_row_click_selects) {
    std::optional<size_t> picked;
    Runtime rt([&] {
        return Navlog({.waypoints = {{.ident = "A", .pos = {0, 0}}, {.ident = "B", .pos = {0, 1}}},
                       .onselect = [&](size_t i) { picked = i; }});
    });
    CONTAINS(rt.render(), R"(data-sk-on="click:0")");
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"click"}})");
    CHECK(picked == 1u);
}

TEST(map_scroll_area_variants) {
    std::string html = render_static([] {
        return Node{ScrollArea({.variant = ScrollAreaVariant::Well, .max_height = "200px", .aria_label = "Log"}, "x"),
                    ScrollArea({.direction = ScrollDirection::Horizontal}, "y")};
    });
    for (const char* needle : {"skeuo-scroll-area skeuo-scroll-area--vertical skeuo-scroll-area--well skeuo-well",
                               "max-height:200px", R"(role="region")", R"(aria-label="Log")", R"(tabindex="0")",
                               "skeuo-scroll-area--horizontal"})
        CONTAINS(html, needle);
}

TEST(map_page_ships_leaflet_widget_and_styles) {
    Runtime rt([] { return Node{}; });
    std::string page = rt.page();
    CONTAINS(page, "window.__skeuoMap");
    CONTAINS(page, "window.L=");
    CONTAINS(stylesheet(), ".leaflet-container");
    CONTAINS(stylesheet(), ".skeuo-map__leg");
    CONTAINS(stylesheet(), "::-webkit-scrollbar-thumb");
}
