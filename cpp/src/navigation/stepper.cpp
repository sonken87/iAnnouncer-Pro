#include <algorithm>
#include <cmath>

#include "detail.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/navigation.hpp"

namespace skeuo {

namespace {

struct StepState {
    int index = 0;
    bool active = false;
    bool completed = false;
    bool disabled = false;
    bool expanded = false;
    Orientation orientation = Orientation::Horizontal;
};

std::optional<bool> tri(const Node& n, std::string_view name) {
    const std::string* v = n.get_attr(name);
    if (!v) return std::nullopt;
    return *v == "true";
}

Node groove_fill(bool on) {
    return el("span").cls(on ? "skeuo-groove__fill skeuo-step-connector__fill skeuo-step-connector__fill--on"
                             : "skeuo-groove__fill skeuo-step-connector__fill");
}

void fill_label(Node& label, const StepState& s) {
    bool error = nav::has_attr(label, "data-sk-error");
    label.set_attr("class", classes({"skeuo-step-label", s.active ? "skeuo-step-label--active" : "",
                                     s.completed ? "skeuo-step-label--completed" : "",
                                     error ? "skeuo-step-label--error" : "",
                                     s.disabled ? "skeuo-step-label--disabled" : "", nav::attr_or(label, "class")}));
    label.remove_attr("data-sk-step-label");
    label.remove_attr("data-sk-error");
    for (auto& icon : label.kids()) {
        if (!nav::has_attr(icon, "data-sk-step-icon")) continue;
        icon.remove_attr("data-sk-step-icon");
        icon.set_attr("class", classes({"skeuo-step-icon skeuo-key skeuo-key--round",
                                        s.active || s.completed || error ? "skeuo-key--filled" : "",
                                        s.active ? "skeuo-step-icon--active" : "",
                                        error ? "skeuo-color-error skeuo-step-icon--error" : ""}));
        if (nav::has_attr(icon, "data-sk-custom-icon")) {
            icon.remove_attr("data-sk-custom-icon");
        } else if (error) {
            icon.child("!");
        } else if (s.completed) {
            icon.child(Icon(IconName::Check));
        } else {
            icon.child(std::to_string(s.index + 1));
        }
    }
}

/// Fills in the labels, buttons and content inside one step.
void fill_step_parts(Node& n, const StepState& s) {
    nav::visit(n, [&](Node& c) {
        if (c.kind() != Node::Kind::Element) return true;
        if (nav::has_attr(c, "data-sk-step-button")) {
            c.remove_attr("data-sk-step-button");
            c.flag("disabled", s.disabled);
            c.attr_if(s.active, "aria-current", "step");
            return true;
        }
        if (nav::has_attr(c, "data-sk-step-label")) {
            fill_label(c, s);
            return false;
        }
        if (nav::has_attr(c, "data-sk-step-content")) {
            if (s.orientation != Orientation::Vertical) {
                c = Node();
                return false;
            }
            bool open = s.active || s.expanded;
            Node content = el("div").attr("class", nav::attr_or(c, "class"));
            content.child(el("span")
                              .cls("skeuo-step-content__line skeuo-groove")
                              .attr("aria-hidden", "true")
                              .child(groove_fill(s.completed)));
            if (open) content.child(el("div").cls("skeuo-step-content__body").children(std::move(c.kids())));
            c = std::move(content);
            return false;
        }
        return true;
    });
}

}  // namespace

Node Stepper(const StepperProps& props, Node children) {
    std::string orientation = "skeuo-stepper--" + std::string(as_str(props.orientation));
    Node root = el("div").cls(classes({"skeuo-stepper", orientation,
                                       props.alternative_label && props.orientation == Orientation::Horizontal
                                           ? "skeuo-stepper--alternative-label"
                                           : "",
                                       color_class(props.color), props.cls}));
    root.child(std::move(children));

    // Hands out step indices in order, like the Rust version's render order.
    int counter = 0;
    nav::visit(root, [&](Node& n) {
        if (!nav::has_attr(n, "data-sk-step")) return true;
        int auto_index = counter++;
        StepState s;
        s.index = n.get_attr("data-sk-step-index") ? std::stoi(*n.get_attr("data-sk-step-index")) : auto_index;
        s.active = props.active_step == s.index;
        if (!s.active && !props.non_linear) {
            s.completed = props.active_step > s.index;
            s.disabled = props.active_step < s.index;
        }
        s.active = tri(n, "data-sk-step-active").value_or(s.active);
        s.completed = tri(n, "data-sk-step-completed").value_or(s.completed);
        s.disabled = tri(n, "data-sk-step-disabled").value_or(s.disabled);
        s.expanded = nav::has_attr(n, "data-sk-step-expanded");
        s.orientation = props.orientation;

        std::string user = nav::attr_or(n, "class");
        Node step = el("div").cls(classes({"skeuo-step", s.index > 0 ? "skeuo-step--joined" : "",
                                           s.active ? "skeuo-step--active" : "",
                                           s.completed ? "skeuo-step--completed" : "",
                                           s.disabled ? "skeuo-step--disabled" : "", user}));
        if (s.index > 0) {
            step.child(el("span")
                           .cls("skeuo-step-connector skeuo-groove")
                           .attr("aria-hidden", "true")
                           .child(groove_fill(s.active || s.completed)));
        }
        step.children(std::move(n.kids()));
        fill_step_parts(step, s);
        n = std::move(step);
        return false;
    });
    return root;
}

Node Step(const StepProps& props, Node children) {
    Node step = el("div").attr("data-sk-step", "");
    if (!props.cls.empty()) step.attr("class", props.cls);
    if (props.index) step.attr("data-sk-step-index", std::to_string(*props.index));
    if (props.active) step.attr("data-sk-step-active", *props.active ? "true" : "false");
    if (props.completed) step.attr("data-sk-step-completed", *props.completed ? "true" : "false");
    if (props.disabled) step.attr("data-sk-step-disabled", *props.disabled ? "true" : "false");
    step.flag("data-sk-step-expanded", props.expanded);
    return std::move(step).child(std::move(children));
}

Node StepLabel(const StepLabelProps& props, Node children) {
    // Empty class placeholders keep the attribute order (class first) when
    // the Stepper fills them in.
    Node label = el("span").attr("class", props.cls).attr("data-sk-step-label", "").flag("data-sk-error", props.error);
    Node icon = el("span").attr("class", "").attr("aria-hidden", "true").attr("data-sk-step-icon", "");
    if (props.icon) icon.attr("data-sk-custom-icon", "").child(*props.icon);
    Node text = el("span").cls("skeuo-step-label__text");
    text.child(el("span").cls("skeuo-step-label__label").child(std::move(children)));
    if (props.optional) text.child(el("span").cls("skeuo-step-label__optional skeuo-caption").child(*props.optional));
    return std::move(label).child(std::move(icon)).child(std::move(text));
}

Node StepButton(const StepButtonProps& props, Node children) {
    Node button = el("button")
                      .cls(classes({"skeuo-step-button", props.cls}))
                      .attr("type", "button")
                      .attr("data-sk-step-button", "");
    if (props.onclick) button.on("click", props.onclick);
    return std::move(button).child(StepLabel({.optional = props.optional, .icon = props.icon}, std::move(children)));
}

Node StepContent(const StepContentProps& props, Node children) {
    return el("div")
        .cls(classes({"skeuo-step-content", props.cls}))
        .attr("data-sk-step-content", "")
        .child(std::move(children));
}

namespace {
std::string_view mobile_variant_name(MobileStepperVariant v) {
    switch (v) {
        case MobileStepperVariant::Dots: return "dots";
        case MobileStepperVariant::Progress: return "progress";
        case MobileStepperVariant::Text: return "text";
    }
    return "dots";
}
std::string_view mobile_position_name(MobileStepperPosition p) {
    switch (p) {
        case MobileStepperPosition::Bottom: return "bottom";
        case MobileStepperPosition::Top: return "top";
        case MobileStepperPosition::Static: return "static";
    }
    return "bottom";
}
}  // namespace

Node MobileStepper(const MobileStepperProps& props) {
    std::string variant = "skeuo-mobile-stepper--" + std::string(mobile_variant_name(props.variant));
    std::string position = "skeuo-mobile-stepper--" + std::string(mobile_position_name(props.position));
    const int steps = std::max(props.steps, 1);
    const int active = std::clamp(props.active_step, 0, steps - 1);
    const double pct = steps > 1 ? active * 100.0 / (steps - 1) : 100.0;

    Node root = el("div").cls(classes({"skeuo-mobile-stepper skeuo-chassis skeuo-elev-1", variant, position,
                                       color_class(props.color), props.cls}));
    Node back = el("span").cls("skeuo-mobile-stepper__end");
    if (props.back_button) back.child(*props.back_button);
    root.child(std::move(back));
    switch (props.variant) {
        case MobileStepperVariant::Dots: {
            Node dots = el("span")
                            .cls("skeuo-mobile-stepper__dots")
                            .attr("role", "img")
                            .attr("aria-label", "Step " + std::to_string(active + 1) + " of " + std::to_string(steps));
            for (int i = 0; i < steps; ++i)
                dots.child(el("span").cls(i == active ? "skeuo-mobile-stepper__dot skeuo-led"
                                                      : "skeuo-mobile-stepper__dot skeuo-led skeuo-led--off"));
            root.child(std::move(dots));
            break;
        }
        case MobileStepperVariant::Progress:
            root.child(el("span")
                           .cls("skeuo-mobile-stepper__progress skeuo-groove")
                           .attr("role", "progressbar")
                           .attr("aria-valuemin", "0")
                           .attr("aria-valuemax", "100")
                           .attr("aria-valuenow", nav::fmt_num(std::round(pct)))
                           .child(el("span")
                                      .cls("skeuo-groove__fill skeuo-mobile-stepper__fill")
                                      .attr("style", "width: " + nav::fmt_num(pct) + "%")));
            break;
        case MobileStepperVariant::Text:
            root.child(el("span")
                           .cls("skeuo-mobile-stepper__text")
                           .child(std::to_string(active + 1) + " / " + std::to_string(steps)));
            break;
    }
    Node next = el("span").cls("skeuo-mobile-stepper__end");
    if (props.next_button) next.child(*props.next_button);
    root.child(std::move(next));
    return root;
}

}  // namespace skeuo
