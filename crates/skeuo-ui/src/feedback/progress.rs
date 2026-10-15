use dioxus::prelude::*;

use crate::{Color, classes};

fn clamp_percent(v: f64) -> f64 {
    if v.is_nan() { 0.0 } else { v.clamp(0.0, 100.0) }
}

/// Formats a number for CSS / SVG without trailing zeros noise.
fn num(v: f64) -> String {
    let s = format!("{v:.3}");
    let s = s.trim_end_matches('0').trim_end_matches('.');
    if s.is_empty() || s == "-" {
        "0".into()
    } else {
        s.to_string()
    }
}

/// Mode of a [`CircularProgress`], as in MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum CircularProgressVariant {
    /// The arc shows `value` percent.
    Determinate,
    /// The arc spins; progress unknown.
    #[default]
    Indeterminate,
}

impl CircularProgressVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            CircularProgressVariant::Determinate => "determinate",
            CircularProgressVariant::Indeterminate => "indeterminate",
        }
    }
}

/// A round progress indicator. Mirrors MUI's `<CircularProgress>`.
///
/// Physically: a ring-shaped groove cut into the aluminium with a colored
/// arc laid in it, like the fill of a slider rail bent into a circle.
#[derive(Props, Clone, PartialEq)]
pub struct CircularProgressProps {
    #[props(default)]
    pub variant: CircularProgressVariant,
    /// Percent complete (0–100) for the determinate variant.
    #[props(default)]
    pub value: f64,
    /// Diameter in px.
    #[props(default = 40)]
    pub size: u32,
    /// Ring thickness in units of a 44-unit-wide box (MUI's default 3.6).
    #[props(default = 3.6)]
    pub thickness: f64,
    #[props(default)]
    pub color: Color,
    /// Spin without the arc growing and shrinking (cheaper to animate).
    #[props(default)]
    pub disable_shrink: bool,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn CircularProgress(props: CircularProgressProps) -> Element {
    const BOX: f64 = 44.0;
    let thickness = props.thickness.clamp(0.5, BOX / 2.0);
    let radius = (BOX - thickness) / 2.0;
    let circumference = 2.0 * std::f64::consts::PI * radius;
    let determinate = props.variant == CircularProgressVariant::Determinate;
    let value = clamp_percent(props.value);

    let variant = format!("skeuo-circular--{}", props.variant.as_str());
    let class = classes([
        "skeuo-circular",
        &variant,
        props.color.class(),
        if props.disable_shrink && !determinate {
            "skeuo-circular--disable-shrink"
        } else {
            ""
        },
        &props.class,
    ]);
    // The groove is a disc with the middle masked out; the hole's radius as
    // a percentage of the outer radius.
    let hole = (BOX / 2.0 - thickness) / (BOX / 2.0) * 100.0;
    let style = format!(
        "width:{size}px;height:{size}px;--circular-hole:{hole}%",
        size = props.size,
        hole = num(hole)
    );
    let arc_style = if determinate {
        format!(
            "stroke-dasharray:{c};stroke-dashoffset:{o}{hide}",
            c = num(circumference),
            o = num((100.0 - value) / 100.0 * circumference),
            // A round cap would still draw a dot at 0%.
            hide = if value <= 0.0 {
                ";visibility:hidden"
            } else {
                ""
            }
        )
    } else {
        String::new()
    };

    rsx! {
        span {
            class,
            style,
            role: "progressbar",
            aria_label: props.aria_label,
            aria_valuemin: if determinate { Some("0") } else { None },
            aria_valuemax: if determinate { Some("100") } else { None },
            aria_valuenow: if determinate { Some(num(value.round())) } else { None },
            span { class: "skeuo-groove skeuo-circular__track" }
            svg {
                class: "skeuo-circular__svg",
                view_box: "22 22 44 44",
                "aria-hidden": "true",
                circle {
                    class: "skeuo-circular__arc",
                    cx: "44",
                    cy: "44",
                    r: num(radius),
                    fill: "none",
                    stroke_width: num(thickness),
                    style: arc_style,
                }
            }
        }
    }
}

/// Mode of a [`LinearProgress`], as in MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum LinearProgressVariant {
    /// The fill shows `value` percent.
    Determinate,
    /// Bars sweep along the groove; progress unknown.
    #[default]
    Indeterminate,
    /// `value` percent plus a fainter `value_buffer` percent (e.g. streaming).
    Buffer,
    /// Like indeterminate, sweeping backwards (before the work starts).
    Query,
}

impl LinearProgressVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            LinearProgressVariant::Determinate => "determinate",
            LinearProgressVariant::Indeterminate => "indeterminate",
            LinearProgressVariant::Buffer => "buffer",
            LinearProgressVariant::Query => "query",
        }
    }
}

/// A horizontal progress bar. Mirrors MUI's `<LinearProgress>`.
///
/// Physically: a `.skeuo-groove` channel cut into the aluminium, filled
/// with the palette color (`.skeuo-groove__fill`) like a slider rail.
#[derive(Props, Clone, PartialEq)]
pub struct LinearProgressProps {
    #[props(default)]
    pub variant: LinearProgressVariant,
    /// Percent complete (0–100) for the determinate and buffer variants.
    #[props(default)]
    pub value: f64,
    /// Percent buffered (0–100) for the buffer variant.
    #[props(default)]
    pub value_buffer: f64,
    #[props(default)]
    pub color: Color,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn LinearProgress(props: LinearProgressProps) -> Element {
    let variant = format!("skeuo-linear--{}", props.variant.as_str());
    let class = classes([
        "skeuo-groove",
        "skeuo-linear",
        &variant,
        props.color.class(),
        &props.class,
    ]);
    let value = clamp_percent(props.value);
    let buffer = clamp_percent(props.value_buffer);
    let has_value = matches!(
        props.variant,
        LinearProgressVariant::Determinate | LinearProgressVariant::Buffer
    );

    rsx! {
        span {
            class,
            role: "progressbar",
            aria_label: props.aria_label,
            aria_valuemin: if has_value { Some("0") } else { None },
            aria_valuemax: if has_value { Some("100") } else { None },
            aria_valuenow: if has_value { Some(num(value.round())) } else { None },
            match props.variant {
                LinearProgressVariant::Determinate => rsx! {
                    span { class: "skeuo-groove__fill skeuo-linear__bar", style: "width:{num(value)}%" }
                },
                LinearProgressVariant::Buffer => rsx! {
                    span { class: "skeuo-groove__fill skeuo-linear__buffer", style: "width:{num(buffer)}%" }
                    span { class: "skeuo-groove__fill skeuo-linear__bar", style: "width:{num(value)}%" }
                },
                LinearProgressVariant::Indeterminate | LinearProgressVariant::Query => rsx! {
                    span { class: "skeuo-groove__fill skeuo-linear__bar skeuo-linear__bar--1" }
                    span { class: "skeuo-groove__fill skeuo-linear__bar skeuo-linear__bar--2" }
                },
            }
        }
    }
}
