//! Snackbar and SnackbarContent.
//!
//! Material choice: MUI draws its snackbar as a dark slab so it stands out
//! from a light page. On a Magic Keyboard there is no dark material in our
//! set (no space-black chassis, keycap or ink tokens), and inventing one
//! would mean new gradients and shadows outside `materials.css`. The
//! snackbar is listed among the chassis surfaces in DESIGN.md, and macOS
//! itself shows transient notifications as light floating plates. So the
//! snackbar is a light level-3 floating chassis that rises in; it stands
//! out from the page by its cast shadow and its float tone, and its action
//! is printed in the palette ink like any text button.

use dioxus::prelude::*;

use crate::classes;

/// Vertical edge a [`Snackbar`] is anchored to.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SnackbarVertical {
    Top,
    #[default]
    Bottom,
}

/// Horizontal position of a [`Snackbar`].
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SnackbarHorizontal {
    #[default]
    Left,
    Center,
    Right,
}

/// Where a [`Snackbar`] appears, as in MUI's `anchorOrigin`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub struct SnackbarOrigin {
    pub vertical: SnackbarVertical,
    pub horizontal: SnackbarHorizontal,
}

impl SnackbarOrigin {
    pub fn new(vertical: SnackbarVertical, horizontal: SnackbarHorizontal) -> Self {
        Self {
            vertical,
            horizontal,
        }
    }
}

/// Why a [`Snackbar`] asks to close (MUI's `reason` argument).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum SnackbarCloseReason {
    /// `auto_hide_duration` elapsed.
    Timeout,
    /// Escape was pressed while focus was inside the snackbar.
    EscapeKeyDown,
}

impl SnackbarCloseReason {
    pub fn as_str(self) -> &'static str {
        match self {
            SnackbarCloseReason::Timeout => "timeout",
            SnackbarCloseReason::EscapeKeyDown => "escapeKeyDown",
        }
    }
}

const TIMER_ANIMATION: &str = "skeuo-snackbar-timer";

/// A brief message about an app process, at the edge of the window.
/// Mirrors MUI's `<Snackbar>`.
///
/// Physically: a light level-3 floating aluminium plate that rises in (see
/// the module docs for why it is not dark). With `auto_hide_duration` it
/// calls `onclose(Timeout)` after that many milliseconds; the countdown is a
/// CSS animation (paused while hovered or focused, as in MUI) ending in
/// `onanimationend`, so no timer runtime is needed.
///
/// Pass `message` / `action` for the standard content, or children for a
/// custom surface (e.g. an `Alert`). `inline` (a skeuo-ui addition) renders
/// it in the page flow for previews.
#[derive(Props, Clone, PartialEq)]
pub struct SnackbarProps {
    pub open: bool,
    #[props(into)]
    pub message: Option<String>,
    /// Element after the message, e.g. an "Undo" button.
    pub action: Option<Element>,
    pub onclose: Option<EventHandler<SnackbarCloseReason>>,
    /// Milliseconds before `onclose(Timeout)`; `None` never hides.
    pub auto_hide_duration: Option<u32>,
    #[props(default)]
    pub anchor_origin: SnackbarOrigin,
    /// Preview mode: render in flow instead of fixed to the window edge.
    #[props(default)]
    pub inline: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Snackbar(props: SnackbarProps) -> Element {
    if !props.open {
        return rsx! {};
    }
    let vertical = match props.anchor_origin.vertical {
        SnackbarVertical::Top => "skeuo-snackbar--top",
        SnackbarVertical::Bottom => "skeuo-snackbar--bottom",
    };
    let horizontal = match props.anchor_origin.horizontal {
        SnackbarHorizontal::Left => "skeuo-snackbar--left",
        SnackbarHorizontal::Center => "skeuo-snackbar--center",
        SnackbarHorizontal::Right => "skeuo-snackbar--right",
    };
    let class = classes([
        "skeuo-snackbar",
        vertical,
        horizontal,
        if props.inline {
            "skeuo-snackbar--inline"
        } else {
            ""
        },
        &props.class,
    ]);
    let onclose = props.onclose;
    let standard = props.message.is_some() || props.action.is_some();

    rsx! {
        div {
            class,
            onkeydown: move |e: KeyboardEvent| {
                if e.key() == Key::Escape && let Some(h) = &onclose {
                    h.call(SnackbarCloseReason::EscapeKeyDown);
                }
            },
            if standard {
                SnackbarContent {
                    message: props.message.clone().unwrap_or_default(),
                    action: props.action.clone(),
                }
            } else {
                {props.children}
            }
            if let Some(ms) = props.auto_hide_duration {
                span {
                    class: "skeuo-snackbar__timer",
                    style: "--snackbar-hide:{ms}ms",
                    "aria-hidden": "true",
                    onanimationend: move |e: AnimationEvent| {
                        if e.animation_name() == TIMER_ANIMATION && let Some(h) = &onclose {
                            h.call(SnackbarCloseReason::Timeout);
                        }
                    },
                }
            }
        }
    }
}

/// The plate of a [`Snackbar`]: message and actions on a level-3 floating
/// chassis. Mirrors MUI's `<SnackbarContent>` and, like it, carries
/// `role="alert"` so the message is announced.
#[derive(Props, Clone, PartialEq)]
pub struct SnackbarContentProps {
    #[props(default, into)]
    pub message: String,
    pub action: Option<Element>,
    #[props(default = "alert".to_string(), into)]
    pub role: String,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn SnackbarContent(props: SnackbarContentProps) -> Element {
    let class = classes([
        "skeuo-chassis skeuo-chassis--float skeuo-elev-3 skeuo-snackbar-content",
        &props.class,
    ]);
    rsx! {
        div { class, role: props.role,
            div { class: "skeuo-snackbar-content__message", "{props.message}" }
            if let Some(action) = props.action {
                div { class: "skeuo-snackbar-content__action", {action} }
            }
        }
    }
}
