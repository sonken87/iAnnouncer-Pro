use dioxus::prelude::*;

use super::form::use_form_control;
use crate::{Icon, IconName, Size, classes, use_unique_id};

/// A star rating printed on the aluminium: filled stars in the warning
/// color, empty ones as outlines in `--sk-ink-3`. Hovering previews the
/// rating. Mirrors MUI's `<Rating>`.
///
/// Interactive ratings are a radio group of visually hidden
/// `<input type="radio">`s (one per star, or per half star with
/// `precision: 0.5`); clicking the current value again clears it, as in
/// MUI. `read_only` renders a plain `role="img"` with an accessible label.
#[derive(Props, Clone, PartialEq)]
pub struct RatingProps {
    /// Controlled value (`None` = no rating).
    pub value: Option<Option<f64>>,
    pub default_value: Option<f64>,
    #[props(default = 5)]
    pub max: u32,
    /// `1.0` (whole stars) or `0.5` (half stars).
    #[props(default = 1.0)]
    pub precision: f64,
    #[props(default)]
    pub read_only: bool,
    #[props(default)]
    pub disabled: bool,
    pub size: Option<Size>,
    /// `name` of the radio inputs. Generated when omitted.
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(default, into)]
    pub class: String,
    /// Called with the new value; `None` when the rating is cleared.
    pub onchange: Option<EventHandler<Option<f64>>>,
    /// Called with the hovered value, `None` when the pointer leaves.
    pub onchange_active: Option<EventHandler<Option<f64>>>,
}

fn star_label(v: f64) -> String {
    if v == 1.0 {
        "1 Star".to_string()
    } else {
        format!("{v} Stars")
    }
}

#[component]
pub fn Rating(props: RatingProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let generated = use_unique_id("skeuo-rating");
    let name = props.name.clone().unwrap_or(generated);
    let mut internal = use_signal(|| props.default_value);
    let mut hover = use_signal(|| None::<f64>);
    let value = props.value.unwrap_or_else(|| internal.cloned());
    let disabled = props.disabled || ctx.disabled;
    let interactive = !props.read_only && !disabled;
    let size = props.size.or(ctx.size).unwrap_or_default();
    let precision = if props.precision > 0.0 && props.precision < 1.0 {
        0.5
    } else {
        1.0
    };
    let round = move |v: f64| (v / precision).round() * precision;
    let shown = round(
        if interactive { hover() } else { None }
            .or(value)
            .unwrap_or(0.0),
    );

    let size_class = format!("skeuo-rating--{}", size.as_str());
    let class = classes([
        "skeuo-rating",
        "skeuo-color-warning",
        &size_class,
        if props.read_only {
            "skeuo-rating--read-only"
        } else {
            ""
        },
        if disabled {
            "skeuo-rating--disabled"
        } else {
            ""
        },
        if interactive && hover().is_some() {
            "skeuo-rating--hovering"
        } else {
            ""
        },
        &props.class,
    ]);

    let controlled = props.value.is_some();
    let onchange = props.onchange;
    let onactive = props.onchange_active;
    let mut set_hover = move |v: Option<f64>| {
        if hover() != v {
            hover.set(v);
            if let Some(h) = &onactive {
                h.call(v);
            }
        }
    };
    let mut commit = move |v: Option<f64>| {
        if !controlled {
            internal.set(v);
        }
        if let Some(h) = &onchange {
            h.call(v);
        }
    };

    let star_items = (1..=props.max).map(|i| {
        let i = i as f64;
        let fill = ((shown - (i - 1.0)).clamp(0.0, 1.0) * 100.0).round();
        let steps: Vec<f64> = if precision < 1.0 {
            vec![i - 0.5, i]
        } else {
            vec![i]
        };
        let item_class = if shown >= i && hover().is_some_and(|h| round(h) == i) {
            "skeuo-rating__item skeuo-rating__item--active"
        } else {
            "skeuo-rating__item"
        };
        let name = name.clone();
        rsx! {
            span { key: "{i}", class: item_class,
                span { class: "skeuo-rating__icon", aria_hidden: "true",
                    Icon { name: IconName::StarBorder, class: "skeuo-rating__empty" }
                    span { class: "skeuo-rating__fill", style: "width: {fill}%;",
                        Icon { name: IconName::Star }
                    }
                }
                if !props.read_only {
                    for (n , v) in steps.into_iter().enumerate() {
                        label {
                            class: if precision < 1.0 { if n == 0 { "skeuo-rating__hit skeuo-rating__hit--first" } else { "skeuo-rating__hit skeuo-rating__hit--second" } } else { "skeuo-rating__hit" },
                            onmouseenter: move |_| {
                                if interactive {
                                    set_hover(Some(v));
                                }
                            },
                            input {
                                class: "skeuo-rating__input",
                                r#type: "radio",
                                name: name.clone(),
                                value: "{v}",
                                checked: value == Some(v),
                                disabled,
                                aria_label: star_label(v),
                                onclick: move |_| {
                                    if value == Some(v) {
                                        commit(None);
                                    }
                                },
                                onchange: move |_| commit(Some(v)),
                            }
                        }
                    }
                }
            }
        }
    });

    if props.read_only {
        let label = props.aria_label.clone().unwrap_or_else(|| match value {
            Some(v) => format!("Rated {} out of {}", round(v), props.max),
            None => format!("Not rated, out of {}", props.max),
        });
        rsx! {
            span { class, role: "img", aria_label: label, {star_items} }
        }
    } else {
        rsx! {
            span {
                class,
                role: "radiogroup",
                aria_label: props.aria_label.clone(),
                aria_labelledby: props.aria_labelledby.clone(),
                aria_disabled: disabled.then_some("true"),
                onmouseleave: move |_| set_hover(None),
                {star_items}
            }
        }
    }
}
