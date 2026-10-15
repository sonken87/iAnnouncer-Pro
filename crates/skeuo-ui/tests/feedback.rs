use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

fn assert_all(html: &str, needles: &[&str]) {
    for needle in needles {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
}

// ---------------------------------------------------------------- Alert

#[test]
fn alert_defaults() {
    let html = render(|| rsx! { Alert { "Saved" } });
    assert_all(
        &html,
        &[
            r#"class="skeuo-chassis skeuo-elev-0 skeuo-alert skeuo-alert--standard skeuo-color-success""#,
            r#"role="alert""#,
            "skeuo-alert__icon",
            "Saved",
        ],
    );
    assert!(!html.contains("skeuo-alert__close"), "{html}");
}

#[test]
fn alert_variants_and_severity_colors() {
    let html = render(|| {
        rsx! {
            Alert { severity: AlertSeverity::Error, variant: AlertVariant::Filled, "a" }
            Alert { severity: AlertSeverity::Warning, variant: AlertVariant::Outlined, "b" }
            Alert { severity: AlertSeverity::Info, color: Color::Secondary, "c" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-key skeuo-key--filled skeuo-alert skeuo-alert--filled skeuo-color-error",
            "skeuo-alert--outlined skeuo-color-warning",
            "skeuo-alert--standard skeuo-color-secondary",
        ],
    );
}

#[test]
fn alert_close_button_title_action_and_icon() {
    let html = render(|| {
        rsx! {
            Alert { onclose: move |_| {}, class: "mine", AlertTitle { "Heads up" } "Body" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-alert__close",
            r#"aria-label="Close""#,
            r#"type="button""#,
            r#"class="skeuo-alert__title""#,
            "Heads up",
            "mine",
        ],
    );

    let html = render(|| {
        rsx! {
            Alert { onclose: move |_| {}, action: rsx! { button { "Undo" } }, hide_icon: true, "x" }
        }
    });
    assert!(html.contains("Undo"), "{html}");
    assert!(!html.contains("skeuo-alert__close"), "{html}");
    assert!(!html.contains("skeuo-alert__icon"), "{html}");
}

// ---------------------------------------------------------------- Backdrop / Modal

#[test]
fn backdrop_open_closed_invisible_inline() {
    assert!(!render(|| rsx! { Backdrop { "x" } }).contains("skeuo-backdrop"));
    let html =
        render(|| rsx! { Backdrop { open: true, invisible: true, inline: true, "spinner" } });
    assert_all(
        &html,
        &[
            "skeuo-scrim skeuo-backdrop skeuo-backdrop--invisible skeuo-backdrop--inline",
            "spinner",
        ],
    );
}

#[test]
fn modal_renders_scrim_and_aria_modal_content() {
    let html = render(|| {
        rsx! {
            Modal { open: true, aria_labelledby: "t", p { id: "t", "Hello" } }
        }
    });
    assert_all(
        &html,
        &[
            r#"class="skeuo-modal""#,
            r#"role="presentation""#,
            "skeuo-scrim skeuo-modal__scrim",
            r#"role="dialog""#,
            r#"aria-modal="true""#,
            r#"aria-labelledby="t""#,
            r#"tabindex="-1""#,
            "Hello",
        ],
    );
}

#[test]
fn modal_closed_hide_backdrop_keep_mounted() {
    assert!(!render(|| rsx! { Modal { open: false, "x" } }).contains("skeuo-modal"));
    let html = render(|| rsx! { Modal { open: false, keep_mounted: true, "kept" } });
    assert_all(&html, &["skeuo-modal--hidden", "hidden", "kept"]);
    let html = render(|| rsx! { Modal { open: true, hide_backdrop: true, "x" } });
    assert!(!html.contains("skeuo-scrim"), "{html}");
}

// ---------------------------------------------------------------- Dialog

#[test]
fn dialog_closed_renders_nothing() {
    let html = render(|| rsx! { Dialog { open: false, DialogTitle { "T" } } });
    assert!(!html.contains("skeuo-dialog"), "{html}");
}

#[test]
fn dialog_open_structure_and_labelling() {
    let html = render(|| {
        rsx! {
            Dialog { open: true, aria_describedby: "d",
                DialogTitle { "Delete?" }
                DialogContent { DialogContentText { id: "d", "Gone for good." } }
                DialogActions { button { "OK" } }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-dialog skeuo-dialog--max-sm skeuo-dialog--scroll-paper",
            "skeuo-scrim skeuo-dialog__scrim",
            "skeuo-chassis skeuo-chassis--float skeuo-elev-4 skeuo-dialog__paper",
            r#"role="dialog""#,
            r#"aria-modal="true""#,
            r#"aria-describedby="d""#,
            "skeuo-dialog__actions skeuo-dialog__actions--spacing",
            r#"class="skeuo-dialog__text" id="d""#,
        ],
    );
    // The title's id is the one the dialog is labelled by.
    let labelled = html
        .split(r#"aria-labelledby=""#)
        .nth(1)
        .and_then(|s| s.split('"').next())
        .expect("aria-labelledby");
    assert!(labelled.starts_with("skeuo-dialog-title-"), "{html}");
    assert!(html.contains(&format!(r#"id="{labelled}""#)), "{html}");
}

#[test]
fn dialog_options_and_inline_preview() {
    let html = render(|| {
        rsx! {
            Dialog {
                open: true,
                inline: true,
                full_width: true,
                full_screen: true,
                max_width: DialogMaxWidth::Xs,
                scroll: DialogScroll::Body,
                aria_labelledby: "custom",
                DialogTitle { "T" }
                DialogContent { dividers: true, "c" }
                DialogActions { disable_spacing: true, "a" }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-dialog--max-xs",
            "skeuo-dialog--scroll-body",
            "skeuo-dialog--full-width",
            "skeuo-dialog--full-screen",
            "skeuo-dialog--inline",
            "skeuo-chassis--square",
            r#"aria-modal="false""#,
            r#"aria-labelledby="custom""#,
            r#"id="custom""#,
            "skeuo-dialog__content--dividers",
        ],
    );
    assert!(!html.contains("skeuo-scrim"), "{html}");
    assert!(!html.contains("skeuo-dialog__actions--spacing"), "{html}");
}

// ---------------------------------------------------------------- Progress

#[test]
fn circular_progress_indeterminate() {
    let html = render(|| rsx! { CircularProgress { aria_label: "Loading" } });
    assert_all(
        &html,
        &[
            "skeuo-circular skeuo-circular--indeterminate skeuo-color-primary",
            r#"role="progressbar""#,
            r#"aria-label="Loading""#,
            "width:40px;height:40px",
            "skeuo-groove skeuo-circular__track",
            "skeuo-circular__arc",
        ],
    );
    assert!(!html.contains("aria-valuenow"), "{html}");
}

#[test]
fn circular_progress_determinate_value() {
    let html = render(|| {
        rsx! {
            CircularProgress {
                variant: CircularProgressVariant::Determinate,
                value: 150.0,
                size: 60,
                color: Color::Success,
                disable_shrink: true,
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-circular--determinate skeuo-color-success",
            r#"aria-valuenow="100""#,
            r#"aria-valuemin="0""#,
            r#"aria-valuemax="100""#,
            "width:60px",
            "stroke-dashoffset:0",
        ],
    );
    // disable_shrink only applies to the indeterminate spinner.
    assert!(!html.contains("disable-shrink"), "{html}");
    let html = render(|| rsx! { CircularProgress { disable_shrink: true } });
    assert!(html.contains("skeuo-circular--disable-shrink"), "{html}");
}

#[test]
fn linear_progress_variants() {
    let html = render(|| {
        rsx! { LinearProgress { variant: LinearProgressVariant::Determinate, value: 42.0, color: Color::Error } }
    });
    assert_all(
        &html,
        &[
            "skeuo-groove skeuo-linear skeuo-linear--determinate skeuo-color-error",
            r#"role="progressbar""#,
            r#"aria-valuenow="42""#,
            "skeuo-groove__fill skeuo-linear__bar",
            "width:42%",
        ],
    );

    let html = render(|| {
        rsx! { LinearProgress { variant: LinearProgressVariant::Buffer, value: 10.0, value_buffer: 30.0 } }
    });
    assert_all(&html, &["skeuo-linear__buffer", "width:30%", "width:10%"]);

    let html = render(|| rsx! { LinearProgress { variant: LinearProgressVariant::Query } });
    assert_all(
        &html,
        &[
            "skeuo-linear--query",
            "skeuo-linear__bar--1",
            "skeuo-linear__bar--2",
        ],
    );
    assert!(!html.contains("aria-valuenow"), "{html}");
}

// ---------------------------------------------------------------- Skeleton

#[test]
fn skeleton_variants_and_sizes() {
    let html = render(|| rsx! { Skeleton {} });
    assert_all(
        &html,
        &[
            "skeuo-well skeuo-well--shallow skeuo-skeleton skeuo-skeleton--text skeuo-skeleton--pulse",
            r#"aria-hidden="true""#,
        ],
    );
    let html = render(|| {
        rsx! {
            Skeleton {
                variant: SkeletonVariant::Circular,
                animation: SkeletonAnimation::Wave,
                width: "40px",
                height: "40px",
            }
            Skeleton { variant: SkeletonVariant::Rounded, animation: SkeletonAnimation::False, "child" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-skeleton--circular skeuo-skeleton--wave",
            "width:40px;height:40px;",
            "skeuo-skeleton--rounded skeuo-skeleton--false",
            "child",
        ],
    );
}

// ---------------------------------------------------------------- Snackbar

#[test]
fn snackbar_closed_renders_nothing() {
    let html = render(|| rsx! { Snackbar { open: false, message: "Hi" } });
    assert!(!html.contains("skeuo-snackbar"), "{html}");
}

#[test]
fn snackbar_message_action_anchor_and_timer() {
    let html = render(|| {
        rsx! {
            Snackbar {
                open: true,
                message: "Saved",
                auto_hide_duration: 3000,
                anchor_origin: SnackbarOrigin::new(SnackbarVertical::Top, SnackbarHorizontal::Center),
                action: rsx! { button { "Undo" } },
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-snackbar skeuo-snackbar--top skeuo-snackbar--center",
            "skeuo-chassis skeuo-chassis--float skeuo-elev-3 skeuo-snackbar-content",
            r#"role="alert""#,
            "Saved",
            "Undo",
            "skeuo-snackbar__timer",
            "--snackbar-hide:3000ms",
        ],
    );
}

#[test]
fn snackbar_defaults_inline_and_children() {
    let html = render(|| {
        rsx! {
            Snackbar { open: true, inline: true, Alert { "Custom" } }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-snackbar--bottom skeuo-snackbar--left skeuo-snackbar--inline",
            "skeuo-alert",
            "Custom",
        ],
    );
    assert!(!html.contains("skeuo-snackbar-content"), "{html}");
    assert!(!html.contains("skeuo-snackbar__timer"), "{html}");

    let html = render(|| rsx! { SnackbarContent { message: "Plain" } });
    assert_all(&html, &["skeuo-snackbar-content", "Plain"]);
}
