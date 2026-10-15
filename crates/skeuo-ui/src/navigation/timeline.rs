use dioxus::prelude::*;

use crate::{Color, classes};

/// Which side the content sits on (MUI lab's `position`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TimelinePosition {
    Left,
    #[default]
    Right,
    /// Content alternates sides, starting on the right.
    Alternate,
    /// Content alternates sides, starting on the left.
    AlternateReverse,
}

impl TimelinePosition {
    fn as_str(self) -> &'static str {
        match self {
            TimelinePosition::Left => "left",
            TimelinePosition::Right => "right",
            TimelinePosition::Alternate => "alternate",
            TimelinePosition::AlternateReverse => "alternate-reverse",
        }
    }
}

/// A vertical sequence of events: LEDs joined by a groove, with printed
/// content beside them. Mirrors MUI lab's `<Timeline>` (an `<ul>`).
#[derive(Props, Clone, PartialEq)]
pub struct TimelineProps {
    #[props(default)]
    pub position: TimelinePosition,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Timeline(props: TimelineProps) -> Element {
    let position = format!("skeuo-timeline--{}", props.position.as_str());
    rsx! {
        ul { class: classes(["skeuo-timeline", &position, &props.class]), {props.children} }
    }
}

/// One event of a [`Timeline`]: opposite content, separator, content.
/// `position` overrides the timeline's side for this item. Mirrors MUI
/// lab's `<TimelineItem>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineItemProps {
    pub position: Option<TimelinePosition>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TimelineItem(props: TimelineItemProps) -> Element {
    let position = props
        .position
        .map(|p| format!("skeuo-timeline-item--{}", p.as_str()))
        .unwrap_or_default();
    rsx! {
        li { class: classes(["skeuo-timeline-item", &position, &props.class]), {props.children} }
    }
}

/// The middle column of a [`TimelineItem`]: dot over connector. Mirrors MUI
/// lab's `<TimelineSeparator>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineSeparatorProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TimelineSeparator(props: TimelineSeparatorProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-timeline-separator", &props.class]), {props.children} }
    }
}

/// Whether a [`TimelineDot`]'s LED is lit (MUI's `variant`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TimelineDotVariant {
    /// Lit LED.
    #[default]
    Filled,
    /// Unlit LED with a colored rim.
    Outlined,
}

/// The LED marking an event. Without `color` it is a grey LED (MUI's
/// `grey`). It can hold a small icon. Mirrors MUI lab's `<TimelineDot>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineDotProps {
    pub color: Option<Color>,
    #[props(default)]
    pub variant: TimelineDotVariant,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TimelineDot(props: TimelineDotProps) -> Element {
    let color = props
        .color
        .map(|c| c.class())
        .unwrap_or("skeuo-timeline-dot--grey");
    let variant = match props.variant {
        TimelineDotVariant::Filled => "skeuo-led skeuo-timeline-dot--filled",
        TimelineDotVariant::Outlined => "skeuo-led skeuo-led--off skeuo-timeline-dot--outlined",
    };
    rsx! {
        span { class: classes(["skeuo-timeline-dot", variant, color, &props.class]),
            {props.children}
        }
    }
}

/// The groove joining one event's LED to the next. Mirrors MUI lab's
/// `<TimelineConnector>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineConnectorProps {
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn TimelineConnector(props: TimelineConnectorProps) -> Element {
    rsx! {
        span { class: classes(["skeuo-timeline-connector skeuo-groove", &props.class]), aria_hidden: "true" }
    }
}

/// The printed description of an event. Mirrors MUI lab's
/// `<TimelineContent>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineContentProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TimelineContent(props: TimelineContentProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-timeline-content", &props.class]), {props.children} }
    }
}

/// Printed on the other side of the separator, e.g. the time of the event.
/// Mirrors MUI lab's `<TimelineOppositeContent>`.
#[derive(Props, Clone, PartialEq)]
pub struct TimelineOppositeContentProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TimelineOppositeContent(props: TimelineOppositeContentProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-timeline-opposite skeuo-caption", &props.class]), {props.children} }
    }
}
