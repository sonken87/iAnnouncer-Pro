use dioxus::prelude::*;

use crate::classes;

/// Paper style, mirroring MUI's `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum PaperVariant {
    /// A raised plate whose cast shadow follows `elevation`.
    #[default]
    Elevation,
    /// A flush plate (no cast shadow) with a crisp machined edge.
    Outlined,
}

impl PaperVariant {
    pub fn as_str(self) -> &'static str {
        match self {
            PaperVariant::Elevation => "elevation",
            PaperVariant::Outlined => "outlined",
        }
    }
}

/// Maps MUI's elevation scale (0–24) onto the five chassis levels of
/// DESIGN.md: 0 → 0, 1–2 → 1, 3–6 → 2, 7–12 → 3, 13–24 → 4.
pub fn chassis_level(elevation: u8) -> u8 {
    match elevation {
        0 => 0,
        1..=2 => 1,
        3..=6 => 2,
        7..=12 => 3,
        _ => 4,
    }
}

/// The material classes of a chassis plate (shared by every surface).
pub(crate) fn chassis_classes(variant: PaperVariant, elevation: u8, square: bool) -> String {
    let level = match variant {
        PaperVariant::Elevation => chassis_level(elevation),
        PaperVariant::Outlined => 0,
    };
    let elev = format!("skeuo-elev-{level}");
    let variant_class = format!("skeuo-paper--{}", variant.as_str());
    classes([
        "skeuo-chassis",
        &elev,
        if level >= 3 {
            "skeuo-chassis--float"
        } else {
            ""
        },
        if square { "skeuo-chassis--square" } else { "" },
        "skeuo-paper",
        &variant_class,
    ])
}

/// A plate of bead-blasted aluminium. Mirrors MUI's `<Paper>`.
///
/// `elevation` (0–24, MUI's scale) only changes the soft shadow the plate
/// casts on the desk; `variant: Outlined` sits flush with a machined edge;
/// `square` removes the rounded corners. Global attributes (`id`, `style`,
/// `role`, `aria-*`, …) pass through to the `<div>`.
#[derive(Props, Clone, PartialEq)]
pub struct PaperProps {
    /// MUI elevation, 0–24 (mapped onto chassis levels 0–4).
    #[props(default = 1)]
    pub elevation: u8,
    #[props(default)]
    pub variant: PaperVariant,
    /// Square corners.
    #[props(default)]
    pub square: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Paper(props: PaperProps) -> Element {
    let class = classes([
        chassis_classes(props.variant, props.elevation, props.square).as_str(),
        &props.class,
    ]);
    rsx! {
        div { class, ..props.attributes, {props.children} }
    }
}
