#include "common.hpp"

#include "skeuo/icons.hpp"

namespace skeuo::inputs_detail {

std::vector<std::string> class_tokens(const Node& node) {
    std::vector<std::string> out;
    const std::string* c = node.get_attr("class");
    if (!c) return out;
    size_t pos = 0;
    while (pos < c->size()) {
        size_t end = c->find(' ', pos);
        if (end == std::string::npos) end = c->size();
        if (end > pos) out.push_back(c->substr(pos, end - pos));
        pos = end + 1;
    }
    return out;
}

std::string join_tokens(const std::vector<std::string>& tokens) {
    std::string out;
    for (const auto& t : tokens) {
        if (t.empty()) continue;
        if (!out.empty()) out += ' ';
        out += t;
    }
    return out;
}

bool starts_with(std::string_view s, std::string_view prefix) { return s.substr(0, prefix.size()) == prefix; }

void walk_below(Node& node, const std::function<void(Node&)>& visit, const std::function<bool(const Node&)>& stop) {
    for (auto& c : node.kids()) {
        if (c.kind() != Node::Kind::Element) continue;
        visit(c);
        if (!stop(c)) walk_below(c, visit, stop);
    }
}

std::string_view field_variant_name(TextFieldVariant v) {
    switch (v) {
        case TextFieldVariant::Outlined: return "outlined";
        case TextFieldVariant::Filled: return "filled";
        case TextFieldVariant::Standard: return "standard";
    }
    return "outlined";
}

std::string field_classes(std::string_view base, TextFieldVariant variant, Color color, Size size, bool error,
                          bool disabled, bool full_width, std::string_view extra) {
    std::string v = "skeuo-field--" + std::string(field_variant_name(variant));
    std::string s = "skeuo-field--" + std::string(as_str(size));
    return classes({"skeuo-field", base, v, color_class(color), s, error ? "skeuo-field--error" : "",
                    disabled ? "skeuo-field--disabled" : "", full_width ? "skeuo-field--full-width" : "", extra});
}

std::string_view control_classes(TextFieldVariant variant) {
    switch (variant) {
        case TextFieldVariant::Outlined: return "skeuo-field__control skeuo-well skeuo-well--focusable";
        case TextFieldVariant::Filled:
            return "skeuo-field__control skeuo-well skeuo-well--shallow skeuo-well--focusable";
        case TextFieldVariant::Standard: return "skeuo-field__control";
    }
    return "skeuo-field__control";
}

std::optional<size_t> step_enabled(const std::vector<bool>& disabled, std::optional<size_t> from, int dir) {
    long n = static_cast<long>(disabled.size());
    if (n == 0) return std::nullopt;
    long i = from ? static_cast<long>(*from) + dir : (dir > 0 ? 0 : n - 1);
    while (i >= 0 && i < n) {
        if (!disabled[static_cast<size_t>(i)]) return static_cast<size_t>(i);
        i += dir;
    }
    return from;
}

Node select_row(const std::string& id, const std::string& label, bool selected, bool active, bool disabled,
                std::function<void()> onpick, std::function<void()> onhover) {
    Node li = el("li")
                  .cls(classes({"skeuo-select__option", selected ? "skeuo-select__option--selected" : "",
                                active ? "skeuo-select__option--active" : "",
                                disabled ? "skeuo-select__option--disabled" : ""}))
                  .attr("id", id)
                  .attr("role", "option")
                  .attr("aria-selected", selected ? "true" : "false")
                  .attr_if(disabled, "aria-disabled", "true");
    if (!disabled && onpick) li.on("click", std::move(onpick));
    // Rows already under the pointer need no round trip to become active.
    if (!active && onhover) li.on("mouseenter", std::move(onhover));
    Node check = el("span").cls("skeuo-select__check");
    if (selected) check.child(Icon(IconName::Check));
    return std::move(li).child(std::move(check)).child(el("span").cls("skeuo-select__option-label").child(label));
}

Node field_label(const std::string& text, bool required) {
    Node label = el("label").cls("skeuo-field__label skeuo-label").child(text);
    if (required) label.child(el("span").cls("skeuo-field__asterisk").attr("aria-hidden", "true").child(" *"));
    return label;
}

}  // namespace skeuo::inputs_detail
