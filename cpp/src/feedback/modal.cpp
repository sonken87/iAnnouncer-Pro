#include "skeuo/feedback.hpp"

namespace skeuo {

Node Modal(const ModalProps& props, Node children) {
    bool open = props.open;
    if (!open && !props.keep_mounted) return {};
    auto onclose = props.onclose;

    Node root = el("div")
                    .cls(classes({"skeuo-modal", open ? "" : "skeuo-modal--hidden", props.cls}))
                    .attr("role", "presentation")
                    .flag("hidden", !open);
    if (!props.disable_escape_key_down && onclose) {
        // Only Escape reaches the handler (data-sk-keys); like the Rust
        // version it stops there so an enclosing overlay doesn't close too.
        root.attr("data-sk-keys", "Escape")
            .on("keydown", [onclose] { onclose(ModalCloseReason::EscapeKeyDown); }, StopPropagation);
    }
    if (!props.hide_backdrop) {
        Node scrim = el("div").cls("skeuo-scrim skeuo-modal__scrim").attr("aria-hidden", "true");
        if (onclose) scrim.on("click", [onclose] { onclose(ModalCloseReason::BackdropClick); });
        root.child(std::move(scrim));
    }
    Node content = el("div").cls("skeuo-modal__content").attr("role", props.role).attr("aria-modal", "true");
    if (props.aria_labelledby) content.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_describedby) content.attr("aria-describedby", *props.aria_describedby);
    content.attr("tabindex", "-1");
    // Focus moves into the overlay when it opens, so Escape reaches it.
    if (open) content.attr("data-sk-autofocus", "");
    root.child(std::move(content).child(std::move(children)));
    return root;
}

}  // namespace skeuo
