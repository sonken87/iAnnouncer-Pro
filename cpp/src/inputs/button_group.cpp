#include "common.hpp"

namespace skeuo {

using namespace inputs_detail;

namespace {
std::string_view variant_name(ButtonVariant v) {
    switch (v) {
        case ButtonVariant::Contained: return "contained";
        case ButtonVariant::Outlined: return "outlined";
        case ButtonVariant::Text: return "text";
    }
    return "contained";
}

bool is_button_style_token(const std::string& t) {
    static const char* const fixed[] = {"skeuo-key",          "skeuo-key--pressable", "skeuo-key--filled",
                                        "skeuo-btn",          "skeuo-btn--contained", "skeuo-btn--outlined",
                                        "skeuo-btn--text",    "skeuo-btn--small",     "skeuo-btn--medium",
                                        "skeuo-btn--large",   "skeuo-btn--full-width"};
    for (const char* f : fixed)
        if (t == f) return true;
    return starts_with(t, "skeuo-color-");
}

// Gives a Button the group's look, as the Rust Button does when it finds a
// ButtonGroup context: the classes are rebuilt in Button's order.
void restyle(Node& button, const ButtonGroupProps& g) {
    std::vector<std::string> tokens = class_tokens(button);
    bool full_width = g.full_width;
    std::vector<std::string> rest;
    for (auto& t : tokens) {
        if (t == "skeuo-btn--full-width") full_width = true;
        if (!is_button_style_token(t)) rest.push_back(t);
    }
    std::string_view material = g.variant == ButtonVariant::Contained
                                    ? "skeuo-key skeuo-key--pressable skeuo-key--filled"
                                : g.variant == ButtonVariant::Outlined ? "skeuo-key skeuo-key--pressable"
                                                                       : "";
    std::vector<std::string> out = {std::string(material),
                                    "skeuo-btn",
                                    "skeuo-btn--" + std::string(variant_name(g.variant)),
                                    std::string(color_class(g.color)),
                                    "skeuo-btn--" + std::string(as_str(g.size)),
                                    full_width ? "skeuo-btn--full-width" : ""};
    out.insert(out.end(), rest.begin(), rest.end());
    button.set_attr("class", join_tokens(out));
    if (g.disabled) button.flag("disabled", true);
}
}  // namespace

Node ButtonGroup(const ButtonGroupProps& props, Node children) {
    std::string variant = "skeuo-btn-group--" + std::string(variant_name(props.variant));
    std::string orientation = "skeuo-btn-group--" + std::string(as_str(props.orientation));
    Node root = el("div")
                    .cls(classes({"skeuo-btn-group", variant, orientation, color_class(props.color),
                                  props.full_width ? "skeuo-btn-group--full-width" : "",
                                  props.disabled ? "skeuo-btn-group--disabled" : "", props.cls}))
                    .attr("role", "group")
                    .attr("aria-orientation", std::string(as_str(props.orientation)));
    for (const auto& [k, v] : props.attrs) root.set_attr(k, v);
    root.child(std::move(children));
    // The nearest group wins, so nested groups keep their own buttons.
    walk_below(
        root,
        [&](Node& n) {
            if (n.tag() == "button" && n.has_class("skeuo-btn")) restyle(n, props);
        },
        [](const Node& n) { return n.has_class("skeuo-btn-group"); });
    return root;
}

}  // namespace skeuo
