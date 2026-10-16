use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Size, classes};

/// Shared by a table and its cells (MUI's `TableContext`).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
struct TableContext {
    size: Size,
    sticky_header: bool,
    padding: TableCellPadding,
}

/// Which section a row/cell belongs to (MUI's `Tablelvl2Context`).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
enum TableSection {
    Head,
    #[default]
    Body,
    Footer,
}

fn flag(on: bool, class: &'static str) -> &'static str {
    if on { class } else { "" }
}

/// The plate a table sits on. Mirrors MUI's `<TableContainer component={Paper}>`.
///
/// An aluminium chassis (resting, elevation 1 by default) that scrolls its
/// table when it overflows.
#[derive(Props, Clone, PartialEq)]
pub struct TableContainerProps {
    /// Chassis elevation 0–4.
    #[props(default = 1)]
    pub elevation: u8,
    /// CSS max-height, e.g. `"320px"`, for a scrolling (sticky header) table.
    #[props(into)]
    pub max_height: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn TableContainer(props: TableContainerProps) -> Element {
    let elev = format!("skeuo-elev-{}", props.elevation.min(4));
    let class = classes([
        "skeuo-chassis",
        &elev,
        "skeuo-table-container",
        &props.class,
    ]);
    let style = match &props.max_height {
        Some(h) => format!("max-height: {h};{}", props.style),
        None => props.style.clone(),
    };
    rsx! {
        div { class, style: super::non_empty(style), {props.children} }
    }
}

/// A data table. Mirrors MUI's `<Table>`.
///
/// The head row is printed lettering on the aluminium; rows are separated by
/// engraved rules; hovered rows take the `--sk-hover` wash and selected rows
/// the `--c-tint` of `color`.
#[derive(Props, Clone, PartialEq)]
pub struct TableProps {
    /// `Small` gives compact rows; `Large` is treated as medium.
    #[props(default)]
    pub size: Size,
    /// Keep the head row visible while the container scrolls.
    #[props(default)]
    pub sticky_header: bool,
    /// Default padding of every cell.
    #[props(default)]
    pub padding: TableCellPadding,
    /// Palette color of selected rows.
    #[props(default)]
    pub color: Color,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Table(props: TableProps) -> Element {
    use_context_provider(|| TableContext {
        size: props.size,
        sticky_header: props.sticky_header,
        padding: props.padding,
    });
    let class = classes([
        "skeuo-table",
        props.color.class(),
        if props.size == Size::Small {
            "skeuo-table--small"
        } else {
            "skeuo-table--medium"
        },
        flag(props.sticky_header, "skeuo-table--sticky-header"),
        &props.class,
    ]);
    rsx! {
        table { class, aria_label: props.aria_label, {props.children} }
    }
}

macro_rules! section {
    ($(#[$doc:meta])* $name:ident, $props:ident, $tag:ident, $section:expr, $class:literal) => {
        $(#[$doc])*
        #[derive(Props, Clone, PartialEq)]
        pub struct $props {
            #[props(default, into)]
            pub class: String,
            pub children: Element,
        }

        $(#[$doc])*
        #[component]
        pub fn $name(props: $props) -> Element {
            use_context_provider(|| $section);
            rsx! {
                $tag { class: classes([$class, &props.class]), {props.children} }
            }
        }
    };
}

section!(
    /// The head rows of a table; its cells are `<th scope="col">`.
    /// Mirrors MUI's `<TableHead>`.
    TableHead,
    TableHeadProps,
    thead,
    TableSection::Head,
    "skeuo-table-head"
);
section!(
    /// The body rows of a table. Mirrors MUI's `<TableBody>`.
    TableBody,
    TableBodyProps,
    tbody,
    TableSection::Body,
    "skeuo-table-body"
);
section!(
    /// The footer rows of a table (totals, pagination).
    /// Mirrors MUI's `<TableFooter>`.
    TableFooter,
    TableFooterProps,
    tfoot,
    TableSection::Footer,
    "skeuo-table-footer"
);

/// A table row. Mirrors MUI's `<TableRow>`.
#[derive(Props, Clone, PartialEq)]
pub struct TableRowProps {
    /// Wash the row on hover.
    #[props(default)]
    pub hover: bool,
    /// Tint the row and set `aria-selected`.
    #[props(default)]
    pub selected: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn TableRow(props: TableRowProps) -> Element {
    let section = try_use_context::<TableSection>().unwrap_or_default();
    let class = classes([
        "skeuo-table-row",
        match section {
            TableSection::Head => "skeuo-table-row--head",
            TableSection::Footer => "skeuo-table-row--footer",
            TableSection::Body => "",
        },
        flag(props.hover, "skeuo-table-row--hover"),
        flag(props.selected, "skeuo-table-row--selected"),
        flag(props.onclick.is_some(), "skeuo-table-row--clickable"),
        &props.class,
    ]);
    rsx! {
        tr {
            class,
            aria_selected: if props.selected { "true" },
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            {props.children}
        }
    }
}

/// Text alignment of a cell, as in MUI's `align`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TableCellAlign {
    #[default]
    Inherit,
    Left,
    Center,
    /// Use for numbers.
    Right,
    Justify,
}

/// Cell padding, as in MUI's `padding`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TableCellPadding {
    #[default]
    Normal,
    /// Tight cell for a leading checkbox.
    Checkbox,
    None,
}

/// Cell style, as in MUI's `variant`.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum TableCellVariant {
    Head,
    Body,
    Footer,
}

/// Sort order, as in MUI's `sortDirection` / `direction`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SortDirection {
    #[default]
    Asc,
    Desc,
}

impl SortDirection {
    /// The other direction.
    pub fn toggled(self) -> Self {
        match self {
            SortDirection::Asc => SortDirection::Desc,
            SortDirection::Desc => SortDirection::Asc,
        }
    }
}

/// A table cell: `<th>` in the head (or with `variant: Head`), `<td>`
/// otherwise. Mirrors MUI's `<TableCell>`.
#[derive(Props, Clone, PartialEq)]
pub struct TableCellProps {
    #[props(default)]
    pub align: TableCellAlign,
    /// Overrides the table's padding.
    pub padding: Option<TableCellPadding>,
    /// Overrides the style implied by the section.
    pub variant: Option<TableCellVariant>,
    /// Overrides the table's size.
    pub size: Option<Size>,
    /// Sets `aria-sort` on the head cell of the sorted column.
    pub sort_direction: Option<SortDirection>,
    /// HTML `scope`; head cells default to `"col"`.
    #[props(into)]
    pub scope: Option<String>,
    pub col_span: Option<u32>,
    pub row_span: Option<u32>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn TableCell(props: TableCellProps) -> Element {
    let table = try_use_context::<TableContext>().unwrap_or_default();
    let section = try_use_context::<TableSection>().unwrap_or_default();
    let variant = props.variant.unwrap_or(match section {
        TableSection::Head => TableCellVariant::Head,
        TableSection::Body => TableCellVariant::Body,
        TableSection::Footer => TableCellVariant::Footer,
    });
    let size = props.size.unwrap_or(table.size);
    let padding = props.padding.unwrap_or(table.padding);
    let class = classes([
        "skeuo-table-cell",
        match variant {
            TableCellVariant::Head => "skeuo-table-cell--head",
            TableCellVariant::Body => "skeuo-table-cell--body",
            TableCellVariant::Footer => "skeuo-table-cell--footer",
        },
        match props.align {
            TableCellAlign::Inherit => "",
            TableCellAlign::Left => "skeuo-table-cell--align-left",
            TableCellAlign::Center => "skeuo-table-cell--align-center",
            TableCellAlign::Right => "skeuo-table-cell--align-right",
            TableCellAlign::Justify => "skeuo-table-cell--align-justify",
        },
        match padding {
            TableCellPadding::Normal => "",
            TableCellPadding::Checkbox => "skeuo-table-cell--padding-checkbox",
            TableCellPadding::None => "skeuo-table-cell--padding-none",
        },
        flag(size == Size::Small, "skeuo-table-cell--small"),
        flag(
            table.sticky_header && section == TableSection::Head,
            "skeuo-table-cell--sticky",
        ),
        &props.class,
    ]);
    let aria_sort = props.sort_direction.map(|d| match d {
        SortDirection::Asc => "ascending",
        SortDirection::Desc => "descending",
    });
    let colspan = props.col_span.map(|n| n.to_string());
    let rowspan = props.row_span.map(|n| n.to_string());

    if variant == TableCellVariant::Head {
        let scope = props.scope.clone().unwrap_or_else(|| {
            if section == TableSection::Head {
                "col".to_string()
            } else {
                "row".to_string()
            }
        });
        rsx! {
            th {
                class,
                style: super::non_empty(props.style),
                scope,
                colspan,
                rowspan,
                aria_sort,
                {props.children}
            }
        }
    } else {
        rsx! {
            td {
                class,
                style: super::non_empty(props.style),
                "scope": props.scope,
                colspan,
                rowspan,
                {props.children}
            }
        }
    }
}

/// A sortable column heading: lettering plus a direction arrow. Mirrors
/// MUI's `<TableSortLabel>`. The active column's arrow is printed in full
/// ink; inactive columns show a faint arrow on hover.
#[derive(Props, Clone, PartialEq)]
pub struct TableSortLabelProps {
    #[props(default)]
    pub active: bool,
    #[props(default)]
    pub direction: SortDirection,
    #[props(default)]
    pub hide_sort_icon: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn TableSortLabel(props: TableSortLabelProps) -> Element {
    let class = classes([
        "skeuo-table-sort-label",
        flag(props.active, "skeuo-table-sort-label--active"),
        match props.direction {
            SortDirection::Asc => "skeuo-table-sort-label--asc",
            SortDirection::Desc => "skeuo-table-sort-label--desc",
        },
        &props.class,
    ]);
    let show_icon = !props.hide_sort_icon || props.active;
    rsx! {
        button {
            class,
            r#type: "button",
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            {props.children}
            if show_icon {
                Icon { name: IconName::ArrowForward, class: "skeuo-table-sort-label__icon" }
            }
        }
    }
}
