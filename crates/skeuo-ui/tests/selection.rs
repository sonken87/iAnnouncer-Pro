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

/// Number of tick marks (not mark labels) in the markup.
fn mark_count(html: &str) -> usize {
    html.matches(r#"class="skeuo-slider__mark"#).count()
        - html.matches(r#"class="skeuo-slider__mark-label"#).count()
}

#[test]
fn checkbox_default_is_unchecked_white_cap() {
    let html = render(|| rsx! { Checkbox { aria_label: "Mute" } });
    assert_all(
        &html,
        &[
            r#"class="skeuo-checkbox skeuo-color-primary skeuo-checkbox--medium""#,
            "skeuo-checkbox__cap skeuo-key skeuo-key--pressable skeuo-key--focus-within",
            r#"type="checkbox""#,
            r#"aria-label="Mute""#,
        ],
    );
    assert!(!html.contains("skeuo-key--filled"), "{html}");
    assert!(!html.contains("checked"), "{html}");
    assert!(!html.contains("<svg"), "{html}");
}

#[test]
fn checkbox_checked_states_and_props() {
    let html = render(|| {
        rsx! {
            Checkbox {
                checked: true,
                color: Color::Success,
                size: Size::Large,
                disabled: true,
                required: true,
                name: "agree",
                value: "yes",
                id: "cb",
                class: "mine",
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-checkbox--checked",
            "skeuo-color-success",
            "skeuo-checkbox--large",
            "skeuo-checkbox--disabled",
            "skeuo-key--filled",
            "mine",
            r#"aria-disabled="true""#,
            r#"name="agree""#,
            r#"value="yes""#,
            r#"id="cb""#,
            "checked",
            "disabled",
            "required",
            "<svg",
        ],
    );
}

#[test]
fn checkbox_indeterminate_and_uncontrolled_default() {
    let html = render(|| {
        rsx! {
            Checkbox { indeterminate: true }
            Checkbox { default_checked: true, class: "second" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-checkbox--indeterminate",
            r#"aria-checked="mixed""#,
            "M5 12h14", // the dash
            "skeuo-checkbox--checked",
        ],
    );
    assert_eq!(html.matches("skeuo-key--filled").count(), 2, "{html}");
}

#[test]
fn checkbox_controlled_value_wins_over_default() {
    let html = render(|| rsx! { Checkbox { checked: false, default_checked: true } });
    assert!(!html.contains("skeuo-checkbox--checked"), "{html}");
}

#[test]
fn radio_group_provides_name_value_and_color() {
    let html = render(|| {
        rsx! {
            RadioGroup {
                name: "band",
                default_value: "fm",
                row: true,
                color: Color::Error,
                aria_label: "Band",
                Radio { value: "am" }
                Radio { value: "fm" }
                Radio { value: "dab", color: Color::Info }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"role="radiogroup""#,
            "skeuo-radio-group--row",
            r#"aria-label="Band""#,
            "skeuo-color-error",
            "skeuo-color-info",
        ],
    );
    assert_eq!(html.matches(r#"name="band""#).count(), 3, "{html}");
    assert_eq!(html.matches("skeuo-radio--checked").count(), 1, "{html}");
    assert_eq!(html.matches("skeuo-led--off").count(), 2, "{html}");
    // The checked one is the "fm" radio.
    let checked_at = html.find("skeuo-radio--checked").unwrap();
    let fm_at = html.find(r#"value="fm""#).unwrap();
    let dab_at = html.find(r#"value="dab""#).unwrap();
    assert!(checked_at < fm_at && fm_at < dab_at, "{html}");
}

#[test]
fn radio_group_controlled_value_and_generated_name() {
    let html = render(|| {
        rsx! {
            RadioGroup { value: "b", default_value: "a", disabled: true,
                Radio { value: "a" }
                Radio { value: "b" }
            }
        }
    });
    assert_eq!(html.matches("skeuo-radio--checked").count(), 1, "{html}");
    assert!(html.contains(r#"name="skeuo-radio-group-"#), "{html}");
    assert_eq!(html.matches("skeuo-radio--disabled").count(), 2, "{html}");
    assert!(html.find("skeuo-radio--checked").unwrap() > html.find(r#"value="a""#).unwrap());
}

#[test]
fn standalone_radio_is_controlled_by_checked() {
    let html = render(|| {
        rsx! {
            Radio { value: "x", checked: true, size: Size::Small }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-radio--checked",
            "skeuo-radio--small",
            "skeuo-key--round",
            "skeuo-key--down",
            r#"type="radio""#,
            "skeuo-radio__led skeuo-led",
        ],
    );
}

#[test]
fn switch_has_switch_role_and_states() {
    let html = render(|| {
        rsx! {
            Switch { aria_label: "On air" }
            Switch { default_checked: true, color: Color::Error, size: Size::Small, disabled: true }
        }
    });
    assert_all(
        &html,
        &[
            r#"role="switch""#,
            r#"aria-label="On air""#,
            r#"aria-checked="false""#,
            r#"aria-checked="true""#,
            "skeuo-switch__track skeuo-groove",
            "skeuo-switch__fill skeuo-groove__fill",
            "skeuo-switch__thumb skeuo-key skeuo-key--round",
            "skeuo-switch--checked",
            "skeuo-switch--small",
            "skeuo-switch--disabled",
            "skeuo-color-error",
        ],
    );
    assert_eq!(html.matches("skeuo-switch--checked").count(), 1, "{html}");
}

#[test]
fn slider_single_value_track_and_label() {
    let html = render(|| {
        rsx! {
            Slider {
                value: 25.0,
                min: 0.0,
                max: 200.0,
                step: 5.0,
                aria_label: "Volume",
                value_label_display: ValueLabelDisplay::On,
                value_label_format: |v: f64| format!("{v} dB"),
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-slider skeuo-color-primary skeuo-slider--medium skeuo-slider--horizontal",
            r#"type="range""#,
            r#"min="0""#,
            r#"max="200""#,
            r#"step="5""#,
            r#"value="25""#,
            r#"aria-label="Volume""#,
            r#"aria-valuetext="25 dB""#,
            "--a: 0%; --b: 12.5%;",
            "--p: 12.5%;",
            "skeuo-slider__value-label--on",
            ">25 dB<",
        ],
    );
    assert_eq!(html.matches("skeuo-slider__thumb ").count(), 1, "{html}");
}

#[test]
fn slider_range_has_two_thumbs_and_marks() {
    let html = render(|| {
        rsx! {
            Slider {
                default_value: (20.0, 60.0),
                aria_label: "Band",
                marks: vec![SliderMark::new(0.0, "Low"), SliderMark::tick(40.0), SliderMark::new(100.0, "High")],
                color: Color::Secondary,
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-slider--range",
            "skeuo-slider--labelled",
            "skeuo-color-secondary",
            "--a: 20%; --b: 60%;",
            r#"aria-label="Band start""#,
            r#"aria-label="Band end""#,
            "skeuo-slider__mark skeuo-slider__mark--active",
            ">Low<",
            ">High<",
        ],
    );
    assert_eq!(html.matches(r#"type="range""#).count(), 2, "{html}");
    assert_eq!(mark_count(&html), 3, "{html}");
    assert_eq!(
        html.matches("skeuo-slider__mark--active").count(),
        1,
        "{html}"
    );
    assert!(!html.contains("skeuo-slider__value-label"), "{html}");
}

#[test]
fn slider_vertical_step_marks_and_disabled() {
    let html = render(|| {
        rsx! {
            Slider {
                orientation: Orientation::Vertical,
                default_value: 0.5,
                min: 0.0,
                max: 1.0,
                step: 0.25,
                marks: true,
                disabled: true,
                track: SliderTrack::Inverted,
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-slider--vertical",
            "skeuo-slider--disabled",
            r#"aria-orientation="vertical""#,
            r#"aria-valuetext="0.50""#,
            "--a: 50%; --b: 100%;",
            "disabled",
        ],
    );
    assert_eq!(mark_count(&html), 5, "{html}");
}

#[test]
fn slider_value_helpers() {
    assert_eq!(SliderValue::from(3.0).single(), 3.0);
    assert_eq!(SliderValue::from((1.0, 2.0)).range(), (1.0, 2.0));
    assert!(SliderValue::from([1.0, 2.0]).is_range());
    assert_eq!(SliderValue::Single(4.0).range(), (4.0, 4.0));
}

#[test]
fn rating_interactive_is_a_radio_group() {
    let html = render(|| {
        rsx! {
            Rating { default_value: 3.0, name: "score", aria_label: "Score" }
        }
    });
    assert_all(
        &html,
        &[
            r#"role="radiogroup""#,
            r#"aria-label="Score""#,
            r#"aria-label="1 Star""#,
            r#"aria-label="5 Stars""#,
            "skeuo-color-warning",
        ],
    );
    assert_eq!(html.matches(r#"type="radio""#).count(), 5, "{html}");
    assert_eq!(html.matches(r#"name="score""#).count(), 5, "{html}");
    assert_eq!(html.matches("width: 100%;").count(), 3, "{html}");
    assert_eq!(html.matches("width: 0%;").count(), 2, "{html}");
    assert_eq!(html.matches("checked").count(), 1, "{html}");
}

#[test]
fn rating_half_precision_and_read_only() {
    let html = render(|| {
        rsx! {
            Rating { value: Some(2.5), precision: 0.5, max: 4 }
            Rating { read_only: true, value: Some(3.5), precision: 0.5 }
        }
    });
    // Interactive half precision: two radios per star.
    assert_eq!(html.matches(r#"type="radio""#).count(), 8, "{html}");
    assert!(html.contains(r#"aria-label="2.5 Stars""#), "{html}");
    assert!(html.contains("width: 50%;"), "{html}");
    assert_all(
        &html,
        &[
            "skeuo-rating--read-only",
            r#"role="img""#,
            r#"aria-label="Rated 3.5 out of 5""#,
        ],
    );
}

#[test]
fn rating_disabled_and_sizes() {
    let html = render(|| rsx! { Rating { disabled: true, size: Size::Large } });
    assert_all(
        &html,
        &[
            "skeuo-rating--disabled",
            "skeuo-rating--large",
            r#"aria-disabled="true""#,
        ],
    );
    assert_eq!(html.matches("disabled=true").count(), 5, "{html}");
}

#[test]
fn form_control_shares_disabled_error_and_size() {
    let html = render(|| {
        rsx! {
            FormControl { disabled: true, error: true, required: true, size: Size::Small, fieldset: true,
                FormLabel { legend: true, "Bulletins" }
                FormGroup { row: true,
                    FormControlLabel { control: rsx! { Checkbox {} }, label: "News" }
                    FormControlLabel { control: rsx! { Switch {} }, label: "Weather" }
                }
                FormHelperText { "Pick one" }
            }
        }
    });
    assert_all(
        &html,
        &[
            "<fieldset",
            "skeuo-form-control--error",
            "skeuo-form-control--disabled",
            "<legend",
            "skeuo-form-label skeuo-label skeuo-form-label--error skeuo-form-label--disabled",
            "skeuo-form-label__asterisk",
            "skeuo-form-group skeuo-form-group--row",
            r#"role="group""#,
            "skeuo-checkbox--small",
            "skeuo-checkbox--disabled",
            "skeuo-switch--small",
            "skeuo-switch--disabled",
            "skeuo-form-control-label--disabled",
            "skeuo-form-helper-text skeuo-caption skeuo-form-helper-text--error",
        ],
    );
}

#[test]
fn form_control_label_placement_and_disabled_reaches_control() {
    let html = render(|| {
        rsx! {
            FormControlLabel {
                control: rsx! { Radio { value: "a", checked: true } },
                label: "Studio A",
                label_placement: LabelPlacement::Top,
                disabled: true,
                required: true,
            }
            FormControlLabel { control: rsx! { Checkbox {} }, label: "Plain" }
        }
    });
    assert_all(
        &html,
        &[
            "<label",
            "skeuo-form-control-label--top",
            "skeuo-form-control-label--end",
            "skeuo-radio--disabled",
            "Studio A",
            "skeuo-form-label__asterisk",
            "required",
        ],
    );
    assert_eq!(
        html.matches("skeuo-form-control-label--disabled").count(),
        1,
        "{html}"
    );
    assert_eq!(
        html.matches("skeuo-checkbox--disabled").count(),
        0,
        "{html}"
    );
}

#[test]
fn form_label_and_helper_text_standalone() {
    let html = render(|| {
        rsx! {
            FormLabel { id: "lbl", html_for: "x", "Name" }
            FormHelperText { id: "help", error: true, "Too short" }
        }
    });
    assert_all(
        &html,
        &[
            r#"<label class="skeuo-form-label skeuo-label" id="lbl" for="x""#,
            r#"id="help""#,
            "skeuo-form-helper-text--error",
        ],
    );
    assert!(!html.contains("asterisk"), "{html}");
}
