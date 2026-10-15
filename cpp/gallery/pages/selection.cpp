// Selection & forms page. Mirrors crates/gallery/src/sections/selection.rs.
#include <array>
#include <cstdio>

#include "../gallery.hpp"

namespace gallery {

namespace {

std::string fixed0(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.0f", v);
    return buf;
}

/// Rust's `{}` for f64 (no trailing ".0").
std::string num(double v) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.10g", v);
    return buf;
}

/// A small form that uses every control together.
Node BroadcastSettingsDemo() {
    bool& on_air = use_state<bool>("sel.on_air", [] { return true; });
    std::string& output = use_state<std::string>("sel.output", [] { return std::string("fm"); });
    double& volume = use_state<double>("sel.volume", [] { return 72.0; });
    auto& window = use_state<std::pair<double, double>>("sel.window", [] { return std::pair{20.0, 65.0}; });
    bool& duck = use_state<bool>("sel.duck", [] { return true; });
    bool& normalise = use_state<bool>("sel.normalise", [] { return false; });
    bool& chime = use_state<bool>("sel.chime", [] { return true; });
    auto& quality = use_state<std::optional<double>>("sel.quality", [] { return std::optional<double>(4.0); });

    int enabled = int(duck) + int(normalise) + int(chime);
    bool all = enabled == 3;
    bool some = enabled > 0 && !all;
    auto [from, to] = window;
    std::string summary = std::string(on_air ? "On air" : "Off air") + " · " +
                          (output == "fm" ? "FM 101.7" : output == "dab" ? "DAB+" : "Web stream") + " · volume " +
                          fixed0(volume) + "% · ducking " + fixed0(from) + "–" + fixed0(to) + "%";

    Node left = el("div").cls("selection-demo__col").child({
        FormControl({.fieldset = true}, {
            FormLabel({.legend = true}, "Transmitter"),
            FormControlLabel({
                .control = Switch({.checked = on_air, .color = Color::Error,
                                   .onchange = [&on_air](bool) { on_air = !on_air; }}),
                .label = on_air ? "On air" : "Off air",
            }),
            FormHelperText({}, on_air ? "Listeners hear everything from now on." : "Rehearsal: nothing leaves the studio."),
        }),
        FormControl({.fieldset = true}, {
            FormLabel({.id = "bs-output", .legend = true}, "Output"),
            RadioGroup({.value = output, .row = true, .aria_labelledby = "bs-output",
                        .onchange = [&output](const std::string& v) { output = v; }},
                       {
                           FormControlLabel({.control = Radio({.value = "fm"}), .label = "FM"}),
                           FormControlLabel({.control = Radio({.value = "dab"}), .label = "DAB+"}),
                           FormControlLabel({.control = Radio({.value = "stream"}), .label = "Stream"}),
                       }),
        }),
        FormControl({.fieldset = true}, {
            FormLabel({.id = "bs-voice", .legend = true}, "Voice quality"),
            Rating({.value = quality, .aria_labelledby = "bs-voice",
                    .onchange = [&quality](std::optional<double> v) { quality = v; }}),
        }),
    });

    Node right = el("div").cls("selection-demo__col").child({
        FormControl({.full_width = true}, {
            FormLabel({.id = "bs-volume"}, "Master volume"),
            Slider({.value = volume,
                    .value_label_display = ValueLabelDisplay::Auto,
                    .value_label_format = [](double v) { return fixed0(v) + "%"; },
                    .aria_labelledby = "bs-volume",
                    .onchange = [&volume](SliderValue v) { volume = v.single(); }}),
        }),
        FormControl({.full_width = true}, {
            FormLabel({.id = "bs-window"}, "Ducking window"),
            Slider({.value = SliderValue::Range(from, to),
                    .step = 5.0,
                    .marks = {SliderMark::make(0.0, "0%"), SliderMark::make(50.0, "50%"), SliderMark::make(100.0, "100%")},
                    .color = Color::Secondary,
                    .aria_label = "Ducking window",
                    .onchange = [&window](SliderValue v) { window = v.range(); }}),
        }),
        FormControl({.error = enabled == 0, .fieldset = true}, {
            FormLabel({.legend = true}, "Processing"),
            FormControlLabel({
                .control = Checkbox({.checked = all, .indeterminate = some,
                                     .onchange = [&duck, &normalise, &chime, all](bool) {
                                         duck = normalise = chime = !all;
                                     }}),
                .label = "All processing",
            }),
            FormGroup({.cls = "selection-demo__nested"}, {
                FormControlLabel({.control = Checkbox({.checked = duck, .onchange = [&duck](bool) { duck = !duck; }}),
                                  .label = "Duck music under voice"}),
                FormControlLabel({.control = Checkbox({.checked = normalise,
                                                       .onchange = [&normalise](bool) { normalise = !normalise; }}),
                                  .label = "Normalise loudness"}),
                FormControlLabel({.control = Checkbox({.checked = chime, .onchange = [&chime](bool) { chime = !chime; }}),
                                  .label = "Play chime before announcements"}),
            }),
            FormHelperText({}, enabled == 0 ? std::string("Pick at least one option.")
                                            : std::to_string(enabled) + " of 3 enabled"),
        }),
    });

    return Demo("Broadcast settings",
                "Every control on one plate, wired with FormControl / FormLabel / FormControlLabel / FormGroup / "
                "FormHelperText",
                {
                    el("div").cls("selection-demo__form").child(std::move(left)).child(std::move(right)),
                    el("p").cls("skeuo-caption selection-demo__summary").child(summary),
                });
}

Node CheckboxDemo() {
    bool& agreed = use_state<bool>("sel.agreed", [] { return false; });
    Node colors;
    for (Color c : kAllColors)
        colors.add_child(Checkbox({.default_checked = true, .color = c, .aria_label = std::string(as_str(c))}));
    return Demo(
        "Checkbox", "square white keycap · checked = colored cap with a check legend · indeterminate = dash",
        {
            DemoRow("colors", std::move(colors)),
            DemoRow("states", {
                Checkbox({.aria_label = "Unchecked"}),
                Checkbox({.default_checked = true, .aria_label = "Checked"}),
                Checkbox({.indeterminate = true, .aria_label = "Indeterminate"}),
                Checkbox({.disabled = true, .aria_label = "Disabled"}),
                Checkbox({.default_checked = true, .disabled = true, .aria_label = "Disabled checked"}),
                Checkbox({.indeterminate = true, .disabled = true, .aria_label = "Disabled indeterminate"}),
            }),
            DemoRow("sizes", {
                Checkbox({.default_checked = true, .size = Size::Small, .aria_label = "Small"}),
                Checkbox({.default_checked = true, .size = Size::Medium, .aria_label = "Medium"}),
                Checkbox({.default_checked = true, .size = Size::Large, .aria_label = "Large"}),
                Checkbox({.size = Size::Small, .aria_label = "Small"}),
                Checkbox({.size = Size::Medium, .aria_label = "Medium"}),
                Checkbox({.size = Size::Large, .aria_label = "Large"}),
            }),
            DemoRow("labelled", {
                FormControlLabel({.control = Checkbox({.default_checked = true}), .label = "End"}),
                FormControlLabel({.control = Checkbox({}), .label = "Start", .label_placement = LabelPlacement::Start}),
                FormControlLabel({.control = Checkbox({}), .label = "Top", .label_placement = LabelPlacement::Top}),
                FormControlLabel({.control = Checkbox({.default_checked = true}), .label = "Bottom",
                                  .label_placement = LabelPlacement::Bottom}),
                FormControlLabel({.control = Checkbox({}), .label = "Disabled", .disabled = true}),
                FormControlLabel({.control = Checkbox({}), .label = "Required", .required = true}),
            }),
            DemoRow("controlled", {
                FormControlLabel({.control = Checkbox({.checked = agreed, .color = Color::Success,
                                                       .onchange = [&agreed](bool v) { agreed = v; }}),
                                  .label = "I have read the station rules"}),
                el("span").cls("skeuo-caption").child(agreed ? "Thanks!" : "Not yet agreed"),
            }),
        });
}

Node RadioDemo() {
    std::string& mode = use_state<std::string>("sel.mode", [] { return std::string("live"); });
    Node colors;
    for (Color c : kAllColors)
        colors.add_child(Radio({.value = std::string(as_str(c)), .checked = true, .color = c,
                                .aria_label = std::string(as_str(c))}));
    return Demo(
        "Radio & RadioGroup", "small round keycap · selected = latched down with a lit LED in the cap",
        {
            DemoRow("colors", std::move(colors)),
            DemoRow("states", {
                Radio({.value = "a", .checked = false, .aria_label = "Unselected"}),
                Radio({.value = "b", .checked = true, .aria_label = "Selected"}),
                Radio({.value = "c", .checked = false, .disabled = true, .aria_label = "Disabled"}),
                Radio({.value = "d", .checked = true, .disabled = true, .aria_label = "Disabled selected"}),
            }),
            DemoRow("sizes", RadioGroup({.default_value = "m", .row = true, .aria_label = "Sizes"}, {
                                            Radio({.value = "s", .size = Size::Small, .aria_label = "Small"}),
                                            Radio({.value = "m", .size = Size::Medium, .aria_label = "Medium"}),
                                            Radio({.value = "l", .size = Size::Large, .aria_label = "Large"}),
                                        })),
            DemoGrid({
                FormControl({.fieldset = true}, {
                    FormLabel({.id = "radio-mode", .legend = true}, "Mode (controlled)"),
                    RadioGroup({.value = mode, .aria_labelledby = "radio-mode",
                                .onchange = [&mode](const std::string& v) { mode = v; }},
                               {
                                   FormControlLabel({.control = Radio({.value = "live"}), .label = "Live"}),
                                   FormControlLabel({.control = Radio({.value = "auto"}), .label = "Automation"}),
                                   FormControlLabel({.control = Radio({.value = "off"}), .label = "Off"}),
                               }),
                    FormHelperText({}, "Selected: " + mode),
                }),
                FormControl({.fieldset = true}, {
                    FormLabel({.id = "radio-color", .legend = true}, "Group color (uncontrolled)"),
                    RadioGroup({.default_value = "b", .color = Color::Success, .aria_labelledby = "radio-color"}, {
                        FormControlLabel({.control = Radio({.value = "a"}), .label = "Studio A"}),
                        FormControlLabel({.control = Radio({.value = "b"}), .label = "Studio B"}),
                        FormControlLabel({.control = Radio({.value = "c"}), .label = "Outside broadcast", .disabled = true}),
                    }),
                }),
                FormControl({.disabled = true, .fieldset = true}, {
                    FormLabel({.id = "radio-disabled", .legend = true}, "Disabled group"),
                    RadioGroup({.default_value = "x", .aria_labelledby = "radio-disabled"}, {
                        FormControlLabel({.control = Radio({.value = "x"}), .label = "Mono"}),
                        FormControlLabel({.control = Radio({.value = "y"}), .label = "Stereo"}),
                    }),
                }),
            }),
        });
}

Node SwitchDemo() {
    bool& mic = use_state<bool>("sel.mic", [] { return false; });
    Node colors;
    for (Color c : kAllColors)
        colors.add_child(Switch({.default_checked = true, .color = c, .aria_label = std::string(as_str(c))}));
    return Demo(
        "Switch", "groove with a sliding round keycap thumb · on = the groove lights in the palette color",
        {
            DemoRow("colors", std::move(colors)),
            DemoRow("states", {
                Switch({.aria_label = "Off"}),
                Switch({.default_checked = true, .aria_label = "On"}),
                Switch({.disabled = true, .aria_label = "Disabled off"}),
                Switch({.default_checked = true, .disabled = true, .aria_label = "Disabled on"}),
            }),
            DemoRow("sizes", {
                Switch({.default_checked = true, .size = Size::Small, .aria_label = "Small"}),
                Switch({.default_checked = true, .size = Size::Medium, .aria_label = "Medium"}),
                Switch({.default_checked = true, .size = Size::Large, .aria_label = "Large"}),
                Switch({.size = Size::Small, .aria_label = "Small off"}),
                Switch({.size = Size::Medium, .aria_label = "Medium off"}),
                Switch({.size = Size::Large, .aria_label = "Large off"}),
            }),
            DemoRow("labelled", FormGroup({.row = true}, {
                FormControlLabel({.control = Switch({.checked = mic, .color = Color::Success,
                                                     .onchange = [&mic](bool) { mic = !mic; }}),
                                  .label = mic ? "Mic live" : "Mic muted"}),
                FormControlLabel({.control = Switch({.default_checked = true}), .label = "Talkback"}),
                FormControlLabel({.control = Switch({}), .label = "Start", .label_placement = LabelPlacement::Start}),
                FormControlLabel({.control = Switch({}), .label = "Disabled", .disabled = true}),
            })),
        });
}

Node SliderDemo() {
    double& gain = use_state<double>("sel.gain", [] { return 30.0; });
    auto& faders = use_state<std::array<double, 4>>("sel.faders", [] { return std::array<double, 4>{80.0, 55.0, 35.0, 65.0}; });
    std::vector<SliderMark> channel_marks = {SliderMark::make(0.0, "−∞"), SliderMark::make(50.0, "−12"),
                                             SliderMark::make(100.0, "0 dB")};

    Node colors;
    size_t ci = 0;
    for (Color c : kAllColors) {
        colors.add_child(Slider({.default_value = 30.0 + 8.0 * double(ci++), .color = c, .size = Size::Small,
                                 .aria_label = std::string(as_str(c)), .cls = "selection-demo__slider--seventh"}));
    }

    Node stack = el("div").cls("selection-demo__stack").child({
        DemoRow("continuous", {
            Slider({.value = gain, .aria_label = "Gain", .cls = "selection-demo__slider",
                    .onchange = [&gain](SliderValue v) { gain = v.single(); }}),
            el("span").cls("skeuo-caption").child(fixed0(gain)),
        }),
        DemoRow("value label", Slider({.default_value = 64.0, .value_label_display = ValueLabelDisplay::On,
                                       .aria_label = "Level",
                                       .cls = "selection-demo__slider selection-demo__slider--label"})),
        DemoRow("steps", Slider({.default_value = 40.0, .step = 10.0, .marks = true, .color = Color::Info,
                                 .aria_label = "Steps", .cls = "selection-demo__slider"})),
        DemoRow("labels", Slider({.default_value = 20.0,
                                  .min = 0.0,
                                  .max = 40.0,
                                  .step = 0.5,
                                  .marks = {SliderMark::make(0.0, "0°C"), SliderMark::make(20.0, "20°C"),
                                            SliderMark::make(37.0, "37°C")},
                                  .value_label_display = ValueLabelDisplay::Auto,
                                  .value_label_format = [](double v) { return num(v) + "°C"; },
                                  .color = Color::Warning,
                                  .aria_label = "Temperature",
                                  .cls = "selection-demo__slider"})),
        DemoRow("range", Slider({.default_value = std::pair{25.0, 70.0}, .value_label_display = ValueLabelDisplay::Auto,
                                 .color = Color::Secondary, .aria_label = "Frequency band",
                                 .cls = "selection-demo__slider"})),
        DemoRow("inverted", Slider({.default_value = 35.0, .track = SliderTrack::Inverted, .color = Color::Error,
                                    .aria_label = "Inverted", .cls = "selection-demo__slider"})),
        DemoRow("sizes", {
            Slider({.default_value = 30.0, .size = Size::Small, .aria_label = "Small", .cls = "selection-demo__slider--third"}),
            Slider({.default_value = 50.0, .aria_label = "Medium", .cls = "selection-demo__slider--third"}),
            Slider({.default_value = 70.0, .size = Size::Large, .aria_label = "Large", .cls = "selection-demo__slider--third"}),
        }),
        DemoRow("colors", std::move(colors)),
        DemoRow("disabled", Slider({.default_value = 40.0, .disabled = true, .aria_label = "Disabled",
                                    .cls = "selection-demo__slider"})),
    });

    Node mixer = el("div").cls("selection-demo__mixer skeuo-chassis skeuo-elev-1");
    const std::pair<const char*, Color> channels[] = {
        {"Mic 1", Color::Error}, {"Mic 2", Color::Warning}, {"Music", Color::Primary}, {"Master", Color::Neutral}};
    for (size_t i = 0; i < 4; ++i) {
        auto [name, color] = channels[i];
        SliderMarks marks = i == 0 ? SliderMarks(channel_marks)
                                   : SliderMarks({SliderMark::tick(0.0), SliderMark::tick(50.0), SliderMark::tick(100.0)});
        mixer.child(el("div").cls("selection-demo__channel").child({
            Slider({.value = faders[i],
                    .marks = std::move(marks),
                    .value_label_display = ValueLabelDisplay::Auto,
                    .orientation = Orientation::Vertical,
                    .color = color,
                    .aria_label = name,
                    .onchange = [&faders, i](SliderValue v) { faders[i] = v.single(); }}),
            el("span").cls("skeuo-label").child(name),
        }));
    }

    return Demo("Slider",
                "groove rail with a lit fill · round keycap thumbs · value label as a floating chassis bubble",
                el("div").cls("selection-demo__sliders").child(std::move(stack)).child(std::move(mixer)));
}

Node RatingDemo() {
    auto& score = use_state<std::optional<double>>("sel.score", [] { return std::optional<double>(3.5); });
    auto& hover = use_state<std::optional<double>>("sel.hover", [] { return std::optional<double>(); });
    auto words = [](double v) {
        if (v <= 1.0) return "Useless";
        if (v <= 2.0) return "Poor";
        if (v <= 3.0) return "OK";
        if (v <= 4.0) return "Good";
        return "Excellent";
    };
    std::optional<double> shown = hover ? hover : score;
    return Demo(
        "Rating", "stars printed on the aluminium · filled in warning ink, empty as outlines · hover previews",
        {
            DemoRow("controlled", {
                Rating({.value = score,
                        .precision = 0.5,
                        .aria_label = "Show rating",
                        .onchange = [&score](std::optional<double> v) { score = v; },
                        .onchange_active = [&hover](std::optional<double> v) { hover = v; }}),
                el("span").cls("skeuo-caption").child(shown ? num(*shown) + " · " + words(*shown) : "No rating"),
            }),
            DemoRow("uncontrolled", {
                Rating({.default_value = 2.0, .aria_label = "Default"}),
                Rating({.default_value = 7.0, .max = 10, .size = Size::Small, .aria_label = "Ten"}),
            }),
            DemoRow("read only", {
                Rating({.value = 4.5, .precision = 0.5, .read_only = true}),
                Rating({.value = 3.0, .read_only = true}),
                Rating({.value = std::optional<double>(), .read_only = true}),
            }),
            DemoRow("sizes", {
                Rating({.default_value = 3.0, .size = Size::Small, .aria_label = "Small"}),
                Rating({.default_value = 3.0, .aria_label = "Medium"}),
                Rating({.default_value = 3.0, .size = Size::Large, .aria_label = "Large"}),
            }),
            DemoRow("disabled", Rating({.default_value = 2.5, .precision = 0.5, .disabled = true, .aria_label = "Disabled"})),
        });
}

/// Layout for this page's demos only.
const char* const kDemoCss = R"(
.selection-demo__form { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 18px 40px; }
.selection-demo__col { display: flex; flex-direction: column; gap: 18px; min-width: 0; }
.selection-demo__nested { padding-left: 26px; }
.selection-demo__summary { margin: 18px 0 0; padding-top: 12px; border-top: 1px solid var(--sk-hairline); }
.selection-demo__sliders { display: flex; flex-wrap: wrap; gap: 28px; align-items: flex-start; }
.selection-demo__stack { flex: 1 1 420px; min-width: 0; }
.selection-demo__stack .demo__row { flex-wrap: nowrap; }
.selection-demo__slider { flex: 1; min-width: 0; width: auto; }
.selection-demo__slider--label { margin-top: 18px; }
.selection-demo__slider--third { flex: 1; min-width: 0; width: auto; }
.selection-demo__slider--seventh { flex: 1; min-width: 0; width: auto; }
.selection-demo__mixer { display: flex; gap: 6px; padding: 20px 14px 14px; }
.selection-demo__channel { display: flex; flex-direction: column; align-items: center; gap: 10px; }
.selection-demo__channel .skeuo-slider { height: 200px; }
.selection-demo__channel .skeuo-slider--labelled { margin-right: 26px; }
)";

Node FormDemo() {
    return {
        Demo("FormControl, FormLabel, FormGroup, FormHelperText",
             "printed lettering around the controls · error and disabled flow down from FormControl",
             DemoGrid({
                 FormControl({.required = true, .fieldset = true}, {
                     FormLabel({.legend = true}, "Required group"),
                     FormGroup({}, {
                         FormControlLabel({.control = Checkbox({.default_checked = true, .name = "news"}), .label = "News"}),
                         FormControlLabel({.control = Checkbox({.name = "weather"}), .label = "Weather"}),
                         FormControlLabel({.control = Checkbox({.name = "traffic"}), .label = "Traffic"}),
                     }),
                     FormHelperText({}, "Choose the bulletins to air."),
                 }),
                 FormControl({.error = true, .fieldset = true}, {
                     FormLabel({.legend = true}, "Error state"),
                     FormGroup({}, {
                         FormControlLabel({.control = Checkbox({}), .label = "Morning show"}),
                         FormControlLabel({.control = Checkbox({}), .label = "Evening show"}),
                     }),
                     FormHelperText({}, "Pick at least one show."),
                 }),
                 FormControl({.disabled = true, .fieldset = true}, {
                     FormLabel({.legend = true}, "Disabled control"),
                     FormGroup({}, {
                         FormControlLabel({.control = Switch({.default_checked = true}), .label = "Record"}),
                         FormControlLabel({.control = Checkbox({.default_checked = true}), .label = "Archive"}),
                     }),
                     FormHelperText({}, "Managed by the station admin."),
                 }),
                 FormControl({.size = Size::Small, .color = Color::Success, .fieldset = true}, {
                     FormLabel({.legend = true}, "size & color from FormControl"),
                     FormGroup({.row = true}, {
                         FormControlLabel({.control = Checkbox({.default_checked = true}), .label = "One"}),
                         FormControlLabel({.control = Switch({.default_checked = true}), .label = "Two"}),
                         FormControlLabel({.control = Radio({.checked = true}), .label = "Three"}),
                     }),
                 }),
             })),
        el("style").child(raw_html(kDemoCss)),
    };
}

}  // namespace

Node SelectionPage() {
    return {BroadcastSettingsDemo(), CheckboxDemo(), RadioDemo(), SwitchDemo(), SliderDemo(), RatingDemo(), FormDemo()};
}

}  // namespace gallery
