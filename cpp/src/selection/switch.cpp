#include "context.hpp"
#include "skeuo/runtime.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

Node Switch(const SwitchProps& props) {
    bool& internal = use_state<bool>("Switch.checked", [&] { return props.default_checked; });
    bool checked = props.checked.value_or(internal);
    Color color = props.color.value_or(Color::Primary);
    Size size = props.size.value_or(Size::Medium);

    unsigned fixed = (props.color ? detail::FcColor : 0) | (props.size ? detail::FcSize : 0);
    Node root = el("span")
                    .cls(classes({"skeuo-switch", color_class(color), "skeuo-switch--" + std::string(as_str(size)),
                                  checked ? "skeuo-switch--checked" : "",
                                  props.disabled ? "skeuo-switch--disabled" : "", props.cls}))
                    .attr("data-sk-fc", detail::fc_marker("switch", fixed));

    root.child(el("span")
                   .cls("skeuo-switch__track skeuo-groove")
                   .attr("aria-hidden", "true")
                   .child(el("span").cls("skeuo-switch__fill skeuo-groove__fill")));
    root.child(el("span").cls("skeuo-switch__thumb skeuo-key skeuo-key--round").attr("aria-hidden", "true"));

    Node input = el("input").cls("skeuo-switch__input").attr("type", "checkbox").attr("role", "switch");
    if (props.id) input.attr("id", *props.id);
    if (props.name) input.attr("name", *props.name);
    if (props.value) input.attr("value", *props.value);
    input.flag("checked", checked).flag("disabled", props.disabled).flag("required", props.required);
    input.attr("aria-checked", checked ? "true" : "false");
    if (props.aria_label) input.attr("aria-label", *props.aria_label);
    if (props.aria_labelledby) input.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_describedby) input.attr("aria-describedby", *props.aria_describedby);
    bool controlled = props.checked.has_value();
    input.on("change", [&internal, controlled, checked, cb = props.onchange](const Event& e) {
        if (!controlled) internal = !checked;
        if (cb) cb(e.checked);
    });
    return std::move(root).child(std::move(input));
}

}  // namespace skeuo
