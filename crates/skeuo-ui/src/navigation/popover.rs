use dioxus::prelude::*;

use crate::{Placement, classes};

/// A floating aluminium plate (level-3 chassis, `--float`) that rises in
/// next to its anchor. Mirrors MUI's `<Popover>`.
///
/// The `anchor` (usually the trigger button) is rendered inside a
/// `position: relative` wrapper and the surface is positioned against it by
/// `placement`. While open, an invisible full-window layer catches clicks
/// outside the surface and calls `onclose`; `Escape` does the same.
///
/// `inline: true` shows the surface statically in the page flow, under the
/// anchor, with no click-catching layer: a preview for docs and snapshots.
#[derive(Props, Clone, PartialEq)]
pub struct PopoverProps {
    #[props(default)]
    pub open: bool,
    pub onclose: Option<EventHandler<()>>,
    /// The element the surface is attached to.
    pub anchor: Option<Element>,
    #[props(default)]
    pub placement: Placement,
    /// Show the surface in the page flow (preview).
    #[props(default)]
    pub inline: bool,
    /// Move focus into the surface when it opens.
    #[props(default = true)]
    pub auto_focus: bool,
    #[props(into)]
    pub id: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    /// Classes for the floating surface.
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Popover(props: PopoverProps) -> Element {
    let placement = format!("skeuo-popover--{}", props.placement.as_str());
    let surface = classes([
        "skeuo-popover",
        "skeuo-chassis",
        "skeuo-elev-3",
        "skeuo-chassis--float",
        &placement,
        if props.inline {
            "skeuo-popover--inline"
        } else {
            ""
        },
        &props.class,
    ]);
    let onclose = props.onclose;
    let close = move || {
        if let Some(h) = &onclose {
            h.call(());
        }
    };
    let onkeydown = move |e: KeyboardEvent| {
        if e.key() == Key::Escape {
            e.stop_propagation();
            close();
        }
    };
    let auto_focus = props.auto_focus && !props.inline;
    rsx! {
        div {
            class: if props.inline { "skeuo-popover-anchor skeuo-popover-anchor--inline" } else { "skeuo-popover-anchor" },
            onkeydown,
            {props.anchor}
            if props.open && !props.inline {
                div {
                    class: "skeuo-popover-backdrop",
                    aria_hidden: "true",
                    onclick: move |_| close(),
                }
            }
            if props.open {
                div {
                    class: surface,
                    id: props.id,
                    tabindex: "-1",
                    aria_label: props.aria_label,
                    aria_labelledby: props.aria_labelledby,
                    onmounted: move |e: MountedEvent| {
                        if auto_focus {
                            spawn(async move {
                                let _ = e.data().set_focus(true).await;
                            });
                        }
                    },
                    {props.children}
                }
            }
        }
    }
}
