use dioxus::prelude::*;

use crate::{Color, Size, classes, use_unique_id};

/// Field style, mirroring MUI's `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TextFieldVariant {
    /// A well cut into the aluminium.
    #[default]
    Outlined,
    /// A shallow, borderless recess.
    Filled,
    /// An engraved line on the aluminium.
    Standard,
}

impl TextFieldVariant {
    pub(crate) fn as_str(self) -> &'static str {
        match self {
            TextFieldVariant::Outlined => "outlined",
            TextFieldVariant::Filled => "filled",
            TextFieldVariant::Standard => "standard",
        }
    }
}

/// A labelled text input. Mirrors MUI's `<TextField>`.
///
/// Pass `value` + `oninput` for a controlled field, or `default_value` for an
/// uncontrolled one. `multiline` renders a `<textarea>`.
#[derive(Props, Clone, PartialEq)]
pub struct TextFieldProps {
    #[props(default)]
    pub variant: TextFieldVariant,
    /// Color of the focus halo.
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(into)]
    pub label: Option<String>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    #[props(into)]
    pub placeholder: Option<String>,
    /// Text under the field; shown in the error color when `error` is set.
    #[props(into)]
    pub helper_text: Option<String>,
    #[props(default)]
    pub error: bool,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub required: bool,
    #[props(default)]
    pub read_only: bool,
    #[props(default)]
    pub full_width: bool,
    #[props(default)]
    pub autofocus: bool,
    #[props(default)]
    pub multiline: bool,
    /// Visible rows when `multiline`.
    #[props(default = 3)]
    pub rows: u32,
    /// HTML input `type`: `"text"` (default), `"password"`, `"email"`, …
    #[props(default = "text".to_string(), into)]
    pub r#type: String,
    #[props(into)]
    pub name: Option<String>,
    /// Id of the input element. Generated when omitted, as in MUI.
    #[props(into)]
    pub id: Option<String>,
    #[props(into)]
    pub autocomplete: Option<String>,
    /// Element inside the field before the text, e.g. a unit or an icon.
    pub start_adornment: Option<Element>,
    /// Element inside the field after the text.
    pub end_adornment: Option<Element>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub oninput: Option<EventHandler<FormEvent>>,
    pub onchange: Option<EventHandler<FormEvent>>,
    pub onkeydown: Option<EventHandler<KeyboardEvent>>,
    pub onfocus: Option<EventHandler<FocusEvent>>,
    pub onblur: Option<EventHandler<FocusEvent>>,
}

#[component]
pub fn TextField(props: TextFieldProps) -> Element {
    let generated_id = use_unique_id("skeuo-field");
    let input_id = props.id.clone().unwrap_or(generated_id);
    let helper_id = format!("{input_id}-helper");
    let described_by = props.helper_text.as_ref().map(|_| helper_id.clone());
    // `default_value` is read once at mount and rendered through `value`.
    // The attribute never changes afterwards, so it is never re-applied and
    // the field stays uncontrolled. (`initial_value` would be lost in SSR.)
    let default_value = use_hook(|| props.default_value.clone());
    let value = props.value.clone().or(default_value);

    let variant = format!("skeuo-field--{}", props.variant.as_str());
    let size = format!("skeuo-field--{}", props.size.as_str());
    let control_class = match props.variant {
        TextFieldVariant::Outlined => "skeuo-field__control skeuo-well skeuo-well--focusable",
        TextFieldVariant::Filled => {
            "skeuo-field__control skeuo-well skeuo-well--shallow skeuo-well--focusable"
        }
        TextFieldVariant::Standard => "skeuo-field__control",
    };
    let class = classes([
        "skeuo-field",
        &variant,
        props.color.class(),
        &size,
        if props.error {
            "skeuo-field--error"
        } else {
            ""
        },
        if props.disabled {
            "skeuo-field--disabled"
        } else {
            ""
        },
        if props.full_width {
            "skeuo-field--full-width"
        } else {
            ""
        },
        if props.multiline {
            "skeuo-field--multiline"
        } else {
            ""
        },
        &props.class,
    ]);

    let oninput = move |e: FormEvent| {
        if let Some(h) = &props.oninput {
            h.call(e);
        }
    };
    let onchange = move |e: FormEvent| {
        if let Some(h) = &props.onchange {
            h.call(e);
        }
    };
    let onkeydown = move |e: KeyboardEvent| {
        if let Some(h) = &props.onkeydown {
            h.call(e);
        }
    };
    let onfocus = move |e: FocusEvent| {
        if let Some(h) = &props.onfocus {
            h.call(e);
        }
    };
    let onblur = move |e: FocusEvent| {
        if let Some(h) = &props.onblur {
            h.call(e);
        }
    };

    rsx! {
        div { class, style: props.style,
            if let Some(label) = props.label {
                label { class: "skeuo-field__label skeuo-label", r#for: input_id.clone(),
                    "{label}"
                    if props.required {
                        span { class: "skeuo-field__asterisk", aria_hidden: "true", " *" }
                    }
                }
            }
            div { class: control_class,
                if let Some(adornment) = props.start_adornment {
                    span { class: "skeuo-field__adornment skeuo-field__adornment--start", {adornment} }
                }
                if props.multiline {
                    textarea {
                        class: "skeuo-field__input",
                        id: input_id.clone(),
                        name: props.name.clone(),
                        rows: props.rows as i64,
                        value: value.clone(),
                        placeholder: props.placeholder.clone(),
                        disabled: props.disabled,
                        required: props.required,
                        readonly: props.read_only,
                        autofocus: props.autofocus,
                        aria_invalid: props.error,
                        aria_describedby: described_by.clone(),
                        oninput,
                        onchange,
                        onkeydown,
                        onfocus,
                        onblur,
                    }
                } else {
                    input {
                        class: "skeuo-field__input",
                        id: input_id.clone(),
                        name: props.name.clone(),
                        r#type: props.r#type.clone(),
                        value: value.clone(),
                        placeholder: props.placeholder.clone(),
                        autocomplete: props.autocomplete.clone(),
                        disabled: props.disabled,
                        required: props.required,
                        readonly: props.read_only,
                        autofocus: props.autofocus,
                        aria_invalid: props.error,
                        aria_describedby: described_by.clone(),
                        oninput,
                        onchange,
                        onkeydown,
                        onfocus,
                        onblur,
                    }
                }
                if let Some(adornment) = props.end_adornment {
                    span { class: "skeuo-field__adornment skeuo-field__adornment--end", {adornment} }
                }
            }
            if let Some(text) = props.helper_text {
                p { class: "skeuo-field__helper skeuo-caption", id: helper_id, "{text}" }
            }
        }
    }
}
