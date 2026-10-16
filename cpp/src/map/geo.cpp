// Great-circle navigation on a spherical earth; same formulas as the Rust
// `geo` module and the map widget (assets/map.js).
#include <cmath>
#include <algorithm>
#include <array>
#include <cstdio>

#include "skeuo/map.hpp"

namespace skeuo {

namespace {
constexpr double kRad = 3.14159265358979323846 / 180.0;

double central_angle(LatLon a, LatLon b) {
    double d_lat = (b.lat - a.lat) * kRad, d_lon = (b.lon - a.lon) * kRad;
    double h = std::pow(std::sin(d_lat / 2), 2) +
               std::cos(a.lat * kRad) * std::cos(b.lat * kRad) * std::pow(std::sin(d_lon / 2), 2);
    return 2 * std::asin(std::min(1.0, std::sqrt(h)));
}
}  // namespace

double distance_nm(LatLon a, LatLon b) { return central_angle(a, b) * kEarthRadiusNm; }

double initial_course_deg(LatLon a, LatLon b) {
    double p1 = a.lat * kRad, p2 = b.lat * kRad, dl = (b.lon - a.lon) * kRad;
    double y = std::sin(dl) * std::cos(p2);
    double x = std::cos(p1) * std::sin(p2) - std::sin(p1) * std::cos(p2) * std::cos(dl);
    return std::fmod(std::atan2(y, x) / kRad + 360.0, 360.0);
}

LatLon intermediate_point(LatLon a, LatLon b, double f) {
    double d = central_angle(a, b);
    if (d < 1e-12) return a;
    auto to_vec = [](LatLon p) {
        double phi = p.lat * kRad, lam = p.lon * kRad;
        return std::array<double, 3>{std::cos(phi) * std::cos(lam), std::cos(phi) * std::sin(lam), std::sin(phi)};
    };
    auto va = to_vec(a), vb = to_vec(b);
    double ka = std::sin((1 - f) * d) / std::sin(d), kb = std::sin(f * d) / std::sin(d);
    double x = ka * va[0] + kb * vb[0], y = ka * va[1] + kb * vb[1], z = ka * va[2] + kb * vb[2];
    return {std::atan2(z, std::hypot(x, y)) / kRad, std::atan2(y, x) / kRad};
}

std::string format_course(double deg) {
    long v = std::lround(deg) % 360;
    if (v < 0) v += 360;
    char buf[24];
    std::snprintf(buf, sizeof buf, "%03ld", v);
    return buf;
}

std::string format_hours(double hours) {
    long minutes = std::max(0L, std::lround(hours * 60.0));
    char buf[32];
    std::snprintf(buf, sizeof buf, "%ld:%02ld", minutes / 60, minutes % 60);
    return buf;
}

std::vector<Leg> route_legs(const std::vector<Waypoint>& waypoints) {
    std::vector<Leg> legs;
    double total = 0;
    for (size_t i = 0; i + 1 < waypoints.size(); ++i) {
        double d = distance_nm(waypoints[i].pos, waypoints[i + 1].pos);
        total += d;
        legs.push_back({i, initial_course_deg(waypoints[i].pos, waypoints[i + 1].pos), d, total});
    }
    return legs;
}

}  // namespace skeuo
