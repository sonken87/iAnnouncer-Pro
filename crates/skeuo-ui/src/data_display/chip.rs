use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Size, classes};

/// Chip style, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ChipVariant {
    /// A small keycap pill in the palette color.
    #[default]
    Filled,
    /// A white keycap pill with colored ink and edge.
    Outlined,
}

impl ChipVariant {
    fn as_str(self) -> &'static str {
        match self {
            ChipVariant::Filled => "filled",
            ChipVariant::Outlined => "outlined",
        }
    }
}

/// A compact token: a tag, a filter, a contact. Mirrors MUI's `<Chip>`.
///
/// Physically it is a small pill-shaped keycap. Clickable chips (`onclick`
/// or `clickable`) travel down when pressed like a key; `on_delete` adds a
/// small round delete button at the end. The press target is a native
/// `<button>`, so chips are keyboard accessible.
#[derive(Props, Clone, PartialEq)]
pub struct ChipProps {
    #[props(into)]
    pub label: String,
    #[props(default)]
    pub variant: ChipVariant,
    /// `Neutral` (MUI's `default`) is a light grey / white cap.
    #[props(default = Color::Neutral)]
    pub color: Color,
    /// small 24px · medium 32px (large is treated as medium).
    #[props(default)]
    pub size: Size,
    /// An [`Avatar`](crate::Avatar) shown at the start.
    pub avatar: Option<Element>,
    /// An icon shown at the start.
    pub icon: Option<Element>,
    /// Force the pressable look even without `onclick`.
    pub clickable: Option<bool>,
    #[props(default)]
    pub disabled: bool,
    pub onclick: Option<EventHandler<MouseEvent>>,
    /// Shows a delete button that calls this.
    pub on_delete: Option<EventHandler<MouseEvent>>,
    /// Replaces the default delete glyph.
    pub delete_icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn Chip(props: ChipProps) -> Element {
    let clickable = props.clickable.unwrap_or(props.onclick.is_some());
    let variant = format!("skeuo-chip--{}", props.variant.as_str());
    let size = match props.size {
        Size::Small => "skeuo-chip--small",
        _ => "skeuo-chip--medium",
    };
    let filled_cap = props.variant == ChipVariant::Filled && props.color != Color::Neutral;
    let class = classes([
        "skeuo-key",
        if filled_cap { "skeuo-key--filled" } else { "" },
        if clickable {
            "skeuo-key--pressable skeuo-key--focus-within skeuo-chip--clickable"
        } else {
            ""
        },
        "skeuo-chip",
        &variant,
        props.color.class(),
        size,
        if props.on_delete.is_some() {
            "skeuo-chip--deletable"
        } else {
            ""
        },
        if props.disabled {
            "skeuo-chip--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let label = props.label.clone();
    let onclick = props.onclick;
    let on_delete = props.on_delete;

    rsx! {
        div { class, aria_disabled: if props.disabled { "true" },
            if let Some(avatar) = props.avatar {
                span { class: "skeuo-chip__avatar", {avatar} }
            } else if let Some(icon) = props.icon {
                span { class: "skeuo-chip__icon", aria_hidden: "true", {icon} }
            }
            if clickable {
                button {
                    class: "skeuo-chip__action",
                    r#type: "button",
                    disabled: props.disabled,
                    onclick: move |e| {
                        if let Some(h) = &onclick {
                            h.call(e);
                        }
                    },
                    span { class: "skeuo-chip__label", "{label}" }
                }
            } else {
                span { class: "skeuo-chip__label", "{label}" }
            }
            if on_delete.is_some() {
                button {
                    class: "skeuo-chip__delete",
                    r#type: "button",
                    aria_label: "Delete {props.label}",
                    disabled: props.disabled,
                    onclick: move |e: MouseEvent| {
                        e.stop_propagation();
                        if let Some(h) = &on_delete {
                            h.call(e);
                        }
                    },
                    if let Some(icon) = props.delete_icon {
                        {icon}
                    } else {
                        Icon { name: IconName::Close }
                    }
                }
            }
        }
    }
}
