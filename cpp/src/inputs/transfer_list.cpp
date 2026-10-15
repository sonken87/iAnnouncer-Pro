#include <algorithm>

#include "common.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {
using Strings = std::vector<std::string>;

bool contains(const Strings& v, const std::string& x) { return std::find(v.begin(), v.end(), x) != v.end(); }

/// A small checkbox keycap for list rows (the public Checkbox lives in the
/// selection module).
Node row_check(bool checked, bool indeterminate, bool disabled, std::optional<std::string> label,
               std::optional<std::string> labelledby, std::function<void()> onchange) {
    bool on = checked || indeterminate;
    Node input = el("input")
                     .cls("skeuo-transfer__native")
                     .attr("type", "checkbox")
                     .flag("checked", checked)
                     .flag("disabled", disabled)
                     .attr("aria-checked", indeterminate ? "mixed" : checked ? "true" : "false");
    if (label) input.attr("aria-label", *label);
    if (labelledby) input.attr("aria-labelledby", *labelledby);
    input.on("change", std::move(onchange));
    Node cap = el("span").cls(classes({"skeuo-transfer__cap skeuo-key", on ? "skeuo-key--filled" : ""}))
                   .attr("aria-hidden", "true");
    if (indeterminate) {
        cap.child(Icon(IconName::Remove));
    } else if (checked) {
        cap.child(Icon(IconName::Check));
    }
    return el("span").cls("skeuo-transfer__check").child(std::move(input)).child(std::move(cap));
}
}  // namespace

Node TransferList(const TransferListProps& props) {
    std::string id = use_unique_id("skeuo-transfer");
    auto& internal_left = use_state<Strings>("TransferList.left", [&] { return props.default_left.value_or(Strings{}); });
    auto& internal_right =
        use_state<Strings>("TransferList.right", [&] { return props.default_right.value_or(Strings{}); });
    auto& checked = use_state<Strings>("TransferList.checked", [&] { return props.default_checked; });
    Strings left = props.left ? *props.left : internal_left;
    Strings right = props.right ? *props.right : internal_right;
    bool disabled = props.disabled;

    // Moves the checked rows (or all of them) from one side to the other.
    auto mv = [&internal_left, &internal_right, &checked, left, right, onchange = props.onchange](bool to_right,
                                                                                                  bool all) {
        const Strings& from = to_right ? left : right;
        const Strings& to = to_right ? right : left;
        Strings moving, rest;
        for (const auto& v : from) (all || contains(checked, v) ? moving : rest).push_back(v);
        if (moving.empty()) return;
        Strings grown = to;
        grown.insert(grown.end(), moving.begin(), moving.end());
        Strings l = to_right ? rest : grown;
        Strings r = to_right ? grown : rest;
        Strings still;
        for (const auto& v : checked)
            if (!contains(moving, v)) still.push_back(v);
        checked = still;
        internal_left = l;
        internal_right = r;
        if (onchange) onchange(std::move(l), std::move(r));
    };

    auto count_checked = [&](const Strings& items) {
        return static_cast<size_t>(std::count_if(items.begin(), items.end(), [&](const auto& v) {
            return contains(checked, v);
        }));
    };
    size_t left_checked = count_checked(left);
    size_t right_checked = count_checked(right);

    auto side = [&](const std::string& side_id, const std::string& title, const Strings& items, size_t n_checked) {
        std::string title_id = side_id + "-title";
        bool all = !items.empty() && n_checked == items.size();
        bool some = n_checked > 0 && !all;
        Node header = el("div").cls("skeuo-transfer__header");
        header.child(row_check(all, some, disabled || items.empty(), "Select all " + title, std::nullopt,
                               [&checked, all, items] {
                                   Strings c = checked;
                                   if (all) {
                                       c.erase(std::remove_if(c.begin(), c.end(),
                                                              [&](const auto& v) { return contains(items, v); }),
                                               c.end());
                                   } else {
                                       for (const auto& v : items)
                                           if (!contains(c, v)) c.push_back(v);
                                   }
                                   checked = c;
                               }));
        header.child(el("span").cls("skeuo-transfer__title").attr("id", title_id).child(title));
        header.child(el("span").cls("skeuo-caption").child(std::to_string(n_checked) + "/" +
                                                           std::to_string(items.size()) + " selected"));

        Node list = el("ul").cls("skeuo-transfer__list skeuo-well").attr("role", "list").attr("aria-labelledby", title_id);
        if (items.empty()) list.child(el("li").cls("skeuo-transfer__empty skeuo-caption").child("Empty"));
        for (size_t i = 0; i < items.size(); ++i) {
            const std::string& item = items[i];
            bool is_checked = contains(checked, item);
            std::string label_id = side_id + "-" + std::to_string(i) + "-label";
            Node label = el("label").cls("skeuo-transfer__row-label");
            label.child(row_check(is_checked, false, disabled, std::nullopt, label_id, [&checked, item] {
                auto pos = std::find(checked.begin(), checked.end(), item);
                if (pos != checked.end()) {
                    checked.erase(pos);
                } else {
                    checked.push_back(item);
                }
            }));
            label.child(el("span").attr("id", label_id).child(item));
            list.child(el("li")
                           .cls(is_checked ? "skeuo-transfer__row skeuo-transfer__row--checked" : "skeuo-transfer__row")
                           .child(std::move(label)));
        }
        return el("div").cls("skeuo-transfer__side").child(std::move(header)).child(std::move(list));
    };

    auto move_button = [&](bool off, const char* label, IconName icon, bool to_right, bool all) {
        return Button({.variant = ButtonVariant::Outlined,
                       .color = Color::Neutral,
                       .size = Size::Small,
                       .disabled = off,
                       .cls = "skeuo-transfer__move",
                       .onclick = [mv, to_right, all] { mv(to_right, all); },
                       .attrs = {{"aria-label", label}}},
                      Icon(icon));
    };

    Node root = el("div").cls(classes({"skeuo-transfer", color_class(props.color),
                                       disabled ? "skeuo-transfer--disabled" : "", props.cls}));
    root.attr("style", props.style);
    root.child(side(id + "-left", props.left_title, left, left_checked));
    root.child(el("div")
                   .cls("skeuo-transfer__moves")
                   .attr("role", "group")
                   .attr("aria-label", "Move items")
                   .child(move_button(disabled || left.empty(), "Move all right", IconName::LastPage, true, true))
                   .child(move_button(disabled || left_checked == 0, "Move selected right", IconName::ChevronRight,
                                      true, false))
                   .child(move_button(disabled || right_checked == 0, "Move selected left", IconName::ChevronLeft,
                                      false, false))
                   .child(move_button(disabled || right.empty(), "Move all left", IconName::FirstPage, false, true)));
    root.child(side(id + "-right", props.right_title, right, right_checked));
    return root;
}

}  // namespace skeuo
