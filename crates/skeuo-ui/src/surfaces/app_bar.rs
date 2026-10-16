use dioxus::prelude::*;

use super::paper::{PaperVariant, chassis_classes};
use crate::{Color, classes};

/// CSS positioning of an app bar, mirroring MUI's `position` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum AppBarPosition {
    /// Glued to the top of the viewport.
    #[default]
    Fixed,
    Absolute,
    /// Scrolls with the page until it reaches the top, then sticks.
    Sticky,
    /// In normal flow.
    Static,
    Relative,
}

impl AppBarPosition {
    pub fn as_str(self) -> &'static str {
        match self {
            AppBarPosition::Fixed => "fixed",
            AppBarPosition::Absolute => "absolute",
            AppBarPosition::Sticky => "sticky",
            AppBarPosition::Static => "static",
            AppBarPosition::Relative => "relative",
        }
    }
}

/// App bar color, mirroring MUI's `color` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum AppBarColor {
    /// A matte plate in the palette color (a colored keycap top, no travel).
    #[default]
    Primary,
    Secondary,
    Success,
    Error,
    Warning,
    Info,
    /// Plain aluminium chassis (MUI `default`).
    Default,
    /// Aluminium chassis that inherits the surrounding text color.
    Inherit,
    /// No plate at all: lettering straight on whatever is behind.
    Transparent,
}

impl AppBarColor {
    pub fn as_str(self) -> &'static str {
        match self {
            AppBarColor::Primary => "primary",
            AppBarColor::Secondary => "secondary",
            AppBarColor::Success => "success",
            AppBarColor::Error => "error",
            AppBarColor::Warning => "warning",
            AppBarColor::Info => "info",
            AppBarColor::Default => "default",
            AppBarColor::Inherit => "inherit",
            AppBarColor::Transparent => "transparent",
        }
    }

    /// The palette entry of a colored plate.
    fn palette(self) -> Option<Color> {
        match self {
            AppBarColor::Primary => Some(Color::Primary),
            AppBarColor::Secondary => Some(Color::Secondary),
            AppBarColor::Success => Some(Color::Success),
            AppBarColor::Error => Some(Color::Error),
            AppBarColor::Warning => Some(Color::Warning),
            AppBarColor::Info => Some(Color::Info),
            _ => None,
        }
    }
}

impl From<Color> for AppBarColor {
    fn from(c: Color) -> Self {
        match c {
            Color::Primary => AppBarColor::Primary,
            Color::Secondary => AppBarColor::Secondary,
            Color::Success => AppBarColor::Success,
            Color::Error => AppBarColor::Error,
            Color::Warning => AppBarColor::Warning,
            Color::Info => AppBarColor::Info,
            Color::Neutral => AppBarColor::Default,
        }
    }
}

/// The bar across the top of a window. Mirrors MUI's `<AppBar>` (a
/// `<header>`).
///
/// `Default` / `Inherit` colors are a square-cornered aluminium chassis whose
/// shadow follows `elevation`; palette colors are a matte colored plate, like
/// the top of a colored keycap without travel; `Transparent` has no plate.
/// Put a [`Toolbar`] inside. Global attributes pass through.
#[derive(Props, Clone, PartialEq)]
pub struct AppBarProps {
    #[props(default)]
    pub position: AppBarPosition,
    #[props(default, into)]
    pub color: AppBarColor,
    /// MUI elevation, 0–24 (default 4 → chassis level 2).
    #[props(default = 4)]
    pub elevation: u8,
    /// Square corners (default). Set `false` for a rounded floating bar.
    #[props(default = true)]
    pub square: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn AppBar(props: AppBarProps) -> Element {
    let position = format!("skeuo-appbar--{}", props.position.as_str());
    let color_class = format!("skeuo-appbar--{}", props.color.as_str());
    let material = match props.color.palette() {
        Some(c) => classes([
            "skeuo-key skeuo-key--filled skeuo-appbar--colored",
            c.class(),
            if props.square {
                ""
            } else {
                "skeuo-appbar--rounded"
            },
        ]),
        None if props.color == AppBarColor::Transparent => String::new(),
        None => chassis_classes(PaperVariant::Elevation, props.elevation, props.square),
    };
    let class = classes([
        material.as_str(),
        "skeuo-appbar",
        &position,
        &color_class,
        &props.class,
    ]);
    rsx! {
        header { class, ..props.attributes, {props.children} }
    }
}

/// Toolbar density, mirroring MUI's `variant` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ToolbarVariant {
    /// 56px tall (64px on wide screens).
    #[default]
    Regular,
    /// 48px tall.
    Dense,
}

impl ToolbarVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            ToolbarVariant::Regular => "regular",
            ToolbarVariant::Dense => "dense",
        }
    }
}

/// A horizontal row of controls and lettering inside an app bar (or any
/// surface). Mirrors MUI's `<Toolbar>`; it has no material of its own.
#[derive(Props, Clone, PartialEq)]
pub struct ToolbarProps {
    #[props(default)]
    pub variant: ToolbarVariant,
    /// Remove the side padding.
    #[props(default)]
    pub disable_gutters: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Toolbar(props: ToolbarProps) -> Element {
    let variant = format!("skeuo-toolbar--{}", props.variant.as_str());
    let class = classes([
        "skeuo-toolbar",
        &variant,
        if props.disable_gutters {
            ""
        } else {
            "skeuo-toolbar--gutters"
        },
        &props.class,
    ]);
    rsx! {
        div { class, ..props.attributes, {props.children} }
    }
}
