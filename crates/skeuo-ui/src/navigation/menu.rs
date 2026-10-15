use dioxus::prelude::*;

use super::Popover;
use super::roving::Roving;
use crate::{Icon, IconName, Placement, classes, use_unique_id};

#[derive(Clone, Copy, PartialEq)]
struct MenuCtx {
    roving: Roving,
    focused: Signal<Option<String>>,
    dense: Signal<bool>,
}

/// A pop-up menu: the [`Popover`] chassis rising in next to its anchor,
/// holding a [`MenuList`]. Hovered or focused items get a palette-colored
/// highlight with white lettering, as in macOS menus. Mirrors MUI's
/// `<Menu>`.
///
/// Focus moves to the first item when it opens; arrow keys move between
/// items, `Enter`/`Space` activate, `Escape` or a click outside calls
/// `onclose`. `inline: true` shows it statically in the page flow.
#[derive(Props, Clone, PartialEq)]
pub struct MenuProps {
    #[props(default)]
    pub open: bool,
    pub onclose: Option<EventHandler<()>>,
    /// The trigger the menu is attached to.
    pub anchor: Option<Element>,
    #[props(default)]
    pub placement: Placement,
    /// Compact rows.
    #[props(default)]
    pub dense: bool,
    /// Show the menu in the page flow (preview).
    #[props(default)]
    pub inline: bool,
    /// Id of the menu list (point the trigger's `aria-controls` at it).
    #[props(into)]
    pub id: Option<String>,
    /// Id of the trigger that labels the menu.
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Menu(props: MenuProps) -> Element {
    rsx! {
        Popover {
            open: props.open,
            onclose: props.onclose,
            anchor: props.anchor,
            placement: props.placement,
            inline: props.inline,
            auto_focus: false,
            class: classes(["skeuo-menu", &props.class]),
            MenuList {
                id: props.id,
                aria_labelledby: props.aria_labelledby,
                dense: props.dense,
                auto_focus_item: props.open && !props.inline,
                {props.children}
            }
        }
    }
}

/// The list of [`MenuItem`]s inside a menu (`role="menu"`), with arrow-key
/// navigation. On its own it sits in the page flow; put it on a chassis to
/// show a menu permanently. Mirrors MUI's `<MenuList>`.
#[derive(Props, Clone, PartialEq)]
pub struct MenuListProps {
    #[props(default)]
    pub dense: bool,
    /// Focus the first enabled item when mounted.
    #[props(default)]
    pub auto_focus_item: bool,
    #[props(into)]
    pub id: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn MenuList(props: MenuListProps) -> Element {
    let roving = use_hook(Roving::new);
    let focused = use_signal(|| None::<String>);
    let mut dense = use_signal(|| props.dense);
    if *dense.peek() != props.dense {
        dense.set(props.dense);
    }
    use_context_provider(|| MenuCtx {
        roving,
        focused,
        dense,
    });

    let onkeydown = move |e: KeyboardEvent| {
        let current = focused.peek().clone();
        let k = e.key();
        let target = if k == Key::ArrowDown {
            roving.neighbor(current.as_deref(), 1)
        } else if k == Key::ArrowUp {
            roving.neighbor(current.as_deref(), -1)
        } else if k == Key::Home {
            roving.first()
        } else if k == Key::End {
            roving.last()
        } else {
            return;
        };
        e.prevent_default();
        if let Some(t) = target {
            roving.focus(&t);
        }
    };
    let auto_focus = props.auto_focus_item;
    rsx! {
        ul {
            class: classes([
                "skeuo-menu-list",
                if props.dense { "skeuo-menu-list--dense" } else { "" },
                &props.class,
            ]),
            role: "menu",
            id: props.id,
            tabindex: "-1",
            aria_labelledby: props.aria_labelledby,
            aria_label: props.aria_label,
            onkeydown,
            onmounted: move |_| {
                if auto_focus {
                    spawn(async move {
                        if let Some(first) = roving.first() {
                            roving.focus(&first);
                        }
                    });
                }
            },
            {props.children}
        }
    }
}

/// One row of a menu: an optional check (when `selected`), an icon, the
/// label and a keyboard shortcut legend on the right ("⌘C"). Mirrors MUI's
/// `<MenuItem>` (`role="menuitem"`; a native button inside, so `Enter` and
/// `Space` activate it). `divider` engraves a rule under the item.
#[derive(Props, Clone, PartialEq)]
pub struct MenuItemProps {
    #[props(default)]
    pub selected: bool,
    #[props(default)]
    pub disabled: bool,
    /// Engrave a rule below this item.
    #[props(default)]
    pub divider: bool,
    /// Compact row; defaults to the list's `dense`.
    pub dense: Option<bool>,
    /// Leading icon (MUI's `ListItemIcon`).
    pub icon: Option<Element>,
    /// Keyboard shortcut legend, e.g. `"⌘C"`.
    #[props(into)]
    pub shortcut: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn MenuItem(props: MenuItemProps) -> Element {
    let ctx = try_use_context::<MenuCtx>();
    let key = use_unique_id("skeuo-menu-item");
    use_drop({
        let key = key.clone();
        move || {
            if let Some(c) = ctx {
                c.roving.unregister(&key);
            }
        }
    });
    let dense = props
        .dense
        .unwrap_or(ctx.map(|c| *c.dense.read()).unwrap_or(false));
    let class = classes([
        "skeuo-menu-item",
        if props.selected {
            "skeuo-menu-item--selected"
        } else {
            ""
        },
        if dense { "skeuo-menu-item--dense" } else { "" },
        &props.class,
    ]);
    let disabled = props.disabled;
    let onmounted = {
        let key = key.clone();
        move |e: MountedEvent| {
            if let Some(c) = ctx {
                c.roving.register(&key, disabled, e.data());
            }
        }
    };
    let onfocus = {
        let key = key.clone();
        move |_| {
            if let Some(mut c) = ctx {
                c.focused.set(Some(key.clone()));
            }
        }
    };
    let onmouseenter = {
        let key = key.clone();
        move |_| {
            // Hovering moves focus too, so the keyboard continues from the
            // pointer's row, as in macOS menus.
            if let Some(c) = ctx.filter(|_| !disabled) {
                c.roving.focus(&key);
            }
        }
    };
    rsx! {
        li { class: "skeuo-menu-item__row", role: "none",
            button {
                class,
                r#type: "button",
                role: "menuitem",
                tabindex: "-1",
                disabled: props.disabled,
                aria_disabled: props.disabled.then_some("true"),
                onclick: move |e| {
                    if let Some(h) = &props.onclick {
                        h.call(e);
                    }
                },
                onmounted,
                onfocus,
                onmouseenter,
                span { class: "skeuo-menu-item__check", aria_hidden: "true",
                    if props.selected {
                        Icon { name: IconName::Check }
                    }
                }
                if let Some(icon) = props.icon {
                    span { class: "skeuo-menu-item__icon", aria_hidden: "true", {icon} }
                }
                span { class: "skeuo-menu-item__label", {props.children} }
                if let Some(shortcut) = props.shortcut {
                    kbd { class: "skeuo-menu-item__shortcut", "{shortcut}" }
                }
            }
            if props.divider {
                hr { class: "skeuo-rule skeuo-menu-item__divider", role: "separator" }
            }
        }
    }
}
