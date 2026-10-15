use dioxus::prelude::*;

use crate::classes;

/// A dimming layer over the whole window, with its children centered on
/// it (typically a progress indicator). Mirrors MUI's `<Backdrop>`.
///
/// Physically: the `.skeuo-scrim` — a slightly blurred, smoky sheet laid
/// over the desk. Nothing is rendered while `open` is false. `invisible`
/// keeps it click-catching but transparent. `inline` (a skeuo-ui addition
/// for previews) fills the nearest positioned ancestor instead of the
/// window.
#[derive(Props, Clone, PartialEq)]
pub struct BackdropProps {
    #[props(default)]
    pub open: bool,
    /// Transparent: catches clicks without dimming.
    #[props(default)]
    pub invisible: bool,
    /// Preview mode: `position: absolute` inside the nearest positioned
    /// ancestor instead of covering the window.
    #[props(default)]
    pub inline: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn Backdrop(props: BackdropProps) -> Element {
    if !props.open {
        return rsx! {};
    }
    let class = classes([
        "skeuo-scrim",
        "skeuo-backdrop",
        if props.invisible {
            "skeuo-backdrop--invisible"
        } else {
            ""
        },
        if props.inline {
            "skeuo-backdrop--inline"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div {
            class,
            "aria-hidden": "true",
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            {props.children}
        }
    }
}
