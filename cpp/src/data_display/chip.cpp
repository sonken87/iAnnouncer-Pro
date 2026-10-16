#include "detail.hpp"
#include "skeuo/icons.hpp"

namespace skeuo {

Node Chip(const ChipProps& props) {
    using data_display_detail::flag;
    bool clickable = props.clickable.value_or(static_cast<bool>(props.onclick));
    bool filled_cap = props.variant == ChipVariant::Filled && props.color != Color::Neutral;
    Node root = el("div").cls(classes({
        "skeuo-key",
        flag(filled_cap, "skeuo-key--filled"),
        flag(clickable, "skeuo-key--pressable skeuo-key--focus-within skeuo-chip--clickable"),
        "skeuo-chip",
        props.variant == ChipVariant::Filled ? "skeuo-chip--filled" : "skeuo-chip--outlined",
        color_class(props.color),
        props.size == Size::Small ? "skeuo-chip--small" : "skeuo-chip--medium",
        flag(static_cast<bool>(props.on_delete), "skeuo-chip--deletable"),
        flag(props.disabled, "skeuo-chip--disabled"),
        props.cls,
    }));
    if (props.disabled) root.attr("aria-disabled", "true");

    if (props.avatar) {
        root.child(el("span").cls("skeuo-chip__avatar").child(*props.avatar));
    } else if (props.icon) {
        root.child(el("span").cls("skeuo-chip__icon").attr("aria-hidden", "true").child(*props.icon));
    }

    Node label = el("span").cls("skeuo-chip__label").child(props.label);
    if (clickable) {
        Node action = el("button").cls("skeuo-chip__action").attr("type", "button").flag("disabled", props.disabled);
        if (props.onclick) action.on("click", props.onclick);
        root.child(std::move(action).child(std::move(label)));
    } else {
        root.child(std::move(label));
    }

    if (props.on_delete) {
        Node del = el("button")
                       .cls("skeuo-chip__delete")
                       .attr("type", "button")
                       .attr("aria-label", "Delete " + props.label)
                       .flag("disabled", props.disabled)
                       .on("click", props.on_delete, StopPropagation);
        del.child(props.delete_icon ? *props.delete_icon : Icon(IconName::Close));
        root.child(std::move(del));
    }
    return root;
}

}  // namespace skeuo
