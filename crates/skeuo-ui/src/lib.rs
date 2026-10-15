//! Skeuomorphic UI components for Dioxus, in the design language of Apple's
//! Magic Keyboard, with an API that mirrors MUI.
//!
//! The component APIs follow MUI (`variant`, `color`, `size`, `disabled`,
//! `label`, `helper_text`, …) so that code written against MUI ports over
//! with minimal changes. Every component is built from the same few
//! physical materials (keycap, well, aluminium chassis, groove, LED); see
//! `DESIGN.md` at the repository root.
//!
//! Wrap your app in [`ThemeProvider`] once; it injects the stylesheet and
//! the palette CSS variables.
//!
//! ```rust,no_run
//! use dioxus::prelude::*;
//! use skeuo_ui::prelude::*;
//!
//! fn App() -> Element {
//!     let mut name = use_signal(String::new);
//!     rsx! {
//!         ThemeProvider {
//!             TextField {
//!                 label: "Name",
//!                 value: name(),
//!                 oninput: move |e: FormEvent| name.set(e.value()),
//!             }
//!             Button { variant: ButtonVariant::Contained, "Save" }
//!         }
//!     }
//! }
//! ```

use std::sync::atomic::{AtomicUsize, Ordering};

use dioxus::prelude::*;

mod icons;
mod theme;

pub mod data_display;
pub mod feedback;
pub mod inputs;
pub mod navigation;
pub mod selection;
pub mod surfaces;

pub use data_display::*;
pub use feedback::*;
pub use icons::{Icon, IconName, IconProps};
pub use inputs::*;
pub use navigation::*;
pub use selection::*;
pub use surfaces::*;
pub use theme::{
    Palette, PaletteColor, STYLESHEET, Theme, ThemeMode, ThemeProvider, ThemeProviderProps,
};

/// Everything a component user needs: `use skeuo_ui::prelude::*;`
pub mod prelude {
    pub use crate::data_display::*;
    pub use crate::feedback::*;
    pub use crate::icons::{Icon, IconName};
    pub use crate::inputs::*;
    pub use crate::navigation::*;
    pub use crate::selection::*;
    pub use crate::surfaces::*;
    pub use crate::theme::{Palette, PaletteColor, Theme, ThemeMode, ThemeProvider};
    pub use crate::{Color, Orientation, Placement, Size};
}

/// Palette color of a component, as in MUI's `color` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum Color {
    #[default]
    Primary,
    Secondary,
    Success,
    Error,
    Warning,
    Info,
    /// The white keycap / plain aluminium. MUI's `inherit` / `default`.
    Neutral,
}

impl Color {
    pub const ALL: [Color; 7] = [
        Color::Primary,
        Color::Secondary,
        Color::Success,
        Color::Error,
        Color::Warning,
        Color::Info,
        Color::Neutral,
    ];

    pub fn as_str(self) -> &'static str {
        match self {
            Color::Primary => "primary",
            Color::Secondary => "secondary",
            Color::Success => "success",
            Color::Error => "error",
            Color::Warning => "warning",
            Color::Info => "info",
            Color::Neutral => "neutral",
        }
    }

    /// The `skeuo-color-*` class that maps this palette entry onto the
    /// `--c-*` CSS variables.
    pub fn class(self) -> &'static str {
        match self {
            Color::Primary => "skeuo-color-primary",
            Color::Secondary => "skeuo-color-secondary",
            Color::Success => "skeuo-color-success",
            Color::Error => "skeuo-color-error",
            Color::Warning => "skeuo-color-warning",
            Color::Info => "skeuo-color-info",
            Color::Neutral => "skeuo-color-neutral",
        }
    }
}

/// Component size, as in MUI's `size` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum Size {
    Small,
    #[default]
    Medium,
    Large,
}

impl Size {
    pub fn as_str(self) -> &'static str {
        match self {
            Size::Small => "small",
            Size::Medium => "medium",
            Size::Large => "large",
        }
    }
}

/// Layout direction, as in MUI's `orientation` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum Orientation {
    #[default]
    Horizontal,
    Vertical,
}

impl Orientation {
    pub fn as_str(self) -> &'static str {
        match self {
            Orientation::Horizontal => "horizontal",
            Orientation::Vertical => "vertical",
        }
    }
}

/// Where a floating element sits relative to its anchor (MUI `placement`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum Placement {
    Top,
    #[default]
    Bottom,
    Left,
    Right,
}

impl Placement {
    pub fn as_str(self) -> &'static str {
        match self {
            Placement::Top => "top",
            Placement::Bottom => "bottom",
            Placement::Left => "left",
            Placement::Right => "right",
        }
    }
}

/// Joins the non-empty class names with spaces.
pub fn classes<'a>(parts: impl IntoIterator<Item = &'a str>) -> String {
    parts
        .into_iter()
        .filter(|p| !p.is_empty())
        .collect::<Vec<_>>()
        .join(" ")
}

static NEXT_ID: AtomicUsize = AtomicUsize::new(0);

/// A DOM id that is unique in the process and stable for the lifetime of
/// the component, e.g. to link a label to its input.
pub fn use_unique_id(prefix: &str) -> String {
    let prefix = prefix.to_string();
    use_hook(move || format!("{prefix}-{}", NEXT_ID.fetch_add(1, Ordering::Relaxed)))
}
