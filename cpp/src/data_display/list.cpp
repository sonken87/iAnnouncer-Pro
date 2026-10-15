#include "detail.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

using data_display_detail::flag;

Node List(const ListProps& props, Node children) {
    if (props.dense) {
        // Rust: ListContext { dense }. Rows of nested lists belong to those.
        data_display_detail::walk_children(
            children, [](const Node& n) { return n.has_class("skeuo-list"); },
            [](Node& n) {
                if (n.has_class("skeuo-list-item") || n.has_class("skeuo-list-item-button"))
                    data_display_detail::insert_class_after_first(n, "skeuo-list-item--dense");
            });
        // `children` may itself be a single row rather than a fragment.
        if (children.has_class("skeuo-list-item") || children.has_class("skeuo-list-item-button"))
            data_display_detail::insert_class_after_first(children, "skeuo-list-item--dense");
    }
    Node ul = el("ul").cls(classes({
        "skeuo-list",
        color_class(props.color),
        flag(props.dense, "skeuo-list--dense"),
        flag(props.disable_padding, "skeuo-list--no-padding"),
        flag(props.subheader.has_value(), "skeuo-list--with-subheader"),
        props.cls,
    }));
    if (!props.style.empty()) ul.attr("style", props.style);
    if (props.aria_label) ul.attr("aria-label", *props.aria_label);
    if (props.subheader) ul.child(*props.subheader);
    return std::move(ul).child(std::move(children));
}

Node ListItem(const ListItemProps& props, Node children) {
    Node li = el("li").cls(classes({
        "skeuo-list-item",
        flag(props.dense, "skeuo-list-item--dense"),
        flag(props.disable_gutters, "skeuo-list-item--no-gutters"),
        flag(props.disable_padding, "skeuo-list-item--no-padding"),
        flag(props.divider, "skeuo-list-item--divider"),
        flag(props.align_items_start, "skeuo-list-item--align-start"),
        flag(props.secondary_action.has_value(), "skeuo-list-item--with-action"),
        props.cls,
    }));
    li.child(std::move(children));
    if (props.secondary_action)
        li.child(el("span").cls("skeuo-list-item__secondary-action").child(*props.secondary_action));
    return li;
}

Node ListItemButton(const ListItemButtonProps& props, Node children) {
    Node b = el("button")
                 .cls(classes({
                     "skeuo-list-item-button",
                     flag(props.dense, "skeuo-list-item--dense"),
                     flag(props.disable_gutters, "skeuo-list-item--no-gutters"),
                     flag(props.divider, "skeuo-list-item--divider"),
                     flag(props.selected, "skeuo-list-item-button--selected"),
                     props.cls,
                 }))
                 .attr("type", "button")
                 .flag("disabled", props.disabled)
                 .flag("autofocus", props.autofocus);
    if (props.selected) b.attr("aria-current", "true");
    if (props.autofocus) b.attr("data-sk-autofocus", "");
    if (props.onclick) b.on("click", props.onclick);
    return std::move(b).child(std::move(children));
}

Node ListItemIcon(const ListItemIconProps& props, Node children) {
    return el("span").cls(classes({"skeuo-list-item-icon", props.cls})).child(std::move(children));
}

Node ListItemAvatar(const ListItemAvatarProps& props, Node children) {
    return el("span").cls(classes({"skeuo-list-item-avatar", props.cls})).child(std::move(children));
}

Node ListItemText(const ListItemTextProps& props, Node children) {
    Node root = el("span").cls(classes({
        "skeuo-list-item-text",
        flag(props.inset, "skeuo-list-item-text--inset"),
        flag(props.secondary.has_value(), "skeuo-list-item-text--multiline"),
        props.cls,
    }));
    bool has_children = !(children.empty() || (children.kind() == Node::Kind::Text && children.content().empty()));
    if (has_children)
        root.child(el("span").cls("skeuo-list-item-text__primary").child(std::move(children)));
    else if (props.primary)
        root.child(el("span").cls("skeuo-list-item-text__primary").child(*props.primary));
    if (props.secondary)
        root.child(el("span").cls("skeuo-list-item-text__secondary skeuo-caption").child(*props.secondary));
    return root;
}

Node ListSubheader(const ListSubheaderProps& props, Node children) {
    Node li = el("li").cls(classes({
        "skeuo-list-subheader",
        "skeuo-label",
        props.color ? color_class(*props.color) : std::string_view(),
        flag(props.color.has_value(), "skeuo-list-subheader--colored"),
        flag(!props.disable_sticky, "skeuo-list-subheader--sticky"),
        flag(props.disable_gutters, "skeuo-list-subheader--no-gutters"),
        flag(props.inset, "skeuo-list-subheader--inset"),
        props.cls,
    }));
    if (props.id) li.attr("id", *props.id);
    return std::move(li).child(std::move(children));
}

}  // namespace skeuo
