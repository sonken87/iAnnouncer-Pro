#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

TEST(button_default_classes_and_type) {
    std::string html = render_static([] { return Button({}, "Save"); });
    CONTAINS(html, R"(class="skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-btn skeuo-btn--contained skeuo-color-primary skeuo-btn--medium")");
    CONTAINS(html, R"(type="button")");
    CONTAINS(html, "Save");
    LACKS(html, "disabled");
}

TEST(button_props_and_passthrough_attributes) {
    std::string html = render_static([] {
        return Button({.variant = ButtonVariant::Outlined, .color = Color::Error, .size = Size::Small,
                       .disabled = true, .full_width = true, .type = "submit", .start_icon = Node("✕"),
                       .cls = "mine", .attrs = {{"id", "del"}, {"aria-label", "Delete item"}}},
                      "Delete");
    });
    for (const char* needle : {"skeuo-btn--outlined", "skeuo-color-error", "skeuo-btn--small",
                               "skeuo-btn--full-width", "mine", R"(type="submit")", R"(id="del")",
                               R"(aria-label="Delete item")", " disabled", "skeuo-btn__icon--start"})
        CONTAINS(html, needle);
    LACKS(html, "skeuo-btn__icon--end");
}

TEST(button_loading_disables_and_shows_spinner) {
    std::string html = render_static([] { return Button({.loading = true}, "Go"); });
    CONTAINS(html, " disabled");
    CONTAINS(html, R"(aria-busy="true")");
    CONTAINS(html, "skeuo-spinner");
    CONTAINS(html, "skeuo-btn--loading-center");
}

TEST(button_click_reaches_handler) {
    int clicks = 0;
    Runtime rt([&] { return Button({.onclick = [&] { ++clicks; }}, "x"); });
    CONTAINS(rt.render(), R"(data-sk-on="click:0")");
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"click"}})");
    CHECK(clicks == 1);
}

TEST(text_field_links_label_and_helper) {
    std::string html = render_static([] {
        return TextField({.label = "Email", .value = "a@b.c", .helper_text = "We never share it", .required = true,
                          .type = "email", .id = "email"});
    });
    for (const char* needle : {R"(for="email")", R"(id="email")", R"(type="email")", R"(value="a@b.c")",
                               R"(aria-describedby="email-helper")", R"(id="email-helper")", "We never share it",
                               "skeuo-field__asterisk", " required"})
        CONTAINS(html, needle);
}

TEST(text_field_generates_unique_ids) {
    std::string html = render_static([] { return Node{TextField({.label = "A"}), TextField({.label = "B"})}; });
    size_t first = html.find("for=\"");
    size_t second = html.find("for=\"", first + 1);
    CHECK(first != std::string::npos && second != std::string::npos);
    auto id_at = [&](size_t pos) { pos += 5; return html.substr(pos, html.find('"', pos) - pos); };
    CHECK(id_at(first) != id_at(second)) << html;
}

TEST(text_field_variants_error_multiline_adornments) {
    std::string html = render_static([] {
        return TextField({.variant = TextFieldVariant::Filled, .default_value = "notes", .error = true,
                          .multiline = true, .rows = 4, .start_adornment = Node("¥")});
    });
    for (const char* needle : {"skeuo-field--filled", "skeuo-well--shallow", "skeuo-field--error",
                               "skeuo-field--multiline", "<textarea", R"(rows="4")", R"(aria-invalid="true")",
                               "skeuo-field__adornment--start", ">notes</textarea>"})
        CONTAINS(html, needle);
    LACKS(html, "<label");
    LACKS(html, "aria-describedby");
}

TEST(text_field_default_value_is_read_once) {
    std::string initial = "first";
    Runtime rt([&] { return TextField({.default_value = initial}); });
    CONTAINS(rt.render(), R"(value="first")");
    initial = "second";
    CONTAINS(rt.render(), R"(value="first")");
}

TEST(text_field_input_reports_value) {
    std::string got;
    Runtime rt([&] { return TextField({.oninput = [&](const std::string& v) { got = v; }}); });
    rt.render();
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"input","value":"typed"}})");
    CHECK(got == "typed");
}

TEST(inputs_text_field_root_has_style_attribute) {
    std::string html = render_static([] { return TextField({}); });
    CONTAINS(html, R"(skeuo-field--medium" style>)");
    html = render_static([] { return TextField({.style = "width: 10px"}); });
    CONTAINS(html, R"(style="width: 10px")");
}

// ======================================================================
// Helpers for the tests below.

namespace {

size_t count(const std::string& hay, std::string_view needle) {
    size_t n = 0;
    for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + needle.size())) ++n;
    return n;
}

/// The id of the `type` handler on the element whose start tag contains
/// `needle` (the n-th such element), or -1.
int handler(const std::string& html, std::string_view needle, std::string_view type, int nth = 0) {
    size_t pos = html.find(needle);
    for (int i = 0; i < nth && pos != std::string::npos; ++i) pos = html.find(needle, pos + 1);
    if (pos == std::string::npos) return -1;
    size_t start = html.rfind('<', pos);
    size_t end = html.find('>', pos);
    std::string tag = html.substr(start, end - start);
    size_t on = tag.find("data-sk-on=\"");
    if (on == std::string::npos) return -1;
    on += 12;
    std::string spec = tag.substr(on, tag.find('"', on) - on);
    size_t p = 0;
    while (p < spec.size()) {
        size_t comma = spec.find(',', p);
        if (comma == std::string::npos) comma = spec.size();
        std::string part = spec.substr(p, comma - p);
        size_t colon = part.find(':');
        if (part.substr(0, colon) == type) return std::stoi(part.substr(colon + 1));
        p = comma + 1;
    }
    return -1;
}

/// Renders, finds the handler and runs it with `e`; returns the new render.
std::string fire(Runtime& rt, std::string_view needle, std::string_view type, Event e = {}, int nth = 0) {
    std::string html = rt.render();
    int id = handler(html, needle, type, nth);
    CHECK(id >= 0) << "no " << type << " handler on " << needle << " in: " << html;
    if (id < 0) return html;
    e.type = std::string(type);
    rt.dispatch({id}, e);
    return rt.render();
}

Event key(std::string k) {
    Event e;
    e.key = std::move(k);
    return e;
}

Event input(std::string v) {
    Event e;
    e.value = std::move(v);
    return e;
}

std::vector<SelectOption> station_options() {
    return {{"a", "Alpha"}, {"b", "Bravo"}, {.value = "c", .label = "Charlie", .disabled = true}};
}

std::vector<std::string> shows() { return {"Breakfast", "Drive", "Jazz", "News"}; }

}  // namespace

// ---------------------------------------------------------------- Button

TEST(inputs_button_loading_start_replaces_start_icon) {
    std::string html = render_static([] {
        return Button({.loading = true, .loading_position = LoadingPosition::Start,
                       .start_icon = el("span").cls("my-icon")},
                      "Save");
    });
    CONTAINS(html, "skeuo-btn--loading-start");
    CONTAINS(html, "skeuo-btn__icon--start skeuo-btn__loader");
    LACKS(html, "my-icon");
    std::string idle = render_static([] { return Button({}, "Save"); });
    LACKS(idle, "loading");
    LACKS(idle, "aria-busy");
}

// ---------------------------------------------------------------- ButtonGroup

TEST(inputs_button_group_hands_props_to_buttons) {
    std::string html = render_static([] {
        return ButtonGroup({.variant = ButtonVariant::Outlined, .orientation = Orientation::Vertical,
                            .color = Color::Error, .size = Size::Small, .disabled = true,
                            .attrs = {{"aria-label", "Edit"}}},
                           {Button({}, "One"), Button({.cls = "mine"}, "Two")});
    });
    for (const char* needle : {R"(role="group")", R"(aria-orientation="vertical")", R"(aria-label="Edit")",
                               "skeuo-btn-group--outlined", "skeuo-btn-group--vertical", "skeuo-btn-group--disabled"})
        CONTAINS(html, needle);
    CHECK(count(html, "skeuo-btn--outlined") == 2) << html;
    CHECK(count(html, "skeuo-btn--small") == 2) << html;
    CHECK(count(html, "skeuo-color-error") == 3) << html;
    CHECK(count(html, " disabled") == 2) << html;
    LACKS(html, "skeuo-key--filled");
    LACKS(html, "skeuo-btn--medium");
    // Classes in the Button's own order, extra classes kept at the end.
    CONTAINS(html, R"(class="skeuo-key skeuo-key--pressable skeuo-btn skeuo-btn--outlined skeuo-color-error skeuo-btn--small mine")");
}

TEST(inputs_button_group_full_width_and_nested_groups) {
    std::string html = render_static([] {
        return ButtonGroup({.variant = ButtonVariant::Text, .full_width = true},
                           {Button({}, "Outer"), ButtonGroup({.color = Color::Success}, Button({}, "Inner"))});
    });
    CONTAINS(html, "skeuo-btn-group--full-width");
    CONTAINS(html, R"(class="skeuo-btn skeuo-btn--text skeuo-color-primary skeuo-btn--medium skeuo-btn--full-width")");
    // The inner group's button keeps the inner group's look.
    CONTAINS(html, R"(class="skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-btn skeuo-btn--contained skeuo-color-success skeuo-btn--medium")");
}

TEST(inputs_button_group_buttons_keep_their_click_handlers) {
    std::string clicked;
    Runtime rt([&] {
        return ButtonGroup({.variant = ButtonVariant::Outlined},
                           {Button({.onclick = [&] { clicked = "A"; }}, "A"),
                            Button({.onclick = [&] { clicked = "B"; }}, "B")});
    });
    fire(rt, "skeuo-btn--outlined", "click", {}, 1);
    CHECK(clicked == "B");
}

// ------------------------------------------------------- IconButton / Fab

TEST(inputs_icon_button_variants_edge_and_label) {
    std::string html = render_static([] {
        return Node{
            IconButton({.edge = IconButtonEdge::End, .attrs = {{"aria-label", "Delete"}}}, Icon(IconName::Delete)),
            IconButton({.variant = IconButtonVariant::Contained, .color = Color::Success, .size = Size::Large,
                        .round = true, .disabled = true, .attrs = {{"aria-label", "Add"}}},
                       Icon(IconName::Add)),
            IconButton({.round = true}, "x"),
        };
    });
    for (const char* needle :
         {R"(class="skeuo-icon-btn skeuo-icon-btn--text skeuo-color-neutral skeuo-icon-btn--medium skeuo-icon-btn--edge-end")",
          R"(aria-label="Delete")",
          R"(class="skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-icon-btn skeuo-icon-btn--contained skeuo-color-success skeuo-icon-btn--large skeuo-key--round")",
          R"(aria-label="Add")", " disabled", R"(type="button")"})
        CONTAINS(html, needle);
    // A text icon button is never round.
    CHECK(count(html, "skeuo-key--round") == 1) << html;
}

TEST(inputs_icon_button_and_fab_clicks) {
    int n = 0;
    Runtime rt([&] { return Node{IconButton({.onclick = [&] { ++n; }}, "i"), Fab({.onclick = [&] { n += 10; }}, "f")}; });
    fire(rt, "skeuo-icon-btn", "click");
    fire(rt, "skeuo-fab", "click");
    CHECK(n == 11);
}

TEST(inputs_fab_is_a_round_key) {
    std::string html = render_static([] {
        return Node{Fab({.color = Color::Primary, .attrs = {{"aria-label", "Add"}}}, Icon(IconName::Add)),
                    Fab({.variant = FabVariant::Extended, .size = Size::Small}, "Go")};
    });
    CONTAINS(html, "skeuo-key skeuo-key--pressable skeuo-key--round skeuo-key--filled skeuo-fab skeuo-fab--circular skeuo-color-primary skeuo-fab--large");
    CONTAINS(html, "skeuo-fab--extended skeuo-color-neutral skeuo-fab--small");
    CONTAINS(html, R"(type="button")");
    // A neutral (default) FAB is a white key.
    CHECK(count(html, "skeuo-key--filled") == 1) << html;
}

// ---------------------------------------------------------- ToggleButton

TEST(inputs_toggle_button_standalone_selected) {
    std::string html = render_static([] {
        return Node{ToggleButton({.value = "bold", .selected = true, .color = Color::Secondary}, "B"),
                    ToggleButton({.value = "italic"}, "I")};
    });
    for (const char* needle : {"skeuo-key--down skeuo-toggle--selected", R"(aria-pressed="true")",
                               R"(aria-pressed="false")", R"(value="bold")", "skeuo-color-secondary"})
        CONTAINS(html, needle);
    CHECK(count(html, "skeuo-key--down") == 1) << html;
}

TEST(inputs_toggle_button_standalone_onchange) {
    std::string got;
    int clicks = 0;
    Runtime rt([&] {
        return ToggleButton(
            {.value = "star", .onchange = [&](const std::string& v) { got = v; }, .onclick = [&] { ++clicks; }}, "*");
    });
    fire(rt, "skeuo-toggle", "click");
    CHECK(got == "star");
    CHECK(clicks == 1);
}

TEST(inputs_toggle_group_controlled_and_uncontrolled_selection) {
    std::string html = render_static([] {
        return ToggleButtonGroup({.value = std::vector<std::string>{"b", "c"}, .size = Size::Large},
                                 {ToggleButton({.value = "a"}, "A"), ToggleButton({.value = "b"}, "B"),
                                  ToggleButton({.value = "c"}, "C")});
    });
    CHECK(count(html, "skeuo-toggle--selected") == 2) << html;
    CHECK(count(html, "skeuo-toggle--large") == 3) << html;
    CONTAINS(html, R"(role="group")");
    CONTAINS(html, "skeuo-toggle-group");

    html = render_static([] {
        return ToggleButtonGroup({.default_value = std::vector<std::string>{"y"}, .exclusive = true, .disabled = true},
                                 {ToggleButton({.value = "x"}, "X"), ToggleButton({.value = "y"}, "Y")});
    });
    CHECK(count(html, "skeuo-toggle--selected") == 1) << html;
    CHECK(count(html, " disabled") == 2) << html;
    size_t sel = html.find("skeuo-toggle--selected");
    CHECK(sel > html.find(">X<") && sel < html.find(">Y<")) << html;
}

TEST(inputs_toggle_group_exclusive_click_selects_and_releases) {
    std::vector<std::string> last;
    int calls = 0;
    Runtime rt([&] {
        return ToggleButtonGroup({.default_value = std::vector<std::string>{"x"},
                                  .exclusive = true,
                                  .onchange = [&](std::vector<std::string> v) { last = std::move(v); ++calls; }},
                                 {ToggleButton({.value = "x"}, "X"), ToggleButton({.value = "y"}, "Y")});
    });
    std::string html = fire(rt, R"(value="y")", "click");
    CHECK((last == std::vector<std::string>{"y"}));
    size_t sel = html.find("skeuo-toggle--selected");
    CHECK(sel > html.find(">X<") && sel < html.find(">Y<")) << html;
    html = fire(rt, R"(value="y")", "click");
    CHECK(last.empty());
    LACKS(html, "skeuo-toggle--selected");
    CHECK(calls == 2);
}

TEST(inputs_toggle_group_multiple_runs_button_onchange_too) {
    std::vector<std::string> sel = {"b"};
    std::string button_got;
    Runtime rt([&] {
        return ToggleButtonGroup(
            {.value = sel, .onchange = [&](std::vector<std::string> v) { sel = std::move(v); }},
            {ToggleButton({.value = "a", .onchange = [&](const std::string& v) { button_got = v; }}, "A"),
             ToggleButton({.value = "b"}, "B")});
    });
    fire(rt, R"(value="a")", "click");
    CHECK((sel == std::vector<std::string>{"b", "a"}));
    CHECK(button_got == "a");
    std::string html = fire(rt, R"(value="b")", "click");
    CHECK((sel == std::vector<std::string>{"a"}));
    CHECK(count(html, "skeuo-toggle--selected") == 1) << html;
}

// ---------------------------------------------------------------- Select

TEST(inputs_select_closed_shows_value_and_combobox_aria) {
    std::string html = render_static([] {
        return Select({.label = "Station", .options = station_options(), .default_value = "b",
                       .helper_text = "Pick one", .name = "station", .id = "st"});
    });
    for (const char* needle :
         {R"(role="combobox")", R"(aria-haspopup="listbox")", R"(aria-expanded="false")",
          R"(aria-controls="st-listbox")", R"(aria-labelledby="st-label st")", R"(aria-describedby="st-helper")",
          R"(id="st-label")", "skeuo-well skeuo-well--focusable skeuo-select__trigger", ">Bravo<",
          R"(type="hidden")", R"(name="station")", R"(value="b")", "skeuo-select__chevron"})
        CONTAINS(html, needle);
    LACKS(html, R"(role="listbox")");
}

TEST(inputs_select_open_lists_options_with_selection_and_disabled) {
    std::string html = render_static([] {
        return Select({.options = station_options(), .value = "a", .open = true,
                       .variant = TextFieldVariant::Filled, .id = "s"});
    });
    for (const char* needle :
         {"skeuo-select--open", "skeuo-well--shallow", R"(aria-expanded="true")", R"(role="listbox")",
          "skeuo-chassis skeuo-chassis--float skeuo-elev-3", R"(id="s-opt-0")", R"(aria-selected="true")",
          R"(aria-selected="false")", R"(aria-disabled="true")", "skeuo-select__option--selected",
          "skeuo-select__option--disabled"})
        CONTAINS(html, needle);
    CHECK(count(html, R"(role="option")") == 3) << html;
    CHECK(count(html, "skeuo-select__option--selected") == 1) << html;
}

TEST(inputs_select_multiple_placeholder_error_disabled) {
    std::string html = render_static([] {
        return Node{
            Select({.options = station_options(), .multiple = true,
                    .default_values = std::vector<std::string>{"a", "b"}, .open = true}),
            Select({.options = station_options(), .open = true, .disabled = true, .error = true,
                    .placeholder = "Choose"}),
        };
    });
    for (const char* needle : {"Alpha, Bravo", R"(aria-multiselectable="true")", "skeuo-select__placeholder",
                               "Choose", "skeuo-field--error", "skeuo-field--disabled", R"(tabindex="-1")"})
        CONTAINS(html, needle);
    // The disabled select stays closed even with `open`.
    CHECK(count(html, R"(role="listbox")") == 1) << html;
    CHECK(count(html, "skeuo-select__option--selected") == 2) << html;
}

TEST(inputs_native_select_marks_selected_option) {
    std::string html = render_static([] {
        return NativeSelect({.label = "Minutes", .options = station_options(), .default_value = "b",
                             .variant = TextFieldVariant::Standard, .id = "n"});
    });
    for (const char* needle : {"<select", R"(for="n")", R"(id="n")", "skeuo-native-select", "skeuo-field--standard",
                               "skeuo-select__chevron"})
        CONTAINS(html, needle);
    CONTAINS(html, R"(<option value="b" selected>Bravo</option>)");
    CONTAINS(html, R"(<option value="c" disabled>Charlie</option>)");
}

TEST(inputs_native_select_change_reports_value) {
    std::string got;
    Runtime rt([&] {
        return NativeSelect({.options = station_options(), .onchange = [&](const std::string& v) { got = v; }});
    });
    fire(rt, "<select", "change", input("b"));
    CHECK(got == "b");
}

TEST(inputs_select_click_opens_and_option_click_chooses) {
    std::string value = "a";
    int opens = 0, closes = 0;
    Runtime rt([&] {
        return Select({.options = station_options(),
                       .value = value,
                       .onchange = [&](const std::string& v) { value = v; },
                       .onopen = [&] { ++opens; },
                       .onclose = [&] { ++closes; },
                       .id = "s"});
    });
    std::string html = fire(rt, "skeuo-select__trigger", "click");
    CONTAINS(html, R"(role="listbox")");
    CONTAINS(html, R"(aria-activedescendant="s-opt-0")");
    CHECK(opens == 1);
    // Disabled rows have no click handler.
    CHECK(handler(html, R"(id="s-opt-2")", "click") == -1);
    html = fire(rt, R"(id="s-opt-1")", "click");
    CHECK(value == "b");
    CHECK(closes == 1);
    LACKS(html, R"(role="listbox")");
    CONTAINS(html, ">Bravo<");
}

TEST(inputs_select_keyboard_open_move_choose_escape) {
    std::string value;
    Runtime rt([&] {
        return Select({.options = {{"a", "Alpha"}, {.value = "b", .label = "Bravo", .disabled = true}, {"c", "Charlie"}},
                       .onchange = [&](const std::string& v) { value = v; },
                       .id = "k"});
    });
    std::string html = rt.render();
    CONTAINS(html, R"(data-sk-keys="ArrowDown ArrowUp Enter Space")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-0")");
    CONTAINS(html, R"(data-sk-keys="ArrowDown ArrowUp Home End Enter Space Escape")");
    // ArrowDown skips the disabled row and stops at the last one.
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-2")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-2")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Home"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-0")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("End"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-2")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Enter"));
    CHECK(value == "c");
    LACKS(html, R"(role="listbox")");
    CONTAINS(html, ">Charlie<");
    // Space opens on the chosen row; Escape closes without choosing.
    html = fire(rt, "skeuo-select__anchor", "keydown", key(" "));
    CONTAINS(html, R"(aria-activedescendant="k-opt-2")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowUp"));
    CONTAINS(html, R"(aria-activedescendant="k-opt-0")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Escape"));
    LACKS(html, R"(role="listbox")");
    CHECK(value == "c");
}

TEST(inputs_select_blur_closes_unless_focus_stays_inside) {
    Runtime rt([&] { return Select({.options = station_options()}); });
    std::string html = fire(rt, "skeuo-select__trigger", "click");
    CONTAINS(html, R"(role="listbox")");
    Event inside;
    inside.related_inside = true;
    html = fire(rt, "skeuo-select__anchor", "blur", inside);
    CONTAINS(html, R"(role="listbox")");
    html = fire(rt, "skeuo-select__anchor", "blur");
    LACKS(html, R"(role="listbox")");
    // Closed, it does not listen for blur at all.
    CHECK(handler(html, "skeuo-select__anchor", "blur") == -1);
}

TEST(inputs_select_multiple_toggles_and_stays_open) {
    std::vector<std::string> got;
    Runtime rt([&] {
        return Select({.options = station_options(),
                       .multiple = true,
                       .default_values = std::vector<std::string>{"a"},
                       .onchange_multiple = [&](std::vector<std::string> v) { got = std::move(v); },
                       .id = "m"});
    });
    fire(rt, "skeuo-select__trigger", "click");
    std::string html = fire(rt, R"(id="m-opt-1")", "click");
    CHECK((got == std::vector<std::string>{"a", "b"}));
    CONTAINS(html, R"(role="listbox")");
    CONTAINS(html, "Alpha, Bravo");
    fire(rt, R"(id="m-opt-0")", "click");
    CHECK((got == std::vector<std::string>{"b"}));
}

TEST(inputs_select_hover_sets_active_row) {
    Runtime rt([&] { return Select({.options = station_options(), .open = true, .id = "h"}); });
    std::string html = fire(rt, R"(id="h-opt-1")", "mouseenter");
    CONTAINS(html, "skeuo-select__option skeuo-select__option--active");
    CONTAINS(html, R"(aria-activedescendant="h-opt-1")");
}

// ---------------------------------------------------------- Autocomplete

TEST(inputs_autocomplete_single_value_and_aria) {
    std::string html = render_static([] {
        return Autocomplete({.options = shows(), .default_value = "Drive", .label = "Show", .id = "ac"});
    });
    for (const char* needle :
         {R"(for="ac")", R"(role="combobox")", R"(aria-autocomplete="list")", R"(aria-expanded="false")",
          R"(aria-controls="ac-listbox")", R"(value="Drive")", R"(autocomplete="off")", "skeuo-autocomplete__clear",
          "skeuo-autocomplete__popup"})
        CONTAINS(html, needle);
    LACKS(html, R"(role="listbox")");
}

TEST(inputs_autocomplete_filters_by_input_and_shows_no_options) {
    std::string html =
        render_static([] { return Autocomplete({.options = shows(), .input_value = "a", .open = true}); });
    CHECK(count(html, R"(role="option")") == 2) << html;
    CONTAINS(html, ">Breakfast<");
    CONTAINS(html, ">Jazz<");
    html = render_static([] {
        return Autocomplete({.options = shows(), .input_value = "zzz", .open = true, .no_options_text = "Nothing"});
    });
    CONTAINS(html, "skeuo-select__empty");
    CONTAINS(html, "Nothing");
    LACKS(html, R"(role="option")");
    // Free solo shows no empty list.
    html = render_static(
        [] { return Autocomplete({.options = shows(), .input_value = "zzz", .open = true, .free_solo = true}); });
    LACKS(html, "skeuo-select__empty");
    LACKS(html, "skeuo-autocomplete__popup");
}

TEST(inputs_autocomplete_multiple_renders_tag_pills) {
    std::string html = render_static([] {
        return Autocomplete({.options = shows(),
                             .multiple = true,
                             .values = std::vector<std::string>{"Jazz", "News"},
                             .open = true,
                             .placeholder = "Add"});
    });
    CHECK(count(html, "skeuo-autocomplete__tag skeuo-key") == 2) << html;
    for (const char* needle :
         {"skeuo-autocomplete--has-tags", R"(aria-label="Remove Jazz")", R"(aria-multiselectable="true")"})
        CONTAINS(html, needle);
    CHECK(count(html, "skeuo-select__option--selected") == 2) << html;
    LACKS(html, R"(placeholder="Add")");
    std::string disabled = render_static([] {
        return Autocomplete({.options = shows(), .multiple = true,
                             .default_values = std::vector<std::string>{"Jazz"}, .disabled = true});
    });
    LACKS(disabled, "skeuo-autocomplete__tag-remove");
    LACKS(disabled, "skeuo-autocomplete__clear");
}

TEST(inputs_autocomplete_typing_filters_and_enter_picks) {
    std::optional<std::string> value;
    Runtime rt([&] {
        return Autocomplete(
            {.options = shows(), .onchange = [&](std::optional<std::string> v) { value = v; }, .id = "t"});
    });
    std::string html = fire(rt, "skeuo-autocomplete__input", "input", input("a"));
    CONTAINS(html, R"(aria-expanded="true")");
    CHECK(count(html, R"(role="option")") == 2) << html;
    fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    CONTAINS(html, R"(aria-activedescendant="t-opt-1")");
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Enter"));
    CHECK(value == std::optional<std::string>("Jazz"));
    CONTAINS(html, R"(value="Jazz")");
    LACKS(html, R"(role="listbox")");
    // The value's own text shows every option again.
    html = fire(rt, "skeuo-select__anchor", "keydown", key("ArrowDown"));
    CHECK(count(html, R"(role="option")") == 4) << html;
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Escape"));
    LACKS(html, R"(role="listbox")");
    html = fire(rt, "skeuo-autocomplete__clear", "click");
    CHECK(!value.has_value());
    CONTAINS(html, R"(autocomplete="off" value aria)");
}

TEST(inputs_autocomplete_blur_restores_or_keeps_text) {
    Runtime rt([&] { return Autocomplete({.options = shows(), .default_value = "Drive"}); });
    std::string html = fire(rt, "skeuo-autocomplete__input", "input", input("Dr"));
    CONTAINS(html, R"(role="listbox")");
    Event inside;
    inside.related_inside = true;
    html = fire(rt, "skeuo-select__anchor", "blur", inside);
    CONTAINS(html, R"(role="listbox")");
    html = fire(rt, "skeuo-select__anchor", "blur");
    LACKS(html, R"(role="listbox")");
    CONTAINS(html, R"(value="Drive")");

    std::optional<std::string> got;
    Runtime free([&] {
        return Autocomplete(
            {.options = shows(), .onchange = [&](std::optional<std::string> v) { got = v; }, .free_solo = true});
    });
    fire(free, "skeuo-autocomplete__input", "input", input(" My show "));
    fire(free, "skeuo-select__anchor", "blur");
    CHECK(got == std::optional<std::string>("My show"));
}

TEST(inputs_autocomplete_multiple_pick_remove_backspace) {
    std::vector<std::string> values = {"News"};
    Runtime rt([&] {
        return Autocomplete({.options = shows(),
                             .multiple = true,
                             .values = values,
                             .onchange_multiple = [&](std::vector<std::string> v) { values = std::move(v); },
                             .id = "mu"});
    });
    fire(rt, "skeuo-autocomplete__input", "input", input("j"));
    std::string html = fire(rt, R"(id="mu-opt-0")", "click");
    CHECK((values == std::vector<std::string>{"News", "Jazz"}));
    // The list stays open and the text is cleared.
    CONTAINS(html, R"(role="listbox")");
    CONTAINS(html, R"(autocomplete="off" value aria)");
    CONTAINS(html, "ArrowDown ArrowUp Enter Escape Backspace");
    fire(rt, R"(aria-label="Remove News")", "click");
    CHECK((values == std::vector<std::string>{"Jazz"}));
    html = fire(rt, "skeuo-select__anchor", "keydown", key("Backspace"));
    CHECK(values.empty());
    LACKS(html, "skeuo-autocomplete__tag ");
    LACKS(html, "Backspace\"");
}

TEST(inputs_autocomplete_input_refocuses_once_after_pills_change) {
    Runtime rt([&] { return Autocomplete({.options = shows(), .multiple = true, .id = "rf"}); });
    LACKS(rt.render(), "data-sk-autofocus");
    fire(rt, "skeuo-autocomplete__input", "input", input("n"));
    std::string html = fire(rt, R"(id="rf-opt-0")", "click");
    CONTAINS(html, "data-sk-autofocus");
    LACKS(rt.render(), "data-sk-autofocus");
}

TEST(inputs_autocomplete_free_solo_enter_keeps_text) {
    std::optional<std::string> got;
    Runtime rt([&] {
        return Autocomplete(
            {.options = shows(), .onchange = [&](std::optional<std::string> v) { got = v; }, .free_solo = true});
    });
    fire(rt, "skeuo-autocomplete__input", "input", input("Late show"));
    fire(rt, "skeuo-select__anchor", "keydown", key("Enter"));
    CHECK(got == std::optional<std::string>("Late show"));
}

// ---------------------------------------------------------- TransferList

TEST(inputs_transfer_list_renders_both_wells_and_checked_rows) {
    std::string html = render_static([] {
        return TransferList({.default_left = std::vector<std::string>{"a", "b"},
                             .default_right = std::vector<std::string>{"c"},
                             .default_checked = {"b"},
                             .left_title = "From",
                             .right_title = "To"});
    });
    CHECK(count(html, "skeuo-transfer__list skeuo-well") == 2) << html;
    CHECK(count(html, R"(type="checkbox")") == 5) << html;
    CHECK(count(html, "skeuo-transfer__row--checked") == 1) << html;
    for (const char* needle : {"From", "To", "1/2 selected", "0/1 selected", R"(aria-checked="mixed")",
                               R"(aria-label="Move selected right")", R"(aria-label="Move all left")"})
        CONTAINS(html, needle);
    // Nothing is checked on the right, so "move selected left" is disabled.
    size_t btn = html.find(R"(aria-label="Move selected left")");
    size_t tag = html.rfind("<button", btn);
    CHECK(html.substr(tag, btn - tag).find(" disabled") != std::string::npos) << html;
}

TEST(inputs_transfer_list_controlled_lists_win) {
    std::string html = render_static([] {
        return TransferList({.left = std::vector<std::string>{"x"},
                             .right = std::vector<std::string>{},
                             .default_left = std::vector<std::string>{"ignored"}});
    });
    CONTAINS(html, ">x<");
    LACKS(html, "ignored");
    CONTAINS(html, "Empty");
}

TEST(inputs_transfer_list_check_and_move) {
    std::vector<std::string> l, r;
    Runtime rt([&] {
        return TransferList({.default_left = std::vector<std::string>{"a", "b", "c"},
                             .default_right = std::vector<std::string>{"d"},
                             .onchange = [&](std::vector<std::string> nl, std::vector<std::string> nr) {
                                 l = std::move(nl);
                                 r = std::move(nr);
                             }});
    });
    // Check "b" (its checkbox is labelled by the row's span).
    std::string html = fire(rt, "-left-1-label\"", "change");
    CONTAINS(html, "1/3 selected");
    html = fire(rt, R"(aria-label="Move selected right")", "click");
    CHECK((l == std::vector<std::string>{"a", "c"}));
    CHECK((r == std::vector<std::string>{"d", "b"}));
    CHECK(count(html, "0/2 selected") == 2) << html;
    // Select all on the right, then move them back.
    html = fire(rt, R"(aria-label="Select all Chosen")", "change");
    CONTAINS(html, "2/2 selected");
    fire(rt, R"(aria-label="Move selected left")", "click");
    CHECK((l == std::vector<std::string>{"a", "c", "d", "b"}));
    CHECK(r.empty());
    fire(rt, R"(aria-label="Move all right")", "click");
    CHECK(l.empty());
    CHECK(r.size() == 4);
}

// ------------------------------------------------------ TextareaAutosize

TEST(inputs_textarea_autosize_rows_follow_content) {
    std::string html = render_static([] {
        return Node{TextareaAutosize({.min_rows = 2, .aria_label = "Empty"}),
                    TextareaAutosize({.max_rows = 3, .default_value = "1\n2\n3\n4\n5"}),
                    TextareaAutosize({.value = "a\nb\nc", .default_value = "ignored"})};
    });
    for (const char* needle : {"skeuo-textarea skeuo-well skeuo-well--focusable", R"(rows="2")", R"(rows="3")",
                               "--ta-min-rows: 2;", "--ta-max-rows: 3;", "skeuo-textarea--bounded",
                               R"(aria-label="Empty")", "1\n2\n3\n4\n5"})
        CONTAINS(html, needle);
    CHECK(count(html, R"(rows="3")") == 2) << html;
    LACKS(html, "ignored");
}

TEST(inputs_textarea_autosize_grows_while_typing) {
    std::string got;
    Runtime rt([&] { return TextareaAutosize({.max_rows = 4, .oninput = [&](const std::string& v) { got = v; }}); });
    CONTAINS(rt.render(), R"(rows="1")");
    std::string html = fire(rt, "<textarea", "input", input("one\ntwo\nthree"));
    CONTAINS(html, R"(rows="3")");
    CHECK(got == "one\ntwo\nthree");
    html = fire(rt, "<textarea", "input", input("1\n2\n3\n4\n5\n6"));
    CONTAINS(html, R"(rows="4")");
}
