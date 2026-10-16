use dioxus::prelude::*;

use crate::{Color, Orientation, Size, classes};

use super::ButtonVariant;

/// What a [`ButtonGroup`] hands down to its [`Button`](super::Button)s.
#[derive(Clone, Copy, Debug, PartialEq)]
pub(crate) struct ButtonGroupState {
    pub variant: ButtonVariant,
    pub color: Color,
    pub size: Size,
    pub disabled: bool,
    pub full_width: bool,
}

/// Keeps a context signal equal to `value`, writing only when it changed
/// (the same thing `use_reactive` does), so descendants re-render with the
/// parent's new props.
pub(crate) fn use_synced_context<T: Clone + PartialEq + 'static>(value: T) -> Signal<T> {
    let initial = value.clone();
    let mut signal = use_context_provider(move || Signal::new(initial));
    if *signal.peek() != value {
        signal.set(value);
    }
    signal
}

/// A row of keycaps joined edge-to-edge, as if they shared one slot in the
/// keyboard; only the outer corners are rounded. Mirrors MUI's
/// `<ButtonGroup>`.
///
/// Its children are [`Button`](super::Button)s; the group's `variant`,
/// `color`, `size` and `disabled` override theirs (MUI lets an explicit
/// button prop win; Rust props cannot tell "unset" from "default").
#[derive(Props, Clone, PartialEq)]
pub struct ButtonGroupProps {
    #[props(default)]
    pub variant: ButtonVariant,
    #[props(default)]
    pub orientation: Orientation,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    /// Stretch the group (and its keys, equally) to the container's width.
    #[props(default)]
    pub full_width: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn ButtonGroup(props: ButtonGroupProps) -> Element {
    use_synced_context(ButtonGroupState {
        variant: props.variant,
        color: props.color,
        size: props.size,
        disabled: props.disabled,
        full_width: props.full_width,
    });
    let variant = format!("skeuo-btn-group--{}", props.variant.as_str());
    let orientation = format!("skeuo-btn-group--{}", props.orientation.as_str());
    let class = classes([
        "skeuo-btn-group",
        &variant,
        &orientation,
        props.color.class(),
        if props.full_width {
            "skeuo-btn-group--full-width"
        } else {
            ""
        },
        if props.disabled {
            "skeuo-btn-group--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div {
            class,
            role: "group",
            aria_orientation: props.orientation.as_str(),
            ..props.attributes,
            {props.children}
        }
    }
}
