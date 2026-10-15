use dioxus::prelude::*;

use crate::{Color, Orientation, Size, classes};

use super::button_group::use_synced_context;

#[derive(Clone, PartialEq)]
struct ToggleGroupState {
    selected: Vec<String>,
    color: Color,
    size: Size,
    disabled: bool,
    full_width: bool,
}

#[derive(Clone, Copy)]
struct ToggleGroupCtx {
    state: Signal<ToggleGroupState>,
    toggle: Callback<String>,
}

/// A white keycap that latches down while selected, its legend turning the
/// palette color. Mirrors MUI's `<ToggleButton>`.
///
/// Standalone, pass `selected` and handle `onchange` (called with `value`).
/// Inside a [`ToggleButtonGroup`] the group owns the selection, `color`,
/// `size` and `disabled`.
#[derive(Props, Clone, PartialEq)]
pub struct ToggleButtonProps {
    /// Identifies the button within its group; passed to `onchange`.
    #[props(into)]
    pub value: String,
    #[props(default)]
    pub selected: bool,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub full_width: bool,
    #[props(default, into)]
    pub class: String,
    /// Called with `value` when the key is pressed.
    pub onchange: Option<EventHandler<String>>,
    pub onclick: Option<EventHandler<MouseEvent>>,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn ToggleButton(props: ToggleButtonProps) -> Element {
    let group = try_use_context::<ToggleGroupCtx>();
    let state = group.map(|g| g.state.read().clone());
    let (selected, color, size, disabled, full_width) = match &state {
        Some(s) => (
            s.selected.contains(&props.value),
            s.color,
            s.size,
            props.disabled || s.disabled,
            props.full_width || s.full_width,
        ),
        None => (
            props.selected,
            props.color,
            props.size,
            props.disabled,
            props.full_width,
        ),
    };
    let size_class = format!("skeuo-toggle--{}", size.as_str());
    let class = classes([
        "skeuo-key skeuo-key--pressable skeuo-toggle",
        color.class(),
        &size_class,
        if selected {
            "skeuo-key--down skeuo-toggle--selected"
        } else {
            ""
        },
        if full_width {
            "skeuo-toggle--full-width"
        } else {
            ""
        },
        &props.class,
    ]);
    let value = props.value.clone();
    rsx! {
        button {
            class,
            r#type: "button",
            value: props.value.clone(),
            disabled,
            aria_pressed: selected,
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
                if let Some(g) = group {
                    g.toggle.call(value.clone());
                }
                if let Some(h) = &props.onchange {
                    h.call(value.clone());
                }
            },
            ..props.attributes,
            {props.children}
        }
    }
}

/// A run of [`ToggleButton`] keycaps sharing one slot, like the segmented
/// formatting keys of a toolbar. Mirrors MUI's `<ToggleButtonGroup>`.
///
/// The selection is a `Vec<String>` of button values in both modes (MUI uses
/// a single value for `exclusive`; here it is a vector of at most one item,
/// empty when nothing is selected). Controlled with `value` + `onchange`, or
/// uncontrolled with `default_value`.
#[derive(Props, Clone, PartialEq)]
pub struct ToggleButtonGroupProps {
    pub value: Option<Vec<String>>,
    pub default_value: Option<Vec<String>>,
    /// Only one button can be selected at a time (pressing the selected one
    /// releases it, as in MUI).
    #[props(default)]
    pub exclusive: bool,
    /// Called with the new selection.
    pub onchange: Option<EventHandler<Vec<String>>>,
    #[props(default)]
    pub orientation: Orientation,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub full_width: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn ToggleButtonGroup(props: ToggleButtonGroupProps) -> Element {
    let mut internal = use_signal(|| props.default_value.clone().unwrap_or_default());
    let selected = props.value.clone().unwrap_or_else(|| internal.cloned());
    let state = use_synced_context(ToggleGroupState {
        selected: selected.clone(),
        color: props.color,
        size: props.size,
        disabled: props.disabled,
        full_width: props.full_width,
    });
    let exclusive = props.exclusive;
    let onchange = props.onchange;
    let toggle = use_callback(move |v: String| {
        let current = state.peek().selected.clone();
        let next = if exclusive {
            if current.contains(&v) {
                Vec::new()
            } else {
                vec![v]
            }
        } else if current.contains(&v) {
            current.into_iter().filter(|x| *x != v).collect()
        } else {
            let mut next = current;
            next.push(v);
            next
        };
        internal.set(next.clone());
        if let Some(h) = &onchange {
            h.call(next);
        }
    });
    use_context_provider(|| ToggleGroupCtx { state, toggle });

    let orientation = format!("skeuo-btn-group--{}", props.orientation.as_str());
    let class = classes([
        "skeuo-btn-group skeuo-toggle-group",
        &orientation,
        props.color.class(),
        if props.full_width {
            "skeuo-btn-group--full-width"
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
