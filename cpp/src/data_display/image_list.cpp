#include "detail.hpp"

namespace skeuo {

namespace {
std::string_view variant_name(ImageListVariant v) {
    switch (v) {
        case ImageListVariant::Standard: return "standard";
        case ImageListVariant::Quilted: return "quilted";
        case ImageListVariant::Masonry: return "masonry";
        case ImageListVariant::Woven: return "woven";
    }
    return "standard";
}

std::string_view position_name(ImageListItemBarPosition p) {
    switch (p) {
        case ImageListItemBarPosition::Bottom: return "bottom";
        case ImageListItemBarPosition::Top: return "top";
        case ImageListItemBarPosition::Below: return "below";
    }
    return "bottom";
}

constexpr std::string_view kSpanPrefix = "grid-column-end: span ";

/// Masonry items keep their natural size: drop the grid spans an
/// ImageListItem renders at the start of its style.
void strip_spans(Node& item) {
    const std::string* style = item.get_attr("style");
    if (!style || style->rfind(kSpanPrefix, 0) != 0) return;
    size_t first = style->find(';');
    size_t second = first == std::string::npos ? first : style->find(';', first + 1);
    std::string rest = second == std::string::npos ? std::string() : style->substr(second + 1);
    if (rest.empty()) item.remove_attr("style");
    else item.set_attr("style", rest);
}
}  // namespace

Node ImageList(const ImageListProps& props, Node children) {
    if (props.variant == ImageListVariant::Masonry) {
        // Rust: ImageListContext { variant }. Items of nested lists belong to those.
        auto visit = [](Node& n) {
            if (n.tag() == "li" && n.has_class("skeuo-image-list-item")) strip_spans(n);
        };
        if (children.kind() == Node::Kind::Element) visit(children);
        data_display_detail::walk_children(
            children, [](const Node& n) { return n.has_class("skeuo-image-list"); }, visit);
    }
    std::string variant = "skeuo-image-list--" + std::string(variant_name(props.variant));
    std::string rows = props.row_height ? "grid-auto-rows: " + std::to_string(*props.row_height) + "px;" : "";
    std::string style = "--image-list-cols: " + std::to_string(props.cols < 1 ? 1 : props.cols) +
                        "; --image-list-gap: " + std::to_string(props.gap) + "px; " + rows + props.style;
    Node ul = el("ul").cls(classes({"skeuo-image-list", variant, props.cls})).attr("style", style);
    if (props.aria_label) ul.attr("aria-label", *props.aria_label);
    return std::move(ul).child(std::move(children));
}

Node ImageListItem(const ImageListItemProps& props, Node children) {
    std::string style = std::string(kSpanPrefix) + std::to_string(props.cols < 1 ? 1 : props.cols) +
                        "; grid-row-end: span " + std::to_string(props.rows < 1 ? 1 : props.rows) + ";" +
                        props.style;
    return el("li")
        .cls(classes({"skeuo-image-list-item", props.cls}))
        .attr("style", style)
        .child(std::move(children))
        .child(el("span").cls("skeuo-well skeuo-image-list-item__rim").attr("aria-hidden", "true"));
}

Node ImageListItemBar(const ImageListItemBarProps& props) {
    std::string position = "skeuo-image-list-item-bar--" + std::string(position_name(props.position));
    Node text = el("span").cls("skeuo-image-list-item-bar__text");
    if (props.title) text.child(el("span").cls("skeuo-image-list-item-bar__title").child(*props.title));
    if (props.subtitle) text.child(el("span").cls("skeuo-image-list-item-bar__subtitle").child(*props.subtitle));
    Node bar = el("div")
                   .cls(classes({"skeuo-image-list-item-bar", position,
                                 props.action_position_left ? "skeuo-image-list-item-bar--action-left" : "",
                                 props.cls}))
                   .child(std::move(text));
    if (props.action_icon) bar.child(el("span").cls("skeuo-image-list-item-bar__action").child(*props.action_icon));
    return bar;
}

}  // namespace skeuo
