#include "detail.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

Node Tooltip(const TooltipProps& props, Node children) {
    using data_display_detail::flag;
    std::string generated = use_unique_id("skeuo-tooltip");
    std::string id = props.id.value_or(generated);
    bool& dismissed = use_state<bool>("Tooltip.dismissed", [] { return false; });
    bool empty = props.title.empty();

    std::string_view state = empty                ? "skeuo-tooltip--closed"
                             : props.open         ? (*props.open ? "skeuo-tooltip--open" : "skeuo-tooltip--closed")
                             : dismissed          ? "skeuo-tooltip--closed"
                                                  : "skeuo-tooltip--auto";
    std::string placement = "skeuo-tooltip--" + std::string(as_str(props.placement));
    Node root = el("span").cls(classes({
        "skeuo-tooltip",
        state,
        placement,
        flag(props.arrow, "skeuo-tooltip--arrow"),
        flag(!props.disable_interactive, "skeuo-tooltip--interactive"),
        flag(props.disable_hover_listener, "skeuo-tooltip--no-hover"),
        flag(props.disable_focus_listener, "skeuo-tooltip--no-focus"),
        props.cls,
    }));

    // Showing and hiding is pure CSS (:hover / :focus-visible). Events only
    // go to C++ when they change something: a callback is set, or the
    // tooltip was dismissed with Escape and has to come back.
    bool hover = !props.disable_hover_listener;
    bool focus = !props.disable_focus_listener;
    auto on_open = props.on_open;
    auto on_close = props.on_close;
    if (hover && (on_open || dismissed)) {
        root.on("mouseenter", [&dismissed, on_open] {
            dismissed = false;
            if (on_open) on_open();
        });
    }
    if (hover && (on_close || dismissed)) {
        root.on("mouseleave", [&dismissed, on_close] {
            dismissed = false;
            if (on_close) on_close();
        });
    }
    if (focus && on_open) root.on("focus", [on_open] { on_open(); });
    if (focus && (on_close || dismissed)) {
        root.on("blur", [&dismissed, on_close](const Event& e) {
            if (e.related_inside) return;  // focus moved within the tooltip's child
            dismissed = false;
            if (on_close) on_close();
        });
    }
    if (on_close || (!empty && !dismissed)) {
        root.attr("data-sk-keys", "Escape");
        root.on("keydown", [&dismissed, on_close] {
            dismissed = true;
            if (on_close) on_close();
        });
    }

    root.child(std::move(children));
    if (!empty) {
        Node bubble = el("span")
                          .cls("skeuo-tooltip__bubble skeuo-chassis skeuo-chassis--float skeuo-elev-3")
                          .child(props.title);
        if (props.arrow) bubble.child(el("span").cls("skeuo-tooltip__arrow").attr("aria-hidden", "true"));
        root.child(el("span").cls("skeuo-tooltip__popper").attr("role", "tooltip").attr("id", id).child(std::move(bubble)));
    }
    return root;
}

}  // namespace skeuo
