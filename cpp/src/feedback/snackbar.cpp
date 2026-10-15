// Snackbar and SnackbarContent: a light level-3 floating chassis (see
// crates/skeuo-ui/src/feedback/snackbar.rs for why it is not dark).
#include "skeuo/feedback.hpp"

namespace skeuo {

Node Snackbar(const SnackbarProps& props, Node children) {
    if (!props.open) return {};
    std::string_view vertical =
        props.anchor_origin.vertical == SnackbarVertical::Top ? "skeuo-snackbar--top" : "skeuo-snackbar--bottom";
    std::string_view horizontal = props.anchor_origin.horizontal == SnackbarHorizontal::Left ? "skeuo-snackbar--left"
                                  : props.anchor_origin.horizontal == SnackbarHorizontal::Center
                                      ? "skeuo-snackbar--center"
                                      : "skeuo-snackbar--right";
    auto onclose = props.onclose;

    Node root = el("div").cls(
        classes({"skeuo-snackbar", vertical, horizontal, props.inline_ ? "skeuo-snackbar--inline" : "", props.cls}));
    if (onclose) {
        root.attr("data-sk-keys", "Escape").on("keydown", [onclose] { onclose(SnackbarCloseReason::EscapeKeyDown); });
    }
    if (props.message || props.action) {
        root.child(SnackbarContent({.message = props.message.value_or(""), .action = props.action}));
    } else {
        root.child(std::move(children));
    }
    if (props.auto_hide_duration) {
        // The countdown is a CSS animation (paused on hover / focus) whose end
        // closes the snackbar. The span has no children and no other
        // animation, so every animationend reaching it is the timer's.
        Node timer = el("span")
                         .cls("skeuo-snackbar__timer")
                         .attr("style", "--snackbar-hide:" + std::to_string(*props.auto_hide_duration) + "ms")
                         .attr("aria-hidden", "true");
        if (onclose) timer.on("animationend", [onclose] { onclose(SnackbarCloseReason::Timeout); });
        root.child(std::move(timer));
    }
    return root;
}

Node SnackbarContent(const SnackbarContentProps& props) {
    Node root = el("div")
                    .cls(classes({"skeuo-chassis skeuo-chassis--float skeuo-elev-3 skeuo-snackbar-content", props.cls}))
                    .attr("role", props.role)
                    .child(el("div").cls("skeuo-snackbar-content__message").child(props.message));
    if (props.action) root.child(el("div").cls("skeuo-snackbar-content__action").child(*props.action));
    return root;
}

}  // namespace skeuo
