#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>

#include "context.hpp"
#include "skeuo/runtime.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

namespace {

int decimals(double step) {
    for (int d = 0; d < 6; ++d) {
        double scaled = step * std::pow(10.0, d);
        if (std::abs(scaled - std::round(scaled)) < 1e-9) return d;
    }
    return 6;
}

std::optional<double> parse(const std::string& s) {
    if (s.empty()) return std::nullopt;
    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if (end != s.c_str() + s.size()) return std::nullopt;
    return v;
}

}  // namespace

Node Slider(const SliderProps& props) {
    using detail::fmt_num;
    double min = props.min;
    double max = std::max(props.max, min + DBL_EPSILON);
    auto& internal = use_state<SliderValue>("Slider.value", [&] { return props.default_value.value_or(SliderValue(props.min)); });
    SliderValue current = props.value.value_or(internal);
    bool vertical = props.orientation == Orientation::Vertical;
    Color color = props.color.value_or(Color::Primary);
    Size size = props.size.value_or(Size::Medium);

    auto pct = [=](double v) { return std::clamp((v - min) / (max - min) * 100.0, 0.0, 100.0); };
    int dec = decimals(props.step);
    auto fmt = [&](double v) { return props.value_label_format ? props.value_label_format(v) : detail::fmt_fixed(v, dec); };

    std::vector<double> thumbs = current.is_range() ? std::vector<double>{current.a, current.b}
                                                    : std::vector<double>{current.a};
    std::vector<std::pair<double, double>> segments;
    switch (props.track) {
        case SliderTrack::Off: break;
        case SliderTrack::Normal:
            if (current.is_range()) segments.push_back({pct(current.a), pct(current.b)});
            else segments.push_back({0.0, pct(current.a)});
            break;
        case SliderTrack::Inverted:
            if (current.is_range()) {
                segments.push_back({0.0, pct(current.a)});
                segments.push_back({pct(current.b), 100.0});
            } else {
                segments.push_back({pct(current.a), 100.0});
            }
            break;
    }

    std::vector<SliderMark> marks;
    switch (props.marks.kind) {
        case SliderMarks::Kind::None: break;
        case SliderMarks::Kind::Steps: {
            double count = std::floor((max - min) / props.step);
            if (props.step > 0.0 && count <= 200.0)
                for (size_t i = 0; i <= size_t(count); ++i) marks.push_back(SliderMark::tick(min + double(i) * props.step));
            break;
        }
        case SliderMarks::Kind::Custom: marks = props.marks.list; break;
    }
    bool labelled = std::any_of(marks.begin(), marks.end(), [](const SliderMark& m) { return m.label.has_value(); });
    auto in_track = [&](double p) {
        return std::any_of(segments.begin(), segments.end(),
                           [&](auto s) { return p >= s.first - 1e-9 && p <= s.second + 1e-9; });
    };

    unsigned fixed = (props.color ? detail::FcColor : 0) | (props.size ? detail::FcSize : 0);
    Node root = el("span")
                    .cls(classes({"skeuo-slider", color_class(color), "skeuo-slider--" + std::string(as_str(size)),
                                  vertical ? "skeuo-slider--vertical" : "skeuo-slider--horizontal",
                                  current.is_range() ? "skeuo-slider--range" : "",
                                  labelled ? "skeuo-slider--labelled" : "",
                                  props.disabled ? "skeuo-slider--disabled" : "", props.cls}))
                    .attr("data-sk-fc", detail::fc_marker("slider", fixed));
    if (!props.style.empty()) root.attr("style", props.style);

    Node rail = el("span").cls("skeuo-slider__rail skeuo-groove").attr("aria-hidden", "true");
    for (auto [a, b] : segments)
        rail.child(el("span")
                       .cls("skeuo-slider__track skeuo-groove__fill")
                       .attr("style", "--a: " + fmt_num(a) + "%; --b: " + fmt_num(b) + "%;"));
    root.child(std::move(rail));

    for (const auto& mark : marks) {
        std::string p = "--p: " + fmt_num(pct(mark.value)) + "%;";
        root.child(el("span")
                       .cls(in_track(pct(mark.value)) ? "skeuo-slider__mark skeuo-slider__mark--active" : "skeuo-slider__mark")
                       .attr("style", p)
                       .attr("aria-hidden", "true"));
        if (mark.label)
            root.child(el("span")
                           .cls("skeuo-slider__mark-label skeuo-caption")
                           .attr("style", p)
                           .attr("aria-hidden", "true")
                           .child(*mark.label));
    }

    // The new value when thumb `i` moves to `x` (range thumbs can't cross).
    auto next = [current](size_t i, double x) {
        if (!current.is_range()) return SliderValue(x);
        if (i == 0) return SliderValue::Range(std::min(x, current.b), current.b);
        return SliderValue::Range(current.a, std::max(x, current.a));
    };

    for (size_t i = 0; i < thumbs.size(); ++i) {
        double v = thumbs[i];
        Node input = el("input")
                         .cls("skeuo-slider__input skeuo-slider__input--" + std::to_string(i))
                         .attr("type", "range");
        if (props.name) input.attr("name", *props.name);
        input.attr("min", fmt_num(min)).attr("max", fmt_num(max)).attr("step", fmt_num(props.step)).attr("value", fmt_num(v));
        input.flag("disabled", props.disabled);
        if (props.aria_label)
            input.attr("aria-label", current.is_range() ? *props.aria_label + (i == 0 ? " start" : " end") : *props.aria_label);
        if (props.aria_labelledby) input.attr("aria-labelledby", *props.aria_labelledby);
        input.attr("aria-valuetext", fmt(v));
        input.attr_if(vertical, "aria-orientation", "vertical");
        input.on("input", [&internal, next, i, cb = props.onchange](const Event& e) {
            if (auto x = parse(e.value)) {
                SliderValue nv = next(i, *x);
                internal = nv;
                if (cb) cb(nv);
            }
        });
        input.on("change", [next, i, cb = props.onchange_committed](const Event& e) {
            auto x = parse(e.value);
            if (cb && x) cb(next(i, *x));
        });
        root.child(std::move(input));
    }

    for (size_t i = 0; i < thumbs.size(); ++i) {
        double v = thumbs[i];
        Node thumb = el("span")
                         .cls("skeuo-slider__thumb skeuo-slider__thumb--" + std::to_string(i) + " skeuo-key skeuo-key--round")
                         .attr("style", "--p: " + fmt_num(pct(v)) + "%;")
                         .attr("aria-hidden", "true");
        if (props.value_label_display == ValueLabelDisplay::Auto)
            thumb.child(el("span").cls("skeuo-slider__value-label skeuo-chassis skeuo-chassis--float skeuo-elev-3").child(fmt(v)));
        else if (props.value_label_display == ValueLabelDisplay::On)
            thumb.child(el("span")
                            .cls("skeuo-slider__value-label skeuo-slider__value-label--on skeuo-chassis skeuo-chassis--float skeuo-elev-3")
                            .child(fmt(v)));
        root.child(std::move(thumb));
    }
    return root;
}

}  // namespace skeuo
