#include "skeuo/surfaces.hpp"

namespace skeuo {

Node ScrollArea(const ScrollAreaProps& props, Node children) {
    std::string_view direction = props.direction == ScrollDirection::Vertical     ? "skeuo-scroll-area--vertical"
                                 : props.direction == ScrollDirection::Horizontal ? "skeuo-scroll-area--horizontal"
                                                                                  : "skeuo-scroll-area--both";
    std::string style;
    if (props.height) style += "height:" + *props.height + ";";
    if (props.max_height) style += "max-height:" + *props.max_height + ";";
    if (props.max_width) style += "max-width:" + *props.max_width + ";";
    style += props.style;
    Node root = el("div")
                    .cls(classes({"skeuo-scroll-area", direction,
                                  props.variant == ScrollAreaVariant::Well ? "skeuo-scroll-area--well skeuo-well" : "",
                                  props.cls}))
                    .attr("style", style);
    if (props.aria_label) root.attr("role", "region").attr("aria-label", *props.aria_label).attr("tabindex", "0");
    return std::move(root).child(std::move(children));
}

}  // namespace skeuo
