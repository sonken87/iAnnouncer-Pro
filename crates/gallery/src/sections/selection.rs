use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

#[component]
pub fn SelectionSection() -> Element {
    rsx! {
        BroadcastSettingsDemo {}
        CheckboxDemo {}
        RadioDemo {}
        SwitchDemo {}
        SliderDemo {}
        RatingDemo {}
        FormDemo {}
    }
}

/// A small form that uses every control together.
#[component]
fn BroadcastSettingsDemo() -> Element {
    let mut on_air = use_signal(|| true);
    let mut output = use_signal(|| "fm".to_string());
    let mut volume = use_signal(|| 72.0);
    let mut window = use_signal(|| (20.0, 65.0));
    let mut duck = use_signal(|| true);
    let mut normalise = use_signal(|| false);
    let mut chime = use_signal(|| true);
    let mut quality = use_signal(|| Some(4.0));
    let options = [*duck.read(), *normalise.read(), *chime.read()];
    let enabled = options.iter().filter(|o| **o).count();
    let all = enabled == options.len();
    let some = enabled > 0 && !all;
    let (from, to) = window();
    let summary = format!(
        "{} · {} · volume {:.0}% · ducking {:.0}–{:.0}%",
        if on_air() { "On air" } else { "Off air" },
        match output.read().as_str() {
            "fm" => "FM 101.7",
            "dab" => "DAB+",
            _ => "Web stream",
        },
        volume(),
        from,
        to,
    );

    rsx! {
        Demo {
            title: "Broadcast settings",
            description: "Every control on one plate, wired with FormControl / FormLabel / FormControlLabel / FormGroup / FormHelperText",
            div { class: "selection-demo__form",
                div { class: "selection-demo__col",
                    FormControl { fieldset: true,
                        FormLabel { legend: true, "Transmitter" }
                        FormControlLabel {
                            control: rsx! {
                                Switch {
                                    checked: on_air(),
                                    color: Color::Error,
                                    onchange: move |_| on_air.toggle(),
                                }
                            },
                            label: if on_air() { "On air" } else { "Off air" },
                        }
                        FormHelperText {
                            if on_air() { "Listeners hear everything from now on." } else { "Rehearsal: nothing leaves the studio." }
                        }
                    }
                    FormControl { fieldset: true,
                        FormLabel { legend: true, id: "bs-output", "Output" }
                        RadioGroup {
                            row: true,
                            aria_labelledby: "bs-output",
                            value: output(),
                            onchange: move |v| output.set(v),
                            FormControlLabel { control: rsx! { Radio { value: "fm" } }, label: "FM" }
                            FormControlLabel { control: rsx! { Radio { value: "dab" } }, label: "DAB+" }
                            FormControlLabel { control: rsx! { Radio { value: "stream" } }, label: "Stream" }
                        }
                    }
                    FormControl { fieldset: true,
                        FormLabel { legend: true, id: "bs-voice", "Voice quality" }
                        Rating {
                            aria_labelledby: "bs-voice",
                            value: quality(),
                            onchange: move |v| quality.set(v),
                        }
                    }
                }
                div { class: "selection-demo__col",
                    FormControl { full_width: true,
                        FormLabel { id: "bs-volume", "Master volume" }
                        Slider {
                            aria_labelledby: "bs-volume",
                            value: volume(),
                            value_label_display: ValueLabelDisplay::Auto,
                            value_label_format: move |v: f64| format!("{v:.0}%"),
                            onchange: move |v: SliderValue| volume.set(v.single()),
                        }
                    }
                    FormControl { full_width: true,
                        FormLabel { id: "bs-window", "Ducking window" }
                        Slider {
                            aria_label: "Ducking window",
                            value: SliderValue::Range(from, to),
                            color: Color::Secondary,
                            step: 5.0,
                            marks: vec![
                                SliderMark::new(0.0, "0%"),
                                SliderMark::new(50.0, "50%"),
                                SliderMark::new(100.0, "100%"),
                            ],
                            onchange: move |v: SliderValue| window.set(v.range()),
                        }
                    }
                    FormControl { fieldset: true, error: enabled == 0,
                        FormLabel { legend: true, "Processing" }
                        FormControlLabel {
                            control: rsx! {
                                Checkbox {
                                    checked: all,
                                    indeterminate: some,
                                    onchange: move |_| {
                                        let v = !all;
                                        duck.set(v);
                                        normalise.set(v);
                                        chime.set(v);
                                    },
                                }
                            },
                            label: "All processing",
                        }
                        FormGroup { class: "selection-demo__nested",
                            FormControlLabel {
                                control: rsx! { Checkbox { checked: duck(), onchange: move |_| duck.toggle() } },
                                label: "Duck music under voice",
                            }
                            FormControlLabel {
                                control: rsx! { Checkbox { checked: normalise(), onchange: move |_| normalise.toggle() } },
                                label: "Normalise loudness",
                            }
                            FormControlLabel {
                                control: rsx! { Checkbox { checked: chime(), onchange: move |_| chime.toggle() } },
                                label: "Play chime before announcements",
                            }
                        }
                        FormHelperText {
                            if enabled == 0 { "Pick at least one option." } else { "{enabled} of 3 enabled" }
                        }
                    }
                }
            }
            p { class: "skeuo-caption selection-demo__summary", "{summary}" }
        }
    }
}

#[component]
fn CheckboxDemo() -> Element {
    let mut agreed = use_signal(|| false);
    rsx! {
        Demo {
            title: "Checkbox",
            description: "square white keycap · checked = colored cap with a check legend · indeterminate = dash",
            DemoRow { label: "colors",
                for color in Color::ALL {
                    Checkbox { color, default_checked: true, aria_label: color.as_str() }
                }
            }
            DemoRow { label: "states",
                Checkbox { aria_label: "Unchecked" }
                Checkbox { default_checked: true, aria_label: "Checked" }
                Checkbox { indeterminate: true, aria_label: "Indeterminate" }
                Checkbox { disabled: true, aria_label: "Disabled" }
                Checkbox { disabled: true, default_checked: true, aria_label: "Disabled checked" }
                Checkbox { disabled: true, indeterminate: true, aria_label: "Disabled indeterminate" }
            }
            DemoRow { label: "sizes",
                Checkbox { size: Size::Small, default_checked: true, aria_label: "Small" }
                Checkbox { size: Size::Medium, default_checked: true, aria_label: "Medium" }
                Checkbox { size: Size::Large, default_checked: true, aria_label: "Large" }
                Checkbox { size: Size::Small, aria_label: "Small" }
                Checkbox { size: Size::Medium, aria_label: "Medium" }
                Checkbox { size: Size::Large, aria_label: "Large" }
            }
            DemoRow { label: "labelled",
                FormControlLabel { control: rsx! { Checkbox { default_checked: true } }, label: "End" }
                FormControlLabel { control: rsx! { Checkbox {} }, label: "Start", label_placement: LabelPlacement::Start }
                FormControlLabel { control: rsx! { Checkbox {} }, label: "Top", label_placement: LabelPlacement::Top }
                FormControlLabel { control: rsx! { Checkbox { default_checked: true } }, label: "Bottom", label_placement: LabelPlacement::Bottom }
                FormControlLabel { control: rsx! { Checkbox {} }, label: "Disabled", disabled: true }
                FormControlLabel { control: rsx! { Checkbox {} }, label: "Required", required: true }
            }
            DemoRow { label: "controlled",
                FormControlLabel {
                    control: rsx! { Checkbox { checked: agreed(), color: Color::Success, onchange: move |e: FormEvent| agreed.set(e.checked()) } },
                    label: "I have read the station rules",
                }
                span { class: "skeuo-caption", if agreed() { "Thanks!" } else { "Not yet agreed" } }
            }
        }
    }
}

#[component]
fn RadioDemo() -> Element {
    let mut mode = use_signal(|| "live".to_string());
    rsx! {
        Demo {
            title: "Radio & RadioGroup",
            description: "small round keycap · selected = latched down with a lit LED in the cap",
            DemoRow { label: "colors",
                for color in Color::ALL {
                    Radio { color, checked: true, value: color.as_str(), aria_label: color.as_str() }
                }
            }
            DemoRow { label: "states",
                Radio { checked: false, value: "a", aria_label: "Unselected" }
                Radio { checked: true, value: "b", aria_label: "Selected" }
                Radio { checked: false, disabled: true, value: "c", aria_label: "Disabled" }
                Radio { checked: true, disabled: true, value: "d", aria_label: "Disabled selected" }
            }
            DemoRow { label: "sizes",
                RadioGroup { row: true, default_value: "m", aria_label: "Sizes",
                    Radio { size: Size::Small, value: "s", aria_label: "Small" }
                    Radio { size: Size::Medium, value: "m", aria_label: "Medium" }
                    Radio { size: Size::Large, value: "l", aria_label: "Large" }
                }
            }
            DemoGrid {
                FormControl { fieldset: true,
                    FormLabel { legend: true, id: "radio-mode", "Mode (controlled)" }
                    RadioGroup {
                        aria_labelledby: "radio-mode",
                        value: mode(),
                        onchange: move |v| mode.set(v),
                        FormControlLabel { control: rsx! { Radio { value: "live" } }, label: "Live" }
                        FormControlLabel { control: rsx! { Radio { value: "auto" } }, label: "Automation" }
                        FormControlLabel { control: rsx! { Radio { value: "off" } }, label: "Off" }
                    }
                    FormHelperText { "Selected: {mode}" }
                }
                FormControl { fieldset: true,
                    FormLabel { legend: true, id: "radio-color", "Group color (uncontrolled)" }
                    RadioGroup { aria_labelledby: "radio-color", default_value: "b", color: Color::Success,
                        FormControlLabel { control: rsx! { Radio { value: "a" } }, label: "Studio A" }
                        FormControlLabel { control: rsx! { Radio { value: "b" } }, label: "Studio B" }
                        FormControlLabel { control: rsx! { Radio { value: "c" } }, label: "Outside broadcast", disabled: true }
                    }
                }
                FormControl { fieldset: true, disabled: true,
                    FormLabel { legend: true, id: "radio-disabled", "Disabled group" }
                    RadioGroup { aria_labelledby: "radio-disabled", default_value: "x",
                        FormControlLabel { control: rsx! { Radio { value: "x" } }, label: "Mono" }
                        FormControlLabel { control: rsx! { Radio { value: "y" } }, label: "Stereo" }
                    }
                }
            }
        }
    }
}

#[component]
fn SwitchDemo() -> Element {
    let mut mic = use_signal(|| false);
    rsx! {
        Demo {
            title: "Switch",
            description: "groove with a sliding round keycap thumb · on = the groove lights in the palette color",
            DemoRow { label: "colors",
                for color in Color::ALL {
                    Switch { color, default_checked: true, aria_label: color.as_str() }
                }
            }
            DemoRow { label: "states",
                Switch { aria_label: "Off" }
                Switch { default_checked: true, aria_label: "On" }
                Switch { disabled: true, aria_label: "Disabled off" }
                Switch { disabled: true, default_checked: true, aria_label: "Disabled on" }
            }
            DemoRow { label: "sizes",
                Switch { size: Size::Small, default_checked: true, aria_label: "Small" }
                Switch { size: Size::Medium, default_checked: true, aria_label: "Medium" }
                Switch { size: Size::Large, default_checked: true, aria_label: "Large" }
                Switch { size: Size::Small, aria_label: "Small off" }
                Switch { size: Size::Medium, aria_label: "Medium off" }
                Switch { size: Size::Large, aria_label: "Large off" }
            }
            DemoRow { label: "labelled",
                FormGroup { row: true,
                    FormControlLabel {
                        control: rsx! { Switch { checked: mic(), color: Color::Success, onchange: move |_| mic.toggle() } },
                        label: if mic() { "Mic live" } else { "Mic muted" },
                    }
                    FormControlLabel { control: rsx! { Switch { default_checked: true } }, label: "Talkback" }
                    FormControlLabel { control: rsx! { Switch {} }, label: "Start", label_placement: LabelPlacement::Start }
                    FormControlLabel { control: rsx! { Switch {} }, label: "Disabled", disabled: true }
                }
            }
        }
    }
}

#[component]
fn SliderDemo() -> Element {
    let mut gain = use_signal(|| 30.0);
    let mut faders = use_signal(|| [80.0, 55.0, 35.0, 65.0]);
    let channel_marks = vec![
        SliderMark::new(0.0, "−∞"),
        SliderMark::new(50.0, "−12"),
        SliderMark::new(100.0, "0 dB"),
    ];
    rsx! {
        Demo {
            title: "Slider",
            description: "groove rail with a lit fill · round keycap thumbs · value label as a floating chassis bubble",
            div { class: "selection-demo__sliders",
                div { class: "selection-demo__stack",
                    DemoRow { label: "continuous",
                        Slider {
                            aria_label: "Gain",
                            value: gain(),
                            onchange: move |v: SliderValue| gain.set(v.single()),
                            class: "selection-demo__slider",
                        }
                        span { class: "skeuo-caption", "{gain():.0}" }
                    }
                    DemoRow { label: "value label",
                        Slider {
                            aria_label: "Level",
                            default_value: 64.0,
                            value_label_display: ValueLabelDisplay::On,
                            class: "selection-demo__slider selection-demo__slider--label",
                        }
                    }
                    DemoRow { label: "steps",
                        Slider {
                            aria_label: "Steps",
                            default_value: 40.0,
                            step: 10.0,
                            marks: true,
                            color: Color::Info,
                            class: "selection-demo__slider",
                        }
                    }
                    DemoRow { label: "labels",
                        Slider {
                            aria_label: "Temperature",
                            default_value: 20.0,
                            min: 0.0,
                            max: 40.0,
                            step: 0.5,
                            value_label_display: ValueLabelDisplay::Auto,
                            value_label_format: move |v: f64| format!("{v}°C"),
                            marks: vec![
                                SliderMark::new(0.0, "0°C"),
                                SliderMark::new(20.0, "20°C"),
                                SliderMark::new(37.0, "37°C"),
                            ],
                            color: Color::Warning,
                            class: "selection-demo__slider",
                        }
                    }
                    DemoRow { label: "range",
                        Slider {
                            aria_label: "Frequency band",
                            default_value: SliderValue::Range(25.0, 70.0),
                            color: Color::Secondary,
                            value_label_display: ValueLabelDisplay::Auto,
                            class: "selection-demo__slider",
                        }
                    }
                    DemoRow { label: "inverted",
                        Slider {
                            aria_label: "Inverted",
                            default_value: 35.0,
                            track: SliderTrack::Inverted,
                            color: Color::Error,
                            class: "selection-demo__slider",
                        }
                    }
                    DemoRow { label: "sizes",
                        Slider { aria_label: "Small", size: Size::Small, default_value: 30.0, class: "selection-demo__slider--third" }
                        Slider { aria_label: "Medium", default_value: 50.0, class: "selection-demo__slider--third" }
                        Slider { aria_label: "Large", size: Size::Large, default_value: 70.0, class: "selection-demo__slider--third" }
                    }
                    DemoRow { label: "colors",
                        for (i , color) in Color::ALL.into_iter().enumerate() {
                            Slider {
                                aria_label: color.as_str(),
                                color,
                                size: Size::Small,
                                default_value: 30.0 + 8.0 * i as f64,
                                class: "selection-demo__slider--seventh",
                            }
                        }
                    }
                    DemoRow { label: "disabled",
                        Slider { aria_label: "Disabled", disabled: true, default_value: 40.0, class: "selection-demo__slider" }
                    }
                }
                div { class: "selection-demo__mixer skeuo-chassis skeuo-elev-1",
                    for (i , (name , color)) in [("Mic 1", Color::Error), ("Mic 2", Color::Warning), ("Music", Color::Primary), ("Master", Color::Neutral)]
                        .into_iter()
                        .enumerate()
                    {
                        div { class: "selection-demo__channel",
                            Slider {
                                orientation: Orientation::Vertical,
                                aria_label: name,
                                color,
                                value: faders.read()[i],
                                marks: if i == 0 { channel_marks.clone() } else { vec![SliderMark::tick(0.0), SliderMark::tick(50.0), SliderMark::tick(100.0)] },
                                value_label_display: ValueLabelDisplay::Auto,
                                onchange: move |v: SliderValue| faders.write()[i] = v.single(),
                            }
                            span { class: "skeuo-label", "{name}" }
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn RatingDemo() -> Element {
    let mut score = use_signal(|| Some(3.5));
    let mut hover = use_signal(|| None::<f64>);
    let words = |v: f64| match v {
        v if v <= 1.0 => "Useless",
        v if v <= 2.0 => "Poor",
        v if v <= 3.0 => "OK",
        v if v <= 4.0 => "Good",
        _ => "Excellent",
    };
    let shown = hover().or(score());
    rsx! {
        Demo {
            title: "Rating",
            description: "stars printed on the aluminium · filled in warning ink, empty as outlines · hover previews",
            DemoRow { label: "controlled",
                Rating {
                    aria_label: "Show rating",
                    value: score(),
                    precision: 0.5,
                    onchange: move |v| score.set(v),
                    onchange_active: move |v| hover.set(v),
                }
                span { class: "skeuo-caption",
                    match shown {
                        Some(v) => format!("{v} · {}", words(v)),
                        None => "No rating".to_string(),
                    }
                }
            }
            DemoRow { label: "uncontrolled",
                Rating { aria_label: "Default", default_value: 2.0 }
                Rating { aria_label: "Ten", default_value: 7.0, max: 10, size: Size::Small }
            }
            DemoRow { label: "read only",
                Rating { read_only: true, value: Some(4.5), precision: 0.5 }
                Rating { read_only: true, value: Some(3.0) }
                Rating { read_only: true, value: None::<f64> }
            }
            DemoRow { label: "sizes",
                Rating { aria_label: "Small", size: Size::Small, default_value: 3.0 }
                Rating { aria_label: "Medium", default_value: 3.0 }
                Rating { aria_label: "Large", size: Size::Large, default_value: 3.0 }
            }
            DemoRow { label: "disabled",
                Rating { aria_label: "Disabled", disabled: true, default_value: 2.5, precision: 0.5 }
            }
        }
    }
}

#[component]
fn FormDemo() -> Element {
    rsx! {
        Demo {
            title: "FormControl, FormLabel, FormGroup, FormHelperText",
            description: "printed lettering around the controls · error and disabled flow down from FormControl",
            DemoGrid {
                FormControl { fieldset: true, required: true,
                    FormLabel { legend: true, "Required group" }
                    FormGroup {
                        FormControlLabel { control: rsx! { Checkbox { default_checked: true, name: "news" } }, label: "News" }
                        FormControlLabel { control: rsx! { Checkbox { name: "weather" } }, label: "Weather" }
                        FormControlLabel { control: rsx! { Checkbox { name: "traffic" } }, label: "Traffic" }
                    }
                    FormHelperText { "Choose the bulletins to air." }
                }
                FormControl { fieldset: true, error: true,
                    FormLabel { legend: true, "Error state" }
                    FormGroup {
                        FormControlLabel { control: rsx! { Checkbox {} }, label: "Morning show" }
                        FormControlLabel { control: rsx! { Checkbox {} }, label: "Evening show" }
                    }
                    FormHelperText { "Pick at least one show." }
                }
                FormControl { fieldset: true, disabled: true,
                    FormLabel { legend: true, "Disabled control" }
                    FormGroup {
                        FormControlLabel { control: rsx! { Switch { default_checked: true } }, label: "Record" }
                        FormControlLabel { control: rsx! { Checkbox { default_checked: true } }, label: "Archive" }
                    }
                    FormHelperText { "Managed by the station admin." }
                }
                FormControl { fieldset: true, size: Size::Small, color: Color::Success,
                    FormLabel { legend: true, "size & color from FormControl" }
                    FormGroup { row: true,
                        FormControlLabel { control: rsx! { Checkbox { default_checked: true } }, label: "One" }
                        FormControlLabel { control: rsx! { Switch { default_checked: true } }, label: "Two" }
                        FormControlLabel { control: rsx! { Radio { checked: true } }, label: "Three" }
                    }
                }
            }
        }
        style { {DEMO_CSS} }
    }
}

/// Layout for this page's demos only.
const DEMO_CSS: &str = r#"
.selection-demo__form { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 18px 40px; }
.selection-demo__col { display: flex; flex-direction: column; gap: 18px; min-width: 0; }
.selection-demo__nested { padding-left: 26px; }
.selection-demo__summary { margin: 18px 0 0; padding-top: 12px; border-top: 1px solid rgba(0,0,0,.1); }
.selection-demo__sliders { display: flex; flex-wrap: wrap; gap: 28px; align-items: flex-start; }
.selection-demo__stack { flex: 1 1 420px; min-width: 0; }
.selection-demo__stack .demo__row { flex-wrap: nowrap; }
.selection-demo__slider { flex: 1; min-width: 0; width: auto; }
.selection-demo__slider--label { margin-top: 18px; }
.selection-demo__slider--third { flex: 1; min-width: 0; width: auto; }
.selection-demo__slider--seventh { flex: 1; min-width: 0; width: auto; }
.selection-demo__mixer { display: flex; gap: 6px; padding: 20px 14px 14px; }
.selection-demo__channel { display: flex; flex-direction: column; align-items: center; gap: 10px; }
.selection-demo__channel .skeuo-slider { height: 200px; }
.selection-demo__channel .skeuo-slider--labelled { margin-right: 26px; }
"#;
