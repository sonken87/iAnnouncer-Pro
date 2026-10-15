#include "skeuo/inputs.hpp"

namespace skeuo {

namespace {
std::string_view variant_name(IconButtonVariant v) {
    switch (v) {
        case IconButtonVariant::Text: return "text";
        case IconButtonVariant::Outlined: return "outlined";
        case IconButtonVariant::Contained: return "contained";
    }
    return "text";
}
}  // namespace

Node IconButton(const IconButtonProps& props, Node children) {
    std::string_view material = props.variant == IconButtonVariant::Text       ? ""
                                : props.variant == IconButtonVariant::Outlined ? "skeuo-key skeuo-key--pressable"
                                    : "skeuo-key skeuo-key--pressable skeuo-key--filled";
    std::string variant = "skeuo-icon-btn--" + std::string(variant_name(props.variant));
    std::string size = "skeuo-icon-btn--" + std::string(as_str(props.size));
    std::string_view edge = !props.edge                             ? ""
                            : *props.edge == IconButtonEdge::Start ? "skeuo-icon-btn--edge-start"
                                                                   : "skeuo-icon-btn--edge-end";
    bool round = props.round && props.variant != IconButtonVariant::Text;

    Node button = el("button")
                      .cls(classes({material, "skeuo-icon-btn", variant, color_class(props.color), size, edge,
                                    round ? "skeuo-key--round" : "", props.cls}))
                      .attr("type", props.type)
                      .flag("disabled", props.disabled);
    if (props.onclick) button.on("click", props.onclick);
    for (const auto& [k, v] : props.attrs) button.set_attr(k, v);
    return std::move(button).child(std::move(children));
}

Node Fab(const FabProps& props, Node children) {
    std::string variant = std::string("skeuo-fab--") + (props.variant == FabVariant::Extended ? "extended" : "circular");
    std::string size = "skeuo-fab--" + std::string(as_str(props.size));
    Node button = el("button")
                      .cls(classes({"skeuo-key skeuo-key--pressable skeuo-key--round",
                                    props.color == Color::Neutral ? "" : "skeuo-key--filled", "skeuo-fab", variant,
                                    color_class(props.color), size, props.cls}))
                      .attr("type", props.type)
                      .flag("disabled", props.disabled);
    if (props.onclick) button.on("click", props.onclick);
    for (const auto& [k, v] : props.attrs) button.set_attr(k, v);
    return std::move(button).child(std::move(children));
}

}  // namespace skeuo
