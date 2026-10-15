use dioxus::prelude::*;

use crate::{Placement, classes, use_unique_id};

fn flag(on: bool, class: &'static str) -> &'static str {
    if on { class } else { "" }
}

/// A short label that floats next to its child on hover and keyboard focus.
/// Mirrors MUI's `<Tooltip>`.
///
/// Physically it is a small floating chassis (level 3, `--float`) with an
/// optional arrow, rising in toward the light when it appears. Showing and
/// hiding is pure CSS (`:hover` / `:focus-visible`), so it works without a
/// runtime; `Escape` dismisses it until the pointer or focus leaves.
///
/// `open: Some(true)` forces it open (controlled, or a static preview),
/// `Some(false)` keeps it closed. The bubble is positioned absolutely next to
/// the child (not `fixed`), so a forced-open tooltip stays in the flow of the
/// page. Its `role="tooltip"` element has the id `id` (generated when
/// omitted); set `aria-describedby` on a focusable child to that id to have
/// screen readers announce it.
#[derive(Props, Clone, PartialEq)]
pub struct TooltipProps {
    /// The tooltip text. An empty title disables the tooltip, as in MUI.
    #[props(into)]
    pub title: String,
    /// MUI's default is below the child.
    #[props(default)]
    pub placement: Placement,
    /// Draw a small arrow pointing at the child.
    #[props(default)]
    pub arrow: bool,
    /// Controlled visibility. `None` shows it on hover and focus.
    pub open: Option<bool>,
    /// The bubble ignores the pointer (hovering it does not keep it open).
    #[props(default)]
    pub disable_interactive: bool,
    #[props(default)]
    pub disable_hover_listener: bool,
    #[props(default)]
    pub disable_focus_listener: bool,
    /// Called when the pointer enters or the child gains focus.
    pub on_open: Option<EventHandler<()>>,
    /// Called when the pointer leaves, focus leaves, or `Escape` is pressed.
    pub on_close: Option<EventHandler<()>>,
    /// Id of the `role="tooltip"` element.
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    /// The element the tooltip describes.
    pub children: Element,
}

#[component]
pub fn Tooltip(props: TooltipProps) -> Element {
    let generated = use_unique_id("skeuo-tooltip");
    let id = props.id.clone().unwrap_or(generated);
    let mut dismissed = use_signal(|| false);
    let empty = props.title.is_empty();

    let state = match props.open {
        _ if empty => "skeuo-tooltip--closed",
        Some(true) => "skeuo-tooltip--open",
        Some(false) => "skeuo-tooltip--closed",
        None if dismissed() => "skeuo-tooltip--closed",
        None => "skeuo-tooltip--auto",
    };
    let placement = format!("skeuo-tooltip--{}", props.placement.as_str());
    let class = classes([
        "skeuo-tooltip",
        state,
        &placement,
        flag(props.arrow, "skeuo-tooltip--arrow"),
        flag(!props.disable_interactive, "skeuo-tooltip--interactive"),
        flag(props.disable_hover_listener, "skeuo-tooltip--no-hover"),
        flag(props.disable_focus_listener, "skeuo-tooltip--no-focus"),
        &props.class,
    ]);

    let on_open = props.on_open;
    let on_close = props.on_close;
    let hover = !props.disable_hover_listener;
    let focus = !props.disable_focus_listener;
    rsx! {
        span {
            class,
            onmouseenter: move |_| {
                if hover {
                    dismissed.set(false);
                    if let Some(h) = &on_open {
                        h.call(());
                    }
                }
            },
            onmouseleave: move |_| {
                if hover {
                    dismissed.set(false);
                    if let Some(h) = &on_close {
                        h.call(());
                    }
                }
            },
            onfocusin: move |_| {
                if let (true, Some(h)) = (focus, &on_open) {
                    h.call(());
                }
            },
            onfocusout: move |_| {
                if focus {
                    dismissed.set(false);
                    if let Some(h) = &on_close {
                        h.call(());
                    }
                }
            },
            onkeydown: move |e: KeyboardEvent| {
                if e.key() == Key::Escape {
                    dismissed.set(true);
                    if let Some(h) = &on_close {
                        h.call(());
                    }
                }
            },
            {props.children}
            if !empty {
                span { class: "skeuo-tooltip__popper", role: "tooltip", id,
                    span { class: "skeuo-tooltip__bubble skeuo-chassis skeuo-chassis--float skeuo-elev-3",
                        "{props.title}"
                        if props.arrow {
                            span { class: "skeuo-tooltip__arrow", aria_hidden: "true" }
                        }
                    }
                }
            }
        }
    }
}
