#include "skeuo/icons.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

Node Breadcrumbs(const BreadcrumbsProps& props) {
    bool& expanded = use_state<bool>("Breadcrumbs.expanded", [] { return false; });
    const size_t n = props.items.size();
    bool collapse = !expanded && n > props.max_items &&
                    props.items_before_collapse + props.items_after_collapse < n;

    // (original index, item) pairs, with index n marking the "…" keycap.
    std::vector<size_t> shown;
    if (collapse) {
        for (size_t i = 0; i < props.items_before_collapse && i < n; ++i) shown.push_back(i);
        shown.push_back(n);
        for (size_t i = n - props.items_after_collapse; i < n; ++i) shown.push_back(i);
    } else {
        for (size_t i = 0; i < n; ++i) shown.push_back(i);
    }

    Node list = el("ol").cls("skeuo-breadcrumbs__list");
    for (size_t pos = 0; pos < shown.size(); ++pos) {
        size_t i = shown[pos];
        if (i < n) {
            bool current = i + 1 == n;
            list.child(el("li")
                           .cls(current ? "skeuo-breadcrumbs__item skeuo-breadcrumbs__item--current"
                                        : "skeuo-breadcrumbs__item")
                           .attr_if(current, "aria-current", "page")
                           .child(props.items[i]));
        } else {
            list.child(el("li").cls("skeuo-breadcrumbs__item").child(
                el("button")
                    .cls("skeuo-key skeuo-key--pressable skeuo-breadcrumbs__collapsed")
                    .attr("type", "button")
                    .attr("aria-label", props.expand_text)
                    .on("click", [&expanded] { expanded = true; })
                    .child(Icon(IconName::MoreHoriz))));
        }
        if (pos + 1 < shown.size()) {
            list.child(el("li")
                           .cls("skeuo-breadcrumbs__separator")
                           .attr("aria-hidden", "true")
                           .child(props.separator ? *props.separator : Icon(IconName::ChevronRight)));
        }
    }
    return el("nav")
        .cls(classes({"skeuo-breadcrumbs", props.cls}))
        .attr("aria-label", props.aria_label)
        .child(std::move(list));
}

}  // namespace skeuo
