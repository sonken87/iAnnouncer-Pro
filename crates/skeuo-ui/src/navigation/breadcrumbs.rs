use dioxus::prelude::*;

use crate::{Icon, IconName, classes};

/// A path of printed links on the aluminium, separated by a printed
/// separator. When there are more than `max_items`, the middle collapses
/// into a small "…" keycap that expands the path when pressed. Mirrors
/// MUI's `<Breadcrumbs>`.
///
/// Dioxus cannot count an element's children, so the crumbs are passed as
/// `items` (usually [`Link`](crate::Link)s, with plain text for the current
/// page). The last item is marked `aria-current="page"`.
#[derive(Props, Clone, PartialEq)]
pub struct BreadcrumbsProps {
    pub items: Vec<Element>,
    /// Printed between items; a chevron by default.
    pub separator: Option<Element>,
    /// Collapse the path when it has more items than this.
    #[props(default = 8)]
    pub max_items: usize,
    /// Items kept before the "…" when collapsed.
    #[props(default = 1)]
    pub items_before_collapse: usize,
    /// Items kept after the "…" when collapsed.
    #[props(default = 1)]
    pub items_after_collapse: usize,
    /// Accessible name of the "…" keycap.
    #[props(default = "Show path".to_string(), into)]
    pub expand_text: String,
    #[props(default = "breadcrumb".to_string(), into)]
    pub aria_label: String,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn Breadcrumbs(props: BreadcrumbsProps) -> Element {
    let mut expanded = use_signal(|| false);
    let n = props.items.len();
    let collapse = !expanded()
        && n > props.max_items
        && props.items_before_collapse + props.items_after_collapse < n;

    // (original index, item) pairs, with `None` marking the "…" keycap.
    let mut shown: Vec<Option<(usize, Element)>> = Vec::new();
    if collapse {
        for (i, item) in props
            .items
            .iter()
            .enumerate()
            .take(props.items_before_collapse)
        {
            shown.push(Some((i, item.clone())));
        }
        shown.push(None);
        let start = n - props.items_after_collapse;
        for (i, item) in props.items.iter().enumerate().skip(start) {
            shown.push(Some((i, item.clone())));
        }
    } else {
        shown.extend(props.items.iter().cloned().enumerate().map(Some));
    }
    let separator = props.separator.clone();
    let count = shown.len();

    rsx! {
        nav {
            class: classes(["skeuo-breadcrumbs", &props.class]),
            aria_label: props.aria_label,
            ol { class: "skeuo-breadcrumbs__list",
                for (pos, entry) in shown.into_iter().enumerate() {
                    match entry {
                        Some((i, item)) => rsx! {
                            li {
                                key: "{i}",
                                class: if i + 1 == n { "skeuo-breadcrumbs__item skeuo-breadcrumbs__item--current" } else { "skeuo-breadcrumbs__item" },
                                aria_current: if i + 1 == n { "page" },
                                {item}
                            }
                        },
                        None => rsx! {
                            li { key: "collapsed-{pos}", class: "skeuo-breadcrumbs__item",
                                button {
                                    class: "skeuo-key skeuo-key--pressable skeuo-breadcrumbs__collapsed",
                                    r#type: "button",
                                    aria_label: props.expand_text.clone(),
                                    onclick: move |_| expanded.set(true),
                                    Icon { name: IconName::MoreHoriz }
                                }
                            }
                        },
                    }
                    if pos + 1 < count {
                        li {
                            key: "sep-{pos}",
                            class: "skeuo-breadcrumbs__separator",
                            aria_hidden: "true",
                            if let Some(sep) = separator.clone() {
                                {sep}
                            } else {
                                Icon { name: IconName::ChevronRight }
                            }
                        }
                    }
                }
            }
        }
    }
}
