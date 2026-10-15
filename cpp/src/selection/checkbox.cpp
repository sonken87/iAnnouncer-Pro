#include "context.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

Node Checkbox(const CheckboxProps& props) {
    bool& internal = use_state<bool>("Checkbox.checked", [&] { return props.default_checked; });
    bool checked = props.checked.value_or(internal);
    bool indeterminate = props.indeterminate;
    bool lit = checked || indeterminate;
    Color color = props.color.value_or(Color::Primary);
    Size size = props.size.value_or(Size::Medium);

    // Disabled / required / color / size may still come from an enclosing
    // FormControl or FormControlLabel (see context.hpp).
    unsigned fixed = (props.color ? detail::FcColor : 0) | (props.size ? detail::FcSize : 0);
    Node root = el("span")
                    .cls(classes({"skeuo-checkbox", color_class(color), "skeuo-checkbox--" + std::string(as_str(size)),
                                  checked ? "skeuo-checkbox--checked" : "",
                                  indeterminate ? "skeuo-checkbox--indeterminate" : "",
                                  props.disabled ? "skeuo-checkbox--disabled" : "", props.cls}))
                    .attr("data-sk-fc", detail::fc_marker("checkbox", fixed));

    Node cap = el("span")
                   .cls(classes({"skeuo-checkbox__cap skeuo-key skeuo-key--pressable skeuo-key--focus-within",
                                 lit ? "skeuo-key--filled" : ""}))
                   .attr_if(props.disabled, "aria-disabled", "true");

    Node input = el("input").cls("skeuo-checkbox__input").attr("type", "checkbox");
    if (props.id) input.attr("id", *props.id);
    if (props.name) input.attr("name", *props.name);
    if (props.value) input.attr("value", *props.value);
    input.flag("checked", checked).flag("disabled", props.disabled).flag("required", props.required);
    input.attr_if(indeterminate, "aria-checked", "mixed");
    if (props.aria_label) input.attr("aria-label", *props.aria_label);
    if (props.aria_labelledby) input.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_describedby) input.attr("aria-describedby", *props.aria_describedby);
    bool controlled = props.checked.has_value();
    input.on("change", [&internal, controlled, checked, cb = props.onchange](const Event& e) {
        if (!controlled) internal = !checked;
        if (cb) cb(e.checked);
    });

    cap.child(std::move(input));
    if (indeterminate) cap.child(Icon({.name = IconName::Remove, .cls = "skeuo-checkbox__mark"}));
    else if (checked) cap.child(Icon({.name = IconName::Check, .cls = "skeuo-checkbox__mark"}));
    return std::move(root).child(std::move(cap));
}

}  // namespace skeuo
