use std::cell::Cell;
use std::rc::Rc;

use dioxus::prelude::*;

use crate::{Color, Icon, IconName, Orientation, classes};

#[derive(Clone, PartialEq)]
struct StepperState {
    active_step: usize,
    orientation: Orientation,
    alternative_label: bool,
    non_linear: bool,
}

#[derive(Clone, PartialEq)]
struct StepperCtx {
    state: Signal<StepperState>,
    /// Hands out step indices in render order.
    counter: Rc<Cell<usize>>,
}

#[derive(Clone, Copy, PartialEq)]
struct StepState {
    index: usize,
    active: bool,
    completed: bool,
    disabled: bool,
    expanded: bool,
    orientation: Orientation,
}

/// A row (or column) of steps: each step's indicator is a small round
/// keycap with its number; it becomes a colored key with an LED-like glow
/// when active and shows a check once completed. Steps are joined by
/// grooves that fill in as the steps are completed. Mirrors MUI's
/// `<Stepper>`.
///
/// Steps are numbered in render order (or by their `index` prop).
#[derive(Props, Clone, PartialEq)]
pub struct StepperProps {
    /// Index of the current step (0-based).
    #[props(default)]
    pub active_step: usize,
    #[props(default)]
    pub orientation: Orientation,
    /// Put labels under the indicators (horizontal only).
    #[props(default)]
    pub alternative_label: bool,
    /// Steps can be visited in any order: none are completed or disabled
    /// just because of `active_step`.
    #[props(default)]
    pub non_linear: bool,
    /// Palette color of the active / completed indicators.
    #[props(default)]
    pub color: Color,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Stepper(props: StepperProps) -> Element {
    let next = StepperState {
        active_step: props.active_step,
        orientation: props.orientation,
        alternative_label: props.alternative_label,
        non_linear: props.non_linear,
    };
    let mut state = use_signal(|| next.clone());
    if *state.peek() != next {
        state.set(next);
    }
    use_context_provider(|| StepperCtx {
        state,
        counter: Rc::new(Cell::new(0)),
    });
    let orientation = format!("skeuo-stepper--{}", props.orientation.as_str());
    rsx! {
        div {
            class: classes([
                "skeuo-stepper",
                &orientation,
                if props.alternative_label && props.orientation == Orientation::Horizontal {
                    "skeuo-stepper--alternative-label"
                } else {
                    ""
                },
                props.color.class(),
                &props.class,
            ]),
            {props.children}
        }
    }
}

/// One step of a [`Stepper`], with the groove that leads to it. Its state
/// follows the stepper's `active_step` unless `active` / `completed` /
/// `disabled` are given. Mirrors MUI's `<Step>`.
#[derive(Props, Clone, PartialEq)]
pub struct StepProps {
    /// Position in the stepper; assigned in render order when omitted.
    pub index: Option<usize>,
    pub active: Option<bool>,
    pub completed: Option<bool>,
    pub disabled: Option<bool>,
    /// Keep the [`StepContent`] open even when the step is not active.
    #[props(default)]
    pub expanded: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Step(props: StepProps) -> Element {
    let ctx = use_context::<StepperCtx>();
    let auto_index = use_hook(|| {
        let i = ctx.counter.get();
        ctx.counter.set(i + 1);
        i
    });
    let index = props.index.unwrap_or(auto_index);
    let st = ctx.state.read().clone();

    let active = st.active_step == index;
    let mut completed = false;
    let mut disabled = false;
    if !active && !st.non_linear {
        completed = st.active_step > index;
        disabled = st.active_step < index;
    }
    let active = props.active.unwrap_or(active);
    let completed = props.completed.unwrap_or(completed);
    let disabled = props.disabled.unwrap_or(disabled);

    let next = StepState {
        index,
        active,
        completed,
        disabled,
        expanded: props.expanded,
        orientation: st.orientation,
    };
    let mut state = use_signal(|| next);
    if *state.peek() != next {
        state.set(next);
    }
    use_context_provider(|| state);

    let filled = active || completed;
    rsx! {
        div {
            class: classes([
                "skeuo-step",
                if index > 0 { "skeuo-step--joined" } else { "" },
                if active { "skeuo-step--active" } else { "" },
                if completed { "skeuo-step--completed" } else { "" },
                if disabled { "skeuo-step--disabled" } else { "" },
                &props.class,
            ]),
            if index > 0 {
                span { class: "skeuo-step-connector skeuo-groove", aria_hidden: "true",
                    span {
                        class: if filled { "skeuo-groove__fill skeuo-step-connector__fill skeuo-step-connector__fill--on" } else { "skeuo-groove__fill skeuo-step-connector__fill" },
                    }
                }
            }
            {props.children}
        }
    }
}

/// The round keycap that marks a step: its number, a check when completed,
/// "!" on error.
#[component]
fn StepIcon(
    index: usize,
    active: bool,
    completed: bool,
    error: bool,
    icon: Option<Element>,
) -> Element {
    let class = classes([
        "skeuo-step-icon skeuo-key skeuo-key--round",
        if active || completed || error {
            "skeuo-key--filled"
        } else {
            ""
        },
        if active {
            "skeuo-step-icon--active"
        } else {
            ""
        },
        if error {
            "skeuo-color-error skeuo-step-icon--error"
        } else {
            ""
        },
    ]);
    rsx! {
        span { class, aria_hidden: "true",
            if let Some(icon) = icon {
                {icon}
            } else if error {
                "!"
            } else if completed {
                Icon { name: IconName::Check }
            } else {
                "{index + 1}"
            }
        }
    }
}

/// The indicator keycap and printed label of a [`Step`]. Mirrors MUI's
/// `<StepLabel>`; `optional` adds a caption line under the label.
#[derive(Props, Clone, PartialEq)]
pub struct StepLabelProps {
    /// Caption under the label, e.g. "Optional".
    pub optional: Option<Element>,
    /// Show the step in the error state.
    #[props(default)]
    pub error: bool,
    /// Replace the number legend of the indicator.
    pub icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn StepLabel(props: StepLabelProps) -> Element {
    let step = use_context::<Signal<StepState>>();
    let s = *step.read();
    rsx! {
        span {
            class: classes([
                "skeuo-step-label",
                if s.active { "skeuo-step-label--active" } else { "" },
                if s.completed { "skeuo-step-label--completed" } else { "" },
                if props.error { "skeuo-step-label--error" } else { "" },
                if s.disabled { "skeuo-step-label--disabled" } else { "" },
                &props.class,
            ]),
            StepIcon {
                index: s.index,
                active: s.active,
                completed: s.completed,
                error: props.error,
                icon: props.icon,
            }
            span { class: "skeuo-step-label__text",
                span { class: "skeuo-step-label__label", {props.children} }
                if let Some(optional) = props.optional {
                    span { class: "skeuo-step-label__optional skeuo-caption", {optional} }
                }
            }
        }
    }
}

/// A [`StepLabel`] you can press, for non-linear steppers. Mirrors MUI's
/// `<StepButton>`.
#[derive(Props, Clone, PartialEq)]
pub struct StepButtonProps {
    pub optional: Option<Element>,
    pub icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn StepButton(props: StepButtonProps) -> Element {
    let step = use_context::<Signal<StepState>>();
    let s = *step.read();
    rsx! {
        button {
            class: classes(["skeuo-step-button", &props.class]),
            r#type: "button",
            disabled: s.disabled,
            aria_current: s.active.then_some("step"),
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            StepLabel { optional: props.optional, icon: props.icon, {props.children} }
        }
    }
}

/// The body of a step in a vertical [`Stepper`], shown while the step is
/// active (or `expanded`), alongside the groove that runs to the next step.
/// Mirrors MUI's `<StepContent>`.
#[derive(Props, Clone, PartialEq)]
pub struct StepContentProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn StepContent(props: StepContentProps) -> Element {
    let step = use_context::<Signal<StepState>>();
    let s = *step.read();
    if s.orientation != Orientation::Vertical {
        return rsx! {};
    }
    let open = s.active || s.expanded;
    rsx! {
        div { class: classes(["skeuo-step-content", &props.class]),
            span { class: "skeuo-step-content__line skeuo-groove", aria_hidden: "true",
                span {
                    class: if s.completed { "skeuo-groove__fill skeuo-step-connector__fill skeuo-step-connector__fill--on" } else { "skeuo-groove__fill skeuo-step-connector__fill" },
                }
            }
            if open {
                div { class: "skeuo-step-content__body", {props.children} }
            }
        }
    }
}

/// How [`MobileStepper`] shows progress (MUI's `variant`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum MobileStepperVariant {
    /// A row of LEDs; the current step's is lit.
    #[default]
    Dots,
    /// A groove filling up.
    Progress,
    /// Printed "2 / 6".
    Text,
}

impl MobileStepperVariant {
    fn as_str(self) -> &'static str {
        match self {
            MobileStepperVariant::Dots => "dots",
            MobileStepperVariant::Progress => "progress",
            MobileStepperVariant::Text => "text",
        }
    }
}

/// Where a [`MobileStepper`] sits (MUI's `position`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum MobileStepperPosition {
    /// Fixed to the bottom of the window.
    #[default]
    Bottom,
    /// Fixed to the top of the window.
    Top,
    /// In the page flow.
    Static,
}

impl MobileStepperPosition {
    fn as_str(self) -> &'static str {
        match self {
            MobileStepperPosition::Bottom => "bottom",
            MobileStepperPosition::Top => "top",
            MobileStepperPosition::Static => "static",
        }
    }
}

/// A compact stepper bar: back and next keys at the ends and the progress
/// between them as LEDs, a groove or printed text. Mirrors MUI's
/// `<MobileStepper>`.
#[derive(Props, Clone, PartialEq)]
pub struct MobileStepperProps {
    pub steps: usize,
    #[props(default)]
    pub active_step: usize,
    #[props(default)]
    pub variant: MobileStepperVariant,
    #[props(default)]
    pub position: MobileStepperPosition,
    #[props(default)]
    pub color: Color,
    pub back_button: Option<Element>,
    pub next_button: Option<Element>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn MobileStepper(props: MobileStepperProps) -> Element {
    let variant = format!("skeuo-mobile-stepper--{}", props.variant.as_str());
    let position = format!("skeuo-mobile-stepper--{}", props.position.as_str());
    let steps = props.steps.max(1);
    let active = props.active_step.min(steps - 1);
    let pct = if steps > 1 {
        active as f64 * 100.0 / (steps - 1) as f64
    } else {
        100.0
    };
    rsx! {
        div {
            class: classes([
                "skeuo-mobile-stepper skeuo-chassis skeuo-elev-1",
                &variant,
                &position,
                props.color.class(),
                &props.class,
            ]),
            span { class: "skeuo-mobile-stepper__end", {props.back_button} }
            match props.variant {
                MobileStepperVariant::Dots => rsx! {
                    span { class: "skeuo-mobile-stepper__dots", role: "img", aria_label: "Step {active + 1} of {steps}",
                        for i in 0..steps {
                            span {
                                key: "{i}",
                                class: if i == active { "skeuo-mobile-stepper__dot skeuo-led" } else { "skeuo-mobile-stepper__dot skeuo-led skeuo-led--off" },
                            }
                        }
                    }
                },
                MobileStepperVariant::Progress => rsx! {
                    span {
                        class: "skeuo-mobile-stepper__progress skeuo-groove",
                        role: "progressbar",
                        aria_valuemin: "0",
                        aria_valuemax: "100",
                        aria_valuenow: "{pct.round()}",
                        span { class: "skeuo-groove__fill skeuo-mobile-stepper__fill", style: "width: {pct}%" }
                    }
                },
                MobileStepperVariant::Text => rsx! {
                    span { class: "skeuo-mobile-stepper__text", "{active + 1} / {steps}" }
                },
            }
            span { class: "skeuo-mobile-stepper__end", {props.next_button} }
        }
    }
}
