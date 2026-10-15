#include <memory>

#include "detail.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {

/// Per-list state that survives re-renders.
struct MenuUi {
    nav::RovingFocus roving;
    /// The first item has been focused since the list appeared.
    bool started = false;
};

bool is_menu_item(const Node& n) {
    const std::string* role = n.get_attr("role");
    return n.kind() == Node::Kind::Element && role && *role == "menuitem" && n.has_class("skeuo-menu-item");
}

Node menu_list_impl(const MenuListProps& props, Node children, MenuUi& ui) {
    Node list = el("ul")
                    .cls(classes({"skeuo-menu-list", props.dense ? "skeuo-menu-list--dense" : "", props.cls}))
                    .attr("role", "menu");
    if (props.id) list.attr("id", *props.id);
    list.attr("tabindex", "-1");
    if (props.aria_labelledby) list.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_label) list.attr("aria-label", *props.aria_label);
    list.child(std::move(children));

    std::vector<Node*> items;
    nav::visit(list, [&](Node& n) {
        if (is_menu_item(n)) {
            items.push_back(&n);
            return false;
        }
        return true;
    });
    auto disabled = std::make_shared<std::vector<bool>>();
    for (size_t i = 0; i < items.size(); ++i) {
        Node& item = *items[i];
        // An item without its own `dense` follows the list.
        if (nav::has_attr(item, "data-sk-dense-auto")) {
            item.remove_attr("data-sk-dense-auto");
            if (props.dense) {
                nav::insert_class_after(item, item.has_class("skeuo-menu-item--selected") ? "skeuo-menu-item--selected"
                                                                                          : "skeuo-menu-item",
                                        "skeuo-menu-item--dense");
            }
        }
        bool off = nav::has_attr(item, "disabled");
        disabled->push_back(off);
        item.on("focus", [&ui, i] { ui.roving.current = i; });
        if (!off) {
            // Hovering moves focus too, so the keyboard continues from the
            // pointer's row, as in macOS menus.
            item.on("mouseenter", [&ui, i] {
                if (ui.roving.current == i || ui.roving.target == i) return;
                ui.roving.move_to(i);
            });
        }
    }
    list.attr("data-sk-keys", "ArrowUp ArrowDown Home End");
    list.on("keydown", [&ui, disabled](const Event& e) {
        std::optional<size_t> target;
        if (e.key == "ArrowDown") target = nav::roving_neighbor(*disabled, ui.roving.current, 1);
        else if (e.key == "ArrowUp") target = nav::roving_neighbor(*disabled, ui.roving.current, -1);
        else if (e.key == "Home") target = nav::roving_neighbor(*disabled, std::nullopt, 1);
        else if (e.key == "End") target = nav::roving_neighbor(*disabled, std::nullopt, -1);
        if (target) ui.roving.move_to(*target);
    });

    // Render the item slots from the start: switching layouts later would
    // replace the elements, possibly the one under the pointer mid-click.
    ui.roving.engaged = true;
    std::optional<size_t> mark = ui.roving.take_mark();
    if (props.auto_focus_item && !ui.started) {
        ui.started = true;
        mark = nav::roving_neighbor(*disabled, std::nullopt, 1);
        if (mark) ui.roving.marked(*mark);
    }
    if (mark && *mark < items.size()) items[*mark]->attr("data-sk-autofocus", "");
    nav::freshen(list, items, ui.roving);
    return list;
}

}  // namespace

Node Popover(const PopoverProps& props, Node children) {
    std::string placement = "skeuo-popover--" + std::string(as_str(props.placement));
    std::string surface_cls = classes({"skeuo-popover", "skeuo-chassis", "skeuo-elev-3", "skeuo-chassis--float",
                                       placement, props.inline_ ? "skeuo-popover--inline" : "", props.cls});
    auto onclose = props.onclose;

    Node root = el("div").cls(props.inline_ ? "skeuo-popover-anchor skeuo-popover-anchor--inline"
                                            : "skeuo-popover-anchor");
    if (onclose) {
        root.attr("data-sk-keys", "Escape");
        root.on(
            "keydown",
            [onclose](const Event& e) {
                if (e.key == "Escape") onclose();
            },
            StopPropagation);
    }
    if (props.anchor) root.child(*props.anchor);
    if (props.open && !props.inline_) {
        Node backdrop = el("div").cls("skeuo-popover-backdrop").attr("aria-hidden", "true");
        if (onclose) backdrop.on("click", onclose);
        root.child(std::move(backdrop));
    }
    if (props.open) {
        Node surface = el("div").cls(surface_cls);
        if (props.id) surface.attr("id", *props.id);
        surface.attr("tabindex", "-1");
        if (props.aria_label) surface.attr("aria-label", *props.aria_label);
        if (props.aria_labelledby) surface.attr("aria-labelledby", *props.aria_labelledby);
        if (props.auto_focus && !props.inline_) surface.attr("data-sk-autofocus", "");
        root.child(std::move(surface).child(std::move(children)));
    }
    return root;
}

Node Menu(const MenuProps& props, Node children) {
    // Called whether or not the menu is open, so hook positions stay stable.
    MenuUi& ui = use_state<MenuUi>("Menu.list", [] { return MenuUi{}; });
    Node body;
    if (props.open) {
        body = menu_list_impl({.dense = props.dense,
                               .auto_focus_item = !props.inline_,
                               .id = props.id,
                               .aria_labelledby = props.aria_labelledby},
                              std::move(children), ui);
    } else {
        ui = MenuUi{};
    }
    return Popover({.open = props.open,
                    .onclose = props.onclose,
                    .anchor = props.anchor,
                    .placement = props.placement,
                    .inline_ = props.inline_,
                    .auto_focus = false,
                    .cls = classes({"skeuo-menu", props.cls})},
                   std::move(body));
}

Node MenuList(const MenuListProps& props, Node children) {
    MenuUi& ui = use_state<MenuUi>("MenuList.list", [] { return MenuUi{}; });
    return menu_list_impl(props, std::move(children), ui);
}

Node MenuItem(const MenuItemProps& props, Node children) {
    Node button = el("button")
                      .cls(classes({"skeuo-menu-item", props.selected ? "skeuo-menu-item--selected" : "",
                                    props.dense.value_or(false) ? "skeuo-menu-item--dense" : "", props.cls}))
                      .attr("type", "button")
                      .attr("role", "menuitem")
                      .attr("tabindex", "-1")
                      .flag("disabled", props.disabled)
                      .attr_if(props.disabled, "aria-disabled", "true")
                      .flag("data-sk-dense-auto", !props.dense.has_value());
    if (props.onclick) button.on("click", props.onclick);
    Node check = el("span").cls("skeuo-menu-item__check").attr("aria-hidden", "true");
    if (props.selected) check.child(Icon(IconName::Check));
    button.child(std::move(check));
    if (props.icon) button.child(el("span").cls("skeuo-menu-item__icon").attr("aria-hidden", "true").child(*props.icon));
    button.child(el("span").cls("skeuo-menu-item__label").child(std::move(children)));
    if (props.shortcut) button.child(el("kbd").cls("skeuo-menu-item__shortcut").child(*props.shortcut));

    Node row = el("li").cls("skeuo-menu-item__row").attr("role", "none").child(std::move(button));
    if (props.divider) row.child(el("hr").cls("skeuo-rule skeuo-menu-item__divider").attr("role", "separator"));
    return row;
}

}  // namespace skeuo
