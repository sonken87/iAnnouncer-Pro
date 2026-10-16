use dioxus::prelude::*;

use crate::{Color, Size, classes};

/// Look of an [`IconButton`].
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum IconButtonVariant {
    /// The icon printed on the aluminium; a round wash appears on hover and
    /// press. MUI's default (borderless) icon button.
    #[default]
    Text,
    /// A small white keycap holding the icon in the palette ink.
    Outlined,
    /// A small keycap in the palette color with a contrast icon.
    Contained,
}

impl IconButtonVariant {
    fn as_str(self) -> &'static str {
        match self {
            IconButtonVariant::Text => "text",
            IconButtonVariant::Outlined => "outlined",
            IconButtonVariant::Contained => "contained",
        }
    }
}

/// Which side of its container an [`IconButton`] hugs (MUI `edge`): a
/// negative margin aligns the icon, not the hover wash, with the content.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum IconButtonEdge {
    Start,
    End,
}

/// A button holding a single icon. Mirrors MUI's `<IconButton>`.
///
/// `variant: Text` (default) prints the icon on the aluminium like MUI's
/// borderless icon button; `Outlined` / `Contained` put it on a small square
/// keycap (`round: true` for a round one). Give it an `aria_label`.
#[derive(Props, Clone, PartialEq)]
pub struct IconButtonProps {
    #[props(default)]
    pub variant: IconButtonVariant,
    /// MUI's `default` color is `Color::Neutral`.
    #[props(default = Color::Neutral)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    pub edge: Option<IconButtonEdge>,
    /// Round keycap instead of a square one (keycap variants only).
    #[props(default)]
    pub round: bool,
    #[props(default)]
    pub disabled: bool,
    #[props(default = "button".to_string(), into)]
    pub r#type: String,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn IconButton(props: IconButtonProps) -> Element {
    let material = match props.variant {
        IconButtonVariant::Text => "",
        IconButtonVariant::Outlined => "skeuo-key skeuo-key--pressable",
        IconButtonVariant::Contained => "skeuo-key skeuo-key--pressable skeuo-key--filled",
    };
    let variant = format!("skeuo-icon-btn--{}", props.variant.as_str());
    let size = format!("skeuo-icon-btn--{}", props.size.as_str());
    let class = classes([
        material,
        "skeuo-icon-btn",
        &variant,
        props.color.class(),
        &size,
        match props.edge {
            Some(IconButtonEdge::Start) => "skeuo-icon-btn--edge-start",
            Some(IconButtonEdge::End) => "skeuo-icon-btn--edge-end",
            None => "",
        },
        if props.round && props.variant != IconButtonVariant::Text {
            "skeuo-key--round"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        button {
            class,
            r#type: props.r#type,
            disabled: props.disabled,
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            ..props.attributes,
            {props.children}
        }
    }
}

/// Shape of a [`Fab`], as in MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum FabVariant {
    /// A round key holding an icon.
    #[default]
    Circular,
    /// A pill-shaped key holding an icon and a legend.
    Extended,
}

impl FabVariant {
    fn as_str(self) -> &'static str {
        match self {
            FabVariant::Circular => "circular",
            FabVariant::Extended => "extended",
        }
    }
}

/// A floating action button: a large round keycap in the palette color,
/// with a deeper travel than ordinary keys. Mirrors MUI's `<Fab>`.
///
/// It is not positioned; place it yourself (e.g. `style: "position: fixed;
/// right: 24px; bottom: 24px"`). Give icon-only FABs an `aria_label`.
#[derive(Props, Clone, PartialEq)]
pub struct FabProps {
    #[props(default)]
    pub variant: FabVariant,
    /// MUI's `default` color is `Color::Neutral` (a white key).
    #[props(default = Color::Neutral)]
    pub color: Color,
    /// small 40px · medium 48px · large 56px, as in MUI.
    #[props(default = Size::Large)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    #[props(default = "button".to_string(), into)]
    pub r#type: String,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Fab(props: FabProps) -> Element {
    let variant = format!("skeuo-fab--{}", props.variant.as_str());
    let size = format!("skeuo-fab--{}", props.size.as_str());
    let class = classes([
        "skeuo-key skeuo-key--pressable skeuo-key--round",
        if props.color == Color::Neutral {
            ""
        } else {
            "skeuo-key--filled"
        },
        "skeuo-fab",
        &variant,
        props.color.class(),
        &size,
        &props.class,
    ]);
    rsx! {
        button {
            class,
            r#type: props.r#type,
            disabled: props.disabled,
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            ..props.attributes,
            {props.children}
        }
    }
}
