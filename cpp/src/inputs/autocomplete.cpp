#include <algorithm>
#include <cctype>

#include "common.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

using namespace inputs_detail;

namespace {
std::string lower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\n\r\f\v");
    if (b == std::string::npos) return {};
    size_t e = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(b, e - b + 1);
}

bool contains(const std::vector<std::string>& v, const std::string& x) {
    return std::find(v.begin(), v.end(), x) != v.end();
}

struct AutocompleteState {
    std::optional<std::string> value;
    std::vector<std::string> values;
    std::string input;
    bool dirty = false;
    bool open = false;
    std::optional<size_t> active;
    /// The pills changed: the page morphs children by position, so the
    /// input after them is a new element and must take the focus back.
    bool refocus = false;
};
}  // namespace

Node Autocomplete(const AutocompleteProps& props) {
    std::string generated = use_unique_id("skeuo-autocomplete");
    std::string id = props.id.value_or(generated);
    std::string listbox_id = id + "-listbox";
    std::string helper_id = id + "-helper";

    bool multiple = props.multiple;
    bool free_solo = props.free_solo;
    bool disabled = props.disabled;

    auto& st = use_state<AutocompleteState>("Autocomplete.state", [&] {
        AutocompleteState s;
        s.value = props.default_value;
        s.values = props.default_values.value_or(std::vector<std::string>{});
        if (!multiple) s.input = props.value ? *props.value : props.default_value.value_or("");
        return s;
    });

    std::optional<std::string> value = props.value ? props.value : st.value;
    std::vector<std::string> values = props.values ? *props.values : st.values;
    std::vector<std::string> selected = multiple ? values : std::vector<std::string>{};
    if (!multiple && value) selected.push_back(*value);
    std::string text = props.input_value ? *props.input_value : st.input;

    // A single value's own text shows the full list (as in MUI); any other
    // text filters it.
    std::string query = lower(text);
    bool filtering = multiple || st.dirty || value != text;
    std::vector<std::string> filtered;
    for (const auto& o : props.options)
        if (!filtering || query.empty() || lower(o).find(query) != std::string::npos) filtered.push_back(o);
    std::vector<bool> no_flags(filtered.size(), false);
    bool open = !disabled && props.open.value_or(st.open);
    bool show_list = open && !(free_solo && filtered.empty());

    auto set_text = [&st, oninputchange = props.oninputchange](const std::string& t) {
        st.input = t;
        if (oninputchange) oninputchange(t);
    };
    // Picks (or, with `multiple`, toggles) an option or free text.
    auto pick = [&st, set_text, values, multiple, onchange = props.onchange,
                 onchange_multiple = props.onchange_multiple](const std::string& v) {
        st.dirty = false;
        if (multiple) {
            std::vector<std::string> next = values;
            auto pos = std::find(next.begin(), next.end(), v);
            if (pos != next.end()) {
                next.erase(pos);
            } else {
                next.push_back(v);
            }
            st.values = next;
            st.refocus = true;
            if (onchange_multiple) onchange_multiple(std::move(next));
            set_text("");
        } else {
            st.value = v;
            if (onchange) onchange(v);
            set_text(v);
            st.open = false;
        }
    };
    auto remove = [&st, values, onchange_multiple = props.onchange_multiple](const std::string& v) {
        std::vector<std::string> next;
        for (const auto& x : values)
            if (x != v) next.push_back(x);
        st.values = next;
        st.refocus = true;
        if (onchange_multiple) onchange_multiple(std::move(next));
    };
    auto clear = [&st, set_text, multiple, onchange = props.onchange, onchange_multiple = props.onchange_multiple] {
        st.dirty = false;
        if (multiple) {
            st.values.clear();
            st.refocus = true;
            if (onchange_multiple) onchange_multiple({});
        } else {
            st.value.reset();
            if (onchange) onchange(std::nullopt);
        }
        set_text("");
    };

    std::string extra = classes({open ? "skeuo-select--open" : "",
                                 multiple && !values.empty() ? "skeuo-autocomplete--has-tags" : "", props.cls});
    Node root = el("div").cls(field_classes("skeuo-select skeuo-autocomplete", props.variant, props.color, props.size,
                                            props.error, disabled, props.full_width, extra));
    root.attr("style", props.style);
    if (props.label) root.child(field_label(*props.label, props.required).attr("for", id));

    Node control = el("div").cls(classes({control_classes(props.variant), "skeuo-autocomplete__control"}));
    if (multiple) {
        for (const auto& v : values) {
            Node tag = el("span").cls("skeuo-autocomplete__tag skeuo-key");
            tag.child(el("span").cls("skeuo-autocomplete__tag-label").child(v));
            if (!disabled) {
                tag.child(el("button")
                              .cls("skeuo-autocomplete__tag-remove")
                              .attr("type", "button")
                              .attr("tabindex", "-1")
                              .attr("aria-label", "Remove " + v)
                              .on("click", [remove, v] { remove(v); })
                              .child(Icon(IconName::Close)));
            }
            control.child(std::move(tag));
        }
    }

    Node input = el("input").cls("skeuo-field__input skeuo-autocomplete__input").attr("id", id);
    if (props.name) input.attr("name", *props.name);
    input.attr("type", "text").attr("role", "combobox").attr("autocomplete", "off").attr("value", text);
    if (!(multiple && !values.empty()) && props.placeholder) input.attr("placeholder", *props.placeholder);
    input.flag("disabled", disabled)
        .flag("required", props.required && selected.empty())
        .attr("aria-autocomplete", "list")
        .attr("aria-expanded", show_list ? "true" : "false")
        .attr("aria-controls", listbox_id);
    if (show_list && st.active) input.attr("aria-activedescendant", id + "-opt-" + std::to_string(*st.active));
    input.attr("aria-invalid", props.error ? "true" : "false");
    if (props.helper_text) input.attr("aria-describedby", helper_id);
    if (st.refocus) {
        input.attr("data-sk-autofocus", "");
        st.refocus = false;
    }
    input.on("input", [&st, set_text](const Event& e) {
        st.dirty = true;
        set_text(e.value);
        st.open = true;
        st.active.reset();
    });
    if (!open) input.on("click", [&st] { st.open = true; });
    control.child(std::move(input));

    bool has_value = !selected.empty() || !text.empty();
    if (has_value && !disabled && !props.disable_clearable) {
        control.child(el("button")
                          .cls("skeuo-autocomplete__button skeuo-autocomplete__clear")
                          .attr("type", "button")
                          .attr("tabindex", "-1")
                          .attr("aria-label", "Clear")
                          .on("click", clear)
                          .child(Icon(IconName::Close)));
    }
    if (!free_solo) {
        control.child(el("button")
                          .cls("skeuo-autocomplete__button skeuo-autocomplete__popup")
                          .attr("type", "button")
                          .attr("tabindex", "-1")
                          .flag("disabled", disabled)
                          .attr("aria-label", open ? "Close" : "Open")
                          .on("click",
                              [&st, open] {
                                  st.dirty = false;
                                  st.open = !open;
                              })
                          .child(Icon({.name = IconName::ChevronDown, .cls = "skeuo-select__chevron"})));
    }

    Node anchor = el("div").cls("skeuo-select__anchor");
    if (!disabled) {
        // Keys are handled on the anchor so they also work while focus is on
        // the list or one of the small buttons. Backspace is only taken when
        // it removes a pill.
        bool backspace = multiple && text.empty() && !values.empty();
        anchor.attr("data-sk-keys", backspace ? "ArrowDown ArrowUp Enter Escape Backspace"
                                              : "ArrowDown ArrowUp Enter Escape");
        anchor.on("keydown", [&st, pick, remove, filtered, no_flags, text, values, open, free_solo,
                              multiple](const Event& e) {
            const std::string& key = e.key;
            if (key == "ArrowDown" || key == "ArrowUp") {
                int dir = key == "ArrowDown" ? 1 : -1;
                if (!open) {
                    st.open = true;
                    st.active = step_enabled(no_flags, std::nullopt, dir);
                } else {
                    st.active = step_enabled(no_flags, st.active, dir);
                }
            } else if (key == "Enter") {
                if (open && st.active && *st.active < filtered.size()) {
                    pick(filtered[*st.active]);
                } else if (free_solo && !trim(text).empty()) {
                    pick(trim(text));
                }
            } else if (key == "Escape") {
                st.open = false;
            } else if (key == "Backspace" && multiple && text.empty() && !values.empty()) {
                remove(values.back());
            }
        });
        if (open || st.dirty) {
            // Leaving the field closes the list and settles the text.
            anchor.on("blur", [&st, pick, set_text, value, text, multiple, free_solo](const Event& e) {
                if (e.related_inside) return;
                st.open = false;
                st.active.reset();
                if (multiple || !st.dirty) return;
                st.dirty = false;
                if (free_solo && !trim(text).empty()) {
                    if (value != trim(text)) pick(trim(text));
                } else {
                    set_text(value.value_or(""));
                }
            });
        }
    }
    anchor.child(std::move(control));

    if (show_list) {
        // tabindex=-1: a click on a row moves focus to the list, which is
        // still inside the field, so the field does not see a blur.
        Node listbox = el("ul")
                           .cls("skeuo-select__listbox skeuo-chassis skeuo-chassis--float skeuo-elev-3")
                           .attr("id", listbox_id)
                           .attr("role", "listbox")
                           .attr_if(multiple, "aria-multiselectable", "true")
                           .attr("tabindex", "-1");
        if (filtered.empty())
            listbox.child(el("li").cls("skeuo-select__empty").attr("role", "presentation").child(props.no_options_text));
        for (size_t i = 0; i < filtered.size(); ++i) {
            const std::string& opt = filtered[i];
            listbox.child(select_row(id + "-opt-" + std::to_string(i), opt, contains(selected, opt), st.active == i,
                                     false, [pick, opt] { pick(opt); }, [&st, i] { st.active = i; }));
        }
        anchor.child(std::move(listbox));
    }
    root.child(std::move(anchor));

    if (props.helper_text)
        root.child(el("p").cls("skeuo-field__helper skeuo-caption").attr("id", helper_id).child(*props.helper_text));
    return root;
}

}  // namespace skeuo
