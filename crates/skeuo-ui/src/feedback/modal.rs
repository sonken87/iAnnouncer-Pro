use std::rc::Rc;

use dioxus::prelude::*;

use crate::classes;

/// Why a [`Modal`] or [`Dialog`](crate::Dialog) asks to close (MUI's
/// `reason` argument of `onClose`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum ModalCloseReason {
    /// The scrim behind the surface was clicked.
    BackdropClick,
    /// Escape was pressed while focus was inside.
    EscapeKeyDown,
}

impl ModalCloseReason {
    pub fn as_str(self) -> &'static str {
        match self {
            ModalCloseReason::BackdropClick => "backdropClick",
            ModalCloseReason::EscapeKeyDown => "escapeKeyDown",
        }
    }
}

/// Focuses `node` once it is mounted and the overlay is open, so keyboard
/// users land inside the overlay and Escape reaches its handler.
pub(crate) fn use_focus_on_open(open: bool) -> Signal<Option<Rc<MountedData>>> {
    let node = use_signal(|| None::<Rc<MountedData>>);
    use_effect(use_reactive((&open,), move |(open,)| {
        if open && let Some(el) = node.peek().clone() {
            spawn(async move {
                let _ = el.set_focus(true).await;
            });
        }
    }));
    node
}

/// The bare overlay that dialogs, drawers and popovers are built on.
/// Mirrors MUI's `<Modal>`.
///
/// Physically: a `.skeuo-scrim` sheet laid over the desk with the children
/// centered above it. Escape (unless `disable_escape_key_down`) and a click
/// on the scrim call `onclose` with the reason; the children are wrapped in
/// an `aria-modal` container that takes focus when the modal opens.
/// Nothing is rendered while closed unless `keep_mounted` is set.
#[derive(Props, Clone, PartialEq)]
pub struct ModalProps {
    pub open: bool,
    pub onclose: Option<EventHandler<ModalCloseReason>>,
    #[props(default)]
    pub disable_escape_key_down: bool,
    /// Do not render the scrim (clicks outside then do nothing).
    #[props(default)]
    pub hide_backdrop: bool,
    /// Keep the children in the DOM (hidden) while closed.
    #[props(default)]
    pub keep_mounted: bool,
    /// ARIA role of the focusable content wrapper.
    #[props(default = "dialog".to_string(), into)]
    pub role: String,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(into)]
    pub aria_describedby: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Modal(props: ModalProps) -> Element {
    let open = props.open;
    let mut node = use_focus_on_open(open);
    if !open && !props.keep_mounted {
        return rsx! {};
    }
    let class = classes([
        "skeuo-modal",
        if open { "" } else { "skeuo-modal--hidden" },
        &props.class,
    ]);
    let onclose = props.onclose;
    let escape_enabled = !props.disable_escape_key_down;

    rsx! {
        div {
            class,
            role: "presentation",
            hidden: !open,
            onkeydown: move |e: KeyboardEvent| {
                if escape_enabled && e.key() == Key::Escape {
                    e.stop_propagation();
                    if let Some(h) = &onclose {
                        h.call(ModalCloseReason::EscapeKeyDown);
                    }
                }
            },
            if !props.hide_backdrop {
                div {
                    class: "skeuo-scrim skeuo-modal__scrim",
                    "aria-hidden": "true",
                    onclick: move |_| {
                        if let Some(h) = &onclose {
                            h.call(ModalCloseReason::BackdropClick);
                        }
                    },
                }
            }
            div {
                class: "skeuo-modal__content",
                role: props.role,
                "aria-modal": "true",
                aria_labelledby: props.aria_labelledby,
                aria_describedby: props.aria_describedby,
                tabindex: "-1",
                onmounted: move |e: MountedEvent| {
                    let el = e.data();
                    node.set(Some(el.clone()));
                    async move {
                        if open {
                            let _ = el.set_focus(true).await;
                        }
                    }
                },
                {props.children}
            }
        }
    }
}
