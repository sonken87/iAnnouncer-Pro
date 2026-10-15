use dioxus::prelude::*;

use super::paper::{PaperVariant, chassis_classes};
use crate::classes;

/// The window edge a drawer is glued to, mirroring MUI's `anchor` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DrawerAnchor {
    #[default]
    Left,
    Right,
    Top,
    Bottom,
}

impl DrawerAnchor {
    pub fn as_str(self) -> &'static str {
        match self {
            DrawerAnchor::Left => "left",
            DrawerAnchor::Right => "right",
            DrawerAnchor::Top => "top",
            DrawerAnchor::Bottom => "bottom",
        }
    }
}

/// How a drawer behaves, mirroring MUI's `variant` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DrawerVariant {
    /// A modal panel sliding in over a scrim; closes on Escape / scrim click.
    #[default]
    Temporary,
    /// Docked in the layout; `open` slides it in and out, pushing content.
    Persistent,
    /// Always docked in the layout.
    Permanent,
}

impl DrawerVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            DrawerVariant::Temporary => "temporary",
            DrawerVariant::Persistent => "persistent",
            DrawerVariant::Permanent => "permanent",
        }
    }
}

/// A panel glued to one edge of the window. Mirrors MUI's `<Drawer>`.
///
/// `Temporary` (default): a floating level-4 aluminium chassis with square
/// corners on its anchored edge that slides in over a scrim, `role="dialog"`
/// with `aria-modal`; Escape and a click on the scrim call `onclose`. Rendered
/// only while `open`. With `inline: true` it fills its nearest positioned
/// ancestor instead of the viewport (an in-flow preview for galleries and
/// docs).
///
/// `Persistent` / `Permanent`: a flush, square chassis docked in normal flow
/// (put it in a flex row next to the content). Global attributes (e.g.
/// `aria-label`) go to the panel element.
#[derive(Props, Clone, PartialEq)]
pub struct DrawerProps {
    #[props(default)]
    pub anchor: DrawerAnchor,
    #[props(default)]
    pub variant: DrawerVariant,
    #[props(default)]
    pub open: bool,
    /// Called on Escape or a scrim click (temporary drawers).
    pub onclose: Option<EventHandler<()>>,
    /// Width (left/right) or height (top/bottom) of the panel, as CSS.
    /// Defaults to 280px / auto.
    #[props(into)]
    pub size: Option<String>,
    /// MUI elevation of a temporary drawer, 0–24 (default 16 → level 4).
    #[props(default = 16)]
    pub elevation: u8,
    /// Don't render the scrim behind a temporary drawer.
    #[props(default)]
    pub hide_backdrop: bool,
    /// Preview mode: position against the nearest positioned ancestor instead
    /// of the viewport, and don't grab focus.
    #[props(default)]
    pub inline: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Drawer(props: DrawerProps) -> Element {
    let anchor = props.anchor.as_str();
    let size = props
        .size
        .as_ref()
        .map(|s| format!("--drawer-size:{s};"))
        .unwrap_or_default();
    let root_base = classes([
        "skeuo-drawer",
        &format!("skeuo-drawer--{}", props.variant.as_str()),
        &format!("skeuo-drawer--{anchor}"),
        if props.inline {
            "skeuo-drawer--inline"
        } else {
            ""
        },
    ]);
    let onclose = props.onclose;
    let close = move || {
        if let Some(h) = &onclose {
            h.call(());
        }
    };

    if props.variant == DrawerVariant::Temporary {
        if !props.open {
            return rsx! {};
        }
        let inline = props.inline;
        let paper = classes([
            chassis_classes(PaperVariant::Elevation, props.elevation, false).as_str(),
            "skeuo-drawer__paper",
            &format!("skeuo-drawer__paper--{anchor}"),
        ]);
        return rsx! {
            div {
                class: classes([root_base.as_str(), &props.class]),
                style: "{size}",
                onkeydown: move |e: KeyboardEvent| {
                    if e.key() == Key::Escape {
                        e.stop_propagation();
                        close();
                    }
                },
                if !props.hide_backdrop {
                    div {
                        class: "skeuo-scrim skeuo-drawer__scrim",
                        "aria-hidden": "true",
                        onclick: move |_| close(),
                    }
                }
                div {
                    class: paper,
                    role: "dialog",
                    aria_modal: "true",
                    tabindex: "-1",
                    onmounted: move |e: MountedEvent| {
                        if !inline {
                            spawn(async move {
                                let _ = e.data().set_focus(true).await;
                            });
                        }
                    },
                    ..props.attributes,
                    {props.children}
                }
            }
        };
    }

    let open = props.variant == DrawerVariant::Permanent || props.open;
    let paper = classes([
        chassis_classes(PaperVariant::Elevation, 0, true).as_str(),
        "skeuo-drawer__paper",
        &format!("skeuo-drawer__paper--{anchor}"),
    ]);
    rsx! {
        div {
            class: classes([
                root_base.as_str(),
                "skeuo-drawer--docked",
                if open { "skeuo-drawer--open" } else { "skeuo-drawer--closed" },
                &props.class,
            ]),
            style: "{size}",
            div { class: paper, ..props.attributes, {props.children} }
        }
    }
}
