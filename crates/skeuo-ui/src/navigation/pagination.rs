use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Size, classes};

/// Look of the page keys, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum PaginationVariant {
    /// Printed numbers; only the current page is a (latched, colored) keycap.
    #[default]
    Text,
    /// Every page is a small white keycap; the current one is latched down
    /// and colored.
    Outlined,
}

impl PaginationVariant {
    fn as_str(self) -> &'static str {
        match self {
            PaginationVariant::Text => "text",
            PaginationVariant::Outlined => "outlined",
        }
    }
}

/// Corner shape of the page keys, mirroring MUI's `shape`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum PaginationShape {
    #[default]
    Circular,
    Rounded,
}

impl PaginationShape {
    fn as_str(self) -> &'static str {
        match self {
            PaginationShape::Circular => "circular",
            PaginationShape::Rounded => "rounded",
        }
    }
}

/// What a [`PaginationItem`] stands for (MUI's `type`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum PaginationItemType {
    Page(usize),
    First,
    Previous,
    Next,
    Last,
    StartEllipsis,
    EndEllipsis,
}

/// The items a pagination bar shows, as MUI's `usePagination` computes them:
/// first/previous buttons, `boundary_count` pages at each end,
/// `siblings_count` pages on each side of `page`, ellipses for the gaps,
/// then next/last buttons.
#[allow(clippy::too_many_arguments)]
pub fn pagination_items(
    count: usize,
    page: usize,
    siblings_count: usize,
    boundary_count: usize,
    show_first_button: bool,
    show_last_button: bool,
    hide_prev_button: bool,
    hide_next_button: bool,
) -> Vec<PaginationItemType> {
    let (count, page) = (count as i64, page as i64);
    let (sib, bnd) = (siblings_count as i64, boundary_count as i64);
    let range = |a: i64, b: i64| (a..=b).collect::<Vec<i64>>();

    let start_pages = range(1, bnd.min(count));
    let end_pages = range((count - bnd + 1).max(bnd + 1), count);
    let siblings_start = (page - sib).min(count - bnd - sib * 2 - 1).max(bnd + 2);
    let siblings_end = (page + sib)
        .max(bnd + sib * 2 + 2)
        .min(end_pages.first().map(|p| p - 2).unwrap_or(count - 1));

    let mut items = Vec::new();
    if show_first_button {
        items.push(PaginationItemType::First);
    }
    if !hide_prev_button {
        items.push(PaginationItemType::Previous);
    }
    let page_item = |p: i64| PaginationItemType::Page(p as usize);
    items.extend(start_pages.into_iter().map(page_item));
    if siblings_start > bnd + 2 {
        items.push(PaginationItemType::StartEllipsis);
    } else if bnd + 1 < count - bnd {
        items.push(page_item(bnd + 1));
    }
    items.extend(
        range(siblings_start, siblings_end)
            .into_iter()
            .map(page_item),
    );
    if siblings_end < count - bnd - 1 {
        items.push(PaginationItemType::EndEllipsis);
    } else if count - bnd > bnd {
        items.push(page_item(count - bnd));
    }
    items.extend(end_pages.into_iter().map(page_item));
    if !hide_next_button {
        items.push(PaginationItemType::Next);
    }
    if show_last_button {
        items.push(PaginationItemType::Last);
    }
    items
}

/// A row of small page keycaps; the current page is latched down in the
/// palette color, gaps are printed ellipses. Mirrors MUI's `<Pagination>`
/// (a `<nav>` of buttons, the current one `aria-current="page"`).
///
/// Controlled with `page` + `onchange`, uncontrolled with `default_page`.
/// Pages are 1-based, as in MUI.
#[derive(Props, Clone, PartialEq)]
pub struct PaginationProps {
    #[props(default = 1)]
    pub count: usize,
    pub page: Option<usize>,
    #[props(default = 1)]
    pub default_page: usize,
    pub onchange: Option<EventHandler<usize>>,
    #[props(default = 1)]
    pub siblings_count: usize,
    #[props(default = 1)]
    pub boundary_count: usize,
    #[props(default)]
    pub variant: PaginationVariant,
    #[props(default)]
    pub shape: PaginationShape,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default)]
    pub show_first_button: bool,
    #[props(default)]
    pub show_last_button: bool,
    #[props(default)]
    pub hide_prev_button: bool,
    #[props(default)]
    pub hide_next_button: bool,
    #[props(default)]
    pub disabled: bool,
    #[props(default = "pagination navigation".to_string(), into)]
    pub aria_label: String,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn Pagination(props: PaginationProps) -> Element {
    let default_page = use_hook(|| props.default_page);
    let mut internal = use_signal(|| default_page);
    let page = props
        .page
        .unwrap_or(*internal.read())
        .clamp(1, props.count.max(1));
    let controlled = props.page.is_some();
    let onchange = props.onchange;
    let count = props.count;
    let mut go = move |p: usize| {
        let p = p.clamp(1, count.max(1));
        if !controlled {
            internal.set(p);
        }
        if let Some(h) = &onchange {
            h.call(p);
        }
    };

    let items = pagination_items(
        props.count,
        page,
        props.siblings_count,
        props.boundary_count,
        props.show_first_button,
        props.show_last_button,
        props.hide_prev_button,
        props.hide_next_button,
    );

    rsx! {
        nav {
            class: classes(["skeuo-pagination", &props.class]),
            aria_label: props.aria_label,
            ul { class: "skeuo-pagination__list",
                for (i, item) in items.into_iter().enumerate() {
                    li { key: "{i}",
                        {
                            let (target, disabled) = match item {
                                PaginationItemType::Page(p) => (p, props.disabled),
                                PaginationItemType::First => (1, props.disabled || page <= 1),
                                PaginationItemType::Previous => (page.saturating_sub(1), props.disabled || page <= 1),
                                PaginationItemType::Next => (page + 1, props.disabled || page >= props.count),
                                PaginationItemType::Last => (props.count, props.disabled || page >= props.count),
                                _ => (0, props.disabled),
                            };
                            rsx! {
                                PaginationItem {
                                    r#type: item,
                                    page: target,
                                    selected: item == PaginationItemType::Page(page),
                                    disabled,
                                    variant: props.variant,
                                    shape: props.shape,
                                    color: props.color,
                                    size: props.size,
                                    onclick: move |_| go(target),
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/// One key of a [`Pagination`] bar: a page number, an arrow, or a printed
/// ellipsis. Mirrors MUI's `<PaginationItem>`.
#[derive(Props, Clone, PartialEq)]
pub struct PaginationItemProps {
    #[props(default = PaginationItemType::Page(1))]
    pub r#type: PaginationItemType,
    /// The page this item leads to.
    #[props(default)]
    pub page: usize,
    #[props(default)]
    pub selected: bool,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub variant: PaginationVariant,
    #[props(default)]
    pub shape: PaginationShape,
    #[props(default)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
}

#[component]
pub fn PaginationItem(props: PaginationItemProps) -> Element {
    let variant = format!("skeuo-pagination-item--{}", props.variant.as_str());
    let shape = format!("skeuo-pagination-item--{}", props.shape.as_str());
    let size = format!("skeuo-pagination-item--{}", props.size.as_str());

    if matches!(
        props.r#type,
        PaginationItemType::StartEllipsis | PaginationItemType::EndEllipsis
    ) {
        return rsx! {
            div {
                class: classes(["skeuo-pagination-item", "skeuo-pagination-item--ellipsis", &size, &props.class]),
                aria_hidden: "true",
                "…"
            }
        };
    }

    // White keycaps for the outlined variant and the arrows; the current page
    // is a colored keycap latched down; text-variant pages are printed.
    let is_page = matches!(props.r#type, PaginationItemType::Page(_));
    let material = if props.selected {
        "skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--down"
    } else if props.variant == PaginationVariant::Outlined || !is_page {
        "skeuo-key skeuo-key--pressable"
    } else {
        ""
    };
    let class = classes([
        material,
        "skeuo-pagination-item",
        &variant,
        &shape,
        &size,
        props.color.class(),
        if props.selected {
            "skeuo-pagination-item--selected"
        } else {
            ""
        },
        &props.class,
    ]);
    let (label, icon) = match props.r#type {
        PaginationItemType::Page(p) => (
            if props.selected {
                format!("page {p}")
            } else {
                format!("Go to page {p}")
            },
            None,
        ),
        PaginationItemType::First => ("Go to first page".into(), Some(IconName::FirstPage)),
        PaginationItemType::Previous => ("Go to previous page".into(), Some(IconName::ChevronLeft)),
        PaginationItemType::Next => ("Go to next page".into(), Some(IconName::ChevronRight)),
        PaginationItemType::Last => ("Go to last page".into(), Some(IconName::LastPage)),
        _ => (String::new(), None),
    };
    rsx! {
        button {
            class,
            r#type: "button",
            aria_label: label,
            aria_current: props.selected.then_some("page"),
            disabled: props.disabled,
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            if let Some(icon) = icon {
                Icon { name: icon }
            } else {
                "{props.page}"
            }
        }
    }
}
