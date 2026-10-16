// skeuo-ui for C++ — Selection controls and forms: Checkbox, Radio, RadioGroup, Switch, Slider, Rating, FormControl, FormLabel, FormControlLabel, FormGroup, FormHelperText.
//
// Mirrors crates/skeuo-ui/src/selection. Props are aggregates; call sites use
// designated initializers in declaration order:
//
//   FormControlLabel({.control = Checkbox({.checked = on, .onchange = [&](bool v) { on = v; }}),
//                     .label = "Duck music"})
//
// Context: in Rust, FormControl / FormControlLabel / RadioGroup share their
// state with the controls inside through Dioxus context. C++ builds children
// before their parent, so these containers instead rewrite the controls they
// were given: each control marks its root with `data-sk-fc="<kind> <props it
// set itself>"` and the container fills in what the control left open
// (disabled / error / required / size / color, and for RadioGroup the name,
// selection and change handler). The result is the same markup as Rust plus
// that one marker attribute.
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/inputs.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ------------------------------------------------------------------ Checkbox

struct CheckboxProps {
    /// Controlled state (pair with `onchange`).
    std::optional<bool> checked = {};
    /// Uncontrolled initial state, read once.
    bool default_checked = false;
    /// Shows a dash instead of a check (`aria-checked="mixed"`).
    bool indeterminate = false;
    /// Defaults to the enclosing form control's color, then primary.
    std::optional<Color> color = {};
    std::optional<Size> size = {};
    bool disabled = false;
    bool required = false;
    std::optional<std::string> name = {};
    std::optional<std::string> value = {};
    /// Id of the `<input>`.
    std::optional<std::string> id = {};
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_describedby = {};
    std::string cls = {};
    /// Called with the input's new checked state.
    std::function<void(bool)> onchange = {};
};

/// A checkbox drawn as a small, square white keycap. Mirrors MUI's <Checkbox>.
Node Checkbox(const CheckboxProps& props);

// ------------------------------------------------------------------ Radio

struct RadioProps {
    /// The value this radio stands for in its group.
    std::string value = {};
    /// Controlled state for a radio outside a RadioGroup.
    std::optional<bool> checked = {};
    std::optional<Color> color = {};
    std::optional<Size> size = {};
    bool disabled = false;
    bool required = false;
    std::optional<std::string> name = {};
    /// Id of the `<input>`.
    std::optional<std::string> id = {};
    std::optional<std::string> aria_label = {};
    std::string cls = {};
    /// Called with `true` when the radio becomes selected.
    std::function<void(bool)> onchange = {};
};

/// A radio button drawn as a small round keycap with an LED. Mirrors MUI's <Radio>.
/// Inside a RadioGroup the group supplies name, selection and change handling.
Node Radio(const RadioProps& props);

struct RadioGroupProps {
    /// Shared `name` of the inputs. Generated when omitted.
    std::optional<std::string> name = {};
    /// Controlled selection (pair with `onchange`).
    std::optional<std::string> value = {};
    /// Uncontrolled initial selection, read once.
    std::optional<std::string> default_value = {};
    /// Lay the radios out in a row.
    bool row = false;
    /// Apply to every radio that doesn't set its own.
    std::optional<Color> color = {};
    std::optional<Size> size = {};
    bool disabled = false;
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_label = {};
    std::string cls = {};
    /// Called with the newly selected radio's `value`.
    std::function<void(const std::string&)> onchange = {};
};

/// A set of Radios sharing one `name`. Mirrors MUI's <RadioGroup>.
Node RadioGroup(const RadioGroupProps& props, Node children);

// ------------------------------------------------------------------ Switch

struct SwitchProps {
    std::optional<bool> checked = {};
    bool default_checked = false;
    std::optional<Color> color = {};
    std::optional<Size> size = {};
    bool disabled = false;
    bool required = false;
    std::optional<std::string> name = {};
    std::optional<std::string> value = {};
    /// Id of the `<input>`.
    std::optional<std::string> id = {};
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_describedby = {};
    std::string cls = {};
    /// Called with the input's new checked state.
    std::function<void(bool)> onchange = {};
};

/// An on/off switch: a groove with a sliding round keycap. Mirrors MUI's <Switch>.
Node Switch(const SwitchProps& props);

// ------------------------------------------------------------------ Slider

/// A slider's value: one number, or a `[start, end]` range with two thumbs
/// (MUI's `number | number[]`). Converts implicitly from `double` and from
/// `std::pair<double, double>`.
struct SliderValue {
    double a = 0.0;
    double b = 0.0;
    bool is_range_ = false;

    SliderValue() = default;
    SliderValue(double v) : a(v), b(v) {}
    SliderValue(std::pair<double, double> r) : a(r.first), b(r.second), is_range_(true) {}

    static SliderValue Single(double v) { return SliderValue(v); }
    static SliderValue Range(double start, double end) { return SliderValue(std::pair{start, end}); }

    /// The value, or the start of a range.
    double single() const { return a; }
    /// `(start, end)`; a single value `v` gives `(v, v)`.
    std::pair<double, double> range() const { return {a, b}; }
    bool is_range() const { return is_range_; }
    bool operator==(const SliderValue&) const = default;
};

/// One tick printed under the rail, optionally with a label.
struct SliderMark {
    double value = 0.0;
    std::optional<std::string> label = {};

    static SliderMark make(double value, std::string label) { return {value, std::move(label)}; }
    /// A tick without a label.
    static SliderMark tick(double value) { return {value, std::nullopt}; }
};

/// MUI's `marks`: none, a tick at every step, or a custom list.
struct SliderMarks {
    enum class Kind { None, Steps, Custom } kind = Kind::None;
    std::vector<SliderMark> list = {};

    SliderMarks() = default;
    /// `true` = a tick at every step.
    SliderMarks(bool on) : kind(on ? Kind::Steps : Kind::None) {}
    SliderMarks(std::vector<SliderMark> marks) : kind(Kind::Custom), list(std::move(marks)) {}
    SliderMarks(std::initializer_list<SliderMark> marks) : kind(Kind::Custom), list(marks) {}
};

/// When the floating value label above a thumb is shown.
enum class ValueLabelDisplay {
    Off,
    Auto,  // while the thumb is hovered, focused or dragged
    On,    // always
};

/// Which part of the rail is filled (MUI's `track`).
enum class SliderTrack {
    Normal,    // from the start (or the first thumb) to the thumb
    Inverted,  // from the thumb to the end
    Off,       // no fill
};

struct SliderProps {
    /// Controlled value (pair with `onchange`); a pair gives two thumbs.
    std::optional<SliderValue> value = {};
    /// Uncontrolled initial value, read once.
    std::optional<SliderValue> default_value = {};
    double min = 0.0;
    double max = 100.0;
    double step = 1.0;
    SliderMarks marks = {};
    ValueLabelDisplay value_label_display = ValueLabelDisplay::Off;
    /// Formats the value label and `aria-valuetext`.
    std::function<std::string(double)> value_label_format = {};
    Orientation orientation = Orientation::Horizontal;
    SliderTrack track = SliderTrack::Normal;
    std::optional<Color> color = {};
    std::optional<Size> size = {};
    bool disabled = false;
    std::optional<std::string> name = {};
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    std::string cls = {};
    std::string style = {};
    /// Called continuously while a thumb moves.
    std::function<void(SliderValue)> onchange = {};
    /// Called when a thumb is released (MUI `onChangeCommitted`).
    std::function<void(SliderValue)> onchange_committed = {};
};

/// A slider on native `<input type="range">`s. Mirrors MUI's <Slider>.
Node Slider(const SliderProps& props);

// ------------------------------------------------------------------ Rating

struct RatingProps {
    /// Controlled value; the inner nullopt means "no rating". Note that
    /// `.value = std::nullopt` means *uncontrolled*: write
    /// `.value = std::optional<double>{}` for a controlled empty rating.
    std::optional<std::optional<double>> value = {};
    std::optional<double> default_value = {};
    unsigned max = 5;
    /// `1.0` (whole stars) or `0.5` (half stars).
    double precision = 1.0;
    bool read_only = false;
    bool disabled = false;
    std::optional<Size> size = {};
    /// `name` of the radio inputs. Generated when omitted.
    std::optional<std::string> name = {};
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    std::string cls = {};
    /// Called with the new value; nullopt when the rating is cleared.
    std::function<void(std::optional<double>)> onchange = {};
    /// Called with the hovered value, nullopt when the pointer leaves.
    std::function<void(std::optional<double>)> onchange_active = {};
};

/// A star rating printed on the aluminium. Mirrors MUI's <Rating>.
Node Rating(const RatingProps& props);

// ------------------------------------------------------------------ Form

struct FormControlProps {
    bool disabled = false;
    bool error = false;
    bool required = false;
    /// Size / color for controls that don't set their own.
    std::optional<Size> size = {};
    std::optional<Color> color = {};
    bool full_width = false;
    /// Render as a `<fieldset>` (MUI `component="fieldset"`).
    bool fieldset = false;
    std::string cls = {};
};

/// Wraps form elements and shares disabled / error / required / size / color
/// with them. Mirrors MUI's <FormControl>.
Node FormControl(const FormControlProps& props, Node children);

struct FormLabelProps {
    std::optional<std::string> id = {};
    /// Id of the control this labels (`for`).
    std::optional<std::string> html_for = {};
    std::optional<bool> error = {};
    std::optional<bool> disabled = {};
    std::optional<bool> required = {};
    /// Render as a `<legend>` (inside a fieldset FormControl).
    bool legend = false;
    std::string cls = {};
};

/// A caption above a group of controls. Mirrors MUI's <FormLabel>.
Node FormLabel(const FormLabelProps& props, Node children);

/// Where a FormControlLabel's text sits relative to its control.
enum class LabelPlacement { End, Start, Top, Bottom };

struct FormControlLabelProps {
    /// The control, e.g. `Checkbox({})`.
    Node control = {};
    std::string label = {};
    LabelPlacement label_placement = LabelPlacement::End;
    std::optional<bool> disabled = {};
    /// Shows an asterisk and marks the control `required`.
    std::optional<bool> required = {};
    std::string cls = {};
};

/// A control with its legend printed next to it. Mirrors MUI's <FormControlLabel>.
Node FormControlLabel(const FormControlLabelProps& props);

struct FormGroupProps {
    bool row = false;
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_label = {};
    std::string cls = {};
};

/// Stacks controls in a column (or a row). Mirrors MUI's <FormGroup>.
Node FormGroup(const FormGroupProps& props, Node children);

struct FormHelperTextProps {
    std::optional<std::string> id = {};
    std::optional<bool> error = {};
    std::optional<bool> disabled = {};
    std::string cls = {};
};

/// Small print under a control. Mirrors MUI's <FormHelperText>.
Node FormHelperText(const FormHelperTextProps& props, Node children);

}  // namespace skeuo
