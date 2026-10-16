use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

/// Small local layout helpers for this page.
const FEEDBACK_CSS: &str = r#"
.fb-stack { display: grid; gap: 10px; }
.fb-alerts { display: grid; grid-template-columns: repeat(auto-fill, minmax(380px, 1fr)); gap: 10px; margin-bottom: 18px; }
.fb-alerts:last-child { margin-bottom: 0; }
.fb-variant { margin: 0 0 8px; }
.fb-linear { display: grid; grid-template-columns: 92px 1fr 44px; align-items: center; gap: 14px; margin-bottom: 14px; }
.fb-linear:last-child { margin-bottom: 0; }
.fb-value { position: relative; display: inline-flex; align-items: center; justify-content: center; }
.fb-value > span.skeuo-caption { position: absolute; font-size: 11px; font-weight: 600; color: var(--sk-ink-2); }
.fb-card { padding: 16px; display: grid; gap: 12px; }
.fb-card__head { display: flex; align-items: center; gap: 12px; }
.fb-card__head > div { flex: 1; }
.fb-stage { position: relative; display: flex; align-items: center; justify-content: center; height: 170px; padding: 16px; overflow: hidden; }
.fb-dialogs { display: grid; grid-template-columns: repeat(auto-fill, minmax(380px, 1fr)); gap: 20px; margin-bottom: 18px; }
.fb-snacks { display: flex; flex-wrap: wrap; gap: 16px; margin-bottom: 16px; }
.fb-modal-panel { padding: 22px 24px; width: 360px; max-width: 100%; }
.fb-modal-panel h3 { margin: 0 0 6px; font-size: 17px; letter-spacing: -.015em; }
.fb-modal-panel p { margin: 0 0 16px; }
.fb-icon-btn { min-width: 0 !important; padding: 0 7px !important; }
"#;

#[component]
pub fn FeedbackSection() -> Element {
    rsx! {
        style { {FEEDBACK_CSS} }
        AlertDemo {}
        ProgressDemo {}
        SkeletonDemo {}
        DialogDemo {}
        SnackbarDemo {}
        BackdropModalDemo {}
    }
}

#[component]
fn AlertDemo() -> Element {
    let mut dismissed = use_signal(|| false);
    rsx! {
        Demo {
            title: "Alert",
            description: "standard = aluminium washed with tint · filled = flat colored plate · outlined = colored hairline edge",
            for variant in AlertVariant::ALL {
                p { class: "skeuo-label fb-variant", "{variant.as_str()}" }
                div { class: "fb-alerts",
                    for severity in AlertSeverity::ALL {
                        Alert { severity, variant,
                            "This is a {severity.as_str()} alert — check it out."
                        }
                    }
                }
            }
            p { class: "skeuo-label fb-variant", "title, action, close, custom icon" }
            div { class: "fb-alerts",
                Alert { severity: AlertSeverity::Error,
                    AlertTitle { "Stream offline" }
                    "The encoder lost its connection at 14:02."
                }
                Alert {
                    severity: AlertSeverity::Info,
                    action: rsx! {
                        Button { variant: ButtonVariant::Text, color: Color::Info, size: Size::Small, "Retry" }
                    },
                    "Reconnecting to the studio…"
                }
                if dismissed() {
                    Button {
                        variant: ButtonVariant::Outlined,
                        color: Color::Neutral,
                        onclick: move |_| dismissed.set(false),
                        "Bring the alert back"
                    }
                } else {
                    Alert {
                        severity: AlertSeverity::Warning,
                        variant: AlertVariant::Outlined,
                        onclose: move |_| dismissed.set(true),
                        "Dismiss me with the close button."
                    }
                }
                Alert {
                    severity: AlertSeverity::Success,
                    variant: AlertVariant::Filled,
                    icon: rsx! { Icon { name: IconName::Mic } },
                    "On air: custom icon."
                }
                Alert { severity: AlertSeverity::Info, hide_icon: true, "No icon, just the message." }
            }
        }
    }
}

#[component]
fn ProgressDemo() -> Element {
    let mut progress = use_signal(|| 40.0f64);
    rsx! {
        Demo {
            title: "Progress",
            description: "CircularProgress = ring groove with a colored arc · LinearProgress = groove with a colored fill",
            DemoRow { label: "indeterminate",
                for color in [Color::Primary, Color::Secondary, Color::Success, Color::Error, Color::Warning] {
                    CircularProgress { color, aria_label: "Loading" }
                }
                CircularProgress { disable_shrink: true, aria_label: "Loading" }
                CircularProgress { size: 24, thickness: 4.5, color: Color::Info, aria_label: "Loading" }
            }
            DemoRow { label: "determinate",
                for (value, color) in [(0.0, Color::Primary), (25.0, Color::Primary), (50.0, Color::Success), (75.0, Color::Warning), (100.0, Color::Error)] {
                    span { class: "fb-value",
                        CircularProgress { variant: CircularProgressVariant::Determinate, value, color, size: 52, thickness: 4.0 }
                        span { class: "skeuo-caption", "{value}%" }
                    }
                }
                CircularProgress { variant: CircularProgressVariant::Determinate, value: 66.0, size: 28, thickness: 5.0, color: Color::Info }
                CircularProgress { variant: CircularProgressVariant::Determinate, value: 85.0, size: 72, thickness: 3.0, color: Color::Secondary }
            }
            div { style: "height: 8px" }
            for (label, value, color) in [("determinate", 30.0, Color::Primary), ("success", 62.0, Color::Success), ("warning", 88.0, Color::Warning), ("error", 12.0, Color::Error)] {
                div { class: "fb-linear",
                    span { class: "demo__row-label", "{label}" }
                    LinearProgress { variant: LinearProgressVariant::Determinate, value, color }
                    span { class: "skeuo-caption", "{value}%" }
                }
            }
            div { class: "fb-linear",
                span { class: "demo__row-label", "buffer" }
                LinearProgress { variant: LinearProgressVariant::Buffer, value: 45.0, value_buffer: 70.0, color: Color::Info }
                span { class: "skeuo-caption", "45%" }
            }
            div { class: "fb-linear",
                span { class: "demo__row-label", "indeterminate" }
                LinearProgress { aria_label: "Loading" }
                span {}
            }
            div { class: "fb-linear",
                span { class: "demo__row-label", "query" }
                LinearProgress { variant: LinearProgressVariant::Query, color: Color::Secondary, aria_label: "Preparing" }
                span {}
            }
            div { class: "fb-linear",
                span { class: "demo__row-label", "interactive" }
                LinearProgress { variant: LinearProgressVariant::Determinate, value: progress(), color: Color::Success }
                span { class: "skeuo-caption", "{progress}%" }
            }
            DemoRow {
                label: "",
                Button {
                    size: Size::Small,
                    color: Color::Success,
                    onclick: move |_| progress.set(if progress() >= 100.0 { 0.0 } else { (progress() + 10.0).min(100.0) }),
                    "Advance 10%"
                }
                CircularProgress { variant: CircularProgressVariant::Determinate, value: progress(), color: Color::Success, size: 28 }
            }
        }
    }
}

#[component]
fn SkeletonDemo() -> Element {
    rsx! {
        Demo {
            title: "Skeleton",
            description: "shallow wells pressed where content will land · text / rectangular / rounded / circular · pulse / wave",
            DemoGrid {
                div { class: "skeuo-chassis skeuo-elev-1 fb-card",
                    div { class: "fb-card__head",
                        Skeleton { variant: SkeletonVariant::Circular, width: "40px", height: "40px" }
                        div {
                            Skeleton { width: "70%" }
                            Skeleton { width: "40%" }
                        }
                    }
                    Skeleton { variant: SkeletonVariant::Rounded, height: "110px" }
                    div {
                        Skeleton {}
                        Skeleton {}
                        Skeleton { width: "60%" }
                    }
                }
                div { class: "skeuo-chassis skeuo-elev-1 fb-card",
                    div { class: "fb-card__head",
                        Skeleton { variant: SkeletonVariant::Circular, animation: SkeletonAnimation::Wave, width: "40px", height: "40px" }
                        div {
                            Skeleton { animation: SkeletonAnimation::Wave, width: "80%" }
                            Skeleton { animation: SkeletonAnimation::Wave, width: "50%" }
                        }
                    }
                    Skeleton { variant: SkeletonVariant::Rectangular, animation: SkeletonAnimation::Wave, height: "110px" }
                    Skeleton { animation: SkeletonAnimation::Wave }
                    Skeleton { animation: SkeletonAnimation::Wave, width: "75%" }
                }
                div { class: "fb-stack",
                    span { class: "skeuo-label", "variants (animation = false)" }
                    Skeleton { animation: SkeletonAnimation::False, width: "100%" }
                    Skeleton { variant: SkeletonVariant::Rectangular, animation: SkeletonAnimation::False, width: "100%", height: "40px" }
                    Skeleton { variant: SkeletonVariant::Rounded, animation: SkeletonAnimation::False, width: "100%", height: "40px" }
                    Skeleton { variant: SkeletonVariant::Circular, animation: SkeletonAnimation::False, width: "40px", height: "40px" }
                    span { class: "skeuo-label", "sized by children" }
                    Skeleton { variant: SkeletonVariant::Rounded,
                        Button { "Hidden button" }
                    }
                }
            }
        }
    }
}

#[component]
fn DialogDemo() -> Element {
    let mut open = use_signal(|| false);
    let mut answer = use_signal(|| "none yet".to_string());
    rsx! {
        Demo {
            title: "Dialog",
            description: "a level-4 floating chassis over the scrim · actions are right-aligned keycaps · previews shown in flow",
            div { class: "fb-dialogs",
                Dialog { open: true, inline: true, max_width: DialogMaxWidth::Xs,
                    DialogTitle { "Use location service?" }
                    DialogContent {
                        DialogContentText {
                            "Let the app determine your location. Anonymous data is sent to the station, even when no apps are running."
                        }
                    }
                    DialogActions {
                        Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "Disagree" }
                        Button { "Agree" }
                    }
                }
                Dialog { open: true, inline: true, full_width: true,
                    DialogTitle { "Schedule announcement" }
                    DialogContent { dividers: true,
                        DialogContentText { "Pick a slot. The clip plays at the top of the hour." }
                        TextField { label: "Title", default_value: "Evening news", full_width: true }
                        div { style: "height: 12px" }
                        LinearProgress { variant: LinearProgressVariant::Determinate, value: 72.0 }
                    }
                    DialogActions {
                        Button { variant: ButtonVariant::Text, "Cancel" }
                        Button { color: Color::Success, "Schedule" }
                    }
                }
            }
            DemoRow { label: "interactive",
                Button { onclick: move |_| open.set(true), "Open dialog" }
                span { class: "skeuo-caption", "Last answer: {answer}" }
            }
            Dialog {
                open: open(),
                onclose: move |reason: ModalCloseReason| {
                    answer.set(format!("dismissed ({})", reason.as_str()));
                    open.set(false);
                },
                aria_describedby: "fb-dialog-desc",
                DialogTitle { "Discard draft?" }
                DialogContent {
                    DialogContentText { id: "fb-dialog-desc",
                        "Your unsaved script will be lost. Press Escape or click the scrim to keep editing."
                    }
                }
                DialogActions {
                    Button {
                        variant: ButtonVariant::Outlined,
                        color: Color::Neutral,
                        onclick: move |_| {
                            answer.set("kept".into());
                            open.set(false);
                        },
                        "Keep editing"
                    }
                    Button {
                        color: Color::Error,
                        onclick: move |_| {
                            answer.set("discarded".into());
                            open.set(false);
                        },
                        "Discard"
                    }
                }
            }
        }
    }
}

#[component]
fn SnackbarDemo() -> Element {
    let mut open = use_signal(|| false);
    let mut origin = use_signal(SnackbarOrigin::default);
    let mut closed_by = use_signal(|| "—".to_string());
    // Bumping the key remounts the snackbar, restarting its countdown.
    let mut shown = use_signal(|| 0u32);
    let mut show = move |o: SnackbarOrigin| {
        origin.set(o);
        shown += 1;
        open.set(true);
    };
    rsx! {
        Demo {
            title: "Snackbar",
            description: "a light level-3 floating chassis that rises in; auto-hide is a CSS countdown (paused on hover)",
            div { class: "fb-snacks",
                Snackbar { open: true, inline: true, message: "Clip saved to the library." }
                Snackbar {
                    open: true,
                    inline: true,
                    message: "Announcement deleted.",
                    action: rsx! {
                        Button { variant: ButtonVariant::Text, size: Size::Small, "Undo" }
                        Button {
                            variant: ButtonVariant::Text,
                            color: Color::Neutral,
                            size: Size::Small,
                            class: "fb-icon-btn",
                            aria_label: "Close",
                            Icon { name: IconName::Close }
                        }
                    },
                }
                Snackbar { open: true, inline: true,
                    Alert { severity: AlertSeverity::Success, variant: AlertVariant::Filled, "Broadcast scheduled." }
                }
            }
            DemoRow { label: "interactive",
                Button {
                    onclick: move |_| show(SnackbarOrigin::default()),
                    "Bottom left"
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    onclick: move |_| show(SnackbarOrigin::new(SnackbarVertical::Top, SnackbarHorizontal::Center)),
                    "Top center"
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    onclick: move |_| show(SnackbarOrigin::new(SnackbarVertical::Bottom, SnackbarHorizontal::Right)),
                    "Bottom right"
                }
                span { class: "skeuo-caption", "Last closed by: {closed_by}" }
            }
            for n in [shown()] {
                Snackbar {
                    key: "{n}",
                    open: open(),
                    anchor_origin: origin(),
                    auto_hide_duration: 4000,
                    message: "Snackbar from the gallery — hides in 4 s.",
                    onclose: move |reason: SnackbarCloseReason| {
                        closed_by.set(reason.as_str().to_string());
                        open.set(false);
                    },
                    action: rsx! {
                        Button {
                            variant: ButtonVariant::Text,
                            size: Size::Small,
                            onclick: move |_| {
                                closed_by.set("action".into());
                                open.set(false);
                            },
                            "Dismiss"
                        }
                    },
                }
            }
        }
    }
}

#[component]
fn BackdropModalDemo() -> Element {
    let mut backdrop = use_signal(|| false);
    let mut modal = use_signal(|| false);
    rsx! {
        Demo {
            title: "Backdrop & Modal",
            description: "the scrim dims the desk; a modal centers focusable content on it (Escape or a scrim click closes)",
            DemoGrid {
                div { class: "skeuo-well fb-stage",
                    span { class: "skeuo-caption", "Content under the backdrop" }
                    Backdrop { open: true, inline: true,
                        CircularProgress { color: Color::Neutral, aria_label: "Loading" }
                    }
                }
                div { class: "fb-stack",
                    span { class: "skeuo-label", "interactive" }
                    DemoRow {
                        Button { onclick: move |_| backdrop.set(true), "Show backdrop" }
                        Button {
                            variant: ButtonVariant::Outlined,
                            onclick: move |_| modal.set(true),
                            "Open modal"
                        }
                    }
                    span { class: "skeuo-caption", "Click the backdrop to dismiss it." }
                }
            }
            Backdrop { open: backdrop(), onclick: move |_| backdrop.set(false),
                CircularProgress { color: Color::Neutral, size: 48, aria_label: "Loading" }
            }
            Modal {
                open: modal(),
                onclose: move |_| modal.set(false),
                aria_labelledby: "fb-modal-title",
                div { class: "skeuo-chassis skeuo-chassis--float skeuo-elev-4 fb-modal-panel",
                    h3 { id: "fb-modal-title", "A bare modal" }
                    p { class: "skeuo-caption",
                        "Modal is the primitive under Dialog: scrim, focus and Escape handling, and nothing else."
                    }
                    Button { onclick: move |_| modal.set(false), "Close" }
                }
            }
        }
    }
}
