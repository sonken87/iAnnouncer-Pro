use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Size, classes, use_unique_id};

use super::TextFieldVariant;
use super::select::{SelectRow, control_classes, field_classes, step_enabled};

/// A text well that suggests matching options on a floating plate as you
/// type. Mirrors MUI's `<Autocomplete>` (with its `renderInput` built in as
/// a `TextField`-style well).
///
/// Options are strings. Single selection uses `value` / `default_value` /
/// `onchange` (`None` when cleared); with `multiple` the chosen options
/// become removable keycap pills inside the well and the selection uses
/// `values` / `default_values` / `onchange_multiple`. `free_solo` accepts
/// text that is not an option (press Enter).
///
/// Keyboard: ↓ / ↑ open and move, Enter picks, Escape closes, Backspace in
/// an empty field removes the last pill. Leaving the field closes the list.
#[derive(Props, Clone, PartialEq)]
pub struct AutocompleteProps {
    pub options: Vec<String>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    pub onchange: Option<EventHandler<Option<String>>>,
    #[props(default)]
    pub multiple: bool,
    pub values: Option<Vec<String>>,
    pub default_values: Option<Vec<String>>,
    pub onchange_multiple: Option<EventHandler<Vec<String>>>,
    /// Controls the text typed in the field.
    #[props(into)]
    pub input_value: Option<String>,
    pub oninputchange: Option<EventHandler<String>>,
    /// Controls whether the suggestions are shown.
    pub open: Option<bool>,
    /// Accept any typed text as a value, not just the options.
    #[props(default)]
    pub free_solo: bool,
    #[props(default = "No options".to_string(), into)]
    pub no_options_text: String,
    #[props(into)]
    pub label: Option<String>,
    #[props(into)]
    pub placeholder: Option<String>,
    #[props(into)]
    pub helper_text: Option<String>,
    #[props(default)]
    pub variant: TextFieldVariant,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub error: bool,
    #[props(default)]
    pub required: bool,
    #[props(default)]
    pub full_width: bool,
    /// Hide the clear (×) button.
    #[props(default)]
    pub disable_clearable: bool,
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
}

#[component]
pub fn Autocomplete(props: AutocompleteProps) -> Element {
    let generated = use_unique_id("skeuo-autocomplete");
    let id = props.id.clone().unwrap_or(generated);
    let listbox_id = format!("{id}-listbox");
    let helper_id = format!("{id}-helper");

    let multiple = props.multiple;
    let free_solo = props.free_solo;
    let disabled = props.disabled;

    let mut internal = use_signal(|| props.default_value.clone());
    let mut internal_multi = use_signal(|| props.default_values.clone().unwrap_or_default());
    let initial_text = if multiple {
        String::new()
    } else {
        props
            .value
            .clone()
            .or_else(|| props.default_value.clone())
            .unwrap_or_default()
    };
    let mut internal_input = use_signal(move || initial_text);
    let mut dirty = use_signal(|| false);
    let mut internal_open = use_signal(|| false);
    let mut active = use_signal(|| None::<usize>);

    let value: Option<String> = props.value.clone().or_else(|| internal.cloned());
    let values: Vec<String> = props
        .values
        .clone()
        .unwrap_or_else(|| internal_multi.cloned());
    let selected: Vec<String> = if multiple {
        values.clone()
    } else {
        value.clone().into_iter().collect()
    };
    let text = props
        .input_value
        .clone()
        .unwrap_or_else(|| internal_input.cloned());

    let query = text.to_lowercase();
    // A single value's own text shows the full list (as in MUI); any other
    // text filters it.
    let filtering = multiple || dirty() || value.as_deref() != Some(text.as_str());
    let filtered: Vec<String> = if filtering && !query.is_empty() {
        props
            .options
            .iter()
            .filter(|o| o.to_lowercase().contains(&query))
            .cloned()
            .collect()
    } else {
        props.options.clone()
    };
    let no_flags = vec![false; filtered.len()];
    let open = !disabled && props.open.unwrap_or_else(|| internal_open.cloned());
    let show_list = open && !(free_solo && filtered.is_empty());

    let (onchange, onchange_multiple, oninputchange) =
        (props.onchange, props.onchange_multiple, props.oninputchange);
    let mut set_text = move |t: String| {
        internal_input.set(t.clone());
        if let Some(h) = oninputchange {
            h.call(t);
        }
    };
    // Picks (or, with `multiple`, toggles) an option or free text.
    let pick = {
        let values = values.clone();
        move |v: String| {
            dirty.set(false);
            if multiple {
                let mut next = values.clone();
                if let Some(pos) = next.iter().position(|x| *x == v) {
                    next.remove(pos);
                } else {
                    next.push(v);
                }
                internal_multi.set(next.clone());
                if let Some(h) = onchange_multiple {
                    h.call(next);
                }
                set_text(String::new());
            } else {
                internal.set(Some(v.clone()));
                if let Some(h) = onchange {
                    h.call(Some(v.clone()));
                }
                set_text(v);
                internal_open.set(false);
            }
        }
    };
    let remove = {
        let values = values.clone();
        move |v: String| {
            let next: Vec<String> = values.iter().filter(|x| **x != v).cloned().collect();
            internal_multi.set(next.clone());
            if let Some(h) = onchange_multiple {
                h.call(next);
            }
        }
    };
    let clear = move |_| {
        dirty.set(false);
        if multiple {
            internal_multi.set(Vec::new());
            if let Some(h) = onchange_multiple {
                h.call(Vec::new());
            }
        } else {
            internal.set(None);
            if let Some(h) = onchange {
                h.call(None);
            }
        }
        set_text(String::new());
    };

    let onkeydown = {
        let mut pick = pick.clone();
        let mut remove = remove.clone();
        let filtered = filtered.clone();
        let no_flags = no_flags.clone();
        let text = text.clone();
        let values = values.clone();
        move |e: KeyboardEvent| match e.key() {
            Key::ArrowDown | Key::ArrowUp => {
                e.prevent_default();
                let dir = if e.key() == Key::ArrowDown { 1 } else { -1 };
                if !open {
                    internal_open.set(true);
                    active.set(step_enabled(&no_flags, None, dir));
                } else {
                    active.set(step_enabled(&no_flags, active(), dir));
                }
            }
            Key::Enter => {
                e.prevent_default();
                match active()
                    .filter(|_| open)
                    .and_then(|i| filtered.get(i).cloned())
                {
                    Some(v) => pick(v),
                    None if free_solo && !text.trim().is_empty() => pick(text.trim().to_string()),
                    None => {}
                }
            }
            Key::Escape => internal_open.set(false),
            Key::Backspace if multiple && text.is_empty() => {
                if let Some(last) = values.last() {
                    remove(last.clone());
                }
            }
            _ => {}
        }
    };
    let onblur = {
        let value = value.clone();
        let text = text.clone();
        let mut pick = pick.clone();
        move |_| {
            internal_open.set(false);
            active.set(None);
            if multiple || !dirty() {
                return;
            }
            dirty.set(false);
            if free_solo && !text.trim().is_empty() {
                if value.as_deref() != Some(text.trim()) {
                    pick(text.trim().to_string());
                }
            } else {
                set_text(value.clone().unwrap_or_default());
            }
        }
    };

    let class = field_classes(
        "skeuo-select skeuo-autocomplete",
        props.variant,
        props.color,
        props.size,
        props.error,
        disabled,
        props.full_width,
        &classes([
            if open { "skeuo-select--open" } else { "" },
            if multiple && !values.is_empty() {
                "skeuo-autocomplete--has-tags"
            } else {
                ""
            },
            &props.class,
        ]),
    );
    let control_class = classes([
        control_classes(props.variant),
        "skeuo-autocomplete__control",
    ]);
    let has_value = !selected.is_empty() || !text.is_empty();
    let active_id = active()
        .filter(|_| show_list)
        .map(|i| format!("{id}-opt-{i}"));
    let placeholder = if multiple && !values.is_empty() {
        None
    } else {
        props.placeholder.clone()
    };

    let described_by = props.helper_text.as_ref().map(|_| helper_id.clone());
    rsx! {
        div { class, style: props.style,
            if let Some(label) = props.label.clone() {
                label { class: "skeuo-field__label skeuo-label", r#for: id.clone(),
                    "{label}"
                    if props.required {
                        span { class: "skeuo-field__asterisk", aria_hidden: "true", " *" }
                    }
                }
            }
            div { class: "skeuo-select__anchor",
                div { class: control_class,
                    if multiple {
                        for v in values.iter().cloned() {
                            span { key: "{v}", class: "skeuo-autocomplete__tag skeuo-key",
                                span { class: "skeuo-autocomplete__tag-label", "{v}" }
                                if !disabled {
                                    button {
                                        class: "skeuo-autocomplete__tag-remove",
                                        r#type: "button",
                                        tabindex: "-1",
                                        aria_label: "Remove {v}",
                                        onclick: {
                                            let mut remove = remove.clone();
                                            let v = v.clone();
                                            move |_| remove(v.clone())
                                        },
                                        Icon { name: IconName::Close }
                                    }
                                }
                            }
                        }
                    }
                    input {
                        class: "skeuo-field__input skeuo-autocomplete__input",
                        id: id.clone(),
                        name: props.name.clone(),
                        r#type: "text",
                        role: "combobox",
                        autocomplete: "off",
                        value: text.clone(),
                        placeholder,
                        disabled,
                        required: props.required && selected.is_empty(),
                        aria_autocomplete: "list",
                        aria_expanded: show_list,
                        aria_controls: listbox_id.clone(),
                        aria_activedescendant: active_id,
                        aria_invalid: props.error,
                        aria_describedby: described_by.clone(),
                        oninput: move |e: FormEvent| {
                            dirty.set(true);
                            set_text(e.value());
                            internal_open.set(true);
                            active.set(None);
                        },
                        onclick: move |_| internal_open.set(true),
                        onkeydown,
                        onblur,
                    }
                    if has_value && !disabled && !props.disable_clearable {
                        button {
                            class: "skeuo-autocomplete__button skeuo-autocomplete__clear",
                            r#type: "button",
                            tabindex: "-1",
                            aria_label: "Clear",
                            onmousedown: |e| e.prevent_default(),
                            onclick: clear,
                            Icon { name: IconName::Close }
                        }
                    }
                    if !free_solo {
                        button {
                            class: "skeuo-autocomplete__button skeuo-autocomplete__popup",
                            r#type: "button",
                            tabindex: "-1",
                            disabled,
                            aria_label: if open { "Close" } else { "Open" },
                            onmousedown: |e| e.prevent_default(),
                            onclick: move |_| {
                                dirty.set(false);
                                internal_open.set(!open);
                            },
                            Icon { name: IconName::ChevronDown, class: "skeuo-select__chevron" }
                        }
                    }
                }
                if show_list {
                    ul {
                        class: "skeuo-select__listbox skeuo-chassis skeuo-chassis--float skeuo-elev-3",
                        id: listbox_id.clone(),
                        role: "listbox",
                        aria_multiselectable: if multiple { "true" },
                        onmousedown: |e| e.prevent_default(),
                        if filtered.is_empty() {
                            li { class: "skeuo-select__empty", role: "presentation", "{props.no_options_text}" }
                        }
                        for (i, opt) in filtered.iter().enumerate() {
                            SelectRow {
                                key: "{opt}",
                                id: format!("{id}-opt-{i}"),
                                label: opt.clone(),
                                selected: selected.contains(opt),
                                active: active() == Some(i),
                                disabled: false,
                                onpick: {
                                    let mut pick = pick.clone();
                                    let opt = opt.clone();
                                    move |_| pick(opt.clone())
                                },
                                onhover: move |_| active.set(Some(i)),
                            }
                        }
                    }
                }
            }
            if let Some(text) = props.helper_text.clone() {
                p { class: "skeuo-field__helper skeuo-caption", id: helper_id, "{text}" }
            }
        }
    }
}
