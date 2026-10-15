// Port of crates/skeuo-ui/tests/selection.rs, plus handler wiring through Runtime.
#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {

size_t count(const std::string& hay, const std::string& needle) {
    size_t n = 0;
    for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + needle.size())) ++n;
    return n;
}

/// Number of tick marks (not mark labels) in the markup.
size_t mark_count(const std::string& html) {
    return count(html, R"(class="skeuo-slider__mark)") - count(html, R"(class="skeuo-slider__mark-label)");
}

void contains_all(const std::string& html, std::initializer_list<const char*> needles) {
    for (const char* n : needles) CONTAINS(html, n);
}

/// The handler id registered for `type` on the first element whose markup
/// contains `marker` (e.g. `value="fm"`), from a Runtime render.
int handler_id(const std::string& html, const std::string& marker, const std::string& type) {
    size_t at = html.find(marker);
    if (at == std::string::npos) return -1;
    size_t open = html.rfind('<', at);
    size_t close = html.find('>', at);
    std::string tag = html.substr(open, close - open);
    size_t on = tag.find("data-sk-on=\"");
    if (on == std::string::npos) return -1;
    std::string spec = tag.substr(on + 12, tag.find('"', on + 12) - on - 12);
    size_t t = spec.find(type + ":");
    if (t == std::string::npos) return -1;
    return std::atoi(spec.c_str() + t + type.size() + 1);
}

std::string event(int id, const std::string& payload) {
    return R"({"t":"event","h":[)" + std::to_string(id) + R"(],"e":)" + payload + "}";
}

}  // namespace

// ------------------------------------------------------------------ Checkbox

TEST(selection_checkbox_default_is_unchecked_white_cap) {
    std::string html = render_static([] { return Checkbox({.aria_label = "Mute"}); });
    contains_all(html, {R"(class="skeuo-checkbox skeuo-color-primary skeuo-checkbox--medium")",
                        "skeuo-checkbox__cap skeuo-key skeuo-key--pressable skeuo-key--focus-within",
                        R"(type="checkbox")", R"(aria-label="Mute")"});
    LACKS(html, "skeuo-key--filled");
    LACKS(html, "checked");
    LACKS(html, "<svg");
}

TEST(selection_checkbox_checked_states_and_props) {
    std::string html = render_static([] {
        return Checkbox({.checked = true, .color = Color::Success, .size = Size::Large, .disabled = true,
                         .required = true, .name = "agree", .value = "yes", .id = "cb", .cls = "mine"});
    });
    contains_all(html, {"skeuo-checkbox--checked", "skeuo-color-success", "skeuo-checkbox--large",
                        "skeuo-checkbox--disabled", "skeuo-key--filled", "mine", R"(aria-disabled="true")",
                        R"(name="agree")", R"(value="yes")", R"(id="cb")", " checked", " disabled", " required",
                        "<svg"});
}

TEST(selection_checkbox_indeterminate_and_uncontrolled_default) {
    std::string html = render_static([] {
        return Node{Checkbox({.indeterminate = true}), Checkbox({.default_checked = true, .cls = "second"})};
    });
    contains_all(html, {"skeuo-checkbox--indeterminate", R"(aria-checked="mixed")", "M5 12h14",
                        "skeuo-checkbox--checked"});
    CHECK(count(html, "skeuo-key--filled") == 2) << html;
}

TEST(selection_checkbox_controlled_value_wins_over_default) {
    std::string html = render_static([] { return Checkbox({.checked = false, .default_checked = true}); });
    LACKS(html, "skeuo-checkbox--checked");
}

TEST(selection_checkbox_uncontrolled_toggles_and_reports) {
    std::vector<bool> got;
    Runtime rt([&] { return Checkbox({.onchange = [&](bool v) { got.push_back(v); }}); });
    std::string html = rt.render();
    LACKS(html, "skeuo-checkbox--checked");
    rt.handle(event(handler_id(html, "skeuo-checkbox__input", "change"), R"({"type":"change","checked":true})"));
    CONTAINS(rt.render(), "skeuo-checkbox--checked");
    CHECK(got.size() == 1 && got[0]);
}

TEST(selection_checkbox_controlled_does_not_toggle_itself) {
    bool on = false;
    int calls = 0;
    Runtime rt([&] { return Checkbox({.checked = on, .onchange = [&](bool) { ++calls; }}); });
    std::string html = rt.render();
    rt.handle(event(handler_id(html, "skeuo-checkbox__input", "change"), R"({"type":"change","checked":true})"));
    CHECK(calls == 1);
    LACKS(rt.render(), "skeuo-checkbox--checked");
}

// ------------------------------------------------------------------ Radio

TEST(selection_radio_group_provides_name_value_and_color) {
    std::string html = render_static([] {
        return RadioGroup({.name = "band", .default_value = "fm", .row = true, .color = Color::Error, .aria_label = "Band"},
                          {Radio({.value = "am"}), Radio({.value = "fm"}), Radio({.value = "dab", .color = Color::Info})});
    });
    contains_all(html, {R"(role="radiogroup")", "skeuo-radio-group--row", R"(aria-label="Band")", "skeuo-color-error",
                        "skeuo-color-info"});
    CHECK(count(html, R"(name="band")") == 3) << html;
    CHECK(count(html, "skeuo-radio--checked") == 1) << html;
    CHECK(count(html, "skeuo-led--off") == 2) << html;
    size_t checked_at = html.find("skeuo-radio--checked");
    size_t fm_at = html.find(R"(value="fm")");
    size_t dab_at = html.find(R"(value="dab")");
    CHECK(checked_at < fm_at && fm_at < dab_at) << html;
    // The selected cap is latched down and its input checked.
    CONTAINS(html, R"(skeuo-key--focus-within skeuo-key--down"><input class="skeuo-radio__input" type="radio" name="band" value="fm" checked>)");
}

TEST(selection_radio_group_controlled_value_and_generated_name) {
    std::string html = render_static([] {
        return RadioGroup({.value = "b", .default_value = "a", .disabled = true},
                          {Radio({.value = "a"}), Radio({.value = "b"})});
    });
    CHECK(count(html, "skeuo-radio--checked") == 1) << html;
    CONTAINS(html, R"(name="skeuo-radio-group-)");
    CHECK(count(html, "skeuo-radio--disabled") == 2) << html;
    CHECK(html.find("skeuo-radio--checked") > html.find(R"(value="a")")) << html;
}

TEST(selection_standalone_radio_is_controlled_by_checked) {
    std::string html = render_static([] { return Radio({.value = "x", .checked = true, .size = Size::Small}); });
    contains_all(html, {"skeuo-radio--checked", "skeuo-radio--small", "skeuo-key--round", "skeuo-key--down",
                        R"(type="radio")", "skeuo-radio__led skeuo-led"});
}

TEST(selection_radio_group_selects_through_labels) {
    std::string picked;
    Runtime rt([&] {
        return RadioGroup({.default_value = "fm", .onchange = [&](const std::string& v) { picked = v; }},
                          {FormControlLabel({.control = Radio({.value = "fm"}), .label = "FM"}),
                           FormControlLabel({.control = Radio({.value = "dab"}), .label = "DAB+"})});
    });
    std::string html = rt.render();
    rt.handle(event(handler_id(html, R"(value="dab")", "change"), R"({"type":"change","checked":true})"));
    CHECK(picked == "dab");
    html = rt.render();
    // Uncontrolled: the group remembers the selection.
    CHECK(html.find("skeuo-radio--checked") > html.find(R"(value="fm")")) << html;
    CONTAINS(html, R"(value="dab" checked)");
}

TEST(selection_radio_group_runs_radio_handler_after_group) {
    std::vector<std::string> order;
    Runtime rt([&] {
        return RadioGroup({.value = "a", .onchange = [&](const std::string& v) { order.push_back("group:" + v); }},
                          {Radio({.value = "a"}), Radio({.value = "b", .onchange = [&](bool) { order.push_back("radio"); }})});
    });
    std::string html = rt.render();
    rt.handle(event(handler_id(html, R"(value="b")", "change"), R"({"type":"change","checked":true})"));
    CHECK(order.size() == 2 && order[0] == "group:b" && order[1] == "radio") << order.size();
    // Controlled: the selection stays with the parent's value.
    CONTAINS(rt.render(), R"(value="a" checked)");
}

// ------------------------------------------------------------------ Switch

TEST(selection_switch_has_switch_role_and_states) {
    std::string html = render_static([] {
        return Node{Switch({.aria_label = "On air"}),
                    Switch({.default_checked = true, .color = Color::Error, .size = Size::Small, .disabled = true})};
    });
    contains_all(html, {R"(role="switch")", R"(aria-label="On air")", R"(aria-checked="false")",
                        R"(aria-checked="true")", "skeuo-switch__track skeuo-groove",
                        "skeuo-switch__fill skeuo-groove__fill", "skeuo-switch__thumb skeuo-key skeuo-key--round",
                        "skeuo-switch--checked", "skeuo-switch--small", "skeuo-switch--disabled", "skeuo-color-error"});
    CHECK(count(html, "skeuo-switch--checked") == 1) << html;
}

TEST(selection_switch_uncontrolled_toggles) {
    bool last = false;
    Runtime rt([&] { return Switch({.default_checked = true, .onchange = [&](bool v) { last = v; }}); });
    std::string html = rt.render();
    CONTAINS(html, "skeuo-switch--checked");
    rt.handle(event(handler_id(html, "skeuo-switch__input", "change"), R"({"type":"change","checked":false})"));
    LACKS(rt.render(), "skeuo-switch--checked");
    CHECK(!last);
}

// ------------------------------------------------------------------ Slider

TEST(selection_slider_single_value_track_and_label) {
    std::string html = render_static([] {
        return Slider({.value = 25.0, .min = 0.0, .max = 200.0, .step = 5.0,
                       .value_label_display = ValueLabelDisplay::On,
                       .value_label_format = [](double v) { return std::to_string(int(v)) + " dB"; },
                       .aria_label = "Volume"});
    });
    contains_all(html, {"skeuo-slider skeuo-color-primary skeuo-slider--medium skeuo-slider--horizontal",
                        R"(type="range")", R"(min="0")", R"(max="200")", R"(step="5")", R"(value="25")",
                        R"(aria-label="Volume")", R"(aria-valuetext="25 dB")", "--a: 0%; --b: 12.5%;", "--p: 12.5%;",
                        "skeuo-slider__value-label--on", ">25 dB<"});
    CHECK(count(html, "skeuo-slider__thumb ") == 1) << html;
}

TEST(selection_slider_range_has_two_thumbs_and_marks) {
    std::string html = render_static([] {
        return Slider({.default_value = std::pair{20.0, 60.0},
                       .marks = {SliderMark::make(0.0, "Low"), SliderMark::tick(40.0), SliderMark::make(100.0, "High")},
                       .color = Color::Secondary,
                       .aria_label = "Band"});
    });
    contains_all(html, {"skeuo-slider--range", "skeuo-slider--labelled", "skeuo-color-secondary", "--a: 20%; --b: 60%;",
                        R"(aria-label="Band start")", R"(aria-label="Band end")",
                        "skeuo-slider__mark skeuo-slider__mark--active", ">Low<", ">High<"});
    CHECK(count(html, R"(type="range")") == 2) << html;
    CHECK(mark_count(html) == 3) << html;
    CHECK(count(html, "skeuo-slider__mark--active") == 1) << html;
    LACKS(html, "skeuo-slider__value-label");
}

TEST(selection_slider_vertical_step_marks_and_disabled) {
    std::string html = render_static([] {
        return Slider({.default_value = 0.5, .min = 0.0, .max = 1.0, .step = 0.25, .marks = true,
                       .orientation = Orientation::Vertical, .track = SliderTrack::Inverted, .disabled = true});
    });
    contains_all(html, {"skeuo-slider--vertical", "skeuo-slider--disabled", R"(aria-orientation="vertical")",
                        R"(aria-valuetext="0.50")", "--a: 50%; --b: 100%;", " disabled"});
    CHECK(mark_count(html) == 5) << html;
}

TEST(selection_slider_value_helpers) {
    CHECK(SliderValue(3.0).single() == 3.0);
    CHECK(SliderValue(std::pair{1.0, 2.0}).range() == std::pair(1.0, 2.0));
    CHECK(SliderValue::Range(1.0, 2.0).is_range());
    CHECK(SliderValue::Single(4.0).range() == std::pair(4.0, 4.0));
    CHECK(!SliderValue(4.0).is_range());
}

TEST(selection_slider_input_reports_doubles_and_keeps_range_ordered) {
    std::vector<SliderValue> got;
    std::optional<SliderValue> committed;
    Runtime rt([&] {
        return Slider({.default_value = std::pair{20.0, 60.0},
                       .onchange = [&](SliderValue v) { got.push_back(v); },
                       .onchange_committed = [&](SliderValue v) { committed = v; }});
    });
    std::string html = rt.render();
    rt.handle(event(handler_id(html, "skeuo-slider__input--0", "input"), R"({"type":"input","value":"35.5"})"));
    CHECK(got.size() == 1 && got[0] == SliderValue::Range(35.5, 60.0));
    html = rt.render();
    CONTAINS(html, "--a: 35.5%; --b: 60%;");
    // The start thumb cannot pass the end thumb.
    rt.handle(event(handler_id(html, "skeuo-slider__input--0", "input"), R"({"type":"input","value":"80"})"));
    CHECK(got.size() == 2 && got[1] == SliderValue::Range(60.0, 60.0));
    html = rt.render();
    rt.handle(event(handler_id(html, "skeuo-slider__input--1", "change"), R"({"type":"change","value":"90"})"));
    CHECK(committed && *committed == SliderValue::Range(60.0, 90.0));
}

// ------------------------------------------------------------------ Rating

TEST(selection_rating_interactive_is_a_radio_group) {
    std::string html = render_static([] { return Rating({.default_value = 3.0, .name = "score", .aria_label = "Score"}); });
    contains_all(html, {R"(role="radiogroup")", R"(aria-label="Score")", R"(aria-label="1 Star")",
                        R"(aria-label="5 Stars")", "skeuo-color-warning"});
    CHECK(count(html, R"(type="radio")") == 5) << html;
    CHECK(count(html, R"(name="score")") == 5) << html;
    CHECK(count(html, "width: 100%;") == 3) << html;
    CHECK(count(html, "width: 0%;") == 2) << html;
    CHECK(count(html, "checked") == 1) << html;
}

TEST(selection_rating_half_precision_and_read_only) {
    std::string html = render_static([] {
        return Node{Rating({.value = 2.5, .max = 4, .precision = 0.5}),
                    Rating({.value = 3.5, .precision = 0.5, .read_only = true})};
    });
    CHECK(count(html, R"(type="radio")") == 8) << html;
    CONTAINS(html, R"(aria-label="2.5 Stars")");
    CONTAINS(html, "width: 50%;");
    contains_all(html, {"skeuo-rating--read-only", R"(role="img")", R"(aria-label="Rated 3.5 out of 5")"});
}

TEST(selection_rating_controlled_empty_and_not_rated_label) {
    std::string html = render_static([] { return Rating({.value = std::optional<double>(), .read_only = true}); });
    CONTAINS(html, R"(aria-label="Not rated, out of 5")");
    CHECK(count(html, "width: 0%;") == 5) << html;
}

TEST(selection_rating_disabled_and_sizes) {
    std::string html = render_static([] { return Rating({.disabled = true, .size = Size::Large}); });
    contains_all(html, {"skeuo-rating--disabled", "skeuo-rating--large", R"(aria-disabled="true")"});
    CHECK(count(html, R"(" disabled)") == 5) << html;
}

TEST(selection_rating_click_sets_and_same_value_clears) {
    std::vector<std::optional<double>> got;
    Runtime rt([&] { return Rating({.default_value = 2.0, .onchange = [&](std::optional<double> v) { got.push_back(v); }}); });
    std::string html = rt.render();
    // A new star: click is a no-op, change commits.
    rt.handle(event(handler_id(html, R"(value="4")", "click"), R"({"type":"click"})"));
    CHECK(got.empty());
    rt.handle(event(handler_id(html, R"(value="4")", "change"), R"({"type":"change","checked":true})"));
    CHECK(got.size() == 1 && got[0] == 4.0);
    html = rt.render();
    CHECK(count(html, "width: 100%;") == 4) << html;
    // Clicking the current value again clears it.
    rt.handle(event(handler_id(html, R"(value="4")", "click"), R"({"type":"click"})"));
    CHECK(got.size() == 2 && !got[1].has_value());
    html = rt.render();
    CHECK(count(html, "width: 0%;") == 5) << html;
    LACKS(html, " checked");
}

TEST(selection_rating_hover_previews) {
    std::vector<std::optional<double>> active;
    Runtime rt([&] {
        return Rating({.default_value = 1.0, .onchange_active = [&](std::optional<double> v) { active.push_back(v); }});
    });
    std::string html = rt.render();
    // The hit label around the 3-star input listens for mouseenter.
    size_t input = html.find(R"(value="3")");
    size_t label = html.rfind("<label", input);
    std::string tag = html.substr(label, html.find('>', label) - label);
    int enter = std::atoi(tag.c_str() + tag.find("mouseenter:") + 11);
    rt.handle(event(enter, R"({"type":"mouseenter"})"));
    html = rt.render();
    CONTAINS(html, "skeuo-rating--hovering");
    CHECK(count(html, "width: 100%;") == 3) << html;
    CONTAINS(html, "skeuo-rating__item skeuo-rating__item--active");
    rt.handle(event(handler_id(html, "skeuo-rating ", "mouseleave"), R"({"type":"mouseleave"})"));
    html = rt.render();
    LACKS(html, "skeuo-rating--hovering");
    CHECK(count(html, "width: 100%;") == 1) << html;
    CHECK(active.size() == 2 && active[0] == 3.0 && !active[1]);
}

// ------------------------------------------------------------------ Form

TEST(selection_form_control_shares_disabled_error_and_size) {
    std::string html = render_static([] {
        return FormControl({.disabled = true, .error = true, .required = true, .size = Size::Small, .fieldset = true},
                           {
                               FormLabel({.legend = true}, "Bulletins"),
                               FormGroup({.row = true},
                                         {FormControlLabel({.control = Checkbox({}), .label = "News"}),
                                          FormControlLabel({.control = Switch({}), .label = "Weather"})}),
                               FormHelperText({}, "Pick one"),
                           });
    });
    contains_all(html, {"<fieldset", "skeuo-form-control--error", "skeuo-form-control--disabled", "<legend",
                        "skeuo-form-label skeuo-label skeuo-form-label--error skeuo-form-label--disabled",
                        "skeuo-form-label__asterisk", "skeuo-form-group skeuo-form-group--row", R"(role="group")",
                        "skeuo-checkbox--small", "skeuo-checkbox--disabled", "skeuo-switch--small",
                        "skeuo-switch--disabled", "skeuo-form-control-label--disabled",
                        "skeuo-form-helper-text skeuo-caption skeuo-form-helper-text--error"});
}

TEST(selection_form_control_label_placement_and_disabled_reaches_control) {
    std::string html = render_static([] {
        return Node{FormControlLabel({.control = Radio({.value = "a", .checked = true}), .label = "Studio A",
                                      .label_placement = LabelPlacement::Top, .disabled = true, .required = true}),
                    FormControlLabel({.control = Checkbox({}), .label = "Plain"})};
    });
    contains_all(html, {"<label", "skeuo-form-control-label--top", "skeuo-form-control-label--end",
                        "skeuo-radio--disabled", "Studio A", "skeuo-form-label__asterisk", " required"});
    CHECK(count(html, "skeuo-form-control-label--disabled") == 1) << html;
    CHECK(count(html, "skeuo-checkbox--disabled") == 0) << html;
}

TEST(selection_form_label_and_helper_text_standalone) {
    std::string html = render_static([] {
        return Node{FormLabel({.id = "lbl", .html_for = "x"}, "Name"),
                    FormHelperText({.id = "help", .error = true}, "Too short")};
    });
    contains_all(html, {R"(<label class="skeuo-form-label skeuo-label" id="lbl" for="x")", R"(id="help")",
                        "skeuo-form-helper-text--error"});
    LACKS(html, "asterisk");
}

TEST(selection_form_control_color_size_and_label_overrides) {
    std::string html = render_static([] {
        return FormControl({.disabled = true, .size = Size::Small, .color = Color::Success}, {
            // The label's own `disabled: false` wins over the FormControl's.
            FormControlLabel({.control = Checkbox({.color = Color::Error}), .label = "Own", .disabled = false}),
            FormControlLabel({.control = Radio({.checked = true}), .label = "Inherited"}),
            Slider({.default_value = 10.0}),
            Rating({.default_value = 2.0}),
        });
    });
    // Explicit color stays; the label's disabled=false shields its checkbox.
    contains_all(html, {"skeuo-checkbox skeuo-color-error skeuo-checkbox--small",
                        "skeuo-radio skeuo-color-success skeuo-radio--small skeuo-radio--checked skeuo-radio--disabled",
                        "skeuo-slider skeuo-color-success skeuo-slider--small skeuo-slider--horizontal skeuo-slider--disabled",
                        "skeuo-rating skeuo-color-warning skeuo-rating--small skeuo-rating--disabled"});
    LACKS(html, "skeuo-checkbox--disabled");
    CHECK(count(html, "skeuo-form-control-label--disabled") == 1) << html;
    LACKS(html, "mouseenter");
}

TEST(selection_form_label_explicit_props_win) {
    std::string html = render_static([] {
        return FormControl({.error = true, .required = true},
                           {FormLabel({.error = false, .required = false}, "Plain"), FormHelperText({.error = false}, "Hint")});
    });
    LACKS(html, "skeuo-form-label--error");
    LACKS(html, "skeuo-form-helper-text--error");
    LACKS(html, "asterisk");
}

TEST(selection_radio_group_inside_disabled_form_control) {
    std::string html = render_static([] {
        return FormControl({.disabled = true, .fieldset = true},
                           RadioGroup({.default_value = "x"},
                                      {FormControlLabel({.control = Radio({.value = "x"}), .label = "Mono"}),
                                       FormControlLabel({.control = Radio({.value = "y"}), .label = "Stereo"})}));
    });
    CHECK(count(html, "skeuo-radio--disabled") == 2) << html;
    CHECK(count(html, R"(aria-disabled="true")") == 2) << html;
    CONTAINS(html, "<fieldset class=\"skeuo-form-control skeuo-form-control--disabled\" disabled>");
    CONTAINS(html, "skeuo-radio skeuo-color-primary skeuo-radio--medium skeuo-radio--checked skeuo-radio--disabled");
}
