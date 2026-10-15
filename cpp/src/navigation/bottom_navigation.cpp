#include <memory>

#include "detail.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

Node BottomNavigation(const BottomNavigationProps& props, Node children) {
    auto& internal =
        use_state<std::optional<std::string>>("BottomNavigation.internal", [&] { return props.default_value; });
    const std::optional<std::string> value = props.value ? props.value : internal;
    const bool controlled = props.value.has_value();

    Node nav_el = el("nav")
                      .cls(classes({"skeuo-bottom-nav skeuo-chassis skeuo-elev-1", color_class(props.color), props.cls}))
                      .attr("aria-label", props.aria_label);
    nav_el.child(std::move(children));

    auto onchange = props.onchange;
    nav::visit(nav_el, [&](Node& n) {
        const std::string* v = n.get_attr("data-sk-bn");
        if (!v) return true;
        std::string action_value = *v;
        bool selected = value == action_value;
        std::optional<bool> own = std::nullopt;
        if (const std::string* s = n.get_attr("data-sk-bn-show")) own = *s == "true";
        bool show_label = own.value_or(props.show_labels) || selected;

        n.set_attr("class", classes({"skeuo-key skeuo-key--pressable skeuo-bottom-nav__action",
                                     selected ? "skeuo-key--down skeuo-bottom-nav__action--selected" : "",
                                     show_label ? "" : "skeuo-bottom-nav__action--icon-only",
                                     nav::attr_or(n, "class")}));
        if (selected) n.set_attr("aria-current", "page");
        else n.remove_attr("aria-current");
        const std::string* label = n.get_attr("data-sk-bn-label");
        if (!show_label && label) n.set_attr("aria-label", *label);
        else n.remove_attr("aria-label");
        n.remove_attr("data-sk-bn");
        n.remove_attr("data-sk-bn-show");
        n.remove_attr("data-sk-bn-label");
        if (!show_label) {
            auto& kids = n.kids();
            std::erase_if(kids, [](const Node& k) { return k.has_class("skeuo-bottom-nav__label"); });
        }
        nav::prepend_listener(n, "click", [&internal, controlled, onchange, value, action_value](const Event&) {
            if (value == action_value) return;
            if (!controlled) internal = action_value;
            if (onchange) onchange(action_value);
        });
        return false;
    });
    return nav_el;
}

Node BottomNavigationAction(const BottomNavigationActionProps& props) {
    // Placeholders (class, aria-current, aria-label) keep the attribute
    // order when BottomNavigation fills them in.
    Node button = el("button")
                      .attr("class", props.cls)
                      .attr("type", "button")
                      .attr("aria-current", "")
                      .attr("aria-label", "")
                      .flag("disabled", props.disabled)
                      .attr("data-sk-bn", props.value);
    if (props.label) button.attr("data-sk-bn-label", *props.label);
    if (props.show_label) button.attr("data-sk-bn-show", *props.show_label ? "true" : "false");
    if (props.onclick) button.on("click", props.onclick);
    if (props.icon) button.child(el("span").cls("skeuo-bottom-nav__icon").attr("aria-hidden", "true").child(*props.icon));
    if (props.label) button.child(el("span").cls("skeuo-bottom-nav__label").child(*props.label));
    return button;
}

}  // namespace skeuo
