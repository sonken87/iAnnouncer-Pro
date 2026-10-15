use dioxus::prelude::*;

use crate::{Color, Icon, IconName, classes, use_unique_id};

use super::{Button, ButtonVariant};

/// A small checkbox keycap for list rows. Private: the public `Checkbox`
/// lives in the selection module.
#[component]
fn RowCheck(
    checked: bool,
    #[props(default)] indeterminate: bool,
    #[props(default)] disabled: bool,
    #[props(into)] label: Option<String>,
    #[props(into)] labelledby: Option<String>,
    onchange: EventHandler<()>,
) -> Element {
    let on = checked || indeterminate;
    let cap = classes([
        "skeuo-transfer__cap skeuo-key",
        if on { "skeuo-key--filled" } else { "" },
    ]);
    rsx! {
        span { class: "skeuo-transfer__check",
            input {
                class: "skeuo-transfer__native",
                r#type: "checkbox",
                checked,
                disabled,
                aria_checked: if indeterminate { "mixed" } else if checked { "true" } else { "false" },
                aria_label: label,
                aria_labelledby: labelledby,
                onchange: move |_| onchange.call(()),
            }
            span { class: cap, aria_hidden: "true",
                if indeterminate {
                    Icon { name: IconName::Remove }
                } else if checked {
                    Icon { name: IconName::Check }
                }
            }
        }
    }
}

/// Two wells of checkable rows with keycaps between them that move the
/// checked rows across. The MUI "Transfer list" pattern (MUI documents it
/// as a recipe built from `List`, `Checkbox` and `Button`, not as a
/// component; this packages that recipe).
///
/// Controlled with `left` + `right` + `onchange` (called with both lists
/// after every move), or uncontrolled with `default_left` /
/// `default_right`. `default_checked` pre-checks rows.
#[derive(Props, Clone, PartialEq)]
pub struct TransferListProps {
    pub left: Option<Vec<String>>,
    pub right: Option<Vec<String>>,
    pub default_left: Option<Vec<String>>,
    pub default_right: Option<Vec<String>>,
    #[props(default)]
    pub default_checked: Vec<String>,
    /// Called with `(left, right)` after a move.
    pub onchange: Option<EventHandler<(Vec<String>, Vec<String>)>>,
    #[props(default = "Choices".to_string(), into)]
    pub left_title: String,
    #[props(default = "Chosen".to_string(), into)]
    pub right_title: String,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub disabled: bool,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
}

#[component]
pub fn TransferList(props: TransferListProps) -> Element {
    let id = use_unique_id("skeuo-transfer");
    let mut internal_left = use_signal(|| props.default_left.clone().unwrap_or_default());
    let mut internal_right = use_signal(|| props.default_right.clone().unwrap_or_default());
    let mut checked = use_signal(|| props.default_checked.clone());
    let left = props.left.clone().unwrap_or_else(|| internal_left.cloned());
    let right = props
        .right
        .clone()
        .unwrap_or_else(|| internal_right.cloned());
    let disabled = props.disabled;
    let onchange = props.onchange;

    // Moves `items` (or the checked ones) from one side to the other.
    let mv = {
        let (left, right) = (left.clone(), right.clone());
        move |to_right: bool, all: bool| {
            let (from, to) = if to_right {
                (&left, &right)
            } else {
                (&right, &left)
            };
            let c = checked();
            let moving: Vec<String> = from
                .iter()
                .filter(|v| all || c.contains(v))
                .cloned()
                .collect();
            if moving.is_empty() {
                return;
            }
            let rest: Vec<String> = from
                .iter()
                .filter(|v| !moving.contains(v))
                .cloned()
                .collect();
            let mut grown = to.clone();
            grown.extend(moving.iter().cloned());
            let (l, r) = if to_right {
                (rest, grown)
            } else {
                (grown, rest)
            };
            checked.set(c.into_iter().filter(|v| !moving.contains(v)).collect());
            internal_left.set(l.clone());
            internal_right.set(r.clone());
            if let Some(h) = onchange {
                h.call((l, r));
            }
        }
    };

    let c = checked();
    let left_checked = left.iter().filter(|v| c.contains(v)).count();
    let right_checked = right.iter().filter(|v| c.contains(v)).count();
    let class = classes([
        "skeuo-transfer",
        props.color.class(),
        if disabled {
            "skeuo-transfer--disabled"
        } else {
            ""
        },
        &props.class,
    ]);

    let side = move |side_id: String, title: String, items: Vec<String>, n_checked: usize| {
        let title_id = format!("{side_id}-title");
        let all = !items.is_empty() && n_checked == items.len();
        let some = n_checked > 0 && !all;
        let items_for_all = items.clone();
        rsx! {
            div { class: "skeuo-transfer__side",
                div { class: "skeuo-transfer__header",
                    RowCheck {
                        checked: all,
                        indeterminate: some,
                        disabled: disabled || items.is_empty(),
                        label: format!("Select all {title}"),
                        onchange: move |_| {
                            let mut c = checked();
                            if all {
                                c.retain(|v| !items_for_all.contains(v));
                            } else {
                                for v in &items_for_all {
                                    if !c.contains(v) {
                                        c.push(v.clone());
                                    }
                                }
                            }
                            checked.set(c);
                        },
                    }
                    span { class: "skeuo-transfer__title", id: title_id.clone(), "{title}" }
                    span { class: "skeuo-caption", "{n_checked}/{items.len()} selected" }
                }
                ul {
                    class: "skeuo-transfer__list skeuo-well",
                    role: "list",
                    aria_labelledby: title_id,
                    if items.is_empty() {
                        li { class: "skeuo-transfer__empty skeuo-caption", "Empty" }
                    }
                    for (i, item) in items.iter().enumerate() {
                        TransferRow {
                            key: "{item}",
                            id: format!("{side_id}-{i}"),
                            label: item.clone(),
                            checked: c.contains(item),
                            disabled,
                            ontoggle: {
                                let item = item.clone();
                                move |_| {
                                    let mut c = checked();
                                    if let Some(p) = c.iter().position(|v| *v == item) {
                                        c.remove(p);
                                    } else {
                                        c.push(item.clone());
                                    }
                                    checked.set(c);
                                }
                            },
                        }
                    }
                }
            }
        }
    };

    let mut m1 = mv.clone();
    let mut m2 = mv.clone();
    let mut m3 = mv.clone();
    let mut m4 = mv;
    rsx! {
        div { class, style: props.style,
            {side(format!("{id}-left"), props.left_title.clone(), left.clone(), left_checked)}
            div { class: "skeuo-transfer__moves", role: "group", aria_label: "Move items",
                Button {
                    variant: ButtonVariant::Outlined,
                    color: Color::Neutral,
                    size: crate::Size::Small,
                    class: "skeuo-transfer__move",
                    disabled: disabled || left.is_empty(),
                    aria_label: "Move all right",
                    onclick: move |_| m1(true, true),
                    Icon { name: IconName::LastPage }
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    color: Color::Neutral,
                    size: crate::Size::Small,
                    class: "skeuo-transfer__move",
                    disabled: disabled || left_checked == 0,
                    aria_label: "Move selected right",
                    onclick: move |_| m2(true, false),
                    Icon { name: IconName::ChevronRight }
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    color: Color::Neutral,
                    size: crate::Size::Small,
                    class: "skeuo-transfer__move",
                    disabled: disabled || right_checked == 0,
                    aria_label: "Move selected left",
                    onclick: move |_| m3(false, false),
                    Icon { name: IconName::ChevronLeft }
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    color: Color::Neutral,
                    size: crate::Size::Small,
                    class: "skeuo-transfer__move",
                    disabled: disabled || right.is_empty(),
                    aria_label: "Move all left",
                    onclick: move |_| m4(false, true),
                    Icon { name: IconName::FirstPage }
                }
            }
            {side(format!("{id}-right"), props.right_title.clone(), right.clone(), right_checked)}
        }
    }
}

#[component]
fn TransferRow(
    id: String,
    label: String,
    checked: bool,
    disabled: bool,
    ontoggle: EventHandler<()>,
) -> Element {
    let label_id = format!("{id}-label");
    rsx! {
        li { class: if checked { "skeuo-transfer__row skeuo-transfer__row--checked" } else { "skeuo-transfer__row" },
            label { class: "skeuo-transfer__row-label",
                RowCheck { checked, disabled, labelledby: label_id.clone(), onchange: ontoggle }
                span { id: label_id, "{label}" }
            }
        }
    }
}
