// Internal helpers shared by the navigation components.
#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "skeuo/node.hpp"

namespace skeuo::nav {

/// Whether a node is an element carrying attribute `name`.
inline bool has_attr(const Node& n, std::string_view name) {
    return n.kind() == Node::Kind::Element && n.get_attr(name) != nullptr;
}

inline std::string attr_or(const Node& n, std::string_view name, std::string fallback = {}) {
    const std::string* v = n.get_attr(name);
    return v ? *v : fallback;
}

/// Pre-order visit that can prune: `visit` returns false to skip a subtree.
inline void visit(Node& n, const std::function<bool(Node&)>& fn) {
    if (!fn(n)) return;
    for (auto& c : n.kids()) visit(c, fn);
}

/// The class tokens of an element.
std::vector<std::string> class_tokens(const Node& n);

/// Rewrites the class attribute so that `extra` follows the token `after`
/// (or comes first when `after` is missing); no-op if already present.
void insert_class_after(Node& n, std::string_view after, std::string_view extra);

/// Adds a listener; if the element already listens to `type`, the new
/// handler runs first and then the old one, in a single listener (the page
/// runtime dispatches only one listener per type and element).
void prepend_listener(Node& n, const std::string& type, Handler handler, unsigned flags = NoFlags);

/// Turns a tab value into something safe inside a DOM id.
std::string id_part(std::string_view value);

/// Formats a double like Rust's `{}` (shortest round-trip, no trailing ".0").
std::string fmt_num(double v);

// ------------------------------------------------------------- roving focus
//
// Moving DOM focus with the arrow keys: the page runtime focuses an element
// rendered with `data-sk-autofocus` once, when that DOM element first carries
// it (it remembers elements, not positions), and it patches children by
// position, replacing a node only when its type or tag differs. So a
// container that moves focus to item i
//
//   1. records the target and sets `pending` (`RovingFocus::move_to`); if
//      item i's current element was already focused this way (`used`), it
//      also flips item i's bit so the element gets replaced (below);
//   2. on the next render marks only the target with `data-sk-autofocus`
//      (`take_mark`, consumed once), and
//   3. once anything has moved (`engaged`), renders each of its children
//      in a slot of two nodes, [child, <!---->] or, with the child's bit
//      set, [<!---->, child] (`freshen`). Flipping a bit swaps an element
//      and a comment in place, so the page creates a fresh element for that
//      one item - which then takes focus - and leaves every other item's
//      element alone (an item under a resting mouse pointer is not
//      replaced, so no synthetic mouseover follows). Empty comments render
//      nothing and, unlike whitespace, the HTML parser never merges them.
//
// Elements are replaced only when needed: the first visit to an item just
// marks its existing element, so hovering an item and clicking it right
// away never races with a replacement of the element under the pointer.
struct RovingFocus {
    std::optional<std::size_t> target;   // last item focus was moved to
    std::optional<std::size_t> current;  // item that reported focus last
    bool pending = false;
    bool engaged = false;
    std::vector<bool> flips;  // per item: element after its comment
    std::vector<bool> used;   // per item: its element took focus already

    void move_to(std::size_t i) {
        target = i;
        pending = true;
        engaged = true;
        if (flips.size() <= i) flips.resize(i + 1, false);
        if (used.size() <= i) used.resize(i + 1, false);
        if (used[i]) flips[i] = !flips[i];
        used[i] = true;
    }
    /// Records a mark the container renders itself (e.g. on a new list).
    void marked(std::size_t i) {
        target = i;
        if (used.size() <= i) used.resize(i + 1, false);
        used[i] = true;
    }
    bool flipped(std::size_t i) const { return i < flips.size() && flips[i]; }
    /// The item to mark in this render, if any (consumes the mark).
    std::optional<std::size_t> take_mark() {
        if (!pending) return std::nullopt;
        pending = false;
        return target;
    }
};

/// The enabled item `step` places after (or before, if negative) `from`,
/// wrapping around; `from` = nullopt starts outside the list.
std::optional<std::size_t> roving_neighbor(const std::vector<bool>& disabled, std::optional<std::size_t> from,
                                           int step);

/// Step 3 above. `items` are the focusable items (in item order) somewhere
/// inside the container's direct children.
void freshen(Node& container, const std::vector<Node*>& items, const RovingFocus& r);

}  // namespace skeuo::nav
