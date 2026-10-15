use dioxus::prelude::*;

use super::form::use_form_control;
use crate::{Color, Size, classes};

/// An on/off switch: a groove cut into the aluminium with a round keycap
/// thumb that slides along it. Switched on, the groove fills with the
/// palette color like a lit channel. Mirrors MUI's `<Switch>`.
///
/// Built on a visually hidden `<input type="checkbox" role="switch">`.
/// Pass `checked` + `onchange` for a controlled switch, or
/// `default_checked` for an uncontrolled one; the new state is
/// `e.checked()`.
#[derive(Props, Clone, PartialEq)]
pub struct SwitchProps {
    pub checked: Option<bool>,
    #[props(default)]
    pub default_checked: bool,
    pub color: Option<Color>,
    pub size: Option<Size>,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub required: bool,
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub value: Option<String>,
    /// Id of the `<input>`.
    #[props(into)]
    pub id: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(into)]
    pub aria_describedby: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub onchange: Option<EventHandler<FormEvent>>,
}

#[component]
pub fn Switch(props: SwitchProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let mut internal = use_signal(|| props.default_checked);
    let checked = props.checked.unwrap_or_else(|| internal.cloned());
    let disabled = props.disabled || ctx.disabled;
    let color = props.color.or(ctx.color).unwrap_or_default();
    let size = props.size.or(ctx.size).unwrap_or_default();

    let size_class = format!("skeuo-switch--{}", size.as_str());
    let class = classes([
        "skeuo-switch",
        color.class(),
        &size_class,
        if checked { "skeuo-switch--checked" } else { "" },
        if disabled {
            "skeuo-switch--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let onchange = move |e: FormEvent| {
        if props.checked.is_none() {
            internal.set(!checked);
        }
        if let Some(h) = &props.onchange {
            h.call(e);
        }
    };

    rsx! {
        span { class,
            span { class: "skeuo-switch__track skeuo-groove", aria_hidden: "true",
                span { class: "skeuo-switch__fill skeuo-groove__fill" }
            }
            span {
                class: "skeuo-switch__thumb skeuo-key skeuo-key--round",
                aria_hidden: "true",
            }
            input {
                class: "skeuo-switch__input",
                r#type: "checkbox",
                role: "switch",
                id: props.id,
                name: props.name,
                value: props.value,
                checked,
                disabled,
                required: props.required || ctx.required,
                aria_checked: if checked { "true" } else { "false" },
                aria_label: props.aria_label,
                aria_labelledby: props.aria_labelledby,
                aria_describedby: props.aria_describedby,
                onchange,
            }
        }
    }
}
