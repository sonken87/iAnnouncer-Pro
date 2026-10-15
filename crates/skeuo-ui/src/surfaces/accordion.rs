use dioxus::prelude::*;

use super::paper::{PaperVariant, chassis_classes};
use crate::{classes, use_unique_id};

/// Shared between an `Accordion` and its summary / details / actions.
#[derive(Clone, Copy)]
struct AccordionCtx {
    expanded: Memo<bool>,
    internal: Signal<bool>,
    controlled: ReadSignal<Option<bool>>,
    disabled: ReadSignal<bool>,
    onchange: Option<EventHandler<bool>>,
    summary_id: Signal<String>,
    region_id: Signal<String>,
}

impl AccordionCtx {
    fn toggle(&self) {
        if (self.disabled)() {
            return;
        }
        let next = !(self.expanded)();
        if (self.controlled)().is_none() {
            let mut internal = self.internal;
            internal.set(next);
        }
        if let Some(h) = &self.onchange {
            h.call(next);
        }
    }
}

fn use_accordion_ctx(component: &str) -> AccordionCtx {
    try_use_context::<AccordionCtx>()
        .unwrap_or_else(|| panic!("{component} must be used inside an Accordion"))
}

/// One section of a stack of aluminium plates that opens to reveal its
/// details. Mirrors MUI's `<Accordion>`.
///
/// Stacked accordions read as one segmented chassis; an expanded one
/// separates slightly from its neighbours (unless `disable_gutters`), its
/// chevron turns and its details slide open. Controlled with `expanded` +
/// `onchange`, or uncontrolled with `default_expanded`. Put an
/// [`AccordionSummary`] first, then [`AccordionDetails`] and optionally
/// [`AccordionActions`].
#[derive(Props, Clone, PartialEq)]
pub struct AccordionProps {
    /// Controlled expanded state.
    pub expanded: ReadSignal<Option<bool>>,
    /// Initial state when uncontrolled (read once at mount).
    #[props(default)]
    pub default_expanded: bool,
    /// Called with the requested new expanded state.
    pub onchange: Option<EventHandler<bool>>,
    #[props(default)]
    pub disabled: ReadSignal<bool>,
    /// Keep the same spacing and corners when expanded.
    #[props(default)]
    pub disable_gutters: bool,
    /// Square corners.
    #[props(default)]
    pub square: bool,
    /// MUI elevation, 0–24 (default 1).
    #[props(default = 1)]
    pub elevation: u8,
    #[props(default)]
    pub variant: PaperVariant,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Accordion(props: AccordionProps) -> Element {
    let default_expanded = props.default_expanded;
    let internal = use_signal(|| default_expanded);
    let controlled = props.expanded;
    let expanded = use_memo(move || controlled().unwrap_or(internal()));
    let summary_id = use_unique_id("skeuo-accordion-summary");
    let region_id = use_unique_id("skeuo-accordion-region");
    let summary_id = use_signal(|| summary_id);
    let region_id = use_signal(|| region_id);
    let disabled = props.disabled;
    let onchange = props.onchange;
    use_context_provider(|| AccordionCtx {
        expanded,
        internal,
        controlled,
        disabled,
        onchange,
        summary_id,
        region_id,
    });

    let class = classes([
        chassis_classes(props.variant, props.elevation, props.square).as_str(),
        "skeuo-accordion",
        if props.square {
            ""
        } else {
            "skeuo-accordion--rounded"
        },
        if props.disable_gutters {
            ""
        } else {
            "skeuo-accordion--gutters"
        },
        if expanded() {
            "skeuo-accordion--expanded"
        } else {
            ""
        },
        if disabled() {
            "skeuo-accordion--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div { class, {props.children} }
    }
}

/// The always-visible header of an accordion: a full-width button printed on
/// the plate, with an optional expand icon that turns when open. Mirrors
/// MUI's `<AccordionSummary>`.
#[derive(Props, Clone, PartialEq)]
pub struct AccordionSummaryProps {
    /// Icon at the end of the row, rotated 180° when expanded (e.g. a chevron).
    pub expand_icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn AccordionSummary(props: AccordionSummaryProps) -> Element {
    let ctx = use_accordion_ctx("AccordionSummary");
    let expanded = (ctx.expanded)();
    let disabled = (ctx.disabled)();
    rsx! {
        h3 { class: "skeuo-accordion__heading",
            button {
                class: classes(["skeuo-accordion-summary", &props.class]),
                r#type: "button",
                id: ctx.summary_id.cloned(),
                aria_expanded: if expanded { "true" } else { "false" },
                aria_controls: ctx.region_id.cloned(),
                disabled,
                onclick: move |_| ctx.toggle(),
                span { class: "skeuo-accordion-summary__content", {props.children} }
                if let Some(icon) = props.expand_icon {
                    span {
                        class: "skeuo-accordion-summary__expand-icon",
                        "aria-hidden": "true",
                        {icon}
                    }
                }
            }
        }
    }
}

/// The collapsible body of an accordion (`role="region"`, labelled by the
/// summary). Mirrors MUI's `<AccordionDetails>`; the height animates open.
#[derive(Props, Clone, PartialEq)]
pub struct AccordionDetailsProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn AccordionDetails(props: AccordionDetailsProps) -> Element {
    let ctx = use_accordion_ctx("AccordionDetails");
    let expanded = (ctx.expanded)();
    rsx! {
        div {
            class: if expanded { "skeuo-accordion__collapse skeuo-accordion__collapse--open" } else { "skeuo-accordion__collapse" },
            id: ctx.region_id.cloned(),
            role: "region",
            aria_labelledby: ctx.summary_id.cloned(),
            div { class: "skeuo-accordion__collapse-inner",
                div { class: classes(["skeuo-accordion-details", &props.class]), {props.children} }
            }
        }
    }
}

/// A right-aligned row of keycaps at the bottom of an expanded accordion.
/// Mirrors MUI's `<AccordionActions>`; collapses with the details.
#[derive(Props, Clone, PartialEq)]
pub struct AccordionActionsProps {
    #[props(default)]
    pub disable_spacing: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn AccordionActions(props: AccordionActionsProps) -> Element {
    let ctx = use_accordion_ctx("AccordionActions");
    let expanded = (ctx.expanded)();
    let class = classes([
        "skeuo-accordion-actions",
        if props.disable_spacing {
            ""
        } else {
            "skeuo-accordion-actions--spacing"
        },
        &props.class,
    ]);
    rsx! {
        div { class: if expanded { "skeuo-accordion__collapse skeuo-accordion__collapse--open" } else { "skeuo-accordion__collapse" },
            div { class: "skeuo-accordion__collapse-inner",
                div { class, {props.children} }
            }
        }
    }
}
