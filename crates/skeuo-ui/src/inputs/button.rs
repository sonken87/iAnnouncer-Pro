use dioxus::prelude::*;

use crate::{Color, Size, classes};

use super::button_group::ButtonGroupState;

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
    pub(crate) fn as_str(self) -> &'static str {
        match self {
            ButtonVariant::Contained => "contained",
            ButtonVariant::Outlined => "outlined",
            ButtonVariant::Text => "text",
        }
    }
}

/// Where the loading spinner sits, as in MUI's `loadingPosition`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum LoadingPosition {
    /// Replaces the start icon.
    Start,
    /// Replaces the end icon.
    End,
    /// Covers the legend (the label stays in place, invisible, so the
    /// keycap keeps its width).
    #[default]
    Center,
}

impl LoadingPosition {
    fn as_str(self) -> &'static str {
        match self {
            LoadingPosition::Start => "start",
            LoadingPosition::End => "end",
            LoadingPosition::Center => "center",
        }
    }
}

/// A push button drawn as a Magic Keyboard keycap. Mirrors MUI's `<Button>`,
/// including the `loading` props of MUI Lab's `<LoadingButton>` (a small
/// spinner printed on the cap; the key is disabled while loading).
///
/// Any global HTML attribute (`id`, `title`, `aria-*`, `style`, …) is passed
/// through to the underlying `<button>`. Inside a
/// [`ButtonGroup`](super::ButtonGroup) the group's `variant`, `color` and
/// `size` apply, and the button is disabled when either it or the group is.
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
    /// Shows a small spinner legend and disables the key.
    #[props(default)]
    pub loading: bool,
    /// Where the spinner goes while `loading`.
    #[props(default)]
    pub loading_position: LoadingPosition,
    /// Replaces the default spinner.
    pub loading_indicator: Option<Element>,
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
    let group = try_use_context::<Signal<ButtonGroupState>>().map(|g| g());
    let (variant, color, size, disabled, full_width) = match &group {
        Some(g) => (
            g.variant,
            g.color,
            g.size,
            props.disabled || g.disabled,
            props.full_width || g.full_width,
        ),
        None => (
            props.variant,
            props.color,
            props.size,
            props.disabled,
            props.full_width,
        ),
    };
    let loading = props.loading;
    let position = props.loading_position;

    let variant_class = format!("skeuo-btn--{}", variant.as_str());
    let size_class = format!("skeuo-btn--{}", size.as_str());
    let loading_class = if loading {
        format!(
            "skeuo-btn--loading skeuo-btn--loading-{}",
            position.as_str()
        )
    } else {
        String::new()
    };
    let material = match variant {
        ButtonVariant::Contained => "skeuo-key skeuo-key--pressable skeuo-key--filled",
        ButtonVariant::Outlined => "skeuo-key skeuo-key--pressable",
        ButtonVariant::Text => "",
    };
    let class = classes([
        material,
        "skeuo-btn",
        &variant_class,
        color.class(),
        &size_class,
        if full_width {
            "skeuo-btn--full-width"
        } else {
            ""
        },
        &loading_class,
        &props.class,
    ]);

    let custom_indicator = props.loading_indicator.clone();
    let indicator = move || match custom_indicator.clone() {
        Some(el) => el,
        None => rsx! {
            span { class: "skeuo-spinner", role: "progressbar", aria_label: "Loading" }
        },
    };
    let show_start = loading && position == LoadingPosition::Start;
    let show_end = loading && position == LoadingPosition::End;
    let show_center = loading && position == LoadingPosition::Center;

    rsx! {
        button {
            class,
            r#type: props.r#type,
            disabled: disabled || loading,
            aria_busy: if loading { "true" },
            onclick: move |e| {
                if let Some(handler) = &props.onclick {
                    handler.call(e);
                }
            },
            ..props.attributes,
            if show_start {
                span { class: "skeuo-btn__icon skeuo-btn__icon--start skeuo-btn__loader",
                    {indicator()}
                }
            } else if let Some(icon) = props.start_icon {
                span { class: "skeuo-btn__icon skeuo-btn__icon--start", aria_hidden: "true", {icon} }
            }
            span { class: "skeuo-btn__label", {props.children} }
            if show_center {
                span { class: "skeuo-btn__loader skeuo-btn__loader--center", {indicator()} }
            }
            if show_end {
                span { class: "skeuo-btn__icon skeuo-btn__icon--end skeuo-btn__loader",
                    {indicator()}
                }
            } else if let Some(icon) = props.end_icon {
                span { class: "skeuo-btn__icon skeuo-btn__icon--end", aria_hidden: "true", {icon} }
            }
        }
    }
}
