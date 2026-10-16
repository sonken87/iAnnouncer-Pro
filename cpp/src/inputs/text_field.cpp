#include "skeuo/inputs.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace {
std::string_view variant_name(TextFieldVariant v) {
    switch (v) {
        case TextFieldVariant::Outlined: return "outlined";
        case TextFieldVariant::Filled: return "filled";
        case TextFieldVariant::Standard: return "standard";
    }
    return "outlined";
}
}  // namespace

Node TextField(const TextFieldProps& props) {
    std::string generated_id = use_unique_id("skeuo-field");
    std::string input_id = props.id.value_or(generated_id);
    std::string helper_id = input_id + "-helper";
    // `default_value` is read once and rendered through `value`. The rendered
    // value then never changes, so the page never rewrites it and the field
    // stays uncontrolled.
    const auto& default_value =
        use_state<std::optional<std::string>>("TextField.default", [&] { return props.default_value; });
    std::optional<std::string> value = props.value ? props.value : default_value;

    std::string_view control = props.variant == TextFieldVariant::Outlined
                                   ? "skeuo-field__control skeuo-well skeuo-well--focusable"
                               : props.variant == TextFieldVariant::Filled
                                   ? "skeuo-field__control skeuo-well skeuo-well--shallow skeuo-well--focusable"
                                   : "skeuo-field__control";

    Node root = el("div").cls(classes({
        "skeuo-field",
        "skeuo-field--" + std::string(variant_name(props.variant)),
        color_class(props.color),
        "skeuo-field--" + std::string(as_str(props.size)),
        props.error ? "skeuo-field--error" : "",
        props.disabled ? "skeuo-field--disabled" : "",
        props.full_width ? "skeuo-field--full-width" : "",
        props.multiline ? "skeuo-field--multiline" : "",
        props.cls,
    }));
    root.attr("style", props.style);  // always present, as in the Rust markup

    if (props.label) {
        Node label = el("label").cls("skeuo-field__label skeuo-label").attr("for", input_id).child(*props.label);
        if (props.required)
            label.child(el("span").cls("skeuo-field__asterisk").attr("aria-hidden", "true").child(" *"));
        root.child(std::move(label));
    }

    Node input = el(props.multiline ? "textarea" : "input").cls("skeuo-field__input").attr("id", input_id);
    if (props.name) input.attr("name", *props.name);
    if (props.multiline) {
        input.attr("rows", std::to_string(props.rows));
    } else {
        input.attr("type", props.type);
    }
    if (value) {
        // A textarea shows its text content on first load; `value` keeps it
        // in sync afterwards.
        input.attr("value", *value);
        if (props.multiline) input.child(*value);
    }
    if (props.placeholder) input.attr("placeholder", *props.placeholder);
    if (props.autocomplete && !props.multiline) input.attr("autocomplete", *props.autocomplete);
    input.flag("disabled", props.disabled)
        .flag("required", props.required)
        .flag("readonly", props.read_only)
        .flag("autofocus", props.autofocus)
        .attr("aria-invalid", props.error ? "true" : "false");
    if (props.helper_text) input.attr("aria-describedby", helper_id);
    if (props.autofocus) input.attr("data-sk-autofocus", "");

    if (props.oninput) input.on("input", [cb = props.oninput](const Event& e) { cb(e.value); });
    if (props.onchange) input.on("change", [cb = props.onchange](const Event& e) { cb(e.value); });
    if (props.onkeydown) input.on("keydown", props.onkeydown);
    if (props.onfocus) input.on("focus", props.onfocus);
    if (props.onblur) input.on("blur", props.onblur);

    Node well = el("div").cls(control);
    if (props.start_adornment)
        well.child(el("span").cls("skeuo-field__adornment skeuo-field__adornment--start").child(*props.start_adornment));
    well.child(std::move(input));
    if (props.end_adornment)
        well.child(el("span").cls("skeuo-field__adornment skeuo-field__adornment--end").child(*props.end_adornment));
    root.child(std::move(well));

    if (props.helper_text)
        root.child(el("p").cls("skeuo-field__helper skeuo-caption").attr("id", helper_id).child(*props.helper_text));
    return root;
}

}  // namespace skeuo
