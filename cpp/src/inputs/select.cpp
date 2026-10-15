#include <algorithm>

#include "common.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

using namespace inputs_detail;

namespace {
bool contains(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

std::string join(const std::vector<std::string>& v, std::string_view sep) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out += sep;
        out += v[i];
    }
    return out;
}
}  // namespace

Node Select(const SelectProps& props) {
    std::string generated = use_unique_id("skeuo-select");
    std::string id = props.id.value_or(generated);
    std::string label_id = id + "-label";
    std::string listbox_id = id + "-listbox";
    std::string helper_id = id + "-helper";

    auto& internal = use_state<std::optional<std::string>>("Select.value", [&] { return props.default_value; });
    auto& internal_multi = use_state<std::vector<std::string>>(
        "Select.values", [&] { return props.default_values.value_or(std::vector<std::string>{}); });
    auto& internal_open = use_state<bool>("Select.open", [] { return false; });
    auto& active = use_state<std::optional<size_t>>("Select.active", [] { return std::optional<size_t>{}; });

    bool multiple = props.multiple;
    bool disabled = props.disabled;
    std::vector<std::string> selected;
    if (multiple) {
        selected = props.values ? *props.values : internal_multi;
    } else if (auto v = props.value ? props.value : internal) {
        selected.push_back(*v);
    }
    bool open = !disabled && props.open.value_or(internal_open);
    const std::vector<SelectOption>& options = props.options;
    std::vector<bool> disabled_flags;
    for (const auto& o : options) disabled_flags.push_back(o.disabled);

    auto set_open = [&internal_open, onopen = props.onopen, onclose = props.onclose](bool want) {
        internal_open = want;
        if (want && onopen) onopen();
        if (!want && onclose) onclose();
    };
    auto choose = [&internal, &internal_multi, set_open, options, selected, multiple,
                   onchange = props.onchange, onchange_multiple = props.onchange_multiple](size_t i) {
        if (i >= options.size() || options[i].disabled) return;
        const SelectOption& opt = options[i];
        if (multiple) {
            std::vector<std::string> next = selected;
            auto pos = std::find(next.begin(), next.end(), opt.value);
            if (pos != next.end()) {
                next.erase(pos);
            } else {
                next.push_back(opt.value);
            }
            internal_multi = next;
            if (onchange_multiple) onchange_multiple(std::move(next));
        } else {
            internal = opt.value;
            if (onchange) onchange(opt.value);
            set_open(false);
        }
    };

    std::optional<size_t> first_selected;
    for (size_t i = 0; i < options.size(); ++i) {
        if (contains(selected, options[i].value)) {
            first_selected = i;
            break;
        }
    }

    std::vector<std::string> labels;
    for (const auto& o : options)
        if (contains(selected, o.value)) labels.push_back(o.label);
    std::string display = join(labels, ", ");

    std::string extra = classes({open ? "skeuo-select--open" : "", props.cls});
    Node root = el("div").cls(field_classes("skeuo-select", props.variant, props.color, props.size, props.error,
                                            disabled, props.full_width, extra));
    root.attr("style", props.style);
    if (props.label) root.child(field_label(*props.label, props.required).attr("id", label_id));

    Node trigger = el("div")
                       .cls(classes({control_classes(props.variant), "skeuo-select__trigger"}))
                       .attr("id", id)
                       .attr("role", "combobox")
                       .attr("tabindex", disabled ? "-1" : "0")
                       .attr("aria-haspopup", "listbox")
                       .attr("aria-expanded", open ? "true" : "false")
                       .attr("aria-controls", listbox_id)
                       .attr("aria-labelledby", props.label ? label_id + " " + id : id);
    if (open && active) trigger.attr("aria-activedescendant", id + "-opt-" + std::to_string(*active));
    if (props.helper_text) trigger.attr("aria-describedby", helper_id);
    trigger.attr("aria-invalid", props.error ? "true" : "false")
        .attr("aria-required", props.required ? "true" : "false")
        .attr("aria-disabled", disabled ? "true" : "false");
    if (!disabled) {
        trigger.on("click", [&active, set_open, first_selected, open] {
            active = first_selected;
            set_open(!open);
        });
    }
    if (display.empty()) {
        trigger.child(el("span")
                          .cls("skeuo-select__value skeuo-select__placeholder")
                          .child(props.placeholder.value_or("​")));
    } else {
        trigger.child(el("span").cls("skeuo-select__value").child(display));
    }
    trigger.child(Icon({.name = IconName::ChevronDown, .cls = "skeuo-select__chevron"}));

    Node anchor = el("div").cls("skeuo-select__anchor");
    if (!disabled) {
        // The keyboard handler sits on the anchor so it also works while
        // focus is on the list (after a row was clicked).
        anchor.attr("data-sk-keys", open ? "ArrowDown ArrowUp Home End Enter Space Escape"
                                         : "ArrowDown ArrowUp Enter Space");
        anchor.on("keydown", [&active, set_open, choose, disabled_flags, first_selected, open](const Event& e) {
            const std::string& key = e.key;
            if (!open) {
                if (key == "ArrowDown" || key == "ArrowUp" || key == "Enter" || key == " ") {
                    active = first_selected ? first_selected : step_enabled(disabled_flags, std::nullopt, 1);
                    set_open(true);
                }
                return;
            }
            if (key == "ArrowDown") {
                active = step_enabled(disabled_flags, active, 1);
            } else if (key == "ArrowUp") {
                active = step_enabled(disabled_flags, active, -1);
            } else if (key == "Home") {
                active = step_enabled(disabled_flags, std::nullopt, 1);
            } else if (key == "End") {
                active = step_enabled(disabled_flags, std::nullopt, -1);
            } else if (key == "Enter" || key == " ") {
                if (active) choose(*active);
            } else if (key == "Escape") {
                set_open(false);
            }
        });
    }
    if (open) {
        // Focus leaving the field (Tab, a click elsewhere) closes the list.
        anchor.on("blur", [set_open](const Event& e) {
            if (!e.related_inside) set_open(false);
        });
    }
    anchor.child(std::move(trigger));

    if (open) {
        // tabindex=-1: a click on a row moves focus to the list, which is
        // still inside the field, instead of to the page.
        Node listbox = el("ul")
                           .cls("skeuo-select__listbox skeuo-chassis skeuo-chassis--float skeuo-elev-3")
                           .attr("id", listbox_id)
                           .attr("role", "listbox")
                           .attr("aria-labelledby", props.label ? label_id : id)
                           .attr_if(multiple, "aria-multiselectable", "true")
                           .attr("tabindex", "-1");
        for (size_t i = 0; i < options.size(); ++i) {
            const auto& opt = options[i];
            listbox.child(select_row(id + "-opt-" + std::to_string(i), opt.label, contains(selected, opt.value),
                                     active == i, opt.disabled, [choose, i] { choose(i); },
                                     [&active, i] { active = i; }));
        }
        anchor.child(std::move(listbox));
    }
    root.child(std::move(anchor));

    if (props.name)
        root.child(el("input").attr("type", "hidden").attr("name", *props.name).attr("value", join(selected, ",")));
    if (props.helper_text)
        root.child(el("p").cls("skeuo-field__helper skeuo-caption").attr("id", helper_id).child(*props.helper_text));
    return root;
}

Node NativeSelect(const NativeSelectProps& props) {
    std::string generated = use_unique_id("skeuo-native-select");
    std::string id = props.id.value_or(generated);
    std::string helper_id = id + "-helper";
    const auto& default_value =
        use_state<std::optional<std::string>>("NativeSelect.default", [&] { return props.default_value; });
    std::optional<std::string> value = props.value ? props.value : default_value;

    Node root = el("div").cls(field_classes("skeuo-select skeuo-native-select", props.variant, props.color,
                                            props.size, props.error, props.disabled, props.full_width, props.cls));
    root.attr("style", props.style);
    if (props.label) root.child(field_label(*props.label, props.required).attr("for", id));

    Node select = el("select").cls("skeuo-native-select__input").attr("id", id);
    if (props.name) select.attr("name", *props.name);
    select.flag("disabled", props.disabled)
        .flag("required", props.required)
        .attr("aria-invalid", props.error ? "true" : "false");
    if (props.helper_text) select.attr("aria-describedby", helper_id);
    if (props.onchange) select.on("change", [cb = props.onchange](const Event& e) { cb(e.value); });
    for (const auto& opt : props.options) {
        select.child(el("option")
                         .attr("value", opt.value)
                         .flag("disabled", opt.disabled)
                         .flag("selected", value == opt.value)
                         .child(opt.label));
    }

    root.child(el("div")
                   .cls(classes({control_classes(props.variant), "skeuo-select__trigger"}))
                   .child(std::move(select))
                   .child(Icon({.name = IconName::ChevronDown, .cls = "skeuo-select__chevron"})));
    if (props.helper_text)
        root.child(el("p").cls("skeuo-field__helper skeuo-caption").attr("id", helper_id).child(*props.helper_text));
    return root;
}

}  // namespace skeuo
