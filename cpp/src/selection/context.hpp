// Internal: how FormControl / FormControlLabel / RadioGroup reach the controls
// inside them.
//
// Rust shares this state top-down through Dioxus context. In C++ the children
// are built first, so each control renders as if it had no context and marks
// its root with
//
//     data-sk-fc="<kind> <field> <field> ..."
//
// listing the fields it decided by itself (an explicit prop, or a nearer
// container that already supplied the value). A container then walks its
// children and applies its values to every field still open, exactly as the
// control would have resolved them in Rust.
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>

#include "skeuo/core.hpp"
#include "skeuo/node.hpp"

namespace skeuo::detail {

using FcField = unsigned;
inline constexpr FcField FcDisabled = 1;
inline constexpr FcField FcError = 2;
inline constexpr FcField FcRequired = 4;
inline constexpr FcField FcColor = 8;
inline constexpr FcField FcSize = 16;
inline constexpr FcField FcChecked = 32;  // radio: `checked` given explicitly
inline constexpr FcField FcName = 64;     // radio: `name` given explicitly

/// The marker attribute value for a control of `kind` with `fixed` fields.
std::string fc_marker(std::string_view kind, unsigned fixed);

/// What a container supplies. A value is applied to every control where the
/// field is still open; `claim` additionally closes the field so containers
/// further out no longer reach it (the nearest provider wins, as in Rust).
struct FcProvision {
    std::optional<bool> disabled;
    bool claim_disabled = false;
    std::optional<bool> error;
    bool claim_error = false;
    std::optional<bool> required;
    bool claim_required = false;
    std::optional<Color> color;
    bool claim_color = false;
    std::optional<Size> size;
    bool claim_size = false;
};

/// Applies `p` to the marked controls in `tree`.
void fc_provide(Node& tree, const FcProvision& p);

/// What a RadioGroup supplies to the radios inside it.
struct RadioGroupProvision {
    std::string name;
    std::optional<std::string> value;
    std::optional<Color> color;
    std::optional<Size> size;
    bool disabled = false;
    std::function<void(const std::string&)> select;
};

void radio_group_provide(Node& tree, const RadioGroupProvision& p);

/// Rust's `{}` for f64: shortest round-trip digits, never an exponent,
/// no trailing ".0" (72.0 -> "72", 12.5 -> "12.5").
std::string fmt_num(double v);
/// Rust's `{:.N}`.
std::string fmt_fixed(double v, int decimals);

}  // namespace skeuo::detail
