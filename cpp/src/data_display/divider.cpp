#include "detail.hpp"

namespace skeuo {

namespace {
std::string_view variant_name(DividerVariant v) {
    switch (v) {
        case DividerVariant::FullWidth: return "full-width";
        case DividerVariant::Inset: return "inset";
        case DividerVariant::Middle: return "middle";
    }
    return "full-width";
}

std::string_view align_name(DividerTextAlign a) {
    switch (a) {
        case DividerTextAlign::Left: return "left";
        case DividerTextAlign::Center: return "center";
        case DividerTextAlign::Right: return "right";
    }
    return "center";
}
}  // namespace

Node Divider(const DividerProps& props, Node children) {
    using data_display_detail::flag;
    bool vertical = props.orientation == Orientation::Vertical;
    bool has_text = !(children.empty() || (children.kind() == Node::Kind::Text && children.content().empty()));
    std::string variant = "skeuo-divider--" + std::string(variant_name(props.variant));
    std::string orientation = "skeuo-divider--" + std::string(as_str(props.orientation));
    std::string_view flex = flag(props.flex_item, "skeuo-divider--flex-item");

    if (has_text) {
        std::string align = "skeuo-divider--text-" + std::string(align_name(props.text_align));
        std::string_view line = vertical ? "skeuo-rule skeuo-rule--vertical skeuo-divider__line"
                                         : "skeuo-rule skeuo-divider__line";
        Node root = el("div")
                        .cls(classes({"skeuo-divider", variant, orientation, flex, "skeuo-divider--with-children",
                                      align, props.cls}))
                        .attr("role", "separator");
        if (vertical) root.attr("aria-orientation", "vertical");
        return std::move(root)
            .child(el("span").cls(line))
            .child(el("span").cls("skeuo-divider__text skeuo-label").child(std::move(children)))
            .child(el("span").cls(line));
    }

    std::string_view material = vertical ? "skeuo-rule skeuo-rule--vertical" : "skeuo-rule";
    Node hr = el("hr").cls(classes({material, "skeuo-divider", variant, orientation, flex, props.cls}));
    if (vertical) hr.attr("aria-orientation", "vertical");
    return hr;
}

}  // namespace skeuo
