#include "detail.hpp"

namespace skeuo {

Node Badge(const BadgeProps& props, Node children) {
    using Kind = BadgeContent::Kind;
    const BadgeContent& content = props.badge_content;
    bool dot = props.variant == BadgeVariant::Dot;

    std::optional<std::string> label;
    if (content.kind == Kind::Number)
        label = content.number > props.max ? std::to_string(props.max) + "+" : std::to_string(content.number);
    else if (content.kind == Kind::Text)
        label = content.text;

    bool invisible = props.invisible ||
                     (!dot && (content.kind == Kind::Empty ||
                               (content.kind == Kind::Number && content.number == 0 && !props.show_zero)));
    std::string anchor = std::string("skeuo-badge__badge--") +
                         (props.anchor_origin.vertical == BadgeVertical::Top ? "top" : "bottom") + "-" +
                         (props.anchor_origin.horizontal == BadgeHorizontal::Left ? "left" : "right");

    Node badge = el("span").cls(classes({
        "skeuo-led",
        "skeuo-badge__badge",
        dot ? "skeuo-badge__badge--dot" : "skeuo-badge__badge--standard",
        anchor,
        props.overlap == BadgeOverlap::Rectangular ? "skeuo-badge__badge--rectangular"
                                                   : "skeuo-badge__badge--circular",
        invisible ? "skeuo-badge__badge--invisible" : "",
        color_class(props.color),
    }));
    if (invisible) badge.attr("aria-hidden", "true");
    if (!dot && label) badge.child(*label);

    return el("span").cls(classes({"skeuo-badge", props.cls})).child(std::move(children)).child(std::move(badge));
}

}  // namespace skeuo
