use dioxus::prelude::*;

use super::form::use_form_control;
use crate::{Color, Orientation, Size, classes};

/// A slider's value: one number, or a `[start, end]` range with two thumbs
/// (MUI's `number | number[]`).
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum SliderValue {
    Single(f64),
    Range(f64, f64),
}

impl SliderValue {
    /// The value, or the start of a range.
    pub fn single(self) -> f64 {
        match self {
            SliderValue::Single(v) | SliderValue::Range(v, _) => v,
        }
    }

    /// `(start, end)`; a single value `v` gives `(v, v)`.
    pub fn range(self) -> (f64, f64) {
        match self {
            SliderValue::Single(v) => (v, v),
            SliderValue::Range(a, b) => (a, b),
        }
    }

    pub fn is_range(self) -> bool {
        matches!(self, SliderValue::Range(..))
    }
}

impl From<f64> for SliderValue {
    fn from(v: f64) -> Self {
        SliderValue::Single(v)
    }
}

impl From<(f64, f64)> for SliderValue {
    fn from((a, b): (f64, f64)) -> Self {
        SliderValue::Range(a, b)
    }
}

impl From<[f64; 2]> for SliderValue {
    fn from([a, b]: [f64; 2]) -> Self {
        SliderValue::Range(a, b)
    }
}

/// Lets `value: 30.0` / `value: (20.0, 80.0)` be written for the optional
/// `value` / `default_value` props.
#[doc(hidden)]
pub struct SliderValueMarker;

impl dioxus::core::SuperFrom<f64, SliderValueMarker> for Option<SliderValue> {
    fn super_from(v: f64) -> Self {
        Some(SliderValue::Single(v))
    }
}

impl dioxus::core::SuperFrom<(f64, f64), SliderValueMarker> for Option<SliderValue> {
    fn super_from(v: (f64, f64)) -> Self {
        Some(v.into())
    }
}

impl dioxus::core::SuperFrom<[f64; 2], SliderValueMarker> for Option<SliderValue> {
    fn super_from(v: [f64; 2]) -> Self {
        Some(v.into())
    }
}

/// One tick printed under the rail, optionally with a label.
#[derive(Clone, Debug, PartialEq)]
pub struct SliderMark {
    pub value: f64,
    pub label: Option<String>,
}

impl SliderMark {
    pub fn new(value: f64, label: impl Into<String>) -> Self {
        Self {
            value,
            label: Some(label.into()),
        }
    }

    /// A tick without a label.
    pub fn tick(value: f64) -> Self {
        Self { value, label: None }
    }
}

/// MUI's `marks`: `false`, `true` (a tick at every step) or a list.
#[derive(Clone, Debug, Default, PartialEq)]
pub enum SliderMarks {
    #[default]
    None,
    Steps,
    Custom(Vec<SliderMark>),
}

impl From<bool> for SliderMarks {
    fn from(on: bool) -> Self {
        if on {
            SliderMarks::Steps
        } else {
            SliderMarks::None
        }
    }
}

impl From<Vec<SliderMark>> for SliderMarks {
    fn from(marks: Vec<SliderMark>) -> Self {
        SliderMarks::Custom(marks)
    }
}

/// When the floating value label above a thumb is shown.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ValueLabelDisplay {
    #[default]
    Off,
    /// While the thumb is hovered, focused or dragged.
    Auto,
    /// Always.
    On,
}

/// Which part of the rail is filled (MUI's `track`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SliderTrack {
    /// From the start (or the first thumb) to the thumb.
    #[default]
    Normal,
    /// From the thumb to the end.
    Inverted,
    /// No fill (`track={false}`).
    Off,
}

/// A slider: a groove in the aluminium with a lit fill and round keycap
/// thumbs; the value label floats above the thumb as a small chassis
/// bubble. Mirrors MUI's `<Slider>`.
///
/// Built on native `<input type="range">`s (one per thumb), so keyboard and
/// pointer handling are the browser's. Pass `value` + `onchange` for a
/// controlled slider or `default_value` for an uncontrolled one; a
/// [`SliderValue::Range`] value gives two thumbs.
#[derive(Props, Clone, PartialEq)]
pub struct SliderProps {
    pub value: Option<SliderValue>,
    pub default_value: Option<SliderValue>,
    #[props(default = 0.0)]
    pub min: f64,
    #[props(default = 100.0)]
    pub max: f64,
    #[props(default = 1.0)]
    pub step: f64,
    #[props(default, into)]
    pub marks: SliderMarks,
    #[props(default)]
    pub value_label_display: ValueLabelDisplay,
    /// Formats the value label and `aria-valuetext`, e.g. `|v| format!("{v} dB")`.
    pub value_label_format: Option<Callback<f64, String>>,
    #[props(default)]
    pub orientation: Orientation,
    #[props(default)]
    pub track: SliderTrack,
    pub color: Option<Color>,
    pub size: Option<Size>,
    #[props(default)]
    pub disabled: bool,
    #[props(into)]
    pub name: Option<String>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    /// Called continuously while a thumb moves.
    pub onchange: Option<EventHandler<SliderValue>>,
    /// Called when a thumb is released (MUI `onChangeCommitted`).
    pub onchange_committed: Option<EventHandler<SliderValue>>,
}

fn decimals(step: f64) -> usize {
    (0..6)
        .find(|d| {
            let scaled = step * 10f64.powi(*d as i32);
            (scaled - scaled.round()).abs() < 1e-9
        })
        .unwrap_or(6)
}

#[component]
pub fn Slider(props: SliderProps) -> Element {
    let ctx = use_form_control().unwrap_or_default();
    let min = props.min;
    let max = props.max.max(min + f64::EPSILON);
    let mut internal = use_signal(|| {
        props
            .default_value
            .unwrap_or(SliderValue::Single(props.min))
    });
    let current = props.value.unwrap_or_else(|| internal.cloned());
    let disabled = props.disabled || ctx.disabled;
    let color = props.color.or(ctx.color).unwrap_or_default();
    let size = props.size.or(ctx.size).unwrap_or_default();
    let vertical = props.orientation == Orientation::Vertical;

    let pct = move |v: f64| ((v - min) / (max - min) * 100.0).clamp(0.0, 100.0);
    let dec = decimals(props.step);
    let format = props.value_label_format;
    let fmt = move |v: f64| match &format {
        Some(f) => f.call(v),
        None => format!("{v:.dec$}"),
    };

    let thumbs: Vec<f64> = match current {
        SliderValue::Single(v) => vec![v],
        SliderValue::Range(a, b) => vec![a, b],
    };
    let segments: Vec<(f64, f64)> = match (props.track, current) {
        (SliderTrack::Off, _) => vec![],
        (SliderTrack::Normal, SliderValue::Single(v)) => vec![(0.0, pct(v))],
        (SliderTrack::Normal, SliderValue::Range(a, b)) => vec![(pct(a), pct(b))],
        (SliderTrack::Inverted, SliderValue::Single(v)) => vec![(pct(v), 100.0)],
        (SliderTrack::Inverted, SliderValue::Range(a, b)) => {
            vec![(0.0, pct(a)), (pct(b), 100.0)]
        }
    };
    let marks: Vec<SliderMark> = match &props.marks {
        SliderMarks::None => vec![],
        SliderMarks::Steps => {
            let count = ((max - min) / props.step).floor();
            if props.step > 0.0 && count <= 200.0 {
                (0..=count as usize)
                    .map(|i| SliderMark::tick(min + i as f64 * props.step))
                    .collect()
            } else {
                vec![]
            }
        }
        SliderMarks::Custom(list) => list.clone(),
    };
    let labelled = marks.iter().any(|m| m.label.is_some());
    let in_track = |p: f64| segments.iter().any(|(a, b)| p >= a - 1e-9 && p <= b + 1e-9);

    let size_class = format!("skeuo-slider--{}", size.as_str());
    let class = classes([
        "skeuo-slider",
        color.class(),
        &size_class,
        if vertical {
            "skeuo-slider--vertical"
        } else {
            "skeuo-slider--horizontal"
        },
        if current.is_range() {
            "skeuo-slider--range"
        } else {
            ""
        },
        if labelled {
            "skeuo-slider--labelled"
        } else {
            ""
        },
        if disabled {
            "skeuo-slider--disabled"
        } else {
            ""
        },
        &props.class,
    ]);
    let label_class = match props.value_label_display {
        ValueLabelDisplay::Off => None,
        ValueLabelDisplay::Auto => {
            Some("skeuo-slider__value-label skeuo-chassis skeuo-chassis--float skeuo-elev-3")
        }
        ValueLabelDisplay::On => Some(
            "skeuo-slider__value-label skeuo-slider__value-label--on skeuo-chassis skeuo-chassis--float skeuo-elev-3",
        ),
    };
    let is_range = current.is_range();
    let next = move |i: usize, x: f64| match current {
        SliderValue::Single(_) => SliderValue::Single(x),
        SliderValue::Range(_, b) if i == 0 => SliderValue::Range(x.min(b), b),
        SliderValue::Range(a, _) => SliderValue::Range(a, x.max(a)),
    };
    let onchange = props.onchange;
    let oncommit = props.onchange_committed;

    rsx! {
        span { class, style: props.style,
            span { class: "skeuo-slider__rail skeuo-groove", aria_hidden: "true",
                for (a, b) in segments.iter().copied() {
                    span {
                        class: "skeuo-slider__track skeuo-groove__fill",
                        style: "--a: {a}%; --b: {b}%;",
                    }
                }
            }
            for mark in marks.iter() {
                span {
                    class: if in_track(pct(mark.value)) { "skeuo-slider__mark skeuo-slider__mark--active" } else { "skeuo-slider__mark" },
                    style: "--p: {pct(mark.value)}%;",
                    aria_hidden: "true",
                }
                if let Some(label) = &mark.label {
                    span {
                        class: "skeuo-slider__mark-label skeuo-caption",
                        style: "--p: {pct(mark.value)}%;",
                        aria_hidden: "true",
                        "{label}"
                    }
                }
            }
            for (i, v) in thumbs.iter().copied().enumerate() {
                input {
                    key: "input-{i}",
                    class: "skeuo-slider__input skeuo-slider__input--{i}",
                    r#type: "range",
                    name: props.name.clone(),
                    min: "{min}",
                    max: "{max}",
                    step: "{props.step}",
                    value: "{v}",
                    disabled,
                    aria_label: match (&props.aria_label, is_range) {
                        (Some(l), true) => Some(format!("{l} {}", if i == 0 { "start" } else { "end" })),
                        (l, _) => l.clone(),
                    },
                    aria_labelledby: props.aria_labelledby.clone(),
                    aria_valuetext: fmt(v),
                    aria_orientation: if vertical { Some("vertical") } else { None },
                    oninput: move |e: FormEvent| {
                        if let Ok(x) = e.value().parse::<f64>() {
                            let new = next(i, x);
                            internal.set(new);
                            if let Some(h) = &onchange {
                                h.call(new);
                            }
                        }
                    },
                    onchange: move |e: FormEvent| {
                        if let (Some(h), Ok(x)) = (&oncommit, e.value().parse::<f64>()) {
                            h.call(next(i, x));
                        }
                    },
                }
            }
            for (i, v) in thumbs.iter().copied().enumerate() {
                span {
                    key: "thumb-{i}",
                    class: "skeuo-slider__thumb skeuo-slider__thumb--{i} skeuo-key skeuo-key--round",
                    style: "--p: {pct(v)}%;",
                    aria_hidden: "true",
                    if let Some(lc) = label_class {
                        span { class: lc, "{fmt(v)}" }
                    }
                }
            }
        }
    }
}
