use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

fn close(a: f64, b: f64, eps: f64) -> bool {
    (a - b).abs() < eps
}

#[test]
fn geo_distance_course_and_midpoint() {
    let o = LatLon::new(0.0, 0.0);
    // One degree of arc on a 3440.065 NM sphere.
    assert!(close(distance_nm(o, LatLon::new(0.0, 1.0)), 60.04, 0.01));
    assert!(close(
        initial_course_deg(o, LatLon::new(0.0, 1.0)),
        90.0,
        1e-9
    ));
    assert!(close(
        initial_course_deg(o, LatLon::new(1.0, 0.0)),
        0.0,
        1e-9
    ));
    assert!(close(
        initial_course_deg(o, LatLon::new(0.0, -1.0)),
        270.0,
        1e-9
    ));
    let mid = intermediate_point(o, LatLon::new(0.0, 10.0), 0.5);
    assert!(close(mid.lat, 0.0, 1e-9) && close(mid.lon, 5.0, 1e-9));
    // Beijing Capital to Shanghai Pudong is about 580 NM.
    let d = distance_nm(
        LatLon::new(40.0801, 116.5846),
        LatLon::new(31.1434, 121.8052),
    );
    assert!((560.0..600.0).contains(&d), "{d}");
    // Great circle from Beijing to Los Angeles heads north-east, crossing high latitudes.
    let (pek, lax) = (LatLon::new(40.08, 116.58), LatLon::new(33.94, -118.41));
    let c = initial_course_deg(pek, lax);
    assert!((30.0..60.0).contains(&c), "{c}");
    assert!(intermediate_point(pek, lax, 0.5).lat > 50.0);
}

#[test]
fn geo_formatting() {
    assert_eq!(format_course(5.4), "005");
    assert_eq!(format_course(359.6), "000");
    assert_eq!(format_course(-10.0), "350");
    assert_eq!(format_hours(1.0 + 5.0 / 60.0), "1:05");
    assert_eq!(format_hours(0.0), "0:00");
}

#[test]
fn route_legs_accumulate() {
    let wps = vec![
        Waypoint::new("A", WaypointKind::Airport, 0.0, 0.0),
        Waypoint::new("B", WaypointKind::Vor, 0.0, 1.0),
        Waypoint::new("C", WaypointKind::Fix, 1.0, 1.0),
    ];
    let legs = route_legs(&wps);
    assert_eq!(legs.len(), 2);
    assert!(close(legs[0].course_deg, 90.0, 1e-6));
    assert!(close(legs[1].course_deg, 0.0, 1e-6));
    assert!(close(
        legs[1].cumulative_nm,
        legs[0].distance_nm + legs[1].distance_nm,
        1e-9
    ));
    assert!(route_legs(&wps[..1]).is_empty());
}

#[test]
fn map_event_parsing() {
    assert_eq!(
        MapEvent::parse(r#"{"type":"move","route":"r","index":2,"lat":40.5,"lon":-116.25}"#),
        Some(MapEvent::MoveWaypoint {
            route: "r".into(),
            index: 2,
            pos: LatLon::new(40.5, -116.25)
        })
    );
    assert_eq!(
        MapEvent::parse(r#"{"type":"click","lat":1,"lon":2}"#),
        Some(MapEvent::Click(LatLon::new(1.0, 2.0)))
    );
    assert_eq!(
        MapEvent::parse(r#"{"type":"marker","id":"CA\"983"}"#),
        Some(MapEvent::MarkerClick {
            id: "CA\"983".into()
        })
    );
    assert_eq!(
        MapEvent::parse(r#"{"type":"view","lat":10,"lon":20,"zoom":5.5}"#),
        Some(MapEvent::ViewChange {
            center: LatLon::new(10.0, 20.0),
            zoom: 5.5
        })
    );
    assert_eq!(MapEvent::parse(r#"{"type":"move","route":"r"}"#), None); // missing fields
    assert_eq!(MapEvent::parse("not json"), None);
    assert_eq!(MapEvent::parse(r#"{"type":"teleport"}"#), None);
}

#[test]
fn flight_route_applies_edits_for_its_own_id_only() {
    let mut r = FlightRoute::new(
        "plan",
        vec![
            Waypoint::new("A", WaypointKind::Airport, 0.0, 0.0),
            Waypoint::new("B", WaypointKind::Airport, 0.0, 2.0),
        ],
    );
    let p = LatLon::new(1.0, 1.0);
    assert!(!r.apply(&MapEvent::AddWaypoint {
        route: "other".into(),
        pos: p
    }));
    assert!(r.apply(&MapEvent::AddWaypoint {
        route: "plan".into(),
        pos: p
    }));
    assert_eq!(r.waypoints[2].ident, "WPT1");
    assert_eq!(r.selected, Some(2));
    assert!(r.apply(&MapEvent::InsertWaypoint {
        route: "plan".into(),
        index: 1,
        pos: p
    }));
    assert_eq!(r.waypoints[1].ident, "WPT2");
    assert!(r.apply(&MapEvent::MoveWaypoint {
        route: "plan".into(),
        index: 0,
        pos: p
    }));
    assert_eq!(r.waypoints[0].pos, p);
    assert!(!r.apply(&MapEvent::MoveWaypoint {
        route: "plan".into(),
        index: 99,
        pos: p
    }));
    assert!(r.apply(&MapEvent::RemoveWaypoint {
        route: "plan".into(),
        index: 1
    }));
    assert_eq!(r.waypoints.len(), 3);
    assert_eq!(r.selected, None);
    // The freed name is reused.
    assert!(r.apply(&MapEvent::AddWaypoint {
        route: "plan".into(),
        pos: p
    }));
    assert_eq!(r.waypoints[3].ident, "WPT2");
}

#[test]
fn map_view_renders_config_viewport_and_event_input() {
    let html = render(|| {
        rsx! {
            MapView {
                routes: vec![FlightRoute::new("r1", vec![
                    Waypoint::new("ZB\"AA", WaypointKind::Airport, 40.08, 116.58).at_altitude(1000.0),
                    Waypoint::new("ZSPD", WaypointKind::Airport, 31.14, 121.81),
                ]).editable(true)],
                markers: vec![MapMarker::aircraft("ac", 35.0, 118.0, 135.0).label("CA983")],
                add_waypoints_to: "r1",
                height: "300px",
            }
        }
    });
    for needle in [
        r#"class="skeuo-map skeuo-well""#,
        "height:300px",
        r#"class="skeuo-map__viewport""#,
        "data-sk-ignore",
        r#"type="hidden""#,
        "skeuo-map__events",
        "data-skeuo-map=",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
    // The config is valid JSON with the route, marker and add_to target.
    let start = html.find("data-skeuo-map=\"").unwrap() + "data-skeuo-map=\"".len();
    let raw = &html[start..start + html[start..].find('"').unwrap()];
    let json = raw
        .replace("&quot;", "\"")
        .replace("&#34;", "\"")
        .replace("&amp;", "&");
    for needle in [
        r#""add_to":"r1""#,
        r#""ident":"ZB\"AA""#,
        r#""kind":"airport""#,
        r#""alt_ft":1000"#,
        r#""editable":true"#,
        r#""kind":"aircraft","heading":135"#,
        r#""url":"https://tile.openstreetmap.org/{z}/{x}/{y}.png""#,
    ] {
        assert!(json.contains(needle), "missing {needle} in {json}");
    }
    assert_eq!(
        json.matches('{').count(),
        json.matches('}').count(),
        "{json}"
    );
    assert_eq!(
        json.matches('[').count(),
        json.matches(']').count(),
        "{json}"
    );
}

#[test]
fn navlog_lists_legs_totals_and_ete() {
    let html = render(|| {
        rsx! {
            Navlog {
                waypoints: vec![
                    Waypoint::new("A", WaypointKind::Airport, 0.0, 0.0),
                    Waypoint::new("B", WaypointKind::Vor, 0.0, 1.0),
                    Waypoint::new("C", WaypointKind::Airport, 0.0, 2.0),
                ],
                ground_speed_kt: Some(120.0),
                selected: Some(2),
            }
        }
    });
    for needle in [
        "Navigation log",
        ">090<",
        ">60.0<",
        ">120.1<",
        ">0:30<",
        ">1:00<",
        "Total",
        "ETE",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
    assert!(html.contains(r#"aria-selected="true""#), "{html}");
    let empty = render(|| rsx! { Navlog { waypoints: vec![], ground_speed_kt: None } });
    assert!(empty.contains("Add at least two waypoints"), "{empty}");
    assert!(!empty.contains("ETE"), "{empty}");
}

#[test]
fn scroll_area_variants() {
    let html = render(|| {
        rsx! {
            ScrollArea { variant: ScrollAreaVariant::Well, max_height: "200px", aria_label: "Log", "x" }
            ScrollArea { direction: ScrollDirection::Horizontal, "y" }
        }
    });
    for needle in [
        "skeuo-scroll-area skeuo-scroll-area--vertical skeuo-scroll-area--well skeuo-well",
        "max-height:200px",
        r#"role="region""#,
        r#"aria-label="Log""#,
        r#"tabindex="0""#,
        "skeuo-scroll-area--horizontal",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
    assert_eq!(html.matches("tabindex").count(), 1, "{html}");
}

#[test]
fn stylesheet_includes_scrollbars_and_map_styles() {
    let css = skeuo_ui::STYLESHEET.as_str();
    assert!(css.contains("::-webkit-scrollbar-thumb"));
    assert!(css.contains(".leaflet-container"));
    assert!(css.contains(".skeuo-map__leg"));
    assert!(map_script().contains("window.__skeuoMap"));
}
