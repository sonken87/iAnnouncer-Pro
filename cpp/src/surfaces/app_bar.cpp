#include "skeuo/surfaces.hpp"

namespace skeuo {

namespace {
/// The palette entry of a colored plate.
std::optional<Color> palette(AppBarColor c) {
    switch (c) {
        case AppBarColor::Primary: return Color::Primary;
        case AppBarColor::Secondary: return Color::Secondary;
        case AppBarColor::Success: return Color::Success;
        case AppBarColor::Error: return Color::Error;
        case AppBarColor::Warning: return Color::Warning;
        case AppBarColor::Info: return Color::Info;
        default: return std::nullopt;
    }
}
}  // namespace

Node AppBar(const AppBarProps& props, Node children) {
    AppBarColor color = props.color.value;
    std::string position = "skeuo-appbar--" + std::string(as_str(props.position));
    std::string color_cls = "skeuo-appbar--" + std::string(as_str(color));
    std::string material;
    if (auto c = palette(color)) {
        material = classes({"skeuo-key skeuo-key--filled skeuo-appbar--colored", color_class(*c),
                            props.square ? "" : "skeuo-appbar--rounded"});
    } else if (color != AppBarColor::Transparent) {
        material = chassis_classes(PaperVariant::Elevation, props.elevation, props.square);
    }
    Node header = el("header").cls(classes({material, "skeuo-appbar", position, color_cls, props.cls}));
    for (const auto& [k, v] : props.attrs) header.set_attr(k, v);
    return std::move(header).child(std::move(children));
}

Node Toolbar(const ToolbarProps& props, Node children) {
    Node div = el("div").cls(classes({
        "skeuo-toolbar",
        props.variant == ToolbarVariant::Dense ? "skeuo-toolbar--dense" : "skeuo-toolbar--regular",
        props.disable_gutters ? "" : "skeuo-toolbar--gutters",
        props.cls,
    }));
    for (const auto& [k, v] : props.attrs) div.set_attr(k, v);
    return std::move(div).child(std::move(children));
}

}  // namespace skeuo
