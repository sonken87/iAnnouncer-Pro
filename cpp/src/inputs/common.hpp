// Helpers shared by the inputs module's sources (not part of the public API).
#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "skeuo/inputs.hpp"

namespace skeuo::inputs_detail {

/// The class list of an element split into tokens.
std::vector<std::string> class_tokens(const Node& node);
/// Joins tokens (skipping empty ones) with spaces.
std::string join_tokens(const std::vector<std::string>& tokens);
bool starts_with(std::string_view s, std::string_view prefix);

/// Calls `visit` on every element below `node` (not `node` itself), without
/// descending into elements for which `stop` returns true (they are visited).
void walk_below(Node& node, const std::function<void(Node&)>& visit,
                const std::function<bool(const Node&)>& stop);

std::string_view field_variant_name(TextFieldVariant v);

/// Root classes of a TextField-like field (Select, NativeSelect, Autocomplete).
std::string field_classes(std::string_view base, TextFieldVariant variant, Color color, Size size, bool error,
                          bool disabled, bool full_width, std::string_view extra);

/// Classes of the well holding the control.
std::string_view control_classes(TextFieldVariant variant);

/// The next selectable index from `from` in direction `dir` (±1), or the
/// first / last selectable one when `from` is empty.
std::optional<size_t> step_enabled(const std::vector<bool>& disabled, std::optional<size_t> from, int dir);

/// One row of a select / autocomplete listbox.
Node select_row(const std::string& id, const std::string& label, bool selected, bool active, bool disabled,
                std::function<void()> onpick, std::function<void()> onhover);

/// `<label class="skeuo-field__label skeuo-label">` with the required asterisk.
Node field_label(const std::string& text, bool required);

}  // namespace skeuo::inputs_detail
