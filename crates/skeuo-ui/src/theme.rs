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
            // Apple system colors. `main` is the increased-contrast variant so
            // white legends stay legible; `light` is the regular one; `dark`
            // is the keycap side.
            primary: PaletteColor::new("#0a84ff", "#0071e3", "#00519f", "#ffffff", "rgba(0,113,227,.35)"),
            secondary: PaletteColor::new("#bf5af2", "#8944ab", "#622f7c", "#ffffff", "rgba(137,68,171,.35)"),
            success: PaletteColor::new("#34c759", "#248a3d", "#18652b", "#ffffff", "rgba(36,138,61,.35)"),
            error: PaletteColor::new("#ff453a", "#d70015", "#9b0010", "#ffffff", "rgba(215,0,21,.32)"),
            warning: PaletteColor::new("#ffb340", "#ff9f0a", "#a35f00", "#1d1d1f", "rgba(255,159,10,.4)"),
            info: PaletteColor::new("#5ac8fa", "#0071a4", "#004f73", "#ffffff", "rgba(0,113,164,.35)"),
            // White keycap; focus uses the accent blue, as on macOS.
            neutral: PaletteColor::new("#ffffff", "#fbfbfc", "#c6c7cb", "#1d1d1f", "rgba(0,113,227,.35)"),
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
