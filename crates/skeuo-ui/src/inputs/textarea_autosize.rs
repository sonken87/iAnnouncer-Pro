use dioxus::prelude::*;

use crate::{Color, classes};

/// A multi-line text well that grows with its content between `min_rows`
/// and `max_rows`, then scrolls. Mirrors MUI's `<TextareaAutosize>` (which
/// is unstyled in MUI; here it is the same well as [`TextField`](super::TextField)).
///
/// Growth follows the number of lines typed (the `rows` attribute) and, in
/// browsers that support CSS `field-sizing: content`, wrapped lines too.
#[derive(Props, Clone, PartialEq)]
pub struct TextareaAutosizeProps {
    #[props(default = 1)]
    pub min_rows: u32,
    pub max_rows: Option<u32>,
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    #[props(into)]
    pub placeholder: Option<String>,
    /// Color of the focus halo.
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub read_only: bool,
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub oninput: Option<EventHandler<FormEvent>>,
    pub onchange: Option<EventHandler<FormEvent>>,
    /// Accessible name when there is no visible `<label for=id>`.
    #[props(into)]
    pub aria_label: Option<String>,
}

/// Rows needed to show `text` without scrolling, ignoring soft wraps.
fn rows_for(text: &str, min: u32, max: Option<u32>) -> u32 {
    let lines = text.split('\n').count().max(1) as u32;
    let min = min.max(1);
    let rows = lines.max(min);
    match max {
        Some(max) => rows.min(max.max(min)),
        None => rows,
    }
}

#[component]
pub fn TextareaAutosize(props: TextareaAutosizeProps) -> Element {
    let default_value = use_hook(|| props.default_value.clone());
    let mut typed = use_signal(|| default_value.clone().unwrap_or_default());
    let shown = props.value.clone().or(default_value);
    let shown_text = shown.clone().unwrap_or_default();
    let measured = props.value.clone().unwrap_or_else(|| typed.cloned());
    let rows = rows_for(&measured, props.min_rows, props.max_rows);

    let min = props.min_rows.max(1);
    let mut style = format!("--ta-min-rows: {min};");
    if let Some(max) = props.max_rows {
        style.push_str(&format!(" --ta-max-rows: {};", max.max(min)));
    }
    style.push_str(&props.style);
    let class = classes([
        "skeuo-textarea skeuo-well skeuo-well--focusable",
        props.color.class(),
        if props.max_rows.is_some() {
            "skeuo-textarea--bounded"
        } else {
            ""
        },
        &props.class,
    ]);
    let (oninput, onchange) = (props.oninput, props.onchange);
    rsx! {
        textarea {
            class,
            style,
            id: props.id.clone(),
            name: props.name.clone(),
            rows: rows as i64,
            value: shown,
            placeholder: props.placeholder.clone(),
            disabled: props.disabled,
            readonly: props.read_only,
            oninput: move |e: FormEvent| {
                typed.set(e.value());
                if let Some(h) = oninput {
                    h.call(e);
                }
            },
            onchange: move |e: FormEvent| {
                if let Some(h) = onchange {
                    h.call(e);
                }
            },
            aria_label: props.aria_label.clone(),
            // Also as content: a browser parsing server-rendered HTML ignores
            // a textarea's `value` attribute.
            {shown_text}
        }
    }
}

#[cfg(test)]
mod tests {
    use super::rows_for;

    #[test]
    fn rows_are_clamped() {
        assert_eq!(rows_for("", 1, None), 1);
        assert_eq!(rows_for("a\nb\nc", 2, None), 3);
        assert_eq!(rows_for("a\nb\nc\nd\ne", 2, Some(4)), 4);
        assert_eq!(rows_for("a", 3, Some(5)), 3);
    }
}
