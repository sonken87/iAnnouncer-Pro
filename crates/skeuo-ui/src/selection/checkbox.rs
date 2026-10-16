use dioxus::prelude::*;

use super::form::use_form_control;
use crate::{Color, Icon, IconName, Size, classes};

/// A checkbox drawn as a small, square white keycap. Checked, the cap turns
/// the palette color with a white check legend; indeterminate shows a dash.
/// Mirrors MUI's `<Checkbox>`.
///
/// A real (visually hidden) `<input type="checkbox">` sits inside the cap,
/// so it works with forms, labels and the keyboard. Pass `checked` +
/// `onchange` for a controlled checkbox, or `default_checked` for an
/// uncontrolled one. `onchange` receives the native change event; the new
/// state is `e.checked()`.
#[derive(Props, Clone, PartialEq)]
pub struct CheckboxProps {
    pub checked: Option<bool>,
    #[props(default)]
    pub default_checked: bool,
    /// Shows a dash instead of a check (`aria-checked="mixed"`).
    #[props(default)]
    pub indeterminate: bool,
    /// Defaults to the enclosing form control's color, then primary.
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
pub fn Checkbox(props: CheckboxProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let mut internal = use_signal(|| props.default_checked);
    let checked = props.checked.unwrap_or_else(|| internal.cloned());
    let disabled = props.disabled || ctx.disabled;
    let required = props.required || ctx.required;
    let color = props.color.or(ctx.color).unwrap_or_default();
    let size = props.size.or(ctx.size).unwrap_or_default();
    let indeterminate = props.indeterminate;
    let lit = checked || indeterminate;

    let size_class = format!("skeuo-checkbox--{}", size.as_str());
    let class = classes([
        "skeuo-checkbox",
        color.class(),
        &size_class,
        if checked {
            "skeuo-checkbox--checked"
        } else {
            ""
        },
        if indeterminate {
            "skeuo-checkbox--indeterminate"
        } else {
            ""
        },
        if disabled {
            "skeuo-checkbox--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let cap = classes([
        "skeuo-checkbox__cap skeuo-key skeuo-key--pressable skeuo-key--focus-within",
        if lit { "skeuo-key--filled" } else { "" },
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
            span { class: cap, aria_disabled: disabled.then_some("true"),
                input {
                    class: "skeuo-checkbox__input",
                    r#type: "checkbox",
                    id: props.id,
                    name: props.name,
                    value: props.value,
                    checked,
                    disabled,
                    required,
                    aria_checked: if indeterminate { Some("mixed") } else { None },
                    aria_label: props.aria_label,
                    aria_labelledby: props.aria_labelledby,
                    aria_describedby: props.aria_describedby,
                    onchange,
                }
                if indeterminate {
                    Icon { name: IconName::Remove, class: "skeuo-checkbox__mark" }
                } else if checked {
                    Icon { name: IconName::Check, class: "skeuo-checkbox__mark" }
                }
            }
        }
    }
}
