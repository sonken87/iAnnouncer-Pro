//! Maps and aviation: MapView (slippy map with flight routes, markers and
//! areas), Navlog (leg table), and great-circle navigation helpers.
//! Not part of MUI.

pub mod geo;
mod map_view;
mod navlog;

pub use geo::{
    LatLon, distance_nm, format_course, format_hours, initial_course_deg, intermediate_point,
};
pub use map_view::{
    FlightRoute, Leg, MapCircle, MapEvent, MapMarker, MapMarkerKind, MapView, MapViewProps,
    TileSource, Waypoint, WaypointKind, map_config, map_script, route_legs,
};
pub use navlog::{Navlog, NavlogProps};
