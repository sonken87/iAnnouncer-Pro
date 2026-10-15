use dioxus::prelude::*;

use super::modal::{ModalCloseReason, use_focus_on_open};
use crate::{classes, use_unique_id};

/// Width cap of a [`Dialog`], as in MUI's `maxWidth` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DialogMaxWidth {
    /// 444px
    Xs,
    /// 600px (MUI's default)
    #[default]
    Sm,
    /// 900px
    Md,
    /// 1200px
    Lg,
    /// 1536px
    Xl,
    /// No cap (MUI `maxWidth={false}`).
    False,
}

impl DialogMaxWidth {
    pub fn as_str(self) -> &'static str {
        match self {
            DialogMaxWidth::Xs => "xs",
            DialogMaxWidth::Sm => "sm",
            DialogMaxWidth::Md => "md",
            DialogMaxWidth::Lg => "lg",
            DialogMaxWidth::Xl => "xl",
            DialogMaxWidth::False => "false",
        }
    }
}

/// What scrolls when a [`Dialog`]'s content is taller than the window, as in
/// MUI's `scroll` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum DialogScroll {
    /// The dialog stays within the window; `DialogContent` scrolls.
    #[default]
    Paper,
    /// The dialog grows; the scrim scrolls.
    Body,
}

impl DialogScroll {
    pub fn as_str(self) -> &'static str {
        match self {
            DialogScroll::Paper => "paper",
            DialogScroll::Body => "body",
        }
    }
}

/// Shared with `DialogTitle` so it can take the id the dialog is labelled by.
#[derive(Clone, PartialEq)]
struct DialogContext {
    title_id: String,
}

/// A modal window. Mirrors MUI's `<Dialog>`.
///
/// Physically: a level-4 floating aluminium chassis that rises in over the
/// `.skeuo-scrim`; its actions are right-aligned keycaps. Escape and a click
/// on the scrim call `onclose` with the reason. The surface has
/// `role="dialog"`, `aria-modal` and is labelled by its `DialogTitle`
/// (or `aria_labelledby`); it takes focus when it opens.
///
/// `inline` (a skeuo-ui addition) renders the same surface in the page
/// flow, on a small contained scrim, for galleries and documentation.
#[derive(Props, Clone, PartialEq)]
pub struct DialogProps {
    pub open: bool,
    pub onclose: Option<EventHandler<ModalCloseReason>>,
    /// Stretch to `max_width`.
    #[props(default)]
    pub full_width: bool,
    #[props(default)]
    pub max_width: DialogMaxWidth,
    /// Cover the whole window, with square corners.
    #[props(default)]
    pub full_screen: bool,
    #[props(default)]
    pub scroll: DialogScroll,
    #[props(default)]
    pub disable_escape_key_down: bool,
    /// Preview mode: render in flow instead of over the window.
    #[props(default)]
    pub inline: bool,
    /// Id of the element labelling the dialog. Defaults to the `DialogTitle`.
    #[props(into)]
    pub aria_labelledby: Option<String>,
    /// Id of the element describing the dialog, e.g. a `DialogContentText`.
    #[props(into)]
    pub aria_describedby: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Dialog(props: DialogProps) -> Element {
    let generated = use_unique_id("skeuo-dialog-title");
    let title_id = props.aria_labelledby.clone().unwrap_or(generated);
    use_context_provider(|| DialogContext {
        title_id: title_id.clone(),
    });
    let open = props.open && !props.inline;
    let mut node = use_focus_on_open(open);
    if !props.open {
        return rsx! {};
    }

    let max_width = format!("skeuo-dialog--max-{}", props.max_width.as_str());
    let scroll = format!("skeuo-dialog--scroll-{}", props.scroll.as_str());
    let class = classes([
        "skeuo-dialog",
        &max_width,
        &scroll,
        if props.full_width {
            "skeuo-dialog--full-width"
        } else {
            ""
        },
        if props.full_screen {
            "skeuo-dialog--full-screen"
        } else {
            ""
        },
        if props.inline {
            "skeuo-dialog--inline"
        } else {
            ""
        },
        &props.class,
    ]);
    let paper_class = classes([
        "skeuo-chassis skeuo-chassis--float skeuo-elev-4 skeuo-dialog__paper",
        if props.full_screen {
            "skeuo-chassis--square"
        } else {
            ""
        },
    ]);
    let onclose = props.onclose;
    let escape_enabled = !props.disable_escape_key_down;
    let inline = props.inline;

    rsx! {
        div {
            class,
            role: "presentation",
            onkeydown: move |e: KeyboardEvent| {
                if escape_enabled && e.key() == Key::Escape {
                    e.stop_propagation();
                    if let Some(h) = &onclose {
                        h.call(ModalCloseReason::EscapeKeyDown);
                    }
                }
            },
            if !inline {
                div { class: "skeuo-scrim skeuo-dialog__scrim", "aria-hidden": "true" }
            }
            div {
                class: "skeuo-dialog__container",
                // Clicks that reach the container (and not the paper) are on
                // the scrim around the dialog.
                onclick: move |_| {
                    if !inline && let Some(h) = &onclose {
                        h.call(ModalCloseReason::BackdropClick);
                    }
                },
                div {
                    class: paper_class,
                    role: "dialog",
                    "aria-modal": if inline { "false" } else { "true" },
                    aria_labelledby: title_id,
                    aria_describedby: props.aria_describedby,
                    tabindex: "-1",
                    onclick: move |e| e.stop_propagation(),
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
}

/// The heading of a [`Dialog`]; its id labels the dialog. Mirrors MUI's
/// `<DialogTitle>`: tight 17px lettering on the aluminium.
#[derive(Props, Clone, PartialEq)]
pub struct DialogTitleProps {
    /// Overrides the id provided by the surrounding `Dialog`.
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn DialogTitle(props: DialogTitleProps) -> Element {
    let ctx = try_use_context::<DialogContext>();
    let id = props.id.clone().or_else(|| ctx.map(|c| c.title_id));
    rsx! {
        h2 { class: classes(["skeuo-dialog__title", &props.class]), id, {props.children} }
    }
}

/// The body of a [`Dialog`]; it scrolls when the dialog is height-capped.
/// Mirrors MUI's `<DialogContent>`. `dividers` engraves rules above and
/// below it.
#[derive(Props, Clone, PartialEq)]
pub struct DialogContentProps {
    #[props(default)]
    pub dividers: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn DialogContent(props: DialogContentProps) -> Element {
    let class = classes([
        "skeuo-dialog__content",
        if props.dividers {
            "skeuo-dialog__content--dividers"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div { class, {props.children} }
    }
}

/// Secondary lettering inside [`DialogContent`]. Mirrors MUI's
/// `<DialogContentText>`; give it an `id` and pass it to the dialog's
/// `aria_describedby`.
#[derive(Props, Clone, PartialEq)]
pub struct DialogContentTextProps {
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn DialogContentText(props: DialogContentTextProps) -> Element {
    rsx! {
        p { class: classes(["skeuo-dialog__text", &props.class]), id: props.id, {props.children} }
    }
}

/// The row of keycaps at the bottom of a [`Dialog`], right-aligned.
/// Mirrors MUI's `<DialogActions>`.
#[derive(Props, Clone, PartialEq)]
pub struct DialogActionsProps {
    /// Remove the gap between the actions.
    #[props(default)]
    pub disable_spacing: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn DialogActions(props: DialogActionsProps) -> Element {
    let class = classes([
        "skeuo-dialog__actions",
        if props.disable_spacing {
            ""
        } else {
            "skeuo-dialog__actions--spacing"
        },
        &props.class,
    ]);
    rsx! {
        div { class, {props.children} }
    }
}
