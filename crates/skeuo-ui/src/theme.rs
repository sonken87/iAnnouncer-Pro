use std::sync::LazyLock;

use dioxus::prelude::*;

use crate::Color;

/// Light/dark rules shared by every mode, in cascade order.
const BASE_CSS: &str = concat!(
    include_str!("../assets/tokens.css"),
    include_str!("../assets/materials.css"),
    include_str!("../assets/inputs.css"),
    include_str!("../assets/selection.css"),
    include_str!("../assets/data_display.css"),
    include_str!("../assets/feedback.css"),
    include_str!("../assets/surfaces.css"),
    include_str!("../assets/navigation.css"),
);

/// Dark tokens, written against the `SKEUO_DARK` selector placeholder.
const DARK_CSS: &str = include_str!("../assets/dark.css");

/// The complete component stylesheet. [`ThemeProvider`] injects it; it is
/// exported for apps that prefer to ship it as a static asset instead.
///
/// The dark tokens apply under `data-skeuo-mode="dark"`, and under
/// `data-skeuo-mode="system"` when the OS prefers a dark appearance.
pub static STYLESHEET: LazyLock<String> = LazyLock::new(|| {
    let dark = |selector: &str| DARK_CSS.replace("SKEUO_DARK", selector);
    format!(
        "{BASE_CSS}\n{}\n@media (prefers-color-scheme: dark) {{\n{}\n}}\n",
        dark(r#".skeuo-root[data-skeuo-mode="dark"]"#),
        dark(r#".skeuo-root[data-skeuo-mode="system"]"#),
    )
});

/// Which appearance a [`ThemeProvider`] renders, as MUI's `palette.mode`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ThemeMode {
    /// Silver aluminium and white keycaps.
    #[default]
    Light,
    /// Space Black: dark aluminium and black keycaps.
    Dark,
    /// Follow the operating system (`prefers-color-scheme`).
    System,
}

impl ThemeMode {
    pub fn as_str(self) -> &'static str {
        match self {
            ThemeMode::Light => "light",
            ThemeMode::Dark => "dark",
            ThemeMode::System => "system",
        }
    }
}

/// One palette entry, as in MUI's `palette.primary`.
///
/// `main` is the colored keycap top (white legends sit on it), `dark` its
/// side, `light` the bright variant (lettering on dark metal, the top of a
/// lit groove), `contrast_text` the legend on the keycap, `glow` the focus
/// halo.
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

/// The seven palette colors for one appearance.
#[derive(Clone, Debug, PartialEq)]
pub struct Palette {
    pub primary: PaletteColor,
    pub secondary: PaletteColor,
    pub success: PaletteColor,
    pub error: PaletteColor,
    pub warning: PaletteColor,
    pub info: PaletteColor,
    pub neutral: PaletteColor,
}

impl Palette {
    /// Apple system colors for the silver keyboard. `main` is the
    /// increased-contrast variant so white legends stay legible.
    pub fn light() -> Self {
        Self {
            primary: PaletteColor::new(
                "#0a84ff",
                "#0071e3",
                "#00519f",
                "#ffffff",
                "rgba(0,113,227,.35)",
            ),
            secondary: PaletteColor::new(
                "#bf5af2",
                "#8944ab",
                "#622f7c",
                "#ffffff",
                "rgba(137,68,171,.35)",
            ),
            success: PaletteColor::new(
                "#34c759",
                "#248a3d",
                "#18652b",
                "#ffffff",
                "rgba(36,138,61,.35)",
            ),
            error: PaletteColor::new(
                "#ff453a",
                "#d70015",
                "#9b0010",
                "#ffffff",
                "rgba(215,0,21,.32)",
            ),
            warning: PaletteColor::new(
                "#ffb340",
                "#ff9f0a",
                "#a35f00",
                "#1d1d1f",
                "rgba(255,159,10,.4)",
            ),
            info: PaletteColor::new(
                "#5ac8fa",
                "#0071a4",
                "#004f73",
                "#ffffff",
                "rgba(0,113,164,.35)",
            ),
            // White keycap; focus uses the accent blue, as on macOS.
            neutral: PaletteColor::new(
                "#ffffff",
                "#fbfbfc",
                "#c6c7cb",
                "#1d1d1f",
                "rgba(0,113,227,.35)",
            ),
        }
    }

    /// Apple's dark-appearance system colors for the Space Black keyboard.
    /// `light` is the bright variant used for lettering on dark metal.
    pub fn dark() -> Self {
        Self {
            primary: PaletteColor::new(
                "#409cff",
                "#0a84ff",
                "#0058b8",
                "#ffffff",
                "rgba(10,132,255,.5)",
            ),
            secondary: PaletteColor::new(
                "#da8fff",
                "#a94fd8",
                "#6c2a92",
                "#ffffff",
                "rgba(191,90,242,.45)",
            ),
            success: PaletteColor::new(
                "#4fdc76",
                "#2a9d48",
                "#186a2e",
                "#ffffff",
                "rgba(48,209,88,.4)",
            ),
            error: PaletteColor::new(
                "#ff7a70",
                "#ff453a",
                "#b0231b",
                "#ffffff",
                "rgba(255,69,58,.45)",
            ),
            warning: PaletteColor::new(
                "#ffc15e",
                "#ff9f0a",
                "#a86400",
                "#1d1d1f",
                "rgba(255,159,10,.45)",
            ),
            info: PaletteColor::new(
                "#7fd8ff",
                "#1a9fd6",
                "#0b6890",
                "#ffffff",
                "rgba(100,210,255,.4)",
            ),
            // Black keycap with a light legend.
            neutral: PaletteColor::new(
                "#4a4a4d",
                "#1e1e20",
                "#050506",
                "#f5f5f7",
                "rgba(10,132,255,.5)",
            ),
        }
    }

    pub fn get(&self, color: Color) -> &PaletteColor {
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

    pub fn get_mut(&mut self, color: Color) -> &mut PaletteColor {
        match color {
            Color::Primary => &mut self.primary,
            Color::Secondary => &mut self.secondary,
            Color::Success => &mut self.success,
            Color::Error => &mut self.error,
            Color::Warning => &mut self.warning,
            Color::Info => &mut self.info,
            Color::Neutral => &mut self.neutral,
        }
    }

    /// The palette as `--skeuo-<set>-<color>-<shade>` custom properties.
    fn css_vars(&self, set: &str, out: &mut String) {
        for color in Color::ALL {
            let p = self.get(color);
            let name = color.as_str();
            out.push_str(&format!(
                "--skeuo-{set}-{name}-light:{};--skeuo-{set}-{name}-main:{};\
                 --skeuo-{set}-{name}-dark:{};--skeuo-{set}-{name}-contrast:{};\
                 --skeuo-{set}-{name}-glow:{};",
                p.light, p.main, p.dark, p.contrast_text, p.glow
            ));
        }
    }
}

/// Theme passed to [`ThemeProvider`], the counterpart of MUI's `createTheme`.
/// Start from `Theme::default()` (light), `Theme::dark()` or
/// `Theme::system()` and override what you need, e.g.
/// `theme.light.primary = PaletteColor::new(…)`.
#[derive(Clone, Debug, PartialEq)]
pub struct Theme {
    pub mode: ThemeMode,
    /// Colors used in light mode.
    pub light: Palette,
    /// Colors used in dark mode.
    pub dark: Palette,
    /// Corner radius of buttons and fields, in px.
    pub radius: f32,
    pub font_family: String,
}

impl Default for Theme {
    fn default() -> Self {
        Self {
            mode: ThemeMode::Light,
            light: Palette::light(),
            dark: Palette::dark(),
            radius: 8.0,
            font_family: r#"-apple-system, BlinkMacSystemFont, "Segoe UI", "Helvetica Neue", "PingFang SC", "Hiragino Sans", "Microsoft YaHei", sans-serif"#.into(),
        }
    }
}

impl Theme {
    pub fn light() -> Self {
        Self::default()
    }

    pub fn dark() -> Self {
        Self {
            mode: ThemeMode::Dark,
            ..Self::default()
        }
    }

    /// Follows the operating system's light/dark setting.
    pub fn system() -> Self {
        Self {
            mode: ThemeMode::System,
            ..Self::default()
        }
    }

    pub fn with_mode(mut self, mode: ThemeMode) -> Self {
        self.mode = mode;
        self
    }

    /// Both palettes and the shape/type settings as CSS custom properties,
    /// for the root's inline `style`. The stylesheet picks the light or dark
    /// set according to `data-skeuo-mode`.
    pub fn css_vars(&self) -> String {
        let mut out = String::new();
        self.light.css_vars("light", &mut out);
        self.dark.css_vars("dark", &mut out);
        out.push_str(&format!(
            "--skeuo-radius:{}px;--skeuo-font:{};",
            self.radius, self.font_family
        ));
        out
    }
}

/// Marks that a stylesheet has already been injected higher up the tree.
#[derive(Clone, Copy)]
struct StylesheetInjected;

/// Injects the stylesheet and theme variables; wrap your app in it once.
/// Like MUI's `ThemeProvider` + `CssBaseline`.
///
/// Providers can be nested to give a region a different palette or radius;
/// the stylesheet is only injected by the outermost one. Light and dark
/// regions should not be nested inside each other.
#[component]
pub fn ThemeProvider(
    #[props(default)] theme: Theme,
    #[props(default)] class: String,
    #[props(default)] style: String,
    children: Element,
) -> Element {
    let outermost = try_use_context::<StylesheetInjected>().is_none();
    use_context_provider(|| StylesheetInjected);
    let vars = theme.css_vars();
    rsx! {
        if outermost {
            style { {STYLESHEET.as_str()} }
        }
        div {
            class: crate::classes(["skeuo-root", class.as_str()]),
            "data-skeuo-mode": theme.mode.as_str(),
            style: "{vars}{style}",
            {children}
        }
    }
}
