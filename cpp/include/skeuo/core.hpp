// skeuo-ui for C++ — shared enums, mirroring MUI's common props.
#pragma once

#include <array>
#include <string_view>

namespace skeuo {

/// Palette color of a component, as in MUI's `color` prop.
enum class Color { Primary, Secondary, Success, Error, Warning, Info, Neutral };

inline constexpr std::array<Color, 7> kAllColors = {
    Color::Primary, Color::Secondary, Color::Success, Color::Error,
    Color::Warning, Color::Info,      Color::Neutral,
};

constexpr std::string_view as_str(Color c) {
    switch (c) {
        case Color::Primary: return "primary";
        case Color::Secondary: return "secondary";
        case Color::Success: return "success";
        case Color::Error: return "error";
        case Color::Warning: return "warning";
        case Color::Info: return "info";
        case Color::Neutral: return "neutral";
    }
    return "primary";
}

/// The `skeuo-color-*` class that maps this palette entry onto `--c-*`.
constexpr std::string_view color_class(Color c) {
    switch (c) {
        case Color::Primary: return "skeuo-color-primary";
        case Color::Secondary: return "skeuo-color-secondary";
        case Color::Success: return "skeuo-color-success";
        case Color::Error: return "skeuo-color-error";
        case Color::Warning: return "skeuo-color-warning";
        case Color::Info: return "skeuo-color-info";
        case Color::Neutral: return "skeuo-color-neutral";
    }
    return "skeuo-color-primary";
}

/// Component size, as in MUI's `size` prop.
enum class Size { Small, Medium, Large };

constexpr std::string_view as_str(Size s) {
    switch (s) {
        case Size::Small: return "small";
        case Size::Medium: return "medium";
        case Size::Large: return "large";
    }
    return "medium";
}

/// Layout direction, as in MUI's `orientation` prop.
enum class Orientation { Horizontal, Vertical };

constexpr std::string_view as_str(Orientation o) {
    return o == Orientation::Vertical ? "vertical" : "horizontal";
}

/// Where a floating element sits relative to its anchor (MUI `placement`).
enum class Placement { Top, Bottom, Left, Right };

constexpr std::string_view as_str(Placement p) {
    switch (p) {
        case Placement::Top: return "top";
        case Placement::Bottom: return "bottom";
        case Placement::Left: return "left";
        case Placement::Right: return "right";
    }
    return "bottom";
}

}  // namespace skeuo
