use dioxus::prelude::*;

use crate::{Color, classes};

/// What a badge shows, as in MUI's `badgeContent`. Numbers are capped by
/// `max` and hidden at zero unless `show_zero`.
#[derive(Clone, Debug, Default, PartialEq, Eq, Hash)]
pub enum BadgeContent {
    /// No content: a standard badge is hidden.
    #[default]
    Empty,
    Number(u64),
    Text(String),
}

macro_rules! from_number {
    ($($t:ty),*) => {$(
        impl From<$t> for BadgeContent {
            fn from(n: $t) -> Self {
                BadgeContent::Number(u64::try_from(n).unwrap_or(0))
            }
        }
    )*};
}
from_number!(i32, i64, u32, usize);

impl From<u64> for BadgeContent {
    fn from(n: u64) -> Self {
        BadgeContent::Number(n)
    }
}

impl From<&str> for BadgeContent {
    fn from(s: &str) -> Self {
        BadgeContent::Text(s.to_string())
    }
}

impl From<String> for BadgeContent {
    fn from(s: String) -> Self {
        BadgeContent::Text(s)
    }
}

/// Badge style, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum BadgeVariant {
    /// An LED pill carrying a count or short text.
    #[default]
    Standard,
    /// A bare status LED.
    Dot,
}

/// Vertical anchor of the badge.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum BadgeVertical {
    #[default]
    Top,
    Bottom,
}

/// Horizontal anchor of the badge.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum BadgeHorizontal {
    Left,
    #[default]
    Right,
}

/// Corner the badge sits on, as in MUI's `anchorOrigin`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct BadgeAnchorOrigin {
    pub vertical: BadgeVertical,
    pub horizontal: BadgeHorizontal,
}

impl BadgeAnchorOrigin {
    pub const TOP_RIGHT: Self = Self {
        vertical: BadgeVertical::Top,
        horizontal: BadgeHorizontal::Right,
    };
    pub const TOP_LEFT: Self = Self {
        vertical: BadgeVertical::Top,
        horizontal: BadgeHorizontal::Left,
    };
    pub const BOTTOM_RIGHT: Self = Self {
        vertical: BadgeVertical::Bottom,
        horizontal: BadgeHorizontal::Right,
    };
    pub const BOTTOM_LEFT: Self = Self {
        vertical: BadgeVertical::Bottom,
        horizontal: BadgeHorizontal::Left,
    };
}

/// Shape the badge is anchored to, as in MUI's `overlap`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum BadgeOverlap {
    #[default]
    Rectangular,
    Circular,
}

/// A small count or status light on the corner of its child. Mirrors MUI's
/// `<Badge>`.
///
/// Physically it is an indicator LED (`.skeuo-led`), like the caps-lock
/// light: a lit pill with the count printed in the contrast color, or a bare
/// dot for `variant: Dot`.
#[derive(Props, Clone, PartialEq)]
pub struct BadgeProps {
    /// A number or short text (`4`, `"new"`).
    #[props(default, into)]
    pub badge_content: BadgeContent,
    /// Numbers above `max` show as `max+`.
    #[props(default = 99)]
    pub max: u64,
    /// Show the badge when the count is 0.
    #[props(default)]
    pub show_zero: bool,
    /// Hide the badge (it shrinks away).
    #[props(default)]
    pub invisible: bool,
    #[props(default)]
    pub variant: BadgeVariant,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub anchor_origin: BadgeAnchorOrigin,
    #[props(default)]
    pub overlap: BadgeOverlap,
    #[props(default, into)]
    pub class: String,
    /// The badged element.
    pub children: Element,
}

#[component]
pub fn Badge(props: BadgeProps) -> Element {
    let dot = props.variant == BadgeVariant::Dot;
    let label = match &props.badge_content {
        BadgeContent::Number(n) if *n > props.max => Some(format!("{}+", props.max)),
        BadgeContent::Number(n) => Some(n.to_string()),
        BadgeContent::Text(t) => Some(t.clone()),
        BadgeContent::Empty => None,
    };
    let invisible = props.invisible
        || (!dot
            && match &props.badge_content {
                BadgeContent::Empty => true,
                BadgeContent::Number(0) => !props.show_zero,
                _ => false,
            });
    let vertical = match props.anchor_origin.vertical {
        BadgeVertical::Top => "top",
        BadgeVertical::Bottom => "bottom",
    };
    let horizontal = match props.anchor_origin.horizontal {
        BadgeHorizontal::Left => "left",
        BadgeHorizontal::Right => "right",
    };
    let anchor = format!("skeuo-badge__badge--{vertical}-{horizontal}");
    let badge_class = classes([
        "skeuo-led",
        "skeuo-badge__badge",
        if dot {
            "skeuo-badge__badge--dot"
        } else {
            "skeuo-badge__badge--standard"
        },
        &anchor,
        match props.overlap {
            BadgeOverlap::Rectangular => "skeuo-badge__badge--rectangular",
            BadgeOverlap::Circular => "skeuo-badge__badge--circular",
        },
        if invisible {
            "skeuo-badge__badge--invisible"
        } else {
            ""
        },
        props.color.class(),
    ]);
    let class = classes(["skeuo-badge", &props.class]);
    rsx! {
        span { class,
            {props.children}
            span { class: badge_class, aria_hidden: if invisible { "true" },
                if !dot {
                    if let Some(label) = label {
                        "{label}"
                    }
                }
            }
        }
    }
}
