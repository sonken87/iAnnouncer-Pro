#include "skeuo/inputs.hpp"

namespace skeuo {

namespace {
std::string_view variant_name(ButtonVariant v) {
    switch (v) {
        case ButtonVariant::Contained: return "contained";
        case ButtonVariant::Outlined: return "outlined";
        case ButtonVariant::Text: return "text";
    }
    return "contained";
}

std::string_view position_name(LoadingPosition p) {
    switch (p) {
        case LoadingPosition::Start: return "start";
        case LoadingPosition::End: return "end";
        case LoadingPosition::Center: return "center";
    }
    return "center";
}
}  // namespace

Node Button(const ButtonProps& props, Node children) {
    std::string_view material = props.variant == ButtonVariant::Contained
                                    ? "skeuo-key skeuo-key--pressable skeuo-key--filled"
                                : props.variant == ButtonVariant::Outlined ? "skeuo-key skeuo-key--pressable"
                                                                           : "";
    std::string variant = "skeuo-btn--" + std::string(variant_name(props.variant));
    std::string size = "skeuo-btn--" + std::string(as_str(props.size));
    std::string loading = props.loading ? "skeuo-btn--loading skeuo-btn--loading-" +
                                              std::string(position_name(props.loading_position))
                                        : "";

    Node button = el("button")
                      .cls(classes({material, "skeuo-btn", variant, color_class(props.color), size,
                                    props.full_width ? "skeuo-btn--full-width" : "", loading, props.cls}))
                      .attr("type", props.type)
                      .flag("disabled", props.disabled || props.loading)
                      .attr_if(props.loading, "aria-busy", "true");
    for (const auto& [k, v] : props.attrs) button.set_attr(k, v);
    if (props.onclick) button.on("click", props.onclick);

    auto indicator = [&] {
        if (props.loading_indicator) return *props.loading_indicator;
        return el("span").cls("skeuo-spinner").attr("role", "progressbar").attr("aria-label", "Loading");
    };
    bool show_start = props.loading && props.loading_position == LoadingPosition::Start;
    bool show_end = props.loading && props.loading_position == LoadingPosition::End;
    bool show_center = props.loading && props.loading_position == LoadingPosition::Center;

    if (show_start) {
        button.child(el("span").cls("skeuo-btn__icon skeuo-btn__icon--start skeuo-btn__loader").child(indicator()));
    } else if (props.start_icon) {
        button.child(el("span").cls("skeuo-btn__icon skeuo-btn__icon--start").attr("aria-hidden", "true")
                         .child(*props.start_icon));
    }
    button.child(el("span").cls("skeuo-btn__label").child(std::move(children)));
    if (show_center) button.child(el("span").cls("skeuo-btn__loader skeuo-btn__loader--center").child(indicator()));
    if (show_end) {
        button.child(el("span").cls("skeuo-btn__icon skeuo-btn__icon--end skeuo-btn__loader").child(indicator()));
    } else if (props.end_icon) {
        button.child(el("span").cls("skeuo-btn__icon skeuo-btn__icon--end").attr("aria-hidden", "true")
                         .child(*props.end_icon));
    }
    return button;
}

}  // namespace skeuo
