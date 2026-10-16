#include "detail.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {
std::string_view direction_name(SpeedDialDirection d) {
    switch (d) {
        case SpeedDialDirection::Up: return "up";
        case SpeedDialDirection::Down: return "down";
        case SpeedDialDirection::Left: return "left";
        case SpeedDialDirection::Right: return "right";
    }
    return "up";
}
}  // namespace

Node SpeedDial(const SpeedDialProps& props, Node children) {
    std::string id = use_unique_id("skeuo-speed-dial");
    std::string actions_id = id + "-actions";
    bool& internal = use_state<bool>("SpeedDial.open", [] { return false; });
    const bool controlled = props.open.has_value();
    const bool open = props.open.value_or(internal);

    auto set_open = [&internal, controlled, onopen = props.onopen, onclose = props.onclose](bool v) {
        if (!controlled) internal = v;
        const auto& h = v ? onopen : onclose;
        if (h) h();
    };
    std::string direction = "skeuo-speed-dial--" + std::string(direction_name(props.direction));

    Node fab = el("button")
                   .cls("skeuo-speed-dial__fab skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--round")
                   .attr("type", "button")
                   .attr("id", id)
                   .attr("aria-label", props.aria_label)
                   .attr("aria-haspopup", "true")
                   .attr("aria-expanded", open ? "true" : "false")
                   .attr("aria-controls", actions_id)
                   .on("click", [set_open, open] { set_open(!open); })
                   .child(props.icon ? *props.icon : SpeedDialIcon());
    // The icon in the key turns as the dial opens.
    if (open) {
        nav::visit(fab, [](Node& n) {
            if (n.has_class("skeuo-speed-dial-icon")) {
                nav::insert_class_after(n, "skeuo-speed-dial-icon", "skeuo-speed-dial-icon--open");
                return false;
            }
            return true;
        });
    }

    bool vertical = props.direction == SpeedDialDirection::Up || props.direction == SpeedDialDirection::Down;
    Node actions = el("div")
                       .cls("skeuo-speed-dial__actions")
                       .attr("id", actions_id)
                       .attr("role", "menu")
                       .attr("aria-orientation", vertical ? "vertical" : "horizontal")
                       .attr("aria-labelledby", id)
                       .child(std::move(children));
    // Closed actions leave the tab order.
    nav::visit(actions, [open](Node& n) {
        if (n.has_class("skeuo-speed-dial-action__key")) {
            n.set_attr("tabindex", open ? "0" : "-1");
            return false;
        }
        return true;
    });

    return el("div")
        .cls(classes({"skeuo-speed-dial", direction, open ? "skeuo-speed-dial--open" : "",
                      props.hidden ? "skeuo-speed-dial--hidden" : "", color_class(props.color), props.cls}))
        .attr("role", "presentation")
        .attr("data-sk-keys", "Escape")
        .on("keydown",
            [set_open, open](const Event& e) {
                if (e.key == "Escape" && open) set_open(false);
            })
        .child(std::move(fab))
        .child(std::move(actions));
}

Node SpeedDialIcon(const SpeedDialIconProps& props) {
    Node icon = el("span")
                    .cls(classes({"skeuo-speed-dial-icon", props.open_icon ? "skeuo-speed-dial-icon--swap" : "",
                                  props.cls}))
                    .attr("aria-hidden", "true")
                    .child(el("span").cls("skeuo-speed-dial-icon__icon").child(props.icon ? *props.icon
                                                                                          : Icon(IconName::Add)));
    if (props.open_icon) icon.child(el("span").cls("skeuo-speed-dial-icon__open-icon").child(*props.open_icon));
    return icon;
}

Node SpeedDialAction(const SpeedDialActionProps& props) {
    Node key = el("button")
                   .cls("skeuo-speed-dial-action__key skeuo-key skeuo-key--pressable skeuo-key--round")
                   .attr("type", "button")
                   .attr("role", "menuitem")
                   .attr("aria-label", props.tooltip_title)
                   .attr("tabindex", "0")
                   .child(props.icon);
    if (props.onclick) key.on("click", props.onclick);
    return el("span")
        .cls(classes({"skeuo-speed-dial-action", props.tooltip_open ? "skeuo-speed-dial-action--tooltip-open" : "",
                      props.cls}))
        .child(std::move(key))
        .child(el("span")
                   .cls("skeuo-speed-dial-action__tooltip skeuo-chassis skeuo-elev-3 skeuo-chassis--float")
                   .attr("aria-hidden", "true")
                   .child(props.tooltip_title));
}

}  // namespace skeuo
