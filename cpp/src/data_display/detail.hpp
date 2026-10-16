// Helpers shared by the data display components (private to src/data_display).
#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "skeuo/data_display.hpp"

namespace skeuo::data_display_detail {

inline std::string_view flag(bool on, std::string_view cls) { return on ? cls : std::string_view(); }

/// Visits every element below `root` (pre-order) except the subtrees of
/// elements for which `stop` is true. Containers use it to configure their
/// own children without reaching into nested containers of the same kind.
inline void walk_children(Node& root, const std::function<bool(const Node&)>& stop,
                          const std::function<void(Node&)>& visit) {
    for (auto& c : root.kids()) {
        if (c.kind() != Node::Kind::Element) continue;
        if (stop(c)) continue;
        visit(c);
        walk_children(c, stop, visit);
    }
}

/// Adds `name` right after the element's first class (where the Rust
/// component puts its context-derived modifier), unless it is already there.
inline void insert_class_after_first(Node& n, std::string_view name) {
    if (n.has_class(name)) return;
    const std::string* cur = n.get_attr("class");
    if (!cur || cur->empty()) {
        n.set_attr("class", std::string(name));
        return;
    }
    std::string s = *cur;
    size_t sp = s.find(' ');
    if (sp == std::string::npos) s += " " + std::string(name);
    else s.insert(sp, " " + std::string(name));
    n.set_attr("class", s);
}

/// Formats a number like Rust's `Display` for f32 (8.0 -> "8", 12.5 -> "12.5").
std::string format_number(double v);

}  // namespace skeuo::data_display_detail
