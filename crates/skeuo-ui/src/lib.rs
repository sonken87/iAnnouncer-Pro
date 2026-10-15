//! Skeuomorphic UI components for Dioxus.
//!
//! The component APIs follow MUI (`variant`, `color`, `size`, `disabled`,
//! `label`, `helper_text`, …) so that code written against MUI ports over
//! with minimal changes. The look is skeuomorphic: every component is lit
//! by a single light source at the top, and is drawn as a physical object
//! (plastic key, brushed metal, carved well, backlit display).
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

mod button;
mod text_field;
mod theme;

pub use button::{Button, ButtonProps, ButtonVariant};
pub use text_field::{TextField, TextFieldProps, TextFieldVariant};
pub use theme::{PaletteColor, STYLESHEET, Theme, ThemeProvider, ThemeProviderProps};

pub mod prelude {
    pub use crate::{
        Button, ButtonVariant, Color, PaletteColor, Size, TextField, TextFieldVariant, Theme,
        ThemeProvider,
    };
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
    /// Brushed aluminium / silver. Not in MUI; the natural "no color" of a
    /// physical control.
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

/// Joins the non-empty class names with spaces.
pub(crate) fn classes<'a>(parts: impl IntoIterator<Item = &'a str>) -> String {
    parts
        .into_iter()
        .filter(|p| !p.is_empty())
        .collect::<Vec<_>>()
        .join(" ")
}
