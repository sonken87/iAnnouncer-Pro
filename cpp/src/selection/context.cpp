#include "context.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <vector>

namespace skeuo::detail {

namespace {

// ---------------------------------------------------------------- markers

struct FcInfo {
    std::string kind;
    unsigned fixed = 0;
};

constexpr std::pair<FcField, std::string_view> kFieldNames[] = {
    {FcDisabled, "disabled"}, {FcError, "error"}, {FcRequired, "required"}, {FcColor, "color"},
    {FcSize, "size"},         {FcChecked, "checked"}, {FcName, "name"},
};

std::vector<std::string> split(std::string_view s) {
    std::vector<std::string> out;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t end = s.find(' ', pos);
        if (end == std::string_view::npos) end = s.size();
        if (end > pos) out.emplace_back(s.substr(pos, end - pos));
        pos = end + 1;
    }
    return out;
}

std::string join(const std::vector<std::string>& parts) {
    std::string out;
    for (const auto& p : parts) {
        if (p.empty()) continue;
        if (!out.empty()) out += ' ';
        out += p;
    }
    return out;
}

std::optional<FcInfo> fc_read(const Node& n) {
    if (n.kind() != Node::Kind::Element) return std::nullopt;
    const std::string* m = n.get_attr("data-sk-fc");
    if (!m) return std::nullopt;
    auto parts = split(*m);
    if (parts.empty()) return std::nullopt;
    FcInfo info{parts[0], 0};
    for (size_t i = 1; i < parts.size(); ++i)
        for (auto [f, name] : kFieldNames)
            if (parts[i] == name) info.fixed |= f;
    return info;
}

void fc_write(Node& n, const FcInfo& info) { n.set_attr("data-sk-fc", fc_marker(info.kind, info.fixed)); }

// ---------------------------------------------------------------- classes

bool starts_with(std::string_view s, std::string_view p) { return s.substr(0, p.size()) == p; }

std::vector<std::string> class_tokens(const Node& n) {
    const std::string* c = n.get_attr("class");
    return c ? split(*c) : std::vector<std::string>{};
}

void set_class_tokens(Node& n, const std::vector<std::string>& t) { n.set_attr("class", join(t)); }

void add_token(Node& n, std::string_view token) {
    auto t = class_tokens(n);
    if (std::find(t.begin(), t.end(), token) != t.end()) return;
    t.emplace_back(token);
    set_class_tokens(n, t);
}

void remove_token(Node& n, std::string_view token) {
    auto t = class_tokens(n);
    auto it = std::remove(t.begin(), t.end(), token);
    if (it == t.end()) return;
    t.erase(it, t.end());
    set_class_tokens(n, t);
}

/// How a control's root classes are laid out, so a modifier added later
/// lands where the control itself would have put it.
struct KindSpec {
    std::string_view base;
    std::vector<std::string_view> extras;     // other leading classes ("skeuo-label")
    std::vector<std::string_view> modifiers;  // optional modifiers in their order
};

const KindSpec* spec_for(std::string_view kind) {
    static const KindSpec checkbox{"skeuo-checkbox", {}, {"skeuo-checkbox--checked", "skeuo-checkbox--indeterminate", "skeuo-checkbox--disabled"}};
    static const KindSpec radio{"skeuo-radio", {}, {"skeuo-radio--checked", "skeuo-radio--disabled"}};
    static const KindSpec sw{"skeuo-switch", {}, {"skeuo-switch--checked", "skeuo-switch--disabled"}};
    static const KindSpec slider{"skeuo-slider", {}, {"skeuo-slider--range", "skeuo-slider--labelled", "skeuo-slider--disabled"}};
    static const KindSpec rating{"skeuo-rating", {}, {"skeuo-rating--read-only", "skeuo-rating--disabled", "skeuo-rating--hovering"}};
    static const KindSpec label{"skeuo-form-label", {"skeuo-label"}, {"skeuo-form-label--error", "skeuo-form-label--disabled"}};
    static const KindSpec helper{"skeuo-form-helper-text", {"skeuo-caption"}, {"skeuo-form-helper-text--error", "skeuo-form-helper-text--disabled"}};
    static const KindSpec fcl{"skeuo-form-control-label", {}, {"skeuo-form-control-label--disabled"}};
    if (kind == "checkbox") return &checkbox;
    if (kind == "radio") return &radio;
    if (kind == "switch") return &sw;
    if (kind == "slider") return &slider;
    if (kind == "rating") return &rating;
    if (kind == "form-label") return &label;
    if (kind == "helper-text") return &helper;
    if (kind == "form-control-label") return &fcl;
    return nullptr;
}

/// Adds a modifier class at the position the control would have rendered it.
void add_modifier(Node& n, const KindSpec& spec, std::string_view token) {
    auto t = class_tokens(n);
    if (std::find(t.begin(), t.end(), token) != t.end()) return;
    auto mod = std::find(spec.modifiers.begin(), spec.modifiers.end(), token);
    // Before the first modifier that comes after this one...
    if (mod != spec.modifiers.end()) {
        for (size_t i = 0; i < t.size(); ++i) {
            if (std::find(mod + 1, spec.modifiers.end(), t[i]) != spec.modifiers.end()) {
                t.insert(t.begin() + long(i), std::string(token));
                set_class_tokens(n, t);
                return;
            }
        }
    }
    // ...else after the control's own classes (before the user's `cls`).
    size_t at = 0;
    for (size_t i = 0; i < t.size(); ++i) {
        bool own = starts_with(t[i], spec.base) || starts_with(t[i], "skeuo-color-") ||
                   std::find(spec.extras.begin(), spec.extras.end(), t[i]) != spec.extras.end();
        if (own) at = i + 1;
    }
    t.insert(t.begin() + long(at), std::string(token));
    set_class_tokens(n, t);
}

void replace_color(Node& n, Color c) {
    auto t = class_tokens(n);
    for (auto& tok : t)
        if (starts_with(tok, "skeuo-color-")) tok = std::string(color_class(c));
    set_class_tokens(n, t);
}

void replace_size(Node& n, const KindSpec& spec, Size s) {
    auto t = class_tokens(n);
    std::string b(spec.base);
    for (auto& tok : t)
        if (tok == b + "--small" || tok == b + "--medium" || tok == b + "--large") tok = b + "--" + std::string(as_str(s));
    set_class_tokens(n, t);
}

// ---------------------------------------------------------------- attributes

/// Sets an attribute right after the last present attribute of `after`
/// (keeps the attribute order of the Rust markup).
void put_attr(Node& n, const std::string& name, const std::string& value, std::initializer_list<std::string_view> after) {
    if (n.get_attr(name)) {
        n.set_attr(name, value);
        return;
    }
    auto attrs = n.attrs();
    size_t at = attrs.size();
    for (size_t i = attrs.size(); i-- > 0;) {
        if (std::find(after.begin(), after.end(), attrs[i].first) != after.end()) {
            at = i + 1;
            break;
        }
    }
    attrs.insert(attrs.begin() + long(at), {name, value});
    std::vector<std::string> names;
    for (const auto& a : n.attrs()) names.push_back(a.first);
    for (const auto& k : names) n.remove_attr(k);
    for (auto& [k, v] : attrs) n.set_attr(k, v);
}

constexpr std::initializer_list<std::string_view> kBeforeDisabled = {"class", "type", "role", "id", "name", "value", "checked", "min", "max", "step"};
constexpr std::initializer_list<std::string_view> kBeforeRequired = {"class", "type", "role", "id", "name", "value", "checked", "disabled"};

template <class F>
void each_with_class(Node& root, std::string_view cls, F&& f) {
    root.walk([&](Node& n) {
        if (n.kind() == Node::Kind::Element && n.has_class(cls)) f(n);
    });
}

// ---------------------------------------------------------------- fields

void apply_disabled(Node& n, const std::string& kind, const KindSpec& spec) {
    add_modifier(n, spec, std::string(spec.base) + "--disabled");
    auto disable_input = [](Node& input) { put_attr(input, "disabled", "", kBeforeDisabled); };
    if (kind == "checkbox" || kind == "radio") {
        std::string cap = std::string(spec.base) + "__cap";
        each_with_class(n, cap, [](Node& c) { put_attr(c, "aria-disabled", "true", {"class"}); });
        each_with_class(n, std::string(spec.base) + "__input", disable_input);
    } else if (kind == "switch" || kind == "slider") {
        each_with_class(n, std::string(spec.base) + "__input", disable_input);
    } else if (kind == "rating") {
        // No longer interactive: no hover preview, inputs disabled.
        remove_token(n, "skeuo-rating--hovering");
        const std::string* role = n.get_attr("role");
        if (role && *role == "radiogroup") put_attr(n, "aria-disabled", "true", {"role", "aria-label", "aria-labelledby"});
        n.walk([&](Node& d) {
            auto& ls = d.listeners();
            ls.erase(std::remove_if(ls.begin(), ls.end(),
                                    [](const Node::Listener& l) { return l.type == "mouseenter" || l.type == "mouseleave"; }),
                     ls.end());
            if (d.kind() == Node::Kind::Element && d.has_class("skeuo-rating__item--active")) remove_token(d, "skeuo-rating__item--active");
        });
        each_with_class(n, "skeuo-rating__input", disable_input);
    }
}

void apply_required(Node& n, const std::string& kind, const KindSpec& spec) {
    if (kind == "checkbox" || kind == "radio" || kind == "switch") {
        each_with_class(n, std::string(spec.base) + "__input",
                        [](Node& input) { put_attr(input, "required", "", kBeforeRequired); });
    } else if (kind == "form-label") {
        bool has = false;
        for (const auto& c : n.kids())
            if (c.kind() == Node::Kind::Element && c.has_class("skeuo-form-label__asterisk")) has = true;
        if (!has) n.child(el("span").cls("skeuo-form-label__asterisk").attr("aria-hidden", "true").child(" *"));
    }
}

bool takes(const std::string& kind, FcField f) {
    switch (f) {
        case FcDisabled: return true;
        case FcError: return kind == "form-label" || kind == "helper-text";
        case FcRequired: return kind == "checkbox" || kind == "radio" || kind == "switch" || kind == "form-label";
        case FcColor: return kind == "checkbox" || kind == "radio" || kind == "switch" || kind == "slider";
        case FcSize: return kind == "checkbox" || kind == "radio" || kind == "switch" || kind == "slider" || kind == "rating";
        default: return false;
    }
}

}  // namespace

std::string fc_marker(std::string_view kind, unsigned fixed) {
    std::string out(kind);
    for (auto [f, name] : kFieldNames) {
        if (fixed & f) {
            out += ' ';
            out += name;
        }
    }
    return out;
}

void fc_provide(Node& tree, const FcProvision& p) {
    tree.walk([&](Node& n) {
        auto info = fc_read(n);
        if (!info) return;
        const KindSpec* spec = spec_for(info->kind);
        if (!spec) return;
        const std::string& kind = info->kind;
        auto open = [&](FcField f) { return takes(kind, f) && !(info->fixed & f); };
        // Same order as the classes: error before disabled.
        if (open(FcError)) {
            if (p.error.value_or(false)) add_modifier(n, *spec, std::string(spec->base) + "--error");
            if (p.claim_error) info->fixed |= FcError;
        }
        if (open(FcDisabled)) {
            if (p.disabled.value_or(false)) apply_disabled(n, kind, *spec);
            if (p.claim_disabled) info->fixed |= FcDisabled;
        }
        if (open(FcRequired)) {
            if (p.required.value_or(false)) apply_required(n, kind, *spec);
            if (p.claim_required) info->fixed |= FcRequired;
        }
        if (open(FcColor)) {
            if (p.color) replace_color(n, *p.color);
            if (p.claim_color) info->fixed |= FcColor;
        }
        if (open(FcSize)) {
            if (p.size) replace_size(n, *spec, *p.size);
            if (p.claim_size) info->fixed |= FcSize;
        }
        fc_write(n, *info);
    });
}

void radio_group_provide(Node& tree, const RadioGroupProvision& p) {
    const KindSpec& spec = *spec_for("radio");
    tree.walk([&](Node& n) {
        auto info = fc_read(n);
        if (!info || info->kind != "radio") return;
        Node* input = nullptr;
        n.walk([&](Node& d) {
            if (!input && d.kind() == Node::Kind::Element && d.has_class("skeuo-radio__input")) input = &d;
        });
        if (!input) return;
        const std::string* v = input->get_attr("value");
        std::string value = v ? *v : std::string();

        if (!(info->fixed & FcName)) {
            put_attr(*input, "name", p.name, {"class", "type", "id"});
            info->fixed |= FcName;
        }
        if (!(info->fixed & FcChecked)) {
            if (p.value && *p.value == value) {
                add_modifier(n, spec, "skeuo-radio--checked");
                each_with_class(n, "skeuo-radio__cap", [](Node& c) { add_token(c, "skeuo-key--down"); });
                put_attr(*input, "checked", "", {"class", "type", "id", "name", "value"});
                each_with_class(n, "skeuo-radio__led", [](Node& l) { remove_token(l, "skeuo-led--off"); });
            }
            info->fixed |= FcChecked;
        }
        // The group's disabled adds to the radio's own and its form control's.
        if (p.disabled && !n.has_class("skeuo-radio--disabled")) apply_disabled(n, "radio", spec);
        if (!(info->fixed & FcColor) && p.color) {
            replace_color(n, *p.color);
            info->fixed |= FcColor;
        }
        if (!(info->fixed & FcSize) && p.size) {
            replace_size(n, spec, *p.size);
            info->fixed |= FcSize;
        }
        // Selecting goes to the group first, then to the radio's own handler.
        if (p.select) {
            auto select = p.select;
            auto& ls = input->listeners();
            auto it = std::find_if(ls.begin(), ls.end(), [](const Node::Listener& l) { return l.type == "change"; });
            if (it == ls.end()) {
                input->on("change", [select, value](const Event&) { select(value); });
            } else {
                it->handler = [select, value, own = it->handler](const Event& e) {
                    select(value);
                    if (own) own(e);
                };
            }
        }
        fc_write(n, *info);
    });
}

std::string fmt_num(double v) {
    if (v == 0.0) v = 0.0;  // no "-0"
    char buf[400];
    auto r = std::to_chars(buf, buf + sizeof buf, v, std::chars_format::fixed);
    if (r.ec != std::errc()) return std::to_string(v);
    return std::string(buf, r.ptr);
}

std::string fmt_fixed(double v, int decimals) {
    char buf[400];
    std::snprintf(buf, sizeof buf, "%.*f", decimals, v);
    return buf;
}

}  // namespace skeuo::detail
