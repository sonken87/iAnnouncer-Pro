#include <algorithm>

#include "common.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

using namespace inputs_detail;

namespace {
std::string toggle_classes(Color color, Size size, bool selected, bool full_width, std::string_view extra) {
    std::string size_class = "skeuo-toggle--" + std::string(as_str(size));
    return classes({"skeuo-key skeuo-key--pressable skeuo-toggle", color_class(color), size_class,
                    selected ? "skeuo-key--down skeuo-toggle--selected" : "",
                    full_width ? "skeuo-toggle--full-width" : "", extra});
}

bool is_toggle_style_token(const std::string& t) {
    static const char* const fixed[] = {"skeuo-key",           "skeuo-key--pressable",    "skeuo-toggle",
                                        "skeuo-toggle--small", "skeuo-toggle--medium",    "skeuo-toggle--large",
                                        "skeuo-key--down",     "skeuo-toggle--selected", "skeuo-toggle--full-width"};
    for (const char* f : fixed)
        if (t == f) return true;
    return starts_with(t, "skeuo-color-");
}
}  // namespace

Node ToggleButton(const ToggleButtonProps& props, Node children) {
    Node button = el("button")
                      .cls(toggle_classes(props.color, props.size, props.selected, props.full_width, props.cls))
                      .attr("type", "button")
                      .attr("value", props.value)
                      .flag("disabled", props.disabled)
                      .attr("aria-pressed", props.selected ? "true" : "false");
    if (props.onclick || props.onchange) {
        button.on("click", [onclick = props.onclick, onchange = props.onchange, value = props.value] {
            if (onclick) onclick();
            if (onchange) onchange(value);
        });
    }
    for (const auto& [k, v] : props.attrs) button.set_attr(k, v);
    return std::move(button).child(std::move(children));
}

Node ToggleButtonGroup(const ToggleButtonGroupProps& props, Node children) {
    auto& internal = use_state<std::vector<std::string>>(
        "ToggleButtonGroup.value", [&] { return props.default_value.value_or(std::vector<std::string>{}); });
    std::vector<std::string> selected = props.value ? *props.value : internal;

    std::string orientation = "skeuo-btn-group--" + std::string(as_str(props.orientation));
    Node root = el("div")
                    .cls(classes({"skeuo-btn-group skeuo-toggle-group", orientation, color_class(props.color),
                                  props.full_width ? "skeuo-btn-group--full-width" : "", props.cls}))
                    .attr("role", "group")
                    .attr("aria-orientation", std::string(as_str(props.orientation)));
    for (const auto& [k, v] : props.attrs) root.set_attr(k, v);
    root.child(std::move(children));

    auto toggle = [&internal, selected, exclusive = props.exclusive, onchange = props.onchange](const std::string& v) {
        bool has = std::find(selected.begin(), selected.end(), v) != selected.end();
        std::vector<std::string> next;
        if (exclusive) {
            if (!has) next.push_back(v);
        } else if (has) {
            for (const auto& x : selected)
                if (x != v) next.push_back(x);
        } else {
            next = selected;
            next.push_back(v);
        }
        internal = next;
        if (onchange) onchange(std::move(next));
    };

    // The group owns its toggle buttons' selection, color, size and
    // disabled state (the Rust ToggleButton reads them from the group's
    // context); nested groups keep their own buttons.
    walk_below(
        root,
        [&](Node& n) {
            if (n.tag() != "button" || !n.has_class("skeuo-toggle")) return;
            const std::string* value_attr = n.get_attr("value");
            std::string value = value_attr ? *value_attr : std::string();
            bool is_selected = std::find(selected.begin(), selected.end(), value) != selected.end();
            bool full_width = props.full_width || n.has_class("skeuo-toggle--full-width");
            std::vector<std::string> rest;
            for (auto& t : class_tokens(n))
                if (!is_toggle_style_token(t)) rest.push_back(t);
            n.set_attr("class", toggle_classes(props.color, props.size, is_selected, full_width, join_tokens(rest)));
            if (props.disabled) n.flag("disabled", true);
            n.set_attr("aria-pressed", is_selected ? "true" : "false");

            // Runs the button's own click handler (onclick, onchange) and
            // then the group's toggle.
            Handler own;
            auto& ls = n.listeners();
            for (auto it = ls.begin(); it != ls.end(); ++it) {
                if (it->type == "click") {
                    own = std::move(it->handler);
                    ls.erase(it);
                    break;
                }
            }
            n.on("click", [own = std::move(own), toggle, value](const Event& e) {
                if (own) own(e);
                toggle(value);
            });
        },
        [](const Node& n) { return n.has_class("skeuo-toggle-group"); });
    return root;
}

}  // namespace skeuo
