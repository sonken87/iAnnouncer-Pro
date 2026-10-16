#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

#include "skeuo/feedback.hpp"

namespace skeuo {

namespace {
double clamp_percent(double v) { return std::isnan(v) ? 0.0 : std::clamp(v, 0.0, 100.0); }

/// Formats a number for CSS / SVG like the Rust crate: 3 decimals, trailing
/// zeros trimmed.
std::string num(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.3f", v);
    std::string s = buf;
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    if (s.empty() || s == "-" || s == "-0") return "0";
    return s;
}
}  // namespace

Node CircularProgress(const CircularProgressProps& props) {
    constexpr double kBox = 44.0;
    const double thickness = std::clamp(props.thickness, 0.5, kBox / 2.0);
    const double radius = (kBox - thickness) / 2.0;
    const double circumference = 2.0 * std::numbers::pi * radius;
    const bool determinate = props.variant == CircularProgressVariant::Determinate;
    const double value = clamp_percent(props.value);

    // The groove is a disc with the middle masked out; the hole's radius as
    // a percentage of the outer radius.
    const double hole = (kBox / 2.0 - thickness) / (kBox / 2.0) * 100.0;
    const std::string size = std::to_string(props.size);
    Node root = el("span")
                    .cls(classes({"skeuo-circular", "skeuo-circular--" + std::string(as_str(props.variant)),
                                  color_class(props.color),
                                  props.disable_shrink && !determinate ? "skeuo-circular--disable-shrink" : "",
                                  props.cls}))
                    .attr("style", "width:" + size + "px;height:" + size + "px;--circular-hole:" + num(hole) + "%")
                    .attr("role", "progressbar");
    if (props.aria_label) root.attr("aria-label", *props.aria_label);
    if (determinate) {
        root.attr("aria-valuemin", "0").attr("aria-valuemax", "100").attr("aria-valuenow", num(std::round(value)));
    }

    std::string arc_style;
    if (determinate) {
        arc_style = "stroke-dasharray:" + num(circumference) +
                    ";stroke-dashoffset:" + num((100.0 - value) / 100.0 * circumference);
        // A round cap would still draw a dot at 0%.
        if (value <= 0.0) arc_style += ";visibility:hidden";
    }
    root.child(el("span").cls("skeuo-groove skeuo-circular__track"));
    root.child(el("svg")
                   .cls("skeuo-circular__svg")
                   .attr("viewBox", "22 22 44 44")
                   .attr("aria-hidden", "true")
                   .child(el("circle")
                              .cls("skeuo-circular__arc")
                              .attr("cx", "44")
                              .attr("cy", "44")
                              .attr("r", num(radius))
                              .attr("fill", "none")
                              .attr("stroke-width", num(thickness))
                              .attr("style", arc_style)));
    return root;
}

Node LinearProgress(const LinearProgressProps& props) {
    const double value = clamp_percent(props.value);
    const double buffer = clamp_percent(props.value_buffer);
    const bool has_value =
        props.variant == LinearProgressVariant::Determinate || props.variant == LinearProgressVariant::Buffer;

    Node root = el("span")
                    .cls(classes({"skeuo-groove", "skeuo-linear",
                                  "skeuo-linear--" + std::string(as_str(props.variant)), color_class(props.color),
                                  props.cls}))
                    .attr("role", "progressbar");
    if (props.aria_label) root.attr("aria-label", *props.aria_label);
    if (has_value) {
        root.attr("aria-valuemin", "0").attr("aria-valuemax", "100").attr("aria-valuenow", num(std::round(value)));
    }
    switch (props.variant) {
        case LinearProgressVariant::Buffer:
            root.child(
                el("span").cls("skeuo-groove__fill skeuo-linear__buffer").attr("style", "width:" + num(buffer) + "%"));
            [[fallthrough]];
        case LinearProgressVariant::Determinate:
            root.child(
                el("span").cls("skeuo-groove__fill skeuo-linear__bar").attr("style", "width:" + num(value) + "%"));
            break;
        case LinearProgressVariant::Indeterminate:
        case LinearProgressVariant::Query:
            root.child(el("span").cls("skeuo-groove__fill skeuo-linear__bar skeuo-linear__bar--1"));
            root.child(el("span").cls("skeuo-groove__fill skeuo-linear__bar skeuo-linear__bar--2"));
            break;
    }
    return root;
}

}  // namespace skeuo
