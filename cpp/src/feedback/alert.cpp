#include "skeuo/feedback.hpp"
#include "skeuo/icons.hpp"

namespace skeuo {

namespace {
IconName severity_icon(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::Success: return IconName::CheckCircle;
        case AlertSeverity::Info: return IconName::Info;
        case AlertSeverity::Warning: return IconName::Warning;
        case AlertSeverity::Error: return IconName::Error;
    }
    return IconName::Info;
}
}  // namespace

Node Alert(const AlertProps& props, Node children) {
    Color color = props.color.value_or(severity_color(props.severity));
    std::string variant = "skeuo-alert--" + std::string(as_str(props.variant));
    std::string_view material =
        props.variant == AlertVariant::Filled ? "skeuo-key skeuo-key--filled" : "skeuo-chassis skeuo-elev-0";

    Node root = el("div")
                    .cls(classes({material, "skeuo-alert", variant, color_class(color), props.cls}))
                    .attr("role", props.role);
    if (!props.hide_icon) {
        root.child(el("div")
                       .cls("skeuo-alert__icon")
                       .attr("aria-hidden", "true")
                       .child(props.icon ? *props.icon : Icon(severity_icon(props.severity))));
    }
    root.child(el("div").cls("skeuo-alert__message").child(std::move(children)));
    if (props.action) {
        root.child(el("div").cls("skeuo-alert__action").child(*props.action));
    } else if (props.onclose) {
        root.child(el("div").cls("skeuo-alert__action").child(
            el("button")
                .cls("skeuo-alert__close")
                .attr("type", "button")
                .attr("aria-label", props.close_text)
                .attr("title", props.close_text)
                .on("click", props.onclose)
                .child(Icon(IconName::Close))));
    }
    return root;
}

Node AlertTitle(const AlertTitleProps& props, Node children) {
    return el("div").cls(classes({"skeuo-alert__title", props.cls})).child(std::move(children));
}

}  // namespace skeuo
