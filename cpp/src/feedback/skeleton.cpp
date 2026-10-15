#include "skeuo/feedback.hpp"

namespace skeuo {

Node Skeleton(const SkeletonProps& props, Node children) {
    std::string style;
    if (props.width) style += "width:" + *props.width + ";";
    if (props.height) style += "height:" + *props.height + ";";
    return el("span")
        .cls(classes({"skeuo-well skeuo-well--shallow skeuo-skeleton",
                      "skeuo-skeleton--" + std::string(as_str(props.variant)),
                      "skeuo-skeleton--" + std::string(as_str(props.animation)), props.cls}))
        .attr("style", style)
        .attr("aria-hidden", "true")
        .child(std::move(children));
}

}  // namespace skeuo
