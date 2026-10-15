#include <algorithm>

#include "skeuo/inputs.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {
/// Rows needed to show `text` without scrolling, ignoring soft wraps.
int rows_for(const std::string& text, int min, std::optional<int> max) {
    int lines = 1 + static_cast<int>(std::count(text.begin(), text.end(), '\n'));
    min = std::max(min, 1);
    int rows = std::max(lines, min);
    if (max) rows = std::min(rows, std::max(*max, min));
    return rows;
}
}  // namespace

Node TextareaAutosize(const TextareaAutosizeProps& props) {
    const auto& default_value =
        use_state<std::optional<std::string>>("TextareaAutosize.default", [&] { return props.default_value; });
    auto& typed = use_state<std::string>("TextareaAutosize.typed", [&] { return default_value.value_or(""); });
    std::optional<std::string> shown = props.value ? props.value : default_value;
    const std::string& measured = props.value ? *props.value : typed;
    int rows = rows_for(measured, props.min_rows, props.max_rows);

    int min = std::max(props.min_rows, 1);
    std::string style = "--ta-min-rows: " + std::to_string(min) + ";";
    if (props.max_rows) style += " --ta-max-rows: " + std::to_string(std::max(*props.max_rows, min)) + ";";
    style += props.style;

    Node textarea = el("textarea")
                        .cls(classes({"skeuo-textarea skeuo-well skeuo-well--focusable", color_class(props.color),
                                      props.max_rows ? "skeuo-textarea--bounded" : "", props.cls}))
                        .attr("style", style);
    if (props.id) textarea.attr("id", *props.id);
    if (props.name) textarea.attr("name", *props.name);
    textarea.attr("rows", std::to_string(rows));
    if (shown) textarea.attr("value", *shown);
    if (props.placeholder) textarea.attr("placeholder", *props.placeholder);
    textarea.flag("disabled", props.disabled).flag("readonly", props.read_only);
    if (props.aria_label) textarea.attr("aria-label", *props.aria_label);
    textarea.on("input", [&typed, cb = props.oninput](const Event& e) {
        typed = e.value;
        if (cb) cb(e.value);
    });
    if (props.onchange) textarea.on("change", [cb = props.onchange](const Event& e) { cb(e.value); });
    // Also as content: a browser parsing the first page ignores a textarea's
    // `value` attribute.
    if (shown) textarea.child(*shown);
    return textarea;
}

}  // namespace skeuo
