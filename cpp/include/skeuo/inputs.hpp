// skeuo-ui for C++ — inputs: Button, TextField, ...
//
// Props are aggregates, so call sites use C++20 designated initializers in
// declaration order, mirroring MUI's props:
//
//   Button({.variant = ButtonVariant::Outlined, .color = Color::Error,
//           .onclick = [&] { remove(); }}, "Delete")
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

/// Extra HTML attributes passed through to a component's root element
/// (`id`, `title`, `aria-label`, `data-*`, ...).
using Attrs = std::vector<std::pair<std::string, std::string>>;

// ------------------------------------------------------------------ Button

enum class ButtonVariant {
    Contained,  // keycap in the palette color with a white legend
    Outlined,   // white keycap with a colored legend and edge
    Text,       // legend printed on the aluminium, no keycap
};

/// Where the loading spinner sits, as in MUI's `loadingPosition`.
enum class LoadingPosition { Start, End, Center };

struct ButtonProps {
    ButtonVariant variant = ButtonVariant::Contained;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool full_width = false;
    /// MUI Lab's LoadingButton: a spinner legend; the key is disabled meanwhile.
    bool loading = false;
    LoadingPosition loading_position = LoadingPosition::Center;
    std::optional<Node> loading_indicator = {};
    /// HTML `type`: "button" (default), "submit" or "reset".
    std::string type = "button";
    std::optional<Node> start_icon = {};
    std::optional<Node> end_icon = {};
    std::string cls = {};
    std::function<void()> onclick = {};
    Attrs attrs = {};
};

/// A push button drawn as a Magic Keyboard keycap. Mirrors MUI's <Button>.
Node Button(const ButtonProps& props, Node children);

// ------------------------------------------------------------------ TextField

enum class TextFieldVariant {
    Outlined,  // a well cut into the aluminium
    Filled,    // a shallow, borderless recess
    Standard,  // an engraved line on the aluminium
};

struct TextFieldProps {
    TextFieldVariant variant = TextFieldVariant::Outlined;
    /// Color of the focus halo.
    Color color = Color::Primary;
    Size size = Size::Medium;
    std::optional<std::string> label = {};
    /// Controlled value (pair with `oninput`).
    std::optional<std::string> value = {};
    /// Uncontrolled initial value, read once.
    std::optional<std::string> default_value = {};
    std::optional<std::string> placeholder = {};
    /// Text under the field; shown in the error color when `error` is set.
    std::optional<std::string> helper_text = {};
    bool error = false;
    bool disabled = false;
    bool required = false;
    bool read_only = false;
    bool full_width = false;
    bool autofocus = false;
    /// Renders a <textarea> with `rows` visible rows.
    bool multiline = false;
    int rows = 3;
    /// HTML input type: "text" (default), "password", "email", ...
    std::string type = "text";
    std::optional<std::string> name = {};
    /// Id of the input element; generated when omitted, as in MUI.
    std::optional<std::string> id = {};
    std::optional<std::string> autocomplete = {};
    std::optional<Node> start_adornment = {};
    std::optional<Node> end_adornment = {};
    std::string cls = {};
    std::string style = {};
    std::function<void(const std::string&)> oninput = {};
    std::function<void(const std::string&)> onchange = {};
    std::function<void(const Event&)> onkeydown = {};
    std::function<void()> onfocus = {};
    std::function<void()> onblur = {};
};

/// A labelled text input in a well. Mirrors MUI's <TextField>.
Node TextField(const TextFieldProps& props);

}  // namespace skeuo
