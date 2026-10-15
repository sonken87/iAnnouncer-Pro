#include "skeuo/surfaces.hpp"

namespace skeuo {

Node Drawer(const DrawerProps& props, Node children) {
    std::string anchor(as_str(props.anchor));
    std::string size = props.size ? "--drawer-size:" + *props.size + ";" : "";
    std::string root_base = classes({
        "skeuo-drawer",
        "skeuo-drawer--" + std::string(as_str(props.variant)),
        "skeuo-drawer--" + anchor,
        props.inline_ ? "skeuo-drawer--inline" : "",
    });
    std::string paper_anchor = "skeuo-drawer__paper--" + anchor;

    if (props.variant == DrawerVariant::Temporary) {
        if (!props.open) return {};
        std::function<void()> close = props.onclose ? props.onclose : [] {};
        Node root = el("div").cls(classes({root_base, props.cls})).attr("style", size);
        // Only Escape reaches C++ (data-sk-keys); it doesn't bubble further.
        root.attr("data-sk-keys", "Escape").on("keydown", close, StopPropagation);
        if (!props.hide_backdrop)
            root.child(el("div").cls("skeuo-scrim skeuo-drawer__scrim").attr("aria-hidden", "true").on("click", close));
        Node paper = el("div")
                         .cls(classes({chassis_classes(PaperVariant::Elevation, props.elevation, false),
                                       "skeuo-drawer__paper", paper_anchor}))
                         .attr("role", "dialog")
                         // An in-flow preview doesn't block the page.
                         .attr("aria-modal", props.inline_ ? "false" : "true")
                         .attr("tabindex", "-1");
        // The page focuses the panel when it appears (Rust: onmounted + set_focus).
        if (!props.inline_) paper.attr("data-sk-autofocus", "");
        for (const auto& [k, v] : props.attrs) paper.set_attr(k, v);
        paper.child(std::move(children));
        return std::move(root).child(std::move(paper));
    }

    bool open = props.variant == DrawerVariant::Permanent || props.open;
    Node paper = el("div").cls(
        classes({chassis_classes(PaperVariant::Elevation, 0, true), "skeuo-drawer__paper", paper_anchor}));
    for (const auto& [k, v] : props.attrs) paper.set_attr(k, v);
    paper.child(std::move(children));
    return el("div")
        .cls(classes({root_base, "skeuo-drawer--docked", open ? "skeuo-drawer--open" : "skeuo-drawer--closed",
                      props.cls}))
        .attr("style", size)
        .child(std::move(paper));
}

}  // namespace skeuo
