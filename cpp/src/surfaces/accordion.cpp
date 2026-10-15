#include "skeuo/runtime.hpp"
#include "skeuo/surfaces.hpp"

namespace skeuo {

namespace {

constexpr const char* kCollapse = "skeuo-accordion__collapse";

/// Wires the summary / details / actions inside one accordion. Nested
/// accordions (already wired by their own Accordion) are left alone.
void wire(Node& node, bool expanded, bool disabled, const std::string& summary_id, const std::string& region_id,
          const std::function<void()>& toggle) {
    if (node.kind() == Node::Kind::Element) {
        if (node.has_class("skeuo-accordion")) return;
        if (node.has_class("skeuo-accordion-summary")) {
            node.set_attr("id", summary_id);
            node.set_attr("aria-expanded", expanded ? "true" : "false");
            node.set_attr("aria-controls", region_id);
            node.flag("disabled", disabled);
            node.on("click", toggle);
        } else if (node.has_class(kCollapse)) {
            if (expanded) node.add_class("skeuo-accordion__collapse--open");
            const std::string* role = node.get_attr("role");
            if (role && *role == "region") {
                node.set_attr("id", region_id);
                node.set_attr("aria-labelledby", summary_id);
            }
        }
    }
    for (auto& kid : node.kids()) wire(kid, expanded, disabled, summary_id, region_id, toggle);
}

}  // namespace

Node Accordion(const AccordionProps& props, Node children) {
    bool& internal = use_state<bool>("Accordion.expanded", [&] { return props.default_expanded; });
    bool expanded = props.expanded.value_or(internal);
    std::string summary_id = use_unique_id("skeuo-accordion-summary");
    std::string region_id = use_unique_id("skeuo-accordion-region");

    std::function<void()> toggle = [&internal, controlled = props.expanded.has_value(), expanded,
                                    disabled = props.disabled, onchange = props.onchange] {
        if (disabled) return;
        bool next = !expanded;
        if (!controlled) internal = next;
        if (onchange) onchange(next);
    };
    wire(children, expanded, props.disabled, summary_id, region_id, toggle);

    return el("div")
        .cls(classes({
            chassis_classes(props.variant, props.elevation, props.square),
            "skeuo-accordion",
            props.square ? "" : "skeuo-accordion--rounded",
            props.disable_gutters ? "" : "skeuo-accordion--gutters",
            expanded ? "skeuo-accordion--expanded" : "",
            props.disabled ? "skeuo-accordion--disabled" : "",
            props.cls,
        }))
        .child(std::move(children));
}

Node AccordionSummary(const AccordionSummaryProps& props, Node children) {
    // id / aria-expanded / aria-controls / disabled and the click handler are
    // filled in by the enclosing Accordion (the attributes are created here so
    // they keep the Rust order).
    Node button = el("button")
                      .cls(classes({"skeuo-accordion-summary", props.cls}))
                      .attr("type", "button")
                      .attr("id", "")
                      .attr("aria-expanded", "false")
                      .attr("aria-controls", "");
    button.child(el("span").cls("skeuo-accordion-summary__content").child(std::move(children)));
    if (props.expand_icon)
        button.child(el("span")
                         .cls("skeuo-accordion-summary__expand-icon")
                         .attr("aria-hidden", "true")
                         .child(*props.expand_icon));
    return el("h3").cls("skeuo-accordion__heading").child(std::move(button));
}

Node AccordionDetails(const AccordionDetailsProps& props, Node children) {
    return el("div")
        .cls(kCollapse)
        .attr("id", "")
        .attr("role", "region")
        .attr("aria-labelledby", "")
        .child(el("div").cls("skeuo-accordion__collapse-inner")
                   .child(el("div").cls(classes({"skeuo-accordion-details", props.cls})).child(std::move(children))));
}

Node AccordionActions(const AccordionActionsProps& props, Node children) {
    std::string cls = classes(
        {"skeuo-accordion-actions", props.disable_spacing ? "" : "skeuo-accordion-actions--spacing", props.cls});
    return el("div").cls(kCollapse).child(
        el("div").cls("skeuo-accordion__collapse-inner").child(el("div").cls(cls).child(std::move(children))));
}

}  // namespace skeuo
