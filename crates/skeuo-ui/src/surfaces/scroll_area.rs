use dioxus::prelude::*;

use crate::classes;

/// Which way a [`ScrollArea`] scrolls.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ScrollDirection {
    #[default]
    Vertical,
    Horizontal,
    Both,
}

impl ScrollDirection {
    fn as_str(self) -> &'static str {
        match self {
            ScrollDirection::Vertical => "vertical",
            ScrollDirection::Horizontal => "horizontal",
            ScrollDirection::Both => "both",
        }
    }
}

/// What a [`ScrollArea`] is made of.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ScrollAreaVariant {
    /// No material: the content scrolls in place on the surrounding surface.
    #[default]
    Plain,
    /// A well cut into the aluminium; content slides under its lip, with a
    /// soft shade at the edge where more content is hidden.
    Well,
}

/// A region with its own scrollbar (a groove with a keycap thumb). Not part
/// of MUI, which leaves scrolling to plain `overflow: auto`; use it for lists,
/// logs and panels that scroll independently of the page.
///
/// Every scrollbar inside `ThemeProvider` is styled the same way; this
/// component adds the sizing, the well and the edge shades.
#[derive(Props, Clone, PartialEq)]
pub struct ScrollAreaProps {
    #[props(default)]
    pub direction: ScrollDirection,
    #[props(default)]
    pub variant: ScrollAreaVariant,
    /// CSS height, e.g. `"240px"` (fixed height).
    #[props(into)]
    pub height: Option<String>,
    /// CSS max-height, e.g. `"40vh"` (grows with content up to this).
    #[props(into)]
    pub max_height: Option<String>,
    /// CSS max-width, for horizontal scrolling.
    #[props(into)]
    pub max_width: Option<String>,
    /// Accessible name; makes the region keyboard-focusable so it can be
    /// scrolled with the arrow keys.
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn ScrollArea(props: ScrollAreaProps) -> Element {
    let direction = format!("skeuo-scroll-area--{}", props.direction.as_str());
    let class = classes([
        "skeuo-scroll-area",
        &direction,
        match props.variant {
            ScrollAreaVariant::Plain => "",
            ScrollAreaVariant::Well => "skeuo-scroll-area--well skeuo-well",
        },
        &props.class,
    ]);
    let mut style = String::new();
    if let Some(h) = &props.height {
        style.push_str(&format!("height:{h};"));
    }
    if let Some(h) = &props.max_height {
        style.push_str(&format!("max-height:{h};"));
    }
    if let Some(w) = &props.max_width {
        style.push_str(&format!("max-width:{w};"));
    }
    style.push_str(&props.style);
    let focusable = props.aria_label.is_some();
    rsx! {
        div {
            class,
            style,
            role: if focusable { "region" },
            aria_label: props.aria_label,
            tabindex: if focusable { "0" },
            {props.children}
        }
    }
}
