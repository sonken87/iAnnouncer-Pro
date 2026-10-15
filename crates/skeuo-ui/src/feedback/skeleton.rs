use dioxus::prelude::*;

use crate::classes;

/// Shape of a [`Skeleton`], as in MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SkeletonVariant {
    /// A line of text: the font's height, slightly squashed.
    #[default]
    Text,
    Rectangular,
    /// Rectangular with the well's corner radius.
    Rounded,
    Circular,
}

impl SkeletonVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            SkeletonVariant::Text => "text",
            SkeletonVariant::Rectangular => "rectangular",
            SkeletonVariant::Rounded => "rounded",
            SkeletonVariant::Circular => "circular",
        }
    }
}

/// Loading animation of a [`Skeleton`], as in MUI's `animation`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SkeletonAnimation {
    /// The well slowly fades in and out.
    #[default]
    Pulse,
    /// A soft sheen sweeps across the well.
    Wave,
    /// Static (MUI `animation={false}`).
    False,
}

impl SkeletonAnimation {
    pub fn as_str(self) -> &'static str {
        match self {
            SkeletonAnimation::Pulse => "pulse",
            SkeletonAnimation::Wave => "wave",
            SkeletonAnimation::False => "false",
        }
    }
}

/// A placeholder shown while content loads. Mirrors MUI's `<Skeleton>`.
///
/// Physically: a shallow, borderless well pressed into the aluminium where
/// the content will sit, catching a soft sheen. Children (if any) are
/// hidden but give the skeleton its size, as in MUI.
#[derive(Props, Clone, PartialEq)]
pub struct SkeletonProps {
    #[props(default)]
    pub variant: SkeletonVariant,
    #[props(default)]
    pub animation: SkeletonAnimation,
    /// CSS width, e.g. `"120px"` or `"60%"`. Text fills the line by default.
    #[props(into)]
    pub width: Option<String>,
    /// CSS height. Text uses the font size by default.
    #[props(into)]
    pub height: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Skeleton(props: SkeletonProps) -> Element {
    let variant = format!("skeuo-skeleton--{}", props.variant.as_str());
    let animation = format!("skeuo-skeleton--{}", props.animation.as_str());
    let class = classes([
        "skeuo-well skeuo-well--shallow skeuo-skeleton",
        &variant,
        &animation,
        &props.class,
    ]);
    let mut style = String::new();
    if let Some(w) = &props.width {
        style.push_str(&format!("width:{w};"));
    }
    if let Some(h) = &props.height {
        style.push_str(&format!("height:{h};"));
    }

    rsx! {
        span { class, style, "aria-hidden": "true", {props.children} }
    }
}
