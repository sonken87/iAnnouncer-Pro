#include "skeuo/navigation.hpp"

namespace skeuo {

namespace {
std::string_view position_name(TimelinePosition p) {
    switch (p) {
        case TimelinePosition::Left: return "left";
        case TimelinePosition::Right: return "right";
        case TimelinePosition::Alternate: return "alternate";
        case TimelinePosition::AlternateReverse: return "alternate-reverse";
    }
    return "right";
}
}  // namespace

Node Timeline(const TimelineProps& props, Node children) {
    std::string position = "skeuo-timeline--" + std::string(position_name(props.position));
    return el("ul").cls(classes({"skeuo-timeline", position, props.cls})).child(std::move(children));
}

Node TimelineItem(const TimelineItemProps& props, Node children) {
    std::string position =
        props.position ? "skeuo-timeline-item--" + std::string(position_name(*props.position)) : std::string();
    return el("li").cls(classes({"skeuo-timeline-item", position, props.cls})).child(std::move(children));
}

Node TimelineSeparator(const TimelineSeparatorProps& props, Node children) {
    return el("div").cls(classes({"skeuo-timeline-separator", props.cls})).child(std::move(children));
}

Node TimelineDot(const TimelineDotProps& props, Node children) {
    std::string_view color = props.color ? color_class(*props.color) : "skeuo-timeline-dot--grey";
    std::string_view variant = props.variant == TimelineDotVariant::Filled
                                   ? "skeuo-led skeuo-timeline-dot--filled"
                                   : "skeuo-led skeuo-led--off skeuo-timeline-dot--outlined";
    return el("span").cls(classes({"skeuo-timeline-dot", variant, color, props.cls})).child(std::move(children));
}

Node TimelineConnector(const TimelineConnectorProps& props) {
    return el("span").cls(classes({"skeuo-timeline-connector skeuo-groove", props.cls})).attr("aria-hidden", "true");
}

Node TimelineContent(const TimelineContentProps& props, Node children) {
    return el("div").cls(classes({"skeuo-timeline-content", props.cls})).child(std::move(children));
}

Node TimelineOppositeContent(const TimelineOppositeContentProps& props, Node children) {
    return el("div").cls(classes({"skeuo-timeline-opposite skeuo-caption", props.cls})).child(std::move(children));
}

}  // namespace skeuo
