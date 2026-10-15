use dioxus::prelude::*;

use super::is_empty;
use crate::{Orientation, classes};

/// How far the divider runs, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DividerVariant {
    /// Edge to edge.
    #[default]
    FullWidth,
    /// Indented at the start (to line up with list text after an icon/avatar).
    Inset,
    /// Indented on both sides.
    Middle,
}

impl DividerVariant {
    fn as_str(self) -> &'static str {
        match self {
            DividerVariant::FullWidth => "full-width",
            DividerVariant::Inset => "inset",
            DividerVariant::Middle => "middle",
        }
    }
}

/// Where the text of a divider with children sits.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DividerTextAlign {
    Left,
    #[default]
    Center,
    Right,
}

impl DividerTextAlign {
    fn as_str(self) -> &'static str {
        match self {
            DividerTextAlign::Left => "left",
            DividerTextAlign::Center => "center",
            DividerTextAlign::Right => "right",
        }
    }
}

/// A separator. Mirrors MUI's `<Divider>`.
///
/// Physically it is the engraved hairline (`.skeuo-rule`) cut into the
/// aluminium: a dark cut with a catch-light under it. With children the cut
/// is interrupted by printed lettering.
#[derive(Props, Clone, PartialEq)]
pub struct DividerProps {
    #[props(default)]
    pub orientation: Orientation,
    #[props(default)]
    pub variant: DividerVariant,
    /// Vertical divider inside a flex container: stretch to its height.
    #[props(default)]
    pub flex_item: bool,
    #[props(default)]
    pub text_align: DividerTextAlign,
    #[props(default, into)]
    pub class: String,
    /// Optional text in the middle of the rule.
    pub children: Element,
}

#[component]
pub fn Divider(props: DividerProps) -> Element {
    let vertical = props.orientation == Orientation::Vertical;
    let has_text = !is_empty(&props.children);
    let variant = format!("skeuo-divider--{}", props.variant.as_str());
    let orientation = format!("skeuo-divider--{}", props.orientation.as_str());
    let align = format!("skeuo-divider--text-{}", props.text_align.as_str());
    let base = [
        "skeuo-divider",
        variant.as_str(),
        orientation.as_str(),
        if props.flex_item {
            "skeuo-divider--flex-item"
        } else {
            ""
        },
    ];
    let aria_orientation = if vertical { Some("vertical") } else { None };

    if has_text {
        let class = classes(base.into_iter().chain([
            "skeuo-divider--with-children",
            align.as_str(),
            props.class.as_str(),
        ]));
        let line = if vertical {
            "skeuo-rule skeuo-rule--vertical skeuo-divider__line"
        } else {
            "skeuo-rule skeuo-divider__line"
        };
        return rsx! {
            div { class, role: "separator", aria_orientation,
                span { class: line }
                span { class: "skeuo-divider__text skeuo-label", {props.children} }
                span { class: line }
            }
        };
    }

    let material = if vertical {
        "skeuo-rule skeuo-rule--vertical"
    } else {
        "skeuo-rule"
    };
    let class = classes(
        [material]
            .into_iter()
            .chain(base)
            .chain([props.class.as_str()]),
    );
    rsx! {
        hr { class, aria_orientation }
    }
}
