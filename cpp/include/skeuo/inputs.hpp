// skeuo-ui for C++ — inputs: Button, TextField, IconButton, Fab, ButtonGroup,
// ToggleButton(Group), Select, NativeSelect, Autocomplete, TransferList,
// TextareaAutosize.
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

// ------------------------------------------------------------------ IconButton / Fab

enum class IconButtonVariant {
    Text,       // the icon printed on the aluminium (MUI's borderless default)
    Outlined,   // a small white keycap holding the icon in the palette ink
    Contained,  // a small keycap in the palette color with a contrast icon
};

/// Which side of its container an IconButton hugs (MUI `edge`).
enum class IconButtonEdge { Start, End };

struct IconButtonProps {
    IconButtonVariant variant = IconButtonVariant::Text;
    /// MUI's `default` color is Color::Neutral.
    Color color = Color::Neutral;
    Size size = Size::Medium;
    std::optional<IconButtonEdge> edge = {};
    /// Round keycap instead of a square one (keycap variants only).
    bool round = false;
    bool disabled = false;
    std::string type = "button";
    std::string cls = {};
    std::function<void()> onclick = {};
    /// Give it an `aria-label` here.
    Attrs attrs = {};
};

/// A button holding a single icon. Mirrors MUI's <IconButton>.
Node IconButton(const IconButtonProps& props, Node children);

enum class FabVariant {
    Circular,  // a round key holding an icon
    Extended,  // a pill-shaped key holding an icon and a legend
};

struct FabProps {
    FabVariant variant = FabVariant::Circular;
    /// MUI's `default` color is Color::Neutral (a white key).
    Color color = Color::Neutral;
    /// small 40px · medium 48px · large 56px, as in MUI.
    Size size = Size::Large;
    bool disabled = false;
    std::string type = "button";
    std::string cls = {};
    std::function<void()> onclick = {};
    Attrs attrs = {};
};

/// A floating action button: a large round keycap. Mirrors MUI's <Fab>.
/// It is not positioned; place it yourself.
Node Fab(const FabProps& props, Node children);

// ------------------------------------------------------------------ ButtonGroup

struct ButtonGroupProps {
    ButtonVariant variant = ButtonVariant::Contained;
    Orientation orientation = Orientation::Horizontal;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    /// Stretch the group (and its keys, equally) to the container's width.
    bool full_width = false;
    std::string cls = {};
    Attrs attrs = {};
};

/// A row of keycaps joined edge-to-edge. Mirrors MUI's <ButtonGroup>. The
/// group's variant, color, size, disabled and full_width are applied to the
/// Buttons among its children (it rewrites their markup).
Node ButtonGroup(const ButtonGroupProps& props, Node children);

// ------------------------------------------------------------------ ToggleButton

struct ToggleButtonProps {
    /// Identifies the button within its group; passed to `onchange`.
    std::string value;
    bool selected = false;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool full_width = false;
    std::string cls = {};
    /// Called with `value` when the key is pressed.
    std::function<void(const std::string&)> onchange = {};
    std::function<void()> onclick = {};
    Attrs attrs = {};
};

/// A white keycap that latches down while selected. Mirrors MUI's
/// <ToggleButton>. Inside a ToggleButtonGroup the group owns the selection,
/// color, size and disabled.
Node ToggleButton(const ToggleButtonProps& props, Node children);

struct ToggleButtonGroupProps {
    /// Controlled selection (button values; at most one when `exclusive`).
    std::optional<std::vector<std::string>> value = {};
    /// Uncontrolled initial selection, read once.
    std::optional<std::vector<std::string>> default_value = {};
    /// Only one button can be selected at a time.
    bool exclusive = false;
    /// Called with the new selection.
    std::function<void(std::vector<std::string>)> onchange = {};
    Orientation orientation = Orientation::Horizontal;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool full_width = false;
    std::string cls = {};
    Attrs attrs = {};
};

/// A run of ToggleButton keycaps sharing one slot. Mirrors MUI's
/// <ToggleButtonGroup>.
Node ToggleButtonGroup(const ToggleButtonGroupProps& props, Node children);

// ------------------------------------------------------------------ Select

/// One choice of a Select or NativeSelect: `{"fm954", "95.4 FM"}`, or
/// `{.value = "off", .label = "Off air", .disabled = true}`.
struct SelectOption {
    std::string value;
    std::string label;
    bool disabled = false;
};

struct SelectProps {
    std::optional<std::string> label = {};
    std::vector<SelectOption> options = {};
    /// Single selection: controlled value / uncontrolled initial value.
    std::optional<std::string> value = {};
    std::optional<std::string> default_value = {};
    std::function<void(const std::string&)> onchange = {};
    /// Multiple selection uses `values` / `default_values` / `onchange_multiple`.
    bool multiple = false;
    std::optional<std::vector<std::string>> values = {};
    std::optional<std::vector<std::string>> default_values = {};
    std::function<void(std::vector<std::string>)> onchange_multiple = {};
    /// Controls whether the list is shown.
    std::optional<bool> open = {};
    std::function<void()> onopen = {};
    std::function<void()> onclose = {};
    TextFieldVariant variant = TextFieldVariant::Outlined;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool error = false;
    bool required = false;
    bool full_width = false;
    std::optional<std::string> helper_text = {};
    /// Shown when nothing is selected.
    std::optional<std::string> placeholder = {};
    /// Form field name; the value is submitted through a hidden input
    /// (comma-separated when `multiple`).
    std::optional<std::string> name = {};
    std::optional<std::string> id = {};
    std::string cls = {};
    std::string style = {};
};

/// A well with a chevron that opens a list of choices on a floating plate.
/// Mirrors MUI's <Select>; options are passed as data.
/// Keyboard: ↓ ↑ Enter Space open; ↓ ↑ Home End move; Enter / Space choose;
/// Escape closes. Leaving the field or clicking elsewhere closes it.
Node Select(const SelectProps& props);

struct NativeSelectProps {
    std::optional<std::string> label = {};
    std::vector<SelectOption> options = {};
    std::optional<std::string> value = {};
    std::optional<std::string> default_value = {};
    /// Called with the chosen option's value.
    std::function<void(const std::string&)> onchange = {};
    TextFieldVariant variant = TextFieldVariant::Outlined;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool error = false;
    bool required = false;
    bool full_width = false;
    std::optional<std::string> helper_text = {};
    std::optional<std::string> name = {};
    std::optional<std::string> id = {};
    std::string cls = {};
    std::string style = {};
};

/// A native <select> in a well with a printed chevron. Mirrors MUI's
/// <NativeSelect>.
Node NativeSelect(const NativeSelectProps& props);

// ------------------------------------------------------------------ Autocomplete

struct AutocompleteProps {
    std::vector<std::string> options = {};
    /// Single selection (std::nullopt when cleared).
    std::optional<std::string> value = {};
    std::optional<std::string> default_value = {};
    std::function<void(std::optional<std::string>)> onchange = {};
    /// Multiple selection: the values become removable pills in the well.
    bool multiple = false;
    std::optional<std::vector<std::string>> values = {};
    std::optional<std::vector<std::string>> default_values = {};
    std::function<void(std::vector<std::string>)> onchange_multiple = {};
    /// Controls the text typed in the field.
    std::optional<std::string> input_value = {};
    std::function<void(const std::string&)> oninputchange = {};
    /// Controls whether the suggestions are shown.
    std::optional<bool> open = {};
    /// Accept any typed text as a value (press Enter), not just the options.
    bool free_solo = false;
    std::string no_options_text = "No options";
    std::optional<std::string> label = {};
    std::optional<std::string> placeholder = {};
    std::optional<std::string> helper_text = {};
    TextFieldVariant variant = TextFieldVariant::Outlined;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool disabled = false;
    bool error = false;
    bool required = false;
    bool full_width = false;
    /// Hide the clear (×) button.
    bool disable_clearable = false;
    std::optional<std::string> name = {};
    std::optional<std::string> id = {};
    std::string cls = {};
    std::string style = {};
};

/// A text well that suggests matching options as you type. Mirrors MUI's
/// <Autocomplete>. Keyboard: ↓ ↑ open and move, Enter picks, Escape
/// closes, Backspace in an empty field removes the last pill.
Node Autocomplete(const AutocompleteProps& props);

// ------------------------------------------------------------------ TransferList

struct TransferListProps {
    /// Controlled lists (pair with `onchange`).
    std::optional<std::vector<std::string>> left = {};
    std::optional<std::vector<std::string>> right = {};
    /// Uncontrolled initial lists, read once.
    std::optional<std::vector<std::string>> default_left = {};
    std::optional<std::vector<std::string>> default_right = {};
    /// Rows checked initially.
    std::vector<std::string> default_checked = {};
    /// Called with (left, right) after a move.
    std::function<void(std::vector<std::string>, std::vector<std::string>)> onchange = {};
    std::string left_title = "Choices";
    std::string right_title = "Chosen";
    Color color = Color::Primary;
    bool disabled = false;
    std::string cls = {};
    std::string style = {};
};

/// Two wells of checkable rows with keycaps between them that move the
/// checked rows across (MUI's "Transfer list" recipe).
Node TransferList(const TransferListProps& props);

// ------------------------------------------------------------------ TextareaAutosize

struct TextareaAutosizeProps {
    int min_rows = 1;
    std::optional<int> max_rows = {};
    std::optional<std::string> value = {};
    std::optional<std::string> default_value = {};
    std::optional<std::string> placeholder = {};
    /// Color of the focus halo.
    Color color = Color::Primary;
    bool disabled = false;
    bool read_only = false;
    std::optional<std::string> name = {};
    std::optional<std::string> id = {};
    std::string cls = {};
    std::string style = {};
    std::function<void(const std::string&)> oninput = {};
    std::function<void(const std::string&)> onchange = {};
    /// Accessible name when there is no visible <label for=id>.
    std::optional<std::string> aria_label = {};
};

/// A multi-line well that grows with its content between `min_rows` and
/// `max_rows`, then scrolls. Mirrors MUI's <TextareaAutosize>.
Node TextareaAutosize(const TextareaAutosizeProps& props);

}  // namespace skeuo
