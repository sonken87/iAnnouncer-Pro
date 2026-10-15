use dioxus::prelude::*;

use crate::{Color, Size, classes};

/// The state a [`FormControl`] shares with the controls, labels and helper
/// text inside it (MUI's `useFormControl()`).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub struct FormControlState {
    pub disabled: bool,
    pub error: bool,
    pub required: bool,
    /// Size for controls that don't set their own.
    pub size: Option<Size>,
    /// Color for controls that don't set their own.
    pub color: Option<Color>,
}

#[derive(Clone, Copy)]
struct FormControlContext(Signal<FormControlState>);

/// The state of the enclosing [`FormControl`] (or [`FormControlLabel`]), if
/// any. Mirrors MUI's `useFormControl()`.
pub fn use_form_control() -> Option<FormControlState> {
    try_use_context::<FormControlContext>().map(|ctx| *ctx.0.read())
}

/// Provides `state` to the subtree, keeping it in sync with the props.
fn use_provide_form_control(state: FormControlState) {
    let FormControlContext(mut signal) =
        use_hook(|| provide_context(FormControlContext(Signal::new(state))));
    if *signal.peek() != state {
        signal.set(state);
    }
}

/// Wraps a group of form elements and shares `disabled` / `error` /
/// `required` / `size` / `color` with them. Mirrors MUI's `<FormControl>`.
///
/// Physically it adds nothing: the controls inside stay on the aluminium,
/// stacked with the lettering above and below them.
#[derive(Props, Clone, PartialEq)]
pub struct FormControlProps {
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub error: bool,
    #[props(default)]
    pub required: bool,
    pub size: Option<Size>,
    pub color: Option<Color>,
    #[props(default)]
    pub full_width: bool,
    /// Render as a `<fieldset>` (MUI `component="fieldset"`), the right
    /// element around a group of checkboxes or radios.
    #[props(default)]
    pub fieldset: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn FormControl(props: FormControlProps) -> Element {
    use_provide_form_control(FormControlState {
        disabled: props.disabled,
        error: props.error,
        required: props.required,
        size: props.size,
        color: props.color,
    });
    let class = classes([
        "skeuo-form-control",
        if props.error {
            "skeuo-form-control--error"
        } else {
            ""
        },
        if props.disabled {
            "skeuo-form-control--disabled"
        } else {
            ""
        },
        if props.full_width {
            "skeuo-form-control--full-width"
        } else {
            ""
        },
        &props.class,
    ]);
    if props.fieldset {
        rsx! {
            fieldset { class, disabled: props.disabled, {props.children} }
        }
    } else {
        rsx! {
            div { class, {props.children} }
        }
    }
}

/// A caption printed on the aluminium above a group of controls. Mirrors
/// MUI's `<FormLabel>`. Turns error-colored and shows a required asterisk
/// from the enclosing [`FormControl`].
#[derive(Props, Clone, PartialEq)]
pub struct FormLabelProps {
    #[props(into)]
    pub id: Option<String>,
    /// Id of the control this labels (`for`).
    #[props(into)]
    pub html_for: Option<String>,
    pub error: Option<bool>,
    pub disabled: Option<bool>,
    pub required: Option<bool>,
    /// Render as a `<legend>` (inside a `fieldset` [`FormControl`]).
    #[props(default)]
    pub legend: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn FormLabel(props: FormLabelProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let error = props.error.unwrap_or(ctx.error);
    let disabled = props.disabled.unwrap_or(ctx.disabled);
    let required = props.required.unwrap_or(ctx.required);
    let class = classes([
        "skeuo-form-label",
        "skeuo-label",
        if error { "skeuo-form-label--error" } else { "" },
        if disabled {
            "skeuo-form-label--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let asterisk = rsx! {
        if required {
            span { class: "skeuo-form-label__asterisk", aria_hidden: "true", " *" }
        }
    };
    if props.legend {
        rsx! {
            legend { class, id: props.id, {props.children} {asterisk} }
        }
    } else {
        rsx! {
            label { class, id: props.id, r#for: props.html_for, {props.children} {asterisk} }
        }
    }
}

/// Where a [`FormControlLabel`]'s text sits relative to its control.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum LabelPlacement {
    #[default]
    End,
    Start,
    Top,
    Bottom,
}

impl LabelPlacement {
    fn as_str(self) -> &'static str {
        match self {
            LabelPlacement::End => "end",
            LabelPlacement::Start => "start",
            LabelPlacement::Top => "top",
            LabelPlacement::Bottom => "bottom",
        }
    }
}

/// A control (Checkbox, Radio, Switch) with its legend printed next to it;
/// clicking the legend toggles the control. Mirrors MUI's
/// `<FormControlLabel>`. `disabled` / `required` reach the control through
/// the form-control context.
#[derive(Props, Clone, PartialEq)]
pub struct FormControlLabelProps {
    /// The control, e.g. `rsx! { Checkbox {} }`.
    pub control: Element,
    #[props(into)]
    pub label: String,
    #[props(default)]
    pub label_placement: LabelPlacement,
    pub disabled: Option<bool>,
    /// Shows an asterisk and marks the control `required`.
    pub required: Option<bool>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn FormControlLabel(props: FormControlLabelProps) -> Element {
    let parent = use_form_control().unwrap_or_default();
    let disabled = props.disabled.unwrap_or(parent.disabled);
    // As in MUI, only `disabled` flows down from the FormControl; the
    // asterisk is shown when this label itself is `required`.
    let required = props.required.unwrap_or(false);
    use_provide_form_control(FormControlState {
        disabled,
        required: required || parent.required,
        ..parent
    });
    let placement = format!(
        "skeuo-form-control-label--{}",
        props.label_placement.as_str()
    );
    let class = classes([
        "skeuo-form-control-label",
        &placement,
        if disabled {
            "skeuo-form-control-label--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        label { class,
            {props.control}
            span { class: "skeuo-form-control-label__label",
                "{props.label}"
                if required {
                    span { class: "skeuo-form-label__asterisk", aria_hidden: "true", " *" }
                }
            }
        }
    }
}

/// Stacks controls in a column (or a `row`). Mirrors MUI's `<FormGroup>`.
#[derive(Props, Clone, PartialEq)]
pub struct FormGroupProps {
    #[props(default)]
    pub row: bool,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn FormGroup(props: FormGroupProps) -> Element {
    let class = classes([
        "skeuo-form-group",
        if props.row {
            "skeuo-form-group--row"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div {
            class,
            role: "group",
            aria_labelledby: props.aria_labelledby,
            aria_label: props.aria_label,
            {props.children}
        }
    }
}

/// Small print under a control: hints, or the reason for an error. Mirrors
/// MUI's `<FormHelperText>`.
#[derive(Props, Clone, PartialEq)]
pub struct FormHelperTextProps {
    #[props(into)]
    pub id: Option<String>,
    pub error: Option<bool>,
    pub disabled: Option<bool>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn FormHelperText(props: FormHelperTextProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let error = props.error.unwrap_or(ctx.error);
    let disabled = props.disabled.unwrap_or(ctx.disabled);
    let class = classes([
        "skeuo-form-helper-text",
        "skeuo-caption",
        if error {
            "skeuo-form-helper-text--error"
        } else {
            ""
        },
        if disabled {
            "skeuo-form-helper-text--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        p { class, id: props.id, {props.children} }
    }
}
