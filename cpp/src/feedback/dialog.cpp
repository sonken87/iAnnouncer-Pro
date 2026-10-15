#include "skeuo/feedback.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

Node Dialog(const DialogProps& props, Node children) {
    std::string generated = use_unique_id("skeuo-dialog-title");
    std::string title_id = props.aria_labelledby.value_or(generated);
    if (!props.open) return {};
    bool inline_ = props.inline_;
    auto onclose = props.onclose;

    // C++ has no context: give the DialogTitle(s) without an id of their own
    // the id the dialog is labelled by.
    children.walk([&](Node& n) {
        if (n.kind() == Node::Kind::Element && n.has_class("skeuo-dialog__title") && !n.get_attr("id"))
            n.set_attr("id", title_id);
    });

    Node root = el("div")
                    .cls(classes({"skeuo-dialog", "skeuo-dialog--max-" + std::string(as_str(props.max_width)),
                                  "skeuo-dialog--scroll-" + std::string(as_str(props.scroll)),
                                  props.full_width ? "skeuo-dialog--full-width" : "",
                                  props.full_screen ? "skeuo-dialog--full-screen" : "",
                                  inline_ ? "skeuo-dialog--inline" : "", props.cls}))
                    .attr("role", "presentation");
    if (!props.disable_escape_key_down && onclose) {
        root.attr("data-sk-keys", "Escape")
            .on("keydown", [onclose] { onclose(ModalCloseReason::EscapeKeyDown); }, StopPropagation);
    }
    if (!inline_) root.child(el("div").cls("skeuo-scrim skeuo-dialog__scrim").attr("aria-hidden", "true"));

    Node paper = el("div")
                     .cls(classes({"skeuo-chassis skeuo-chassis--float skeuo-elev-4 skeuo-dialog__paper",
                                   props.full_screen ? "skeuo-chassis--square" : ""}))
                     .attr("role", "dialog")
                     .attr("aria-modal", inline_ ? "false" : "true")
                     .attr("aria-labelledby", title_id);
    if (props.aria_describedby) paper.attr("aria-describedby", *props.aria_describedby);
    paper.attr("tabindex", "-1");
    if (!inline_) paper.attr("data-sk-autofocus", "");

    Node container = el("div").cls("skeuo-dialog__container");
    if (!inline_ && onclose) {
        // Clicks that reach the container (and not the paper) are on the
        // scrim around the dialog.
        container.on("click", [onclose] { onclose(ModalCloseReason::BackdropClick); });
        paper.on("click", [] {}, StopPropagation);
    }
    container.child(std::move(paper).child(std::move(children)));
    root.child(std::move(container));
    return root;
}

Node DialogTitle(const DialogTitleProps& props, Node children) {
    Node h = el("h2").cls(classes({"skeuo-dialog__title", props.cls}));
    if (props.id) h.attr("id", *props.id);
    return std::move(h).child(std::move(children));
}

Node DialogContent(const DialogContentProps& props, Node children) {
    return el("div")
        .cls(classes({"skeuo-dialog__content", props.dividers ? "skeuo-dialog__content--dividers" : "", props.cls}))
        .child(std::move(children));
}

Node DialogContentText(const DialogContentTextProps& props, Node children) {
    Node p = el("p").cls(classes({"skeuo-dialog__text", props.cls}));
    if (props.id) p.attr("id", *props.id);
    return std::move(p).child(std::move(children));
}

Node DialogActions(const DialogActionsProps& props, Node children) {
    return el("div")
        .cls(classes({"skeuo-dialog__actions", props.disable_spacing ? "" : "skeuo-dialog__actions--spacing",
                      props.cls}))
        .child(std::move(children));
}

}  // namespace skeuo
