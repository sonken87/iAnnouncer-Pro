//! Great-circle navigation on a spherical earth, in aviation units
//! (nautical miles, degrees true). The map widget (`assets/map.js`) uses the
//! same formulas, so labels on the map and the navlog always agree.

/// Mean earth radius in nautical miles.
pub const EARTH_RADIUS_NM: f64 = 3440.065;

/// A position in decimal degrees (north and east positive).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct LatLon {
    pub lat: f64,
    pub lon: f64,
}

impl LatLon {
    pub const fn new(lat: f64, lon: f64) -> Self {
        Self { lat, lon }
    }
}

fn central_angle(a: LatLon, b: LatLon) -> f64 {
    let (d_lat, d_lon) = ((b.lat - a.lat).to_radians(), (b.lon - a.lon).to_radians());
    let h = (d_lat / 2.0).sin().powi(2)
        + a.lat.to_radians().cos() * b.lat.to_radians().cos() * (d_lon / 2.0).sin().powi(2);
    2.0 * h.sqrt().min(1.0).asin()
}

/// Great-circle distance in nautical miles (haversine).
pub fn distance_nm(a: LatLon, b: LatLon) -> f64 {
    central_angle(a, b) * EARTH_RADIUS_NM
}

/// Initial true course from `a` to `b`, in degrees 0..360.
pub fn initial_course_deg(a: LatLon, b: LatLon) -> f64 {
    let (p1, p2) = (a.lat.to_radians(), b.lat.to_radians());
    let dl = (b.lon - a.lon).to_radians();
    let y = dl.sin() * p2.cos();
    let x = p1.cos() * p2.sin() - p1.sin() * p2.cos() * dl.cos();
    (y.atan2(x).to_degrees() + 360.0) % 360.0
}

/// The point a fraction `f` (0..=1) of the way from `a` to `b` along the
/// great circle.
pub fn intermediate_point(a: LatLon, b: LatLon, f: f64) -> LatLon {
    let d = central_angle(a, b);
    if d < 1e-12 {
        return a;
    }
    let to_vec = |p: LatLon| {
        let (phi, lam) = (p.lat.to_radians(), p.lon.to_radians());
        [phi.cos() * lam.cos(), phi.cos() * lam.sin(), phi.sin()]
    };
    let (va, vb) = (to_vec(a), to_vec(b));
    let (ka, kb) = (((1.0 - f) * d).sin() / d.sin(), (f * d).sin() / d.sin());
    let (x, y, z) = (
        ka * va[0] + kb * vb[0],
        ka * va[1] + kb * vb[1],
        ka * va[2] + kb * vb[2],
    );
    LatLon::new(z.atan2(x.hypot(y)).to_degrees(), y.atan2(x).to_degrees())
}

/// Course formatted as on a chart or flight plan: three digits, `"045"`.
pub fn format_course(deg: f64) -> String {
    format!("{:03}", (deg.round() as i64).rem_euclid(360))
}

/// Duration in hours formatted as `h:mm` (`"1:05"`).
pub fn format_hours(hours: f64) -> String {
    let minutes = (hours * 60.0).round().max(0.0) as i64;
    format!("{}:{:02}", minutes / 60, minutes % 60)
}
