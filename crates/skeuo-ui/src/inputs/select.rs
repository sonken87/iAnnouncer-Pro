use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Size, classes, use_unique_id};

use super::TextFieldVariant;

/// One choice of a [`Select`] or [`NativeSelect`].
///
/// MUI composes `<Select>` from `<MenuItem>` children; here the options are
/// plain data so the select does not depend on the menu components.
#[derive(Clone, Debug, Default, PartialEq, Eq, Hash)]
pub struct SelectOption {
    pub value: String,
    pub label: String,
    pub disabled: bool,
}

impl SelectOption {
    pub fn new(value: impl Into<String>, label: impl Into<String>) -> Self {
        Self {
            value: value.into(),
            label: label.into(),
            disabled: false,
        }
    }

    /// The same option, not selectable.
    pub fn disabled(mut self) -> Self {
        self.disabled = true;
        self
    }
}

impl From<&str> for SelectOption {
    /// An option whose value and label are the same text.
    fn from(s: &str) -> Self {
        SelectOption::new(s, s)
    }
}

/// The next selectable index from `from` in direction `dir` (±1), or the
/// first / last selectable one when `from` is `None`.
pub(crate) fn step_enabled(disabled: &[bool], from: Option<usize>, dir: isize) -> Option<usize> {
    let n = disabled.len() as isize;
    if n == 0 {
        return None;
    }
    let mut i = match from {
        Some(i) => i as isize + dir,
        None if dir > 0 => 0,
        None => n - 1,
    };
    while (0..n).contains(&i) {
        if !disabled[i as usize] {
            return Some(i as usize);
        }
        i += dir;
    }
    from
}

#[allow(clippy::too_many_arguments)]
pub(crate) fn field_classes(
    base: &str,
    variant: TextFieldVariant,
    color: Color,
    size: Size,
    error: bool,
    disabled: bool,
    full_width: bool,
    extra: &str,
) -> String {
    let variant = format!("skeuo-field--{}", variant.as_str());
    let size = format!("skeuo-field--{}", size.as_str());
    classes([
        "skeuo-field",
        base,
        &variant,
        color.class(),
        &size,
        if error { "skeuo-field--error" } else { "" },
        if disabled {
            "skeuo-field--disabled"
        } else {
            ""
        },
        if full_width {
            "skeuo-field--full-width"
        } else {
            ""
        },
        extra,
    ])
}

pub(crate) fn control_classes(variant: TextFieldVariant) -> &'static str {
    match variant {
        TextFieldVariant::Outlined => "skeuo-field__control skeuo-well skeuo-well--focusable",
        TextFieldVariant::Filled => {
            "skeuo-field__control skeuo-well skeuo-well--shallow skeuo-well--focusable"
        }
        TextFieldVariant::Standard => "skeuo-field__control",
    }
}

/// A well with a chevron that opens a list of choices on a floating
/// aluminium plate (chassis level 3); the chosen row is tinted and ticked.
/// Mirrors MUI's `<Select>` (in its `TextField`-like outlined / filled /
/// standard variants).
///
/// Deviation from MUI: the choices are passed as `options` data instead of
/// `<MenuItem>` children. Single selection uses `value` / `default_value` /
/// `onchange`; with `multiple` use `values` / `default_values` /
/// `onchange_multiple` (MUI overloads one `value` prop with an array).
///
/// Keyboard: ↓ / ↑ / Enter / Space open; ↓ ↑ Home End move; Enter / Space
/// choose; Escape or Tab close. Clicking elsewhere closes it.
#[derive(Props, Clone, PartialEq)]
pub struct SelectProps {
    #[props(into)]
    pub label: Option<String>,
    pub options: Vec<SelectOption>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    pub onchange: Option<EventHandler<String>>,
    #[props(default)]
    pub multiple: bool,
    pub values: Option<Vec<String>>,
    pub default_values: Option<Vec<String>>,
    pub onchange_multiple: Option<EventHandler<Vec<String>>>,
    /// Controls whether the list is shown.
    pub open: Option<bool>,
    pub onopen: Option<EventHandler<()>>,
    pub onclose: Option<EventHandler<()>>,
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
    #[props(into)]
    pub helper_text: Option<String>,
    /// Shown when nothing is selected (MUI needs `displayEmpty` + `renderValue`).
    #[props(into)]
    pub placeholder: Option<String>,
    /// Form field name; the value is submitted through a hidden input
    /// (comma-separated when `multiple`).
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
pub fn Select(props: SelectProps) -> Element {
    let generated = use_unique_id("skeuo-select");
    let id = props.id.clone().unwrap_or(generated);
    let label_id = format!("{id}-label");
    let listbox_id = format!("{id}-listbox");
    let helper_id = format!("{id}-helper");

    let mut internal = use_signal(|| props.default_value.clone());
    let mut internal_multi = use_signal(|| props.default_values.clone().unwrap_or_default());
    let mut internal_open = use_signal(|| false);
    let mut active = use_signal(|| None::<usize>);

    let multiple = props.multiple;
    let selected: Vec<String> = if multiple {
        props
            .values
            .clone()
            .unwrap_or_else(|| internal_multi.cloned())
    } else {
        props
            .value
            .clone()
            .or_else(|| internal.cloned())
            .into_iter()
            .collect()
    };
    let open = !props.disabled && props.open.unwrap_or_else(|| internal_open.cloned());
    let options = props.options.clone();
    let disabled_flags: Vec<bool> = options.iter().map(|o| o.disabled).collect();

    let (onopen, onclose) = (props.onopen, props.onclose);
    let set_open = move |want: bool| {
        internal_open.set(want);
        match (want, onopen, onclose) {
            (true, Some(h), _) => h.call(()),
            (false, _, Some(h)) => h.call(()),
            _ => {}
        }
    };

    let (onchange, onchange_multiple) = (props.onchange, props.onchange_multiple);
    let choose = {
        let options = options.clone();
        let selected = selected.clone();
        let mut set_open = set_open;
        move |i: usize| {
            let Some(opt) = options.get(i) else { return };
            if opt.disabled {
                return;
            }
            if multiple {
                let mut next = selected.clone();
                if let Some(pos) = next.iter().position(|v| *v == opt.value) {
                    next.remove(pos);
                } else {
                    next.push(opt.value.clone());
                }
                internal_multi.set(next.clone());
                if let Some(h) = onchange_multiple {
                    h.call(next);
                }
            } else {
                internal.set(Some(opt.value.clone()));
                if let Some(h) = onchange {
                    h.call(opt.value.clone());
                }
                set_open(false);
            }
        }
    };

    let first_selected = options.iter().position(|o| selected.contains(&o.value));
    let onkeydown = {
        let mut choose = choose.clone();
        let mut set_open = set_open;
        let disabled_flags = disabled_flags.clone();
        let disabled = props.disabled;
        move |e: KeyboardEvent| {
            if disabled {
                return;
            }
            let key = e.key();
            if !open {
                if matches!(key, Key::ArrowDown | Key::ArrowUp | Key::Enter)
                    || key == Key::Character(" ".into())
                {
                    e.prevent_default();
                    active.set(first_selected.or_else(|| step_enabled(&disabled_flags, None, 1)));
                    set_open(true);
                }
                return;
            }
            match key {
                Key::ArrowDown => {
                    e.prevent_default();
                    active.set(step_enabled(&disabled_flags, active(), 1));
                }
                Key::ArrowUp => {
                    e.prevent_default();
                    active.set(step_enabled(&disabled_flags, active(), -1));
                }
                Key::Home => {
                    e.prevent_default();
                    active.set(step_enabled(&disabled_flags, None, 1));
                }
                Key::End => {
                    e.prevent_default();
                    active.set(step_enabled(&disabled_flags, None, -1));
                }
                Key::Enter => {
                    e.prevent_default();
                    if let Some(i) = active() {
                        choose(i);
                    }
                }
                Key::Character(c) if c == " " => {
                    e.prevent_default();
                    if let Some(i) = active() {
                        choose(i);
                    }
                }
                Key::Escape | Key::Tab => set_open(false),
                _ => {}
            }
        }
    };

    let display: Vec<String> = options
        .iter()
        .filter(|o| selected.contains(&o.value))
        .map(|o| o.label.clone())
        .collect();
    let display = display.join(", ");
    let active_id = active().filter(|_| open).map(|i| format!("{id}-opt-{i}"));
    let described_by = props.helper_text.as_ref().map(|_| helper_id.clone());
    let class = field_classes(
        "skeuo-select",
        props.variant,
        props.color,
        props.size,
        props.error,
        props.disabled,
        props.full_width,
        &classes([if open { "skeuo-select--open" } else { "" }, &props.class]),
    );
    let trigger_class = classes([control_classes(props.variant), "skeuo-select__trigger"]);
    let labelledby = if props.label.is_some() {
        format!("{label_id} {id}")
    } else {
        id.clone()
    };
    let disabled = props.disabled;
    let mut toggle_open = set_open;
    let mut close_on_blur = set_open;

    rsx! {
        div { class, style: props.style,
            if let Some(label) = props.label.clone() {
                label { class: "skeuo-field__label skeuo-label", id: label_id.clone(),
                    "{label}"
                    if props.required {
                        span { class: "skeuo-field__asterisk", aria_hidden: "true", " *" }
                    }
                }
            }
            div { class: "skeuo-select__anchor",
                div {
                    class: trigger_class,
                    id: id.clone(),
                    role: "combobox",
                    tabindex: if disabled { "-1" } else { "0" },
                    aria_haspopup: "listbox",
                    aria_expanded: open,
                    aria_controls: listbox_id.clone(),
                    aria_labelledby: labelledby,
                    aria_activedescendant: active_id,
                    aria_describedby: described_by,
                    aria_invalid: props.error,
                    aria_required: props.required,
                    aria_disabled: disabled,
                    onclick: move |_| {
                        if !disabled {
                            active.set(first_selected);
                            toggle_open(!open);
                        }
                    },
                    onkeydown,
                    onfocusout: move |_| {
                        if open {
                            close_on_blur(false);
                        }
                    },
                    if display.is_empty() {
                        span { class: "skeuo-select__value skeuo-select__placeholder",
                            {props.placeholder.clone().unwrap_or_else(|| "\u{200b}".into())}
                        }
                    } else {
                        span { class: "skeuo-select__value", "{display}" }
                    }
                    Icon { name: IconName::ChevronDown, class: "skeuo-select__chevron" }
                }
                if open {
                    ul {
                        class: "skeuo-select__listbox skeuo-chassis skeuo-chassis--float skeuo-elev-3",
                        id: listbox_id.clone(),
                        role: "listbox",
                        aria_labelledby: if props.label.is_some() { label_id.clone() } else { id.clone() },
                        aria_multiselectable: if multiple { "true" },
                        onmousedown: |e| e.prevent_default(),
                        for (i, opt) in options.iter().enumerate() {
                            SelectRow {
                                key: "{opt.value}",
                                id: format!("{id}-opt-{i}"),
                                label: opt.label.clone(),
                                selected: selected.contains(&opt.value),
                                active: active() == Some(i),
                                disabled: opt.disabled,
                                onpick: {
                                    let mut choose = choose.clone();
                                    move |_| choose(i)
                                },
                                onhover: move |_| active.set(Some(i)),
                            }
                        }
                    }
                }
            }
            if let Some(name) = props.name.clone() {
                input { r#type: "hidden", name, value: selected.join(",") }
            }
            if let Some(text) = props.helper_text.clone() {
                p { class: "skeuo-field__helper skeuo-caption", id: helper_id, "{text}" }
            }
        }
    }
}

/// One row of a select / autocomplete listbox.
#[component]
pub(crate) fn SelectRow(
    id: String,
    label: String,
    selected: bool,
    active: bool,
    disabled: bool,
    onpick: EventHandler<()>,
    onhover: EventHandler<()>,
) -> Element {
    let class = classes([
        "skeuo-select__option",
        if selected {
            "skeuo-select__option--selected"
        } else {
            ""
        },
        if active {
            "skeuo-select__option--active"
        } else {
            ""
        },
        if disabled {
            "skeuo-select__option--disabled"
        } else {
            ""
        },
    ]);
    rsx! {
        li {
            class,
            id,
            role: "option",
            aria_selected: selected,
            aria_disabled: if disabled { "true" },
            onclick: move |_| {
                if !disabled {
                    onpick.call(())
                }
            },
            onmouseenter: move |_| onhover.call(()),
            span { class: "skeuo-select__check",
                if selected {
                    Icon { name: IconName::Check }
                }
            }
            span { class: "skeuo-select__option-label", "{label}" }
        }
    }
}

/// A native `<select>` sitting in a well, with a chevron printed at its
/// end. Mirrors MUI's `<NativeSelect>` (with the label / helper text of a
/// `TextField select SelectProps={{ native: true }}`).
///
/// The open list is the platform's own. Options are passed as data (see
/// [`SelectOption`]).
#[derive(Props, Clone, PartialEq)]
pub struct NativeSelectProps {
    #[props(into)]
    pub label: Option<String>,
    pub options: Vec<SelectOption>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    /// Called with the chosen option's value.
    pub onchange: Option<EventHandler<String>>,
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
    #[props(into)]
    pub helper_text: Option<String>,
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
pub fn NativeSelect(props: NativeSelectProps) -> Element {
    let generated = use_unique_id("skeuo-native-select");
    let id = props.id.clone().unwrap_or(generated);
    let helper_id = format!("{id}-helper");
    let default_value = use_hook(|| props.default_value.clone());
    let value = props.value.clone().or(default_value);
    let class = field_classes(
        "skeuo-select skeuo-native-select",
        props.variant,
        props.color,
        props.size,
        props.error,
        props.disabled,
        props.full_width,
        &props.class,
    );
    let onchange = props.onchange;
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
            div { class: classes([control_classes(props.variant), "skeuo-select__trigger"]),
                select {
                    class: "skeuo-native-select__input",
                    id: id.clone(),
                    name: props.name.clone(),
                    disabled: props.disabled,
                    required: props.required,
                    aria_invalid: props.error,
                    aria_describedby: described_by.clone(),
                    onchange: move |e: FormEvent| {
                        if let Some(h) = onchange {
                            h.call(e.value());
                        }
                    },
                    for opt in props.options.iter() {
                        option {
                            key: "{opt.value}",
                            value: opt.value.clone(),
                            disabled: opt.disabled,
                            selected: value.as_deref() == Some(opt.value.as_str()),
                            "{opt.label}"
                        }
                    }
                }
                Icon { name: IconName::ChevronDown, class: "skeuo-select__chevron" }
            }
            if let Some(text) = props.helper_text.clone() {
                p { class: "skeuo-field__helper skeuo-caption", id: helper_id, "{text}" }
            }
        }
    }
}
