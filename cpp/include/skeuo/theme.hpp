// skeuo-ui for C++ — theme (light / dark Space Black / system).
#pragma once

#include <string>

#include "skeuo/core.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

/// Which appearance a ThemeProvider renders, as MUI's `palette.mode`.
enum class ThemeMode { Light, Dark, System };

constexpr std::string_view as_str(ThemeMode m) {
    switch (m) {
        case ThemeMode::Light: return "light";
        case ThemeMode::Dark: return "dark";
        case ThemeMode::System: return "system";
    }
    return "light";
}

/// One palette entry, as in MUI's `palette.primary`. `main` is the colored
/// keycap top, `dark` its side, `light` the bright variant (lettering on dark
/// metal), `contrast_text` the legend on the keycap, `glow` the focus halo.
struct PaletteColor {
    std::string light;
    std::string main;
    std::string dark;
    std::string contrast_text;
    std::string glow;
};

/// The seven palette colors for one appearance.
struct Palette {
    PaletteColor primary, secondary, success, error, warning, info, neutral;

    static Palette light();
    static Palette dark();
    const PaletteColor& get(Color c) const;
    PaletteColor& get(Color c);
};

/// The counterpart of MUI's `createTheme`.
struct Theme {
    ThemeMode mode = ThemeMode::Light;
    Palette light = Palette::light();
    Palette dark = Palette::dark();
    /// Corner radius of buttons and fields, in px.
    double radius = 8.0;
    std::string font_family =
        R"(-apple-system, BlinkMacSystemFont, "Segoe UI", "Helvetica Neue", "PingFang SC", "Hiragino Sans", "Microsoft YaHei", sans-serif)";

    static Theme make_light() { return {}; }
    static Theme make_dark() { Theme t; t.mode = ThemeMode::Dark; return t; }
    static Theme make_system() { Theme t; t.mode = ThemeMode::System; return t; }
    Theme with_mode(ThemeMode m) const { Theme t = *this; t.mode = m; return t; }

    /// Both palettes and the shape/type settings as CSS custom properties.
    std::string css_vars() const;
};

/// The complete component stylesheet (shared with the Rust crate's
/// `crates/skeuo-ui/assets`). The runtime puts it in the page <head>.
const std::string& stylesheet();

struct ThemeProviderProps {
    Theme theme = {};
    std::string cls = {};
    std::string style = {};
};

/// Applies a theme to its children (MUI's ThemeProvider + CssBaseline).
/// Light and dark regions should not be nested inside each other.
Node ThemeProvider(const ThemeProviderProps& props, Node children);

}  // namespace skeuo
