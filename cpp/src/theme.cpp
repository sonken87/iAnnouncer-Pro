#include "skeuo/theme.hpp"

#include <cstdio>

#include "assets.hpp"

namespace skeuo {

Palette Palette::light() {
    // Apple system colors; `main` is the increased-contrast variant so white
    // legends stay legible. Neutral is the white keycap.
    return {
        {"#0a84ff", "#0071e3", "#00519f", "#ffffff", "rgba(0,113,227,.35)"},
        {"#bf5af2", "#8944ab", "#622f7c", "#ffffff", "rgba(137,68,171,.35)"},
        {"#34c759", "#248a3d", "#18652b", "#ffffff", "rgba(36,138,61,.35)"},
        {"#ff453a", "#d70015", "#9b0010", "#ffffff", "rgba(215,0,21,.32)"},
        {"#ffb340", "#ff9f0a", "#a35f00", "#1d1d1f", "rgba(255,159,10,.4)"},
        {"#5ac8fa", "#0071a4", "#004f73", "#ffffff", "rgba(0,113,164,.35)"},
        {"#ffffff", "#fbfbfc", "#c6c7cb", "#1d1d1f", "rgba(0,113,227,.35)"},
    };
}

Palette Palette::dark() {
    // Apple dark-appearance colors for the Space Black keyboard. Neutral is
    // the black keycap with a light legend.
    return {
        {"#409cff", "#0a84ff", "#0058b8", "#ffffff", "rgba(10,132,255,.5)"},
        {"#da8fff", "#a94fd8", "#6c2a92", "#ffffff", "rgba(191,90,242,.45)"},
        {"#4fdc76", "#2a9d48", "#186a2e", "#ffffff", "rgba(48,209,88,.4)"},
        {"#ff7a70", "#ff453a", "#b0231b", "#ffffff", "rgba(255,69,58,.45)"},
        {"#ffc15e", "#ff9f0a", "#a86400", "#1d1d1f", "rgba(255,159,10,.45)"},
        {"#7fd8ff", "#1a9fd6", "#0b6890", "#ffffff", "rgba(100,210,255,.4)"},
        {"#4a4a4d", "#1e1e20", "#050506", "#f5f5f7", "rgba(10,132,255,.5)"},
    };
}

const PaletteColor& Palette::get(Color c) const {
    switch (c) {
        case Color::Primary: return primary;
        case Color::Secondary: return secondary;
        case Color::Success: return success;
        case Color::Error: return error;
        case Color::Warning: return warning;
        case Color::Info: return info;
        case Color::Neutral: return neutral;
    }
    return primary;
}

PaletteColor& Palette::get(Color c) {
    return const_cast<PaletteColor&>(static_cast<const Palette&>(*this).get(c));
}

namespace {
void palette_vars(const Palette& p, std::string_view set, std::string& out) {
    for (Color c : kAllColors) {
        const PaletteColor& pc = p.get(c);
        std::string prefix = "--skeuo-" + std::string(set) + "-" + std::string(as_str(c)) + "-";
        out += prefix + "light:" + pc.light + ";";
        out += prefix + "main:" + pc.main + ";";
        out += prefix + "dark:" + pc.dark + ";";
        out += prefix + "contrast:" + pc.contrast_text + ";";
        out += prefix + "glow:" + pc.glow + ";";
    }
}

std::string replace_all(std::string_view text, std::string_view from, std::string_view to) {
    std::string out;
    size_t pos = 0;
    while (true) {
        size_t hit = text.find(from, pos);
        if (hit == std::string_view::npos) break;
        out += text.substr(pos, hit - pos);
        out += to;
        pos = hit + from.size();
    }
    out += text.substr(pos);
    return out;
}
}  // namespace

std::string Theme::css_vars() const {
    std::string out;
    palette_vars(light, "light", out);
    palette_vars(dark, "dark", out);
    char radius_buf[32];
    std::snprintf(radius_buf, sizeof radius_buf, "%g", radius);
    out += "--skeuo-radius:" + std::string(radius_buf) + "px;";
    out += "--skeuo-font:" + font_family + ";";
    return out;
}

const std::string& stylesheet() {
    // Same assembly as the Rust crate: base rules, then the dark tokens for
    // data-skeuo-mode="dark" and, inside a media query, for "system".
    static const std::string css = [] {
        std::string out;
        for (std::string_view part : {assets::tokens_css, assets::materials_css, assets::inputs_css,
                                      assets::selection_css, assets::data_display_css,
                                      assets::feedback_css, assets::surfaces_css,
                                      assets::navigation_css}) {
            out += part;
        }
        out += "\n";
        out += replace_all(assets::dark_css, "SKEUO_DARK", R"(.skeuo-root[data-skeuo-mode="dark"])");
        out += "\n@media (prefers-color-scheme: dark) {\n";
        out += replace_all(assets::dark_css, "SKEUO_DARK", R"(.skeuo-root[data-skeuo-mode="system"])");
        out += "\n}\n";
        return out;
    }();
    return css;
}

Node ThemeProvider(const ThemeProviderProps& props, Node children) {
    return el("div")
        .cls(classes({"skeuo-root", props.cls}))
        .attr("data-skeuo-mode", std::string(as_str(props.theme.mode)))
        .attr("style", props.theme.css_vars() + props.style)
        .child(std::move(children));
}

}  // namespace skeuo
