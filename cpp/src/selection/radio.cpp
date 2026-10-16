#include "context.hpp"
#include "skeuo/runtime.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

Node Radio(const RadioProps& props) {
    // Rendered as if standalone; an enclosing RadioGroup fills in the name,
    // the selection, its defaults and its change handler (see context.hpp).
    bool checked = props.checked.value_or(false);
    Color color = props.color.value_or(Color::Primary);
    Size size = props.size.value_or(Size::Medium);

    unsigned fixed = (props.color ? detail::FcColor : 0) | (props.size ? detail::FcSize : 0) |
                     (props.checked ? detail::FcChecked : 0) | (props.name ? detail::FcName : 0);
    Node root = el("span")
                    .cls(classes({"skeuo-radio", color_class(color), "skeuo-radio--" + std::string(as_str(size)),
                                  checked ? "skeuo-radio--checked" : "",
                                  props.disabled ? "skeuo-radio--disabled" : "", props.cls}))
                    .attr("data-sk-fc", detail::fc_marker("radio", fixed));

    Node cap = el("span")
                   .cls(checked ? "skeuo-radio__cap skeuo-key skeuo-key--round skeuo-key--pressable skeuo-key--focus-within skeuo-key--down"
                                : "skeuo-radio__cap skeuo-key skeuo-key--round skeuo-key--pressable skeuo-key--focus-within")
                   .attr_if(props.disabled, "aria-disabled", "true");

    Node input = el("input").cls("skeuo-radio__input").attr("type", "radio");
    if (props.id) input.attr("id", *props.id);
    if (props.name) input.attr("name", *props.name);
    input.attr("value", props.value);
    input.flag("checked", checked).flag("disabled", props.disabled).flag("required", props.required);
    if (props.aria_label) input.attr("aria-label", *props.aria_label);
    if (props.onchange) input.on("change", [cb = props.onchange](const Event& e) { cb(e.checked); });

    cap.child(std::move(input));
    cap.child(el("span")
                  .cls(checked ? "skeuo-radio__led skeuo-led" : "skeuo-radio__led skeuo-led skeuo-led--off")
                  .attr("aria-hidden", "true"));
    return std::move(root).child(std::move(cap));
}

Node RadioGroup(const RadioGroupProps& props, Node children) {
    std::string generated = use_unique_id("skeuo-radio-group");
    auto& internal = use_state<std::optional<std::string>>("RadioGroup.value", [&] { return props.default_value; });

    detail::RadioGroupProvision p;
    p.name = props.name.value_or(generated);
    p.value = props.value ? props.value : internal;
    p.color = props.color;
    p.size = props.size;
    p.disabled = props.disabled;
    p.select = [&internal, cb = props.onchange](const std::string& v) {
        internal = v;
        if (cb) cb(v);
    };
    detail::radio_group_provide(children, p);

    Node root = el("div")
                    .cls(classes({"skeuo-radio-group", props.row ? "skeuo-radio-group--row" : "", props.cls}))
                    .attr("role", "radiogroup");
    if (props.aria_labelledby) root.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_label) root.attr("aria-label", *props.aria_label);
    return std::move(root).child(std::move(children));
}

}  // namespace skeuo
