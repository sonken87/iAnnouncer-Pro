#include "context.hpp"
#include "skeuo/selection.hpp"

namespace skeuo {

namespace {
std::string_view placement_name(LabelPlacement p) {
    switch (p) {
        case LabelPlacement::End: return "end";
        case LabelPlacement::Start: return "start";
        case LabelPlacement::Top: return "top";
        case LabelPlacement::Bottom: return "bottom";
    }
    return "end";
}

Node asterisk() { return el("span").cls("skeuo-form-label__asterisk").attr("aria-hidden", "true").child(" *"); }
}  // namespace

Node FormControl(const FormControlProps& props, Node children) {
    // The nearest provider wins in Rust, so every field is claimed here.
    detail::fc_provide(children, {
                                     .disabled = props.disabled,
                                     .claim_disabled = true,
                                     .error = props.error,
                                     .claim_error = true,
                                     .required = props.required,
                                     .claim_required = true,
                                     .color = props.color,
                                     .claim_color = true,
                                     .size = props.size,
                                     .claim_size = true,
                                 });
    std::string cls = classes({
        "skeuo-form-control",
        props.error ? "skeuo-form-control--error" : "",
        props.disabled ? "skeuo-form-control--disabled" : "",
        props.full_width ? "skeuo-form-control--full-width" : "",
        props.cls,
    });
    if (props.fieldset) return el("fieldset").cls(cls).flag("disabled", props.disabled).child(std::move(children));
    return el("div").cls(cls).child(std::move(children));
}

Node FormLabel(const FormLabelProps& props, Node children) {
    bool error = props.error.value_or(false);
    bool disabled = props.disabled.value_or(false);
    bool required = props.required.value_or(false);
    unsigned fixed = (props.error ? detail::FcError : 0) | (props.disabled ? detail::FcDisabled : 0) |
                     (props.required ? detail::FcRequired : 0);
    Node label = el(props.legend ? "legend" : "label")
                     .cls(classes({"skeuo-form-label", "skeuo-label", error ? "skeuo-form-label--error" : "",
                                   disabled ? "skeuo-form-label--disabled" : "", props.cls}));
    if (props.id) label.attr("id", *props.id);
    if (!props.legend && props.html_for) label.attr("for", *props.html_for);
    label.attr("data-sk-fc", detail::fc_marker("form-label", fixed));
    label.child(std::move(children));
    if (required) label.child(asterisk());
    return label;
}

Node FormControlLabel(const FormControlLabelProps& props) {
    // As in MUI, only `disabled` flows down from the FormControl; the
    // asterisk is shown when this label itself is `required`.
    bool disabled = props.disabled.value_or(false);
    bool required = props.required.value_or(false);
    Node control = props.control;
    detail::fc_provide(control, {
                                    .disabled = props.disabled,
                                    .claim_disabled = props.disabled.has_value(),
                                    .error = std::nullopt,
                                    .claim_error = false,
                                    .required = required ? std::optional<bool>(true) : std::nullopt,
                                    .claim_required = false,
                                    .color = std::nullopt,
                                    .claim_color = false,
                                    .size = std::nullopt,
                                    .claim_size = false,
                                });

    Node root = el("label")
                    .cls(classes({"skeuo-form-control-label",
                                  "skeuo-form-control-label--" + std::string(placement_name(props.label_placement)),
                                  disabled ? "skeuo-form-control-label--disabled" : "", props.cls}))
                    .attr("data-sk-fc", detail::fc_marker("form-control-label",
                                                          props.disabled ? detail::FcDisabled : 0u));
    root.child(std::move(control));
    Node text = el("span").cls("skeuo-form-control-label__label").child(props.label);
    if (required) text.child(asterisk());
    return std::move(root).child(std::move(text));
}

Node FormGroup(const FormGroupProps& props, Node children) {
    Node root = el("div")
                    .cls(classes({"skeuo-form-group", props.row ? "skeuo-form-group--row" : "", props.cls}))
                    .attr("role", "group");
    if (props.aria_labelledby) root.attr("aria-labelledby", *props.aria_labelledby);
    if (props.aria_label) root.attr("aria-label", *props.aria_label);
    return std::move(root).child(std::move(children));
}

Node FormHelperText(const FormHelperTextProps& props, Node children) {
    bool error = props.error.value_or(false);
    bool disabled = props.disabled.value_or(false);
    unsigned fixed = (props.error ? detail::FcError : 0) | (props.disabled ? detail::FcDisabled : 0);
    Node p = el("p").cls(classes({"skeuo-form-helper-text", "skeuo-caption",
                                  error ? "skeuo-form-helper-text--error" : "",
                                  disabled ? "skeuo-form-helper-text--disabled" : "", props.cls}));
    if (props.id) p.attr("id", *props.id);
    p.attr("data-sk-fc", detail::fc_marker("helper-text", fixed));
    return std::move(p).child(std::move(children));
}

}  // namespace skeuo
