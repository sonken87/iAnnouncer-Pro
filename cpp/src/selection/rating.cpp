#include <algorithm>
#include <cmath>

#include "context.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

namespace {
std::string star_label(double v) { return v == 1.0 ? "1 Star" : detail::fmt_num(v) + " Stars"; }
}  // namespace

Node Rating(const RatingProps& props) {
    using detail::fmt_num;
    std::string generated = use_unique_id("skeuo-rating");
    std::string name = props.name.value_or(generated);
    auto& internal = use_state<std::optional<double>>("Rating.value", [&] { return props.default_value; });
    auto& hover = use_state<std::optional<double>>("Rating.hover", [] { return std::optional<double>(); });
    std::optional<double> value = props.value ? *props.value : internal;
    bool interactive = !props.read_only && !props.disabled;
    Size size = props.size.value_or(Size::Medium);
    double precision = props.precision > 0.0 && props.precision < 1.0 ? 0.5 : 1.0;
    auto round = [precision](double v) { return std::round(v / precision) * precision; };
    double shown = round((interactive && hover ? hover : value).value_or(0.0));

    // Disabled and size may still come from an enclosing FormControl; a
    // disabled rating also drops its hover handlers then (see context.hpp).
    unsigned fixed = props.size ? detail::FcSize : 0;
    Node root = el("span").cls(classes({
        "skeuo-rating",
        "skeuo-color-warning",
        "skeuo-rating--" + std::string(as_str(size)),
        props.read_only ? "skeuo-rating--read-only" : "",
        props.disabled ? "skeuo-rating--disabled" : "",
        interactive && hover ? "skeuo-rating--hovering" : "",
        props.cls,
    }));
    root.attr("data-sk-fc", detail::fc_marker("rating", fixed));

    bool controlled = props.value.has_value();
    auto set_hover = [&hover, cb = props.onchange_active](std::optional<double> v) {
        if (hover != v) {
            hover = v;
            if (cb) cb(v);
        }
    };
    auto commit = [&internal, controlled, cb = props.onchange](std::optional<double> v) {
        if (!controlled) internal = v;
        if (cb) cb(v);
    };

    Node items;
    for (unsigned n = 1; n <= props.max; ++n) {
        double i = double(n);
        double fill = std::round(std::clamp(shown - (i - 1.0), 0.0, 1.0) * 100.0);
        std::vector<double> steps = precision < 1.0 ? std::vector<double>{i - 0.5, i} : std::vector<double>{i};
        bool active = shown >= i && hover && round(*hover) == i;
        Node item = el("span").cls(active ? "skeuo-rating__item skeuo-rating__item--active" : "skeuo-rating__item");
        item.child(el("span")
                       .cls("skeuo-rating__icon")
                       .attr("aria-hidden", "true")
                       .child(Icon({.name = IconName::StarBorder, .cls = "skeuo-rating__empty"}))
                       .child(el("span")
                                  .cls("skeuo-rating__fill")
                                  .attr("style", "width: " + fmt_num(fill) + "%;")
                                  .child(Icon(IconName::Star))));
        if (!props.read_only) {
            for (size_t k = 0; k < steps.size(); ++k) {
                double v = steps[k];
                Node label = el("label").cls(precision < 1.0 ? (k == 0 ? "skeuo-rating__hit skeuo-rating__hit--first"
                                                                       : "skeuo-rating__hit skeuo-rating__hit--second")
                                                             : "skeuo-rating__hit");
                if (interactive) label.on("mouseenter", [set_hover, v] { set_hover(v); });
                Node input = el("input")
                                 .cls("skeuo-rating__input")
                                 .attr("type", "radio")
                                 .attr("name", name)
                                 .attr("value", fmt_num(v))
                                 .flag("checked", value == v)
                                 .flag("disabled", props.disabled)
                                 .attr("aria-label", star_label(v));
                // Clicking the current value again clears it, as in MUI.
                bool current = value == v;
                input.on("click", [commit, current] {
                    if (current) commit(std::nullopt);
                });
                input.on("change", [commit, v] { commit(v); });
                label.child(std::move(input));
                item.child(std::move(label));
            }
        }
        items.add_child(std::move(item));
    }

    if (props.read_only) {
        std::string label = props.aria_label.value_or(
            value ? "Rated " + fmt_num(round(*value)) + " out of " + std::to_string(props.max)
                  : "Not rated, out of " + std::to_string(props.max));
        return std::move(root).attr("role", "img").attr("aria-label", label).child(std::move(items));
    }
    root.attr("role", "radiogroup");
    if (props.aria_label) root.attr("aria-label", *props.aria_label);
    if (props.aria_labelledby) root.attr("aria-labelledby", *props.aria_labelledby);
    root.attr_if(props.disabled, "aria-disabled", "true");
    root.on("mouseleave", [set_hover] { set_hover(std::nullopt); });
    return std::move(root).child(std::move(items));
}

}  // namespace skeuo
