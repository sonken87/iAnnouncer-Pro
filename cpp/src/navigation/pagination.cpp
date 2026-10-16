#include <algorithm>

#include "skeuo/icons.hpp"
#include "skeuo/navigation.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {
using Kind = PaginationItemType::Kind;

std::string_view variant_name(PaginationVariant v) {
    return v == PaginationVariant::Outlined ? "outlined" : "text";
}
std::string_view shape_name(PaginationShape s) { return s == PaginationShape::Rounded ? "rounded" : "circular"; }
}  // namespace

std::vector<PaginationItemType> pagination_items(int count, int page, int sib, int bnd, bool show_first_button,
                                                 bool show_last_button, bool hide_prev_button,
                                                 bool hide_next_button) {
    auto range = [](int a, int b) {
        std::vector<int> out;
        for (int i = a; i <= b; ++i) out.push_back(i);
        return out;
    };
    std::vector<int> start_pages = range(1, std::min(bnd, count));
    std::vector<int> end_pages = range(std::max(count - bnd + 1, bnd + 1), count);
    int siblings_start = std::max(std::min(page - sib, count - bnd - sib * 2 - 1), bnd + 2);
    int siblings_end = std::min(std::max(page + sib, bnd + sib * 2 + 2),
                                end_pages.empty() ? count - 1 : end_pages.front() - 2);

    std::vector<PaginationItemType> items;
    if (show_first_button) items.push_back(PaginationItemType::first());
    if (!hide_prev_button) items.push_back(PaginationItemType::previous());
    for (int p : start_pages) items.push_back(PaginationItemType::page(p));
    if (siblings_start > bnd + 2) items.push_back(PaginationItemType::start_ellipsis());
    else if (bnd + 1 < count - bnd) items.push_back(PaginationItemType::page(bnd + 1));
    for (int p : range(siblings_start, siblings_end)) items.push_back(PaginationItemType::page(p));
    if (siblings_end < count - bnd - 1) items.push_back(PaginationItemType::end_ellipsis());
    else if (count - bnd > bnd) items.push_back(PaginationItemType::page(count - bnd));
    for (int p : end_pages) items.push_back(PaginationItemType::page(p));
    if (!hide_next_button) items.push_back(PaginationItemType::next());
    if (show_last_button) items.push_back(PaginationItemType::last());
    return items;
}

Node Pagination(const PaginationProps& props) {
    int& internal = use_state<int>("Pagination.page", [&] { return props.default_page; });
    const int count = props.count;
    const int page = std::clamp(props.page.value_or(internal), 1, std::max(count, 1));
    const bool controlled = props.page.has_value();
    auto onchange = props.onchange;
    auto go = [&internal, controlled, onchange, count](int p) {
        p = std::clamp(p, 1, std::max(count, 1));
        if (!controlled) internal = p;
        if (onchange) onchange(p);
    };

    Node list = el("ul").cls("skeuo-pagination__list");
    for (const auto& item : pagination_items(count, page, props.siblings_count, props.boundary_count,
                                             props.show_first_button, props.show_last_button,
                                             props.hide_prev_button, props.hide_next_button)) {
        int target = 0;
        bool disabled = props.disabled;
        switch (item.kind) {
            case Kind::Page: target = item.number; break;
            case Kind::First: target = 1; disabled = disabled || page <= 1; break;
            case Kind::Previous: target = std::max(page - 1, 0); disabled = disabled || page <= 1; break;
            case Kind::Next: target = page + 1; disabled = disabled || page >= count; break;
            case Kind::Last: target = count; disabled = disabled || page >= count; break;
            default: break;
        }
        list.child(el("li").child(PaginationItem({.type = item,
                                                  .page = target,
                                                  .selected = item == PaginationItemType::page(page),
                                                  .disabled = disabled,
                                                  .variant = props.variant,
                                                  .shape = props.shape,
                                                  .color = props.color,
                                                  .size = props.size,
                                                  .onclick = [go, target] { go(target); }})));
    }
    return el("nav")
        .cls(classes({"skeuo-pagination", props.cls}))
        .attr("aria-label", props.aria_label)
        .child(std::move(list));
}

Node PaginationItem(const PaginationItemProps& props) {
    std::string variant = "skeuo-pagination-item--" + std::string(variant_name(props.variant));
    std::string shape = "skeuo-pagination-item--" + std::string(shape_name(props.shape));
    std::string size = "skeuo-pagination-item--" + std::string(as_str(props.size));

    if (props.type.kind == Kind::StartEllipsis || props.type.kind == Kind::EndEllipsis) {
        return el("div")
            .cls(classes({"skeuo-pagination-item", "skeuo-pagination-item--ellipsis", size, props.cls}))
            .attr("aria-hidden", "true")
            .child("…");
    }

    bool is_page = props.type.kind == Kind::Page;
    std::string_view material = props.selected ? "skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--down"
                                : props.variant == PaginationVariant::Outlined || !is_page
                                    ? "skeuo-key skeuo-key--pressable"
                                    : "";
    std::string label;
    std::optional<IconName> icon;
    switch (props.type.kind) {
        case Kind::Page:
            label = (props.selected ? "page " : "Go to page ") + std::to_string(props.type.number);
            break;
        case Kind::First: label = "Go to first page"; icon = IconName::FirstPage; break;
        case Kind::Previous: label = "Go to previous page"; icon = IconName::ChevronLeft; break;
        case Kind::Next: label = "Go to next page"; icon = IconName::ChevronRight; break;
        case Kind::Last: label = "Go to last page"; icon = IconName::LastPage; break;
        default: break;
    }
    Node button = el("button")
                      .cls(classes({material, "skeuo-pagination-item", variant, shape, size, color_class(props.color),
                                    props.selected ? "skeuo-pagination-item--selected" : "", props.cls}))
                      .attr("type", "button")
                      .attr("aria-label", label)
                      .attr_if(props.selected, "aria-current", "page")
                      .flag("disabled", props.disabled);
    if (props.onclick) button.on("click", props.onclick);
    if (icon) button.child(Icon(*icon));
    else button.child(std::to_string(props.page));
    return button;
}

}  // namespace skeuo
