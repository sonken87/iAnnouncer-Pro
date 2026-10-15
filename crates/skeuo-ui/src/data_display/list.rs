use dioxus::prelude::*;

use super::is_empty;
use crate::{Color, classes};

/// Shared by a list and its rows (MUI's `ListContext`).
#[derive(Clone, Copy, Debug, Default, PartialEq)]
struct ListContext {
    dense: bool,
}

fn flag(on: bool, class: &'static str) -> &'static str {
    if on { class } else { "" }
}

/// A vertical list of rows. Mirrors MUI's `<List>`.
///
/// Rows are printed on whatever surface holds the list (usually a chassis);
/// selected rows take a `--c-tint` wash and `divider` rows are separated by
/// engraved rules.
#[derive(Props, Clone, PartialEq)]
pub struct ListProps {
    /// Compact rows for every item.
    #[props(default)]
    pub dense: bool,
    /// Drop the top/bottom padding.
    #[props(default)]
    pub disable_padding: bool,
    /// Rendered first, usually a [`ListSubheader`].
    pub subheader: Option<Element>,
    /// Palette color of selected rows.
    #[props(default)]
    pub color: Color,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn List(props: ListProps) -> Element {
    use_context_provider(|| ListContext { dense: props.dense });
    let class = classes([
        "skeuo-list",
        props.color.class(),
        flag(props.dense, "skeuo-list--dense"),
        flag(props.disable_padding, "skeuo-list--no-padding"),
        flag(props.subheader.is_some(), "skeuo-list--with-subheader"),
        &props.class,
    ]);
    rsx! {
        ul { class, style: super::non_empty(props.style), aria_label: props.aria_label,
            if let Some(subheader) = props.subheader {
                {subheader}
            }
            {props.children}
        }
    }
}

/// A row of a [`List`]. Mirrors MUI's `<ListItem>`.
///
/// Wrap a [`ListItemButton`] in a `ListItem { disable_padding: true }` for a
/// clickable row, as in MUI.
#[derive(Props, Clone, PartialEq)]
pub struct ListItemProps {
    #[props(default)]
    pub dense: bool,
    #[props(default)]
    pub disable_gutters: bool,
    #[props(default)]
    pub disable_padding: bool,
    /// Engraved rule under the row.
    #[props(default)]
    pub divider: bool,
    /// Align the icon/avatar to the top (multi-line rows).
    #[props(default)]
    pub align_items_start: bool,
    /// Placed at the end of the row, e.g. an icon button or a switch.
    pub secondary_action: Option<Element>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn ListItem(props: ListItemProps) -> Element {
    let ctx = try_use_context::<ListContext>().unwrap_or_default();
    let class = classes([
        "skeuo-list-item",
        flag(props.dense || ctx.dense, "skeuo-list-item--dense"),
        flag(props.disable_gutters, "skeuo-list-item--no-gutters"),
        flag(props.disable_padding, "skeuo-list-item--no-padding"),
        flag(props.divider, "skeuo-list-item--divider"),
        flag(props.align_items_start, "skeuo-list-item--align-start"),
        flag(
            props.secondary_action.is_some(),
            "skeuo-list-item--with-action",
        ),
        &props.class,
    ]);
    rsx! {
        li { class,
            {props.children}
            if let Some(action) = props.secondary_action {
                span { class: "skeuo-list-item__secondary-action", {action} }
            }
        }
    }
}

/// A pressable row. Mirrors MUI's `<ListItemButton>`.
///
/// Printed lettering on the aluminium: hover washes it with `--sk-hover`,
/// pressing with `--sk-press`, and `selected` with the `--c-tint` of the
/// list's color. A native `<button>`; `selected` sets `aria-current`.
#[derive(Props, Clone, PartialEq)]
pub struct ListItemButtonProps {
    #[props(default)]
    pub selected: bool,
    #[props(default)]
    pub disabled: bool,
    #[props(default)]
    pub dense: bool,
    #[props(default)]
    pub disable_gutters: bool,
    /// Engraved rule under the row.
    #[props(default)]
    pub divider: bool,
    #[props(default)]
    pub autofocus: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn ListItemButton(props: ListItemButtonProps) -> Element {
    let ctx = try_use_context::<ListContext>().unwrap_or_default();
    let class = classes([
        "skeuo-list-item-button",
        flag(props.dense || ctx.dense, "skeuo-list-item--dense"),
        flag(props.disable_gutters, "skeuo-list-item--no-gutters"),
        flag(props.divider, "skeuo-list-item--divider"),
        flag(props.selected, "skeuo-list-item-button--selected"),
        &props.class,
    ]);
    rsx! {
        button {
            class,
            r#type: "button",
            disabled: props.disabled,
            autofocus: props.autofocus,
            aria_current: if props.selected { "true" },
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            {props.children}
        }
    }
}

/// The leading icon of a row, in secondary ink. Mirrors MUI's `<ListItemIcon>`.
#[derive(Props, Clone, PartialEq)]
pub struct ListItemIconProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn ListItemIcon(props: ListItemIconProps) -> Element {
    rsx! {
        span { class: classes(["skeuo-list-item-icon", &props.class]), {props.children} }
    }
}

/// The leading avatar of a row. Mirrors MUI's `<ListItemAvatar>`.
#[derive(Props, Clone, PartialEq)]
pub struct ListItemAvatarProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn ListItemAvatar(props: ListItemAvatarProps) -> Element {
    rsx! {
        span { class: classes(["skeuo-list-item-avatar", &props.class]), {props.children} }
    }
}

/// Primary and secondary lettering of a row. Mirrors MUI's `<ListItemText>`.
/// Children, if given, replace `primary`.
#[derive(Props, Clone, PartialEq)]
pub struct ListItemTextProps {
    #[props(into)]
    pub primary: Option<String>,
    #[props(into)]
    pub secondary: Option<String>,
    /// Indent as if there were a leading icon.
    #[props(default)]
    pub inset: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn ListItemText(props: ListItemTextProps) -> Element {
    let class = classes([
        "skeuo-list-item-text",
        flag(props.inset, "skeuo-list-item-text--inset"),
        flag(props.secondary.is_some(), "skeuo-list-item-text--multiline"),
        &props.class,
    ]);
    let has_children = !is_empty(&props.children);
    rsx! {
        span { class,
            if has_children {
                span { class: "skeuo-list-item-text__primary", {props.children} }
            } else if let Some(primary) = props.primary {
                span { class: "skeuo-list-item-text__primary", "{primary}" }
            }
            if let Some(secondary) = props.secondary {
                span { class: "skeuo-list-item-text__secondary skeuo-caption", "{secondary}" }
            }
        }
    }
}

/// A section title inside a list, printed in label lettering and sticky
/// while its section scrolls. Mirrors MUI's `<ListSubheader>`.
#[derive(Props, Clone, PartialEq)]
pub struct ListSubheaderProps {
    /// Palette ink for the lettering; `None` is the secondary ink.
    pub color: Option<Color>,
    #[props(default)]
    pub disable_sticky: bool,
    #[props(default)]
    pub disable_gutters: bool,
    #[props(default)]
    pub inset: bool,
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn ListSubheader(props: ListSubheaderProps) -> Element {
    let class = classes([
        "skeuo-list-subheader",
        "skeuo-label",
        props.color.map(Color::class).unwrap_or(""),
        flag(props.color.is_some(), "skeuo-list-subheader--colored"),
        flag(!props.disable_sticky, "skeuo-list-subheader--sticky"),
        flag(props.disable_gutters, "skeuo-list-subheader--no-gutters"),
        flag(props.inset, "skeuo-list-subheader--inset"),
        &props.class,
    ]);
    rsx! {
        li { class, id: props.id, {props.children} }
    }
}
