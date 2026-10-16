use dioxus::prelude::*;

use super::form::use_form_control;
use crate::{Color, Size, classes, use_unique_id};

#[derive(Clone, Debug, PartialEq)]
struct RadioGroupState {
    name: String,
    value: Option<String>,
    color: Option<Color>,
    size: Option<Size>,
    disabled: bool,
}

#[derive(Clone, Copy)]
struct RadioGroupContext {
    state: Signal<RadioGroupState>,
    select: Callback<String>,
}

/// A radio button drawn as a small round keycap. When selected, an LED dot
/// lights up in the middle of the cap, like the caps-lock light. Mirrors
/// MUI's `<Radio>`.
///
/// Inside a [`RadioGroup`] the group supplies `name`, the selection and the
/// change handler; standalone, pass `checked` and `onchange`.
#[derive(Props, Clone, PartialEq)]
pub struct RadioProps {
    /// The value this radio stands for in its group.
    #[props(default, into)]
    pub value: String,
    /// Controlled state for a radio outside a [`RadioGroup`].
    pub checked: Option<bool>,
    pub color: Option<Color>,
    pub size: Option<Size>,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub required: bool,
    #[props(into)]
    pub name: Option<String>,
    /// Id of the `<input>`.
    #[props(into)]
    pub id: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub onchange: Option<EventHandler<FormEvent>>,
}

#[component]
pub fn Radio(props: RadioProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let group = try_use_context::<RadioGroupContext>();
    let group_state = group.map(|g| g.state.read().clone());

    let checked = match (&group_state, props.checked) {
        (_, Some(c)) => c,
        (Some(g), None) => g.value.as_deref() == Some(props.value.as_str()),
        (None, None) => false,
    };
    let name = props
        .name
        .clone()
        .or_else(|| group_state.as_ref().map(|g| g.name.clone()));
    let disabled =
        props.disabled || ctx.disabled || group_state.as_ref().is_some_and(|g| g.disabled);
    let color = props
        .color
        .or(group_state.as_ref().and_then(|g| g.color))
        .or(ctx.color)
        .unwrap_or_default();
    let size = props
        .size
        .or(group_state.as_ref().and_then(|g| g.size))
        .or(ctx.size)
        .unwrap_or_default();

    let size_class = format!("skeuo-radio--{}", size.as_str());
    let class = classes([
        "skeuo-radio",
        color.class(),
        &size_class,
        if checked { "skeuo-radio--checked" } else { "" },
        if disabled {
            "skeuo-radio--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let value = props.value.clone();
    let onchange = move |e: FormEvent| {
        if let Some(g) = group {
            g.select.call(value.clone());
        }
        if let Some(h) = &props.onchange {
            h.call(e);
        }
    };

    rsx! {
        span { class,
            span {
                class: if checked { "skeuo-radio__cap skeuo-key skeuo-key--round skeuo-key--pressable skeuo-key--focus-within skeuo-key--down" } else { "skeuo-radio__cap skeuo-key skeuo-key--round skeuo-key--pressable skeuo-key--focus-within" },
                aria_disabled: disabled.then_some("true"),
                input {
                    class: "skeuo-radio__input",
                    r#type: "radio",
                    id: props.id,
                    name,
                    value: props.value.clone(),
                    checked,
                    disabled,
                    required: props.required || ctx.required,
                    aria_label: props.aria_label,
                    onchange,
                }
                span {
                    class: if checked { "skeuo-radio__led skeuo-led" } else { "skeuo-radio__led skeuo-led skeuo-led--off" },
                    aria_hidden: "true",
                }
            }
        }
    }
}

/// A set of [`Radio`]s sharing one `name`, of which exactly one is
/// selected. Mirrors MUI's `<RadioGroup>` (`role="radiogroup"`).
///
/// Pass `value` + `onchange` for a controlled group or `default_value` for
/// an uncontrolled one. `color`, `size` and `disabled` apply to every radio
/// that doesn't set its own.
#[derive(Props, Clone, PartialEq)]
pub struct RadioGroupProps {
    /// Shared `name` of the inputs. Generated when omitted.
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    /// Lay the radios out in a row.
    #[props(default)]
    pub row: bool,
    pub color: Option<Color>,
    pub size: Option<Size>,
    #[props(default)]
    pub disabled: bool,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    /// Called with the newly selected radio's `value`.
    pub onchange: Option<EventHandler<String>>,
    pub children: Element,
}

#[component]
pub fn RadioGroup(props: RadioGroupProps) -> Element {
    let generated = use_unique_id("skeuo-radio-group");
    let mut internal = use_signal(|| props.default_value.clone());
    let state = RadioGroupState {
        name: props.name.clone().unwrap_or(generated),
        value: props.value.clone().or_else(|| internal.cloned()),
        color: props.color,
        size: props.size,
        disabled: props.disabled,
    };
    let onchange = props.onchange;
    let select = use_callback(move |v: String| {
        internal.set(Some(v.clone()));
        if let Some(h) = &onchange {
            h.call(v);
        }
    });
    let RadioGroupContext {
        state: mut signal, ..
    } = use_hook(|| {
        provide_context(RadioGroupContext {
            state: Signal::new(state.clone()),
            select,
        })
    });
    if *signal.peek() != state {
        signal.set(state);
    }

    let class = classes([
        "skeuo-radio-group",
        if props.row {
            "skeuo-radio-group--row"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div {
            class,
            role: "radiogroup",
            aria_labelledby: props.aria_labelledby,
            aria_label: props.aria_label,
            {props.children}
        }
    }
}
