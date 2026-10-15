#include "skeuo/feedback.hpp"

namespace skeuo {

Node Backdrop(const BackdropProps& props, Node children) {
    if (!props.open) return {};
    Node root = el("div")
                    .cls(classes({"skeuo-scrim", "skeuo-backdrop",
                                  props.invisible ? "skeuo-backdrop--invisible" : "",
                                  props.inline_ ? "skeuo-backdrop--inline" : "", props.cls}))
                    .attr("aria-hidden", "true");
    if (props.onclick) root.on("click", props.onclick);
    return std::move(root).child(std::move(children));
}

}  // namespace skeuo
