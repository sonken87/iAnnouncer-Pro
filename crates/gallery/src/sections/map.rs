use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoRow};

/// Demo data only (airports are real, the en-route points are illustrative):
/// not for navigation.
fn beijing_shanghai() -> Vec<Waypoint> {
    vec![
        Waypoint::new("ZBAA", WaypointKind::Airport, 40.0801, 116.5846).named("Beijing Capital"),
        Waypoint::new("TJ", WaypointKind::Vor, 39.12, 117.35)
            .named("Tianjin")
            .at_altitude(9800.0),
        Waypoint::new("JNN", WaypointKind::Vor, 36.86, 117.22)
            .named("Jinan")
            .at_altitude(29100.0),
        Waypoint::new("P42", WaypointKind::Fix, 34.30, 118.10).at_altitude(29100.0),
        Waypoint::new("SSN", WaypointKind::Ndb, 32.05, 120.90).at_altitude(12000.0),
        Waypoint::new("ZSPD", WaypointKind::Airport, 31.1434, 121.8052).named("Shanghai Pudong"),
    ]
}

#[component]
pub fn MapSection() -> Element {
    rsx! {
        RoutePlanningDemo {}
        RouteMapDemo {}
    }
}

#[component]
fn RoutePlanningDemo() -> Element {
    let mut route = use_signal(|| FlightRoute::new("plan", beijing_shanghai()).editable(true));
    let mut view = use_signal(|| 0u32);
    let mut gs = use_signal(|| "450".to_string());
    let ground_speed = gs.read().trim().parse::<f64>().ok().filter(|v| *v > 0.0);
    let total = route.read().total_distance_nm();
    let summary = match ground_speed {
        Some(g) => format!(
            "{} waypoints · {total:.0} NM · {} at {g:.0} kt",
            route.read().waypoints.len(),
            format_hours(total / g)
        ),
        None => format!("{} waypoints · {total:.0} NM", route.read().waypoints.len()),
    };
    rsx! {
        Demo {
            title: "MapView · flight route planning",
            description: "a map set into a well · keycap zoom / fit · great-circle legs labelled with true course and NM · click to add, drag to move, drag a leg's midpoint to insert, double-click to delete",
            MapView {
                routes: vec![route()],
                add_waypoints_to: "plan",
                fit_routes: true,
                view_key: view(),
                height: "480px",
                aria_label: "Route planning map",
                onevent: move |e: MapEvent| {
                    route.write().apply(&e);
                },
            }
            div { style: "height: 14px" }
            DemoRow { label: "route",
                Button { variant: ButtonVariant::Outlined, start_icon: rsx! { Icon { name: IconName::Map } },
                    onclick: move |_| view += 1, "Fit route" }
                Button { variant: ButtonVariant::Outlined, color: Color::Neutral,
                    onclick: move |_| {
                        route.write().waypoints.reverse();
                        route.write().selected = None;
                    },
                    "Reverse" }
                Button { variant: ButtonVariant::Text, color: Color::Error,
                    onclick: move |_| {
                        route.set(FlightRoute::new("plan", beijing_shanghai()).editable(true));
                        view += 1;
                    },
                    "Reset" }
                TextField {
                    label: "Ground speed",
                    size: Size::Small,
                    value: gs(),
                    oninput: move |e: FormEvent| gs.set(e.value()),
                    end_adornment: rsx! { "kt" },
                    style: "min-width: 140px; width: 140px",
                }
                span { class: "skeuo-caption", "{summary}" }
            }
            Navlog {
                waypoints: route.read().waypoints.clone(),
                ground_speed_kt: ground_speed,
                selected: route.read().selected,
                onselect: move |i: usize| route.write().selected = Some(i),
            }
            p { class: "skeuo-caption", style: "margin: 10px 0 0",
                "Demo data, not for navigation. Tiles © OpenStreetMap contributors."
            }
        }
    }
}

#[component]
fn RouteMapDemo() -> Element {
    let route = vec![
        Waypoint::new("ZBAA", WaypointKind::Airport, 40.0801, 116.5846).named("Beijing Capital"),
        Waypoint::new("KLAX", WaypointKind::Airport, 33.9425, -118.4081).named("Los Angeles"),
    ];
    let (a, b) = (route[0].pos, route[1].pos);
    let now = intermediate_point(a, b, 0.3);
    let ahead = intermediate_point(a, b, 0.31);
    let heading = initial_course_deg(now, ahead);
    let great_circle = FlightRoute::new("gc", route.clone()).color(Color::Primary);
    let mut rhumb = FlightRoute::new("straight", route).color(Color::Neutral);
    rhumb.geodesic = false;
    rhumb.leg_labels = false;
    rsx! {
        Demo {
            title: "MapView · flight route map",
            description: "display only · the great-circle arc (blue) is the shortest path; the straight map line (grey) is longer · aircraft symbol rotated to its heading",
            MapView {
                routes: vec![rhumb, great_circle],
                markers: vec![
                    MapMarker::aircraft("CA983", now.lat, now.lon, heading).label("CA983 · FL370"),
                ],
                circles: vec![MapCircle {
                    id: "anchorage".into(),
                    center: LatLon::new(61.17, -149.99),
                    radius_nm: 250.0,
                    color: Color::Warning,
                    label: Some("Diversion area".into()),
                }],
                fit_routes: true,
                height: "360px",
                aria_label: "Beijing to Los Angeles route map",
            }
        }
    }
}
