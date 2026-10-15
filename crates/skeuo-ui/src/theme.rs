use dioxus::prelude::*;

use crate::Color;

/// The component stylesheet. [`ThemeProvider`] injects it; it is exported
/// for apps that prefer to ship it as a static asset instead.
pub const STYLESHEET: &str = include_str!("../assets/skeuo.css");

/// One palette entry, as in MUI's `palette.primary`.
///
/// `light` / `main` / `dark` are the top, middle and bottom of a lit
/// surface's gradient; `glow` is the focus halo.
#[derive(Clone, Debug, PartialEq)]
pub struct PaletteColor {
    pub light: String,
    pub main: String,
    pub dark: String,
    pub contrast_text: String,
    pub glow: String,
}

impl PaletteColor {
    pub fn new(light: &str, main: &str, dark: &str, contrast_text: &str, glow: &str) -> Self {
        Self {
            light: light.into(),
            main: main.into(),
            dark: dark.into(),
            contrast_text: contrast_text.into(),
            glow: glow.into(),
        }
    }
}

/// Theme passed to [`ThemeProvider`], the counterpart of MUI's `createTheme`.
/// Start from `Theme::default()` and override what you need.
#[derive(Clone, Debug, PartialEq)]
pub struct Theme {
    pub primary: PaletteColor,
    pub secondary: PaletteColor,
    pub success: PaletteColor,
    pub error: PaletteColor,
    pub warning: PaletteColor,
    pub info: PaletteColor,
    pub neutral: PaletteColor,
    /// Corner radius of buttons and fields, in px.
    pub radius: f32,
    pub font_family: String,
}

impl Default for Theme {
    fn default() -> Self {
        Self {
            primary: PaletteColor::new("#6cb2ff", "#2475d6", "#0f4a96", "#ffffff", "rgba(36,117,214,.45)"),
            secondary: PaletteColor::new("#bb95f2", "#7c4cc6", "#4f2a8c", "#ffffff", "rgba(124,76,198,.45)"),
            success: PaletteColor::new("#8bd876", "#3f9c2d", "#236716", "#ffffff", "rgba(63,156,45,.45)"),
            error: PaletteColor::new("#ff8577", "#d43b2d", "#8c1c12", "#ffffff", "rgba(212,59,45,.45)"),
            warning: PaletteColor::new("#ffd875", "#f0a81c", "#b0700a", "#3a2500", "rgba(240,168,28,.5)"),
            info: PaletteColor::new("#7bd8f4", "#1b9ac4", "#0b6487", "#ffffff", "rgba(27,154,196,.45)"),
            neutral: PaletteColor::new("#fbfbfb", "#d9d9d9", "#a8a8a8", "#2e2e2e", "rgba(120,120,120,.4)"),
            radius: 8.0,
            font_family: r#"-apple-system, BlinkMacSystemFont, "Segoe UI", "Helvetica Neue", "PingFang SC", "Hiragino Sans", "Microsoft YaHei", sans-serif"#.into(),
        }
    }
}

impl Theme {
    pub fn palette(&self, color: Color) -> &PaletteColor {
        match color {
            Color::Primary => &self.primary,
            Color::Secondary => &self.secondary,
            Color::Success => &self.success,
            Color::Error => &self.error,
            Color::Warning => &self.warning,
            Color::Info => &self.info,
            Color::Neutral => &self.neutral,
        }
    }

    /// The theme as CSS custom properties, for an inline `style` attribute.
    pub fn css_vars(&self) -> String {
        let mut out = String::new();
        for color in Color::ALL {
            let p = self.palette(color);
            let name = color.as_str();
            out.push_str(&format!(
                "--skeuo-{name}-light:{};--skeuo-{name}-main:{};--skeuo-{name}-dark:{};\
                 --skeuo-{name}-contrast:{};--skeuo-{name}-glow:{};",
                p.light, p.main, p.dark, p.contrast_text, p.glow
            ));
        }
        out.push_str(&format!(
            "--skeuo-radius:{}px;--skeuo-font:{};",
            self.radius, self.font_family
        ));
        out
    }
}

/// Injects the stylesheet and theme variables; wrap your app in it once.
/// Like MUI's `ThemeProvider` + `CssBaseline`.
#[component]
pub fn ThemeProvider(
    #[props(default)] theme: Theme,
    #[props(default)] class: String,
    #[props(default)] style: String,
    children: Element,
) -> Element {
    let vars = theme.css_vars();
    rsx! {
        style { {STYLESHEET} }
        div {
            class: crate::classes(["skeuo-root", class.as_str()]),
            style: "{vars}{style}",
            {children}
        }
    }
}
