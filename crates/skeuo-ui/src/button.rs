use dioxus::prelude::*;

use crate::{Color, Size, classes};

/// Button style, mirroring MUI's `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ButtonVariant {
    /// Keycap in the palette color with a white legend.
    #[default]
    Contained,
    /// White keycap with a colored legend and edge.
    Outlined,
    /// Legend printed on the aluminium, no keycap.
    Text,
}

impl ButtonVariant {
    fn as_str(self) -> &'static str {
        match self {
            ButtonVariant::Contained => "contained",
            ButtonVariant::Outlined => "outlined",
            ButtonVariant::Text => "text",
        }
    }
}

/// A push button drawn as a Magic Keyboard keycap. Mirrors MUI's `<Button>`.
///
/// Any global HTML attribute (`id`, `title`, `aria-*`, `style`, …) is passed
/// through to the underlying `<button>`.
#[derive(Props, Clone, PartialEq)]
pub struct ButtonProps {
    #[props(default)]
    pub variant: ButtonVariant,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    /// Stretch to the container's width.
    #[props(default)]
    pub full_width: bool,
    /// HTML `type`: `"button"` (default), `"submit"` or `"reset"`.
    #[props(default = "button".to_string(), into)]
    pub r#type: String,
    /// Element placed before the label, e.g. an icon.
    pub start_icon: Option<Element>,
    /// Element placed after the label.
    pub end_icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Button(props: ButtonProps) -> Element {
    let variant = format!("skeuo-btn--{}", props.variant.as_str());
    let color = format!("skeuo-color-{}", props.color.as_str());
    let size = format!("skeuo-btn--{}", props.size.as_str());
    let class = classes([
        "skeuo-btn",
        &variant,
        &color,
        &size,
        if props.full_width {
            "skeuo-btn--full-width"
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
                if let Some(handler) = &props.onclick {
                    handler.call(e);
                }
            },
            ..props.attributes,
            if let Some(icon) = props.start_icon {
                span { class: "skeuo-btn__icon skeuo-btn__icon--start", aria_hidden: "true", {icon} }
            }
            span { class: "skeuo-btn__label", {props.children} }
            if let Some(icon) = props.end_icon {
                span { class: "skeuo-btn__icon skeuo-btn__icon--end", aria_hidden: "true", {icon} }
            }
        }
    }
}
