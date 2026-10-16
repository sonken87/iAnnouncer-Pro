#include <algorithm>

#include "../gallery.hpp"

namespace gallery {

namespace {

using Strings = std::vector<std::string>;

Attrs aria(std::string label) { return {{"aria-label", std::move(label)}}; }

std::string join(const Strings& v, std::string_view sep) {
    std::string out;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) out += sep;
        out += v[i];
    }
    return out;
}

Node ButtonDemo() {
    int& clicks = use_state<int>("demo.clicks", [] { return 0; });
    bool& saving = use_state<bool>("demo.saving", [] { return false; });
    Node rows;
    for (auto [variant, label] : {std::pair{ButtonVariant::Contained, "contained"},
                                  std::pair{ButtonVariant::Outlined, "outlined"},
                                  std::pair{ButtonVariant::Text, "text"}}) {
        Node buttons;
        for (Color c : kAllColors)
            buttons.add_child(Button({.variant = variant, .color = c, .onclick = [&clicks] { ++clicks; }},
                                     std::string(as_str(c))));
        rows.add_child(DemoRow(label, std::move(buttons)));
    }
    Node loading = {
        Button({.loading = true}, "Submit"),
        Button({.color = Color::Success, .loading = true, .loading_position = LoadingPosition::Start,
                .start_icon = Icon(IconName::Check)},
               "Saving"),
        Button({.variant = ButtonVariant::Outlined, .loading = true, .loading_position = LoadingPosition::End,
                .end_icon = Icon(IconName::ArrowForward)},
               "Sending"),
        Button({.variant = ButtonVariant::Text, .loading = true}, "Fetch"),
        Button({.loading = saving, .loading_position = LoadingPosition::Start, .start_icon = Icon(IconName::Bookmark),
                .onclick = [&saving] { saving = true; }},
               "Save (click)"),
    };
    if (saving)
        loading.add_child(Button({.variant = ButtonVariant::Text, .color = Color::Neutral,
                                  .onclick = [&saving] { saving = false; }},
                                 "Reset"));
    return Demo("Button",
                "contained = colored keycap · outlined = white keycap · text = printed legend · loading = spinner "
                "legend, key disabled",
                {
                    std::move(rows),
                    DemoRow("sizes", {
                        Button({.size = Size::Small}, "Small"),
                        Button({.size = Size::Medium}, "Medium"),
                        Button({.size = Size::Large}, "Large"),
                        Button({.disabled = true}, "Disabled"),
                        Button({.variant = ButtonVariant::Outlined, .disabled = true}, "Disabled"),
                    }),
                    DemoRow("icons", {
                        Button({.color = Color::Error, .start_icon = Icon(IconName::Dot)}, "Record"),
                        Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral,
                                .end_icon = Icon(IconName::Play)}, "Play"),
                        Button({.color = Color::Success, .end_icon = Icon(IconName::ArrowForward)}, "On Air"),
                    }),
                    DemoRow("loading", std::move(loading)),
                    DemoRow("clicks", el("span").cls("skeuo-caption")
                                          .child("Clicked " + std::to_string(clicks) + " times")),
                });
}

Node ButtonGroupDemo() {
    std::string& channel = use_state<std::string>("demo.channel", [] { return std::string("A"); });
    Node channels;
    for (const char* ch : {"A", "B", "C", "D"})
        channels.add_child(Button({.onclick = [&channel, ch] { channel = ch; }}, std::string("Ch ") + ch));
    auto three = [] { return Node{Button({}, "One"), Button({}, "Two"), Button({}, "Three")}; };
    auto tmb = [] { return Node{Button({}, "Top"), Button({}, "Middle"), Button({}, "Bottom")}; };
    return Demo(
        "ButtonGroup",
        "keycaps joined edge-to-edge in one slot, inner corners square · contained / outlined / text · horizontal / "
        "vertical",
        {
            DemoRow("contained", {
                ButtonGroup({.attrs = aria("Transport")}, three()),
                ButtonGroup({.color = Color::Success},
                            {Button({.start_icon = Icon(IconName::Play)}, "Play"),
                             Button({.start_icon = Icon(IconName::Pause)}, "Pause")}),
                ButtonGroup({.color = Color::Neutral, .size = Size::Small},
                            {Button({}, "Small"), Button({}, "Neutral"), Button({}, "Group")}),
            }),
            DemoRow("outlined", {
                ButtonGroup({.variant = ButtonVariant::Outlined}, three()),
                ButtonGroup({.variant = ButtonVariant::Outlined, .color = Color::Error},
                            {Button({}, "Cut"), Button({}, "Mute")}),
                ButtonGroup({.variant = ButtonVariant::Outlined, .disabled = true},
                            {Button({}, "Disabled"), Button({}, "Group")}),
            }),
            DemoRow("text", {
                ButtonGroup({.variant = ButtonVariant::Text}, three()),
                ButtonGroup({.variant = ButtonVariant::Text, .color = Color::Secondary, .size = Size::Large},
                            {Button({}, "Large"), Button({}, "Text")}),
            }),
            DemoRow("vertical", {
                ButtonGroup({.orientation = Orientation::Vertical}, tmb()),
                ButtonGroup({.variant = ButtonVariant::Outlined, .orientation = Orientation::Vertical}, tmb()),
                ButtonGroup({.variant = ButtonVariant::Text, .orientation = Orientation::Vertical}, tmb()),
                el("div")
                    .style("flex: 1; min-width: 260px; display: flex; flex-direction: column; gap: 10px")
                    .child(ButtonGroup({.variant = ButtonVariant::Outlined, .color = Color::Info, .full_width = true},
                                       std::move(channels)))
                    .child(el("span").cls("skeuo-caption").child("full_width · on channel " + channel)),
            }),
        });
}

Node IconButtonDemo() {
    bool& liked = use_state<bool>("demo.liked", [] { return false; });
    Node text_row = {IconButton({.attrs = aria("Delete")}, Icon(IconName::Delete))};
    for (Color c : {Color::Primary, Color::Secondary, Color::Success, Color::Error, Color::Warning})
        text_row.add_child(IconButton({.color = c, .attrs = aria("Favorite")}, Icon(IconName::Favorite)));
    text_row.add_child(IconButton({.size = Size::Small, .attrs = aria("Settings")}, Icon(IconName::Settings)));
    text_row.add_child(
        IconButton({.size = Size::Large, .attrs = aria("Notifications")}, Icon(IconName::Notifications)));
    text_row.add_child(IconButton({.disabled = true, .attrs = aria("Mail")}, Icon(IconName::Mail)));

    Node contained;
    for (Color c : kAllColors)
        contained.add_child(
            IconButton({.variant = IconButtonVariant::Contained, .color = c, .attrs = aria("Add")}, Icon(IconName::Add)));
    contained.add_child(IconButton({.variant = IconButtonVariant::Contained, .color = Color::Error, .size = Size::Large,
                                    .round = true, .attrs = aria("Record")},
                                   Icon(IconName::Mic)));
    const auto O = IconButtonVariant::Outlined;
    return Demo(
        "IconButton",
        "text = icon printed on the aluminium (MUI default) · outlined = small white keycap · contained = colored "
        "keycap · round",
        {
            DemoRow("text", std::move(text_row)),
            DemoRow("outlined", {
                IconButton({.variant = O, .size = Size::Small, .attrs = aria("Edit")}, Icon(IconName::Edit)),
                IconButton({.variant = O, .attrs = aria("Edit")}, Icon(IconName::Edit)),
                IconButton({.variant = O, .size = Size::Large, .attrs = aria("Edit")}, Icon(IconName::Edit)),
                IconButton({.variant = O, .color = Color::Primary, .attrs = aria("Search")}, Icon(IconName::Search)),
                IconButton({.variant = O, .color = Color::Error, .round = true, .attrs = aria("Delete")},
                           Icon(IconName::Delete)),
                IconButton({.variant = O, .disabled = true, .attrs = aria("Copy")}, Icon(IconName::Copy)),
            }),
            DemoRow("contained", std::move(contained)),
            DemoRow("edge", {
                el("div")
                    .cls("skeuo-chassis skeuo-elev-1")
                    .style("display: flex; align-items: center; gap: 8px; width: 320px; padding: 6px 16px")
                    .child(IconButton({.edge = IconButtonEdge::Start, .attrs = aria("Menu")}, Icon(IconName::Menu)))
                    .child(el("span").style("flex: 1; font-weight: 600").child("Edge start / end"))
                    .child(IconButton({.color = liked ? Color::Error : Color::Neutral,
                                       .edge = IconButtonEdge::End,
                                       .onclick = [&liked] { liked = !liked; },
                                       .attrs = {{"aria-label", "Like"}, {"aria-pressed", liked ? "true" : "false"}}},
                                      Icon(liked ? IconName::Favorite : IconName::FavoriteBorder))),
                el("span").cls("skeuo-caption").child(liked ? "Liked — click the heart again" : "Click the heart"),
            }),
        });
}

Node FabDemo() {
    int& count = use_state<int>("demo.fab", [] { return 0; });
    return Demo(
        "Fab", "a large round keycap with a deeper travel · circular / extended · small / medium / large",
        {
            DemoRow("circular", {
                Fab({.color = Color::Primary, .onclick = [&count] { ++count; }, .attrs = aria("Add")},
                    Icon(IconName::Add)),
                Fab({.color = Color::Secondary, .attrs = aria("Edit")}, Icon(IconName::Edit)),
                Fab({.attrs = aria("Like")}, Icon(IconName::Favorite)),
                Fab({.color = Color::Error, .attrs = aria("Record")}, Icon(IconName::Mic)),
                Fab({.disabled = true, .attrs = aria("Navigate")}, Icon(IconName::ArrowForward)),
                el("span").cls("skeuo-caption").child("Added " + std::to_string(count)),
            }),
            DemoRow("sizes", {
                Fab({.color = Color::Primary, .size = Size::Small, .attrs = aria("Add")}, Icon(IconName::Add)),
                Fab({.color = Color::Primary, .size = Size::Medium, .attrs = aria("Add")}, Icon(IconName::Add)),
                Fab({.color = Color::Primary, .size = Size::Large, .attrs = aria("Add")}, Icon(IconName::Add)),
            }),
            DemoRow("extended", {
                Fab({.variant = FabVariant::Extended, .color = Color::Primary, .size = Size::Small},
                    {Icon(IconName::Play), "Small"}),
                Fab({.variant = FabVariant::Extended, .color = Color::Success, .size = Size::Medium},
                    {Icon(IconName::Mic), "Go live"}),
                Fab({.variant = FabVariant::Extended}, {Icon(IconName::ArrowForward), "Navigate"}),
            }),
        });
}

Node ToggleButtonDemo() {
    Strings& align = use_state<Strings>("demo.align", [] { return Strings{"left"}; });
    Strings& formats = use_state<Strings>("demo.formats", [] { return Strings{"bold", "italic"}; });
    bool& solo = use_state<bool>("demo.solo", [] { return true; });
    std::string align_text = align.empty() ? "none" : align.front();
    std::string formats_text = formats.empty() ? "none" : join(formats, " + ");
    auto platforms = [] {
        return Node{ToggleButton({.value = "web"}, "Web"), ToggleButton({.value = "android"}, "Android"),
                    ToggleButton({.value = "ios"}, "iOS")};
    };
    return Demo(
        "ToggleButton",
        "white keycaps; selected = latched down with the legend lit in color · exclusive / multiple · horizontal / "
        "vertical",
        {
            DemoRow("exclusive", {
                ToggleButtonGroup({.value = align,
                                   .exclusive = true,
                                   .onchange = [&align](Strings v) { align = std::move(v); },
                                   .attrs = aria("Text alignment")},
                                  {
                                      ToggleButton({.value = "left", .attrs = aria("Left")}, Icon(IconName::FirstPage)),
                                      ToggleButton({.value = "center", .attrs = aria("Center")}, Icon(IconName::Menu)),
                                      ToggleButton({.value = "right", .attrs = aria("Right")}, Icon(IconName::LastPage)),
                                      ToggleButton({.value = "justify", .disabled = true, .attrs = aria("Justify")},
                                                   Icon(IconName::MoreHoriz)),
                                  }),
                el("span").cls("skeuo-caption").child("align = " + align_text),
            }),
            DemoRow("multiple", {
                ToggleButtonGroup({.value = formats,
                                   .onchange = [&formats](Strings v) { formats = std::move(v); },
                                   .color = Color::Secondary,
                                   .attrs = aria("Formatting")},
                                  {
                                      ToggleButton({.value = "bold"}, el("b").child("B")),
                                      ToggleButton({.value = "italic"}, el("i").child("I")),
                                      ToggleButton({.value = "underline"}, el("u").child("U")),
                                      ToggleButton({.value = "color"}, "Color"),
                                  }),
                el("span").cls("skeuo-caption").child("formats = " + formats_text),
            }),
            DemoRow("sizes", {
                ToggleButtonGroup({.default_value = Strings{"web"}, .exclusive = true, .size = Size::Small},
                                  platforms()),
                ToggleButtonGroup({.default_value = Strings{"android"}, .exclusive = true, .color = Color::Success},
                                  platforms()),
                ToggleButtonGroup(
                    {.default_value = Strings{"ios"}, .exclusive = true, .color = Color::Error, .size = Size::Large},
                    platforms()),
            }),
            DemoRow("vertical", {
                ToggleButtonGroup(
                    {.default_value = Strings{"list"}, .exclusive = true, .orientation = Orientation::Vertical},
                    {
                        ToggleButton({.value = "list", .attrs = aria("List")}, Icon(IconName::Menu)),
                        ToggleButton({.value = "module", .attrs = aria("Module")}, Icon(IconName::Copy)),
                        ToggleButton({.value = "quilt", .attrs = aria("Quilt")}, Icon(IconName::Folder)),
                    }),
                ToggleButton({.value = "check",
                              .selected = solo,
                              .color = Color::Success,
                              .onchange = [&solo](const std::string&) { solo = !solo; },
                              .attrs = aria("Standalone")},
                             Icon(IconName::Check)),
                ToggleButton({.value = "star", .attrs = aria("Star")}, Icon(IconName::Star)),
                el("span").cls("skeuo-caption").child(std::string("standalone: ") + (solo ? "true" : "false")),
                el("div").style("flex: 1; min-width: 260px")
                    .child(ToggleButtonGroup({.default_value = Strings{"day"}, .exclusive = true, .full_width = true},
                                             {ToggleButton({.value = "day"}, "Day"),
                                              ToggleButton({.value = "week"}, "Week"),
                                              ToggleButton({.value = "month"}, "Month")})),
            }),
        });
}

Node TextFieldDemo() {
    std::string& name = use_state<std::string>("demo.name", [] { return std::string(); });
    std::string& email = use_state<std::string>("demo.email", [] { return std::string("operator@station"); });
    bool email_invalid = email.find('@') == std::string::npos;
    return Demo(
        "TextField", "outlined = well · filled = shallow recess · standard = engraved line",
        {
            DemoGrid({
                TextField({.label = "Announcer",
                           .value = name,
                           .placeholder = "Your name",
                           .helper_text = name.empty() ? std::string("Required") : "Hello, " + name,
                           .required = true,
                           .oninput = [&name](const std::string& v) { name = v; }}),
                TextField({.label = "Email",
                           .value = email,
                           .helper_text = email_invalid ? "Must contain @" : "Looks good",
                           .error = email_invalid,
                           .type = "email",
                           .oninput = [&email](const std::string& v) { email = v; }}),
                TextField({.label = "Password", .default_value = "secret", .type = "password"}),
                TextField({.variant = TextFieldVariant::Filled, .label = "Filled", .placeholder = "Shallow recess"}),
                TextField({.variant = TextFieldVariant::Filled, .label = "Frequency", .default_value = "101.7",
                           .end_adornment = Node("MHz")}),
                TextField({.label = "Disabled", .default_value = "Read only", .disabled = true}),
                TextField({.variant = TextFieldVariant::Standard, .label = "Standard", .placeholder = "Engraved line"}),
                TextField({.variant = TextFieldVariant::Standard, .label = "Search", .placeholder = "Find a clip",
                           .start_adornment = Icon(IconName::Search)}),
                TextField({.size = Size::Small, .label = "Small", .placeholder = "size=small"}),
            }),
            el("div").style("margin-top: 20px").child(
                TextField({.label = "Script", .placeholder = "Type the announcement…", .full_width = true,
                           .multiline = true, .rows = 4})),
        });
}

Node TextareaAutosizeDemo() {
    std::string& notes = use_state<std::string>("demo.notes", [] { return std::string("Line one\nLine two"); });
    size_t lines = 1 + static_cast<size_t>(std::count(notes.begin(), notes.end(), '\n'));
    auto cell = [](const char* label, Node content) {
        return el("div")
            .style("display: flex; flex-direction: column; gap: 6px")
            .child(el("span").cls("skeuo-label").child(label))
            .child(std::move(content));
    };
    return Demo("TextareaAutosize", "the TextField well, growing with its content between min_rows and max_rows",
                DemoGrid({
                    cell("Empty, min_rows = 2", TextareaAutosize({.min_rows = 2, .placeholder = "Grows as you type…",
                                                                  .aria_label = "Notes"})),
                    cell("Controlled, max_rows = 4",
                         {TextareaAutosize({.max_rows = 4,
                                            .value = notes,
                                            .oninput = [&notes](const std::string& v) { notes = v; },
                                            .aria_label = "Controlled notes"}),
                          el("span").cls("skeuo-caption").child(std::to_string(lines) + " line(s)")}),
                    cell("Capped at max_rows = 3",
                         TextareaAutosize(
                             {.max_rows = 3,
                              .default_value = "Station ID\nTraffic\nWeather\nNews at the top of the hour\nMusic",
                              .aria_label = "Running order"})),
                    cell("Disabled", TextareaAutosize({.default_value = "Locked", .disabled = true,
                                                       .aria_label = "Locked"})),
                }));
}

std::vector<SelectOption> stations() {
    return {{"fm1017", "101.7 FM"},
            {"fm954", "95.4 FM"},
            {"am810", "810 AM"},
            {"dab", "DAB+ Digital"},
            {.value = "off", .label = "Off air", .disabled = true}};
}

Node SelectDemo() {
    std::string& station = use_state<std::string>("demo.station", [] { return std::string("fm954"); });
    Strings& days = use_state<Strings>("demo.days", [] { return Strings{"mon", "wed"}; });
    std::string& native = use_state<std::string>("demo.native", [] { return std::string("15"); });
    std::vector<SelectOption> days_options = {
        {"mon", "Monday"}, {"tue", "Tuesday"}, {"wed", "Wednesday"}, {"thu", "Thursday"}, {"fri", "Friday"}};
    std::vector<SelectOption> minutes;
    for (const char* m : {"5", "10", "15", "30", "60"}) minutes.push_back({m, std::string(m) + " minutes"});
    return Demo(
        "Select",
        "a well with a chevron; the open list is a floating plate (chassis 3) with tinted, ticked rows · options "
        "passed as data",
        {
            DemoGrid({
                Select({.label = "Station",
                        .options = stations(),
                        .value = station,
                        .onchange = [&station](const std::string& v) { station = v; },
                        .helper_text = "value = " + station}),
                Select({.label = "Days",
                        .options = days_options,
                        .multiple = true,
                        .values = days,
                        .onchange_multiple = [&days](Strings v) { days = std::move(v); },
                        .helper_text = "multiple"}),
                Select({.label = "Placeholder", .options = stations(), .placeholder = "Choose a station"}),
                Select({.label = "Filled", .options = stations(), .default_value = "am810",
                        .variant = TextFieldVariant::Filled}),
                Select({.label = "Standard", .options = stations(), .default_value = "dab",
                        .variant = TextFieldVariant::Standard}),
                Select({.label = "Small", .options = stations(), .default_value = "fm1017", .size = Size::Small}),
                Select({.label = "Error",
                        .options = stations(),
                        .error = true,
                        .required = true,
                        .helper_text = "A station is required",
                        .placeholder = "Pick one"}),
                Select({.label = "Disabled", .options = stations(), .default_value = "fm1017", .disabled = true}),
                NativeSelect({.label = "NativeSelect",
                              .options = minutes,
                              .value = native,
                              .onchange = [&native](const std::string& v) { native = v; },
                              .helper_text = "platform list · " + native + " min"}),
            }),
            el("div").style("margin-top: 20px; min-height: 252px").child(DemoGrid({
                Select({.label = "Open (static preview)", .options = stations(), .default_value = "fm954",
                        .open = true}),
                Select({.label = "Open, multiple",
                        .options = days_options,
                        .multiple = true,
                        .default_values = Strings{"tue", "thu"},
                        .open = true,
                        .color = Color::Secondary}),
            })),
        });
}

Strings programmes() {
    return {"Breakfast Show", "Drive Time",  "Late Night Jazz", "Morning News",
            "Sports Roundup", "The Chart",   "Weather Watch",   "Weekend Brunch"};
}

Node AutocompleteDemo() {
    auto& pick = use_state<std::optional<std::string>>("demo.pick", [] { return std::optional<std::string>("Drive Time"); });
    Strings& tags = use_state<Strings>("demo.tags", [] { return Strings{"Morning News", "The Chart"}; });
    std::string pick_text = pick.value_or("none");
    return Demo(
        "Autocomplete",
        "a well that filters options as you type; multiple values become keycap pills inside the well · free_solo "
        "accepts any text",
        {
            DemoGrid({
                Autocomplete({.options = programmes(),
                              .value = pick,
                              .onchange = [&pick](std::optional<std::string> v) { pick = std::move(v); },
                              .label = "Programme",
                              .placeholder = "Type to filter",
                              .helper_text = "value = " + pick_text}),
                Autocomplete({.options = programmes(),
                              .free_solo = true,
                              .label = "Free solo",
                              .placeholder = "Any title",
                              .helper_text = "Press Enter to keep your own text"}),
                Autocomplete({.options = programmes(), .default_value = "The Chart", .label = "Disabled",
                              .disabled = true}),
            }),
            el("div").style("margin-top: 20px").child(Autocomplete({
                .options = programmes(),
                .multiple = true,
                .values = tags,
                .onchange_multiple = [&tags](Strings v) { tags = std::move(v); },
                .label = "Playlist (multiple)",
                .placeholder = "Add programmes",
                .helper_text = std::to_string(tags.size()) + " selected · Backspace removes the last pill",
                .full_width = true,
            })),
            el("div").style("margin-top: 20px; min-height: 196px").child(DemoGrid({
                Autocomplete({.options = programmes(),
                              .multiple = true,
                              .default_values = Strings{"Late Night Jazz"},
                              .input_value = "n",
                              .open = true,
                              .label = "Open (static preview)"}),
                Autocomplete({.options = programmes(),
                              .input_value = "xyz",
                              .open = true,
                              .no_options_text = "No programmes found",
                              .label = "No match"}),
            })),
        });
}

Node TransferListDemo() {
    using Lists = std::pair<Strings, Strings>;
    Lists& lists = use_state<Lists>("demo.lists", [] {
        return Lists{{"Jingle A", "Jingle B", "Station ID", "Traffic", "Weather"}, {"Top of hour", "News"}};
    });
    return Demo("TransferList",
                "two wells of rows with small checkbox keycaps; keycaps between them move the checked rows across",
                {
                    TransferList({.left = lists.first,
                                  .right = lists.second,
                                  .default_checked = {"Station ID", "Weather"},
                                  .onchange = [&lists](Strings l, Strings r) { lists = {std::move(l), std::move(r)}; },
                                  .left_title = "Library",
                                  .right_title = "Running order"}),
                    el("p").cls("skeuo-caption").style("margin: 12px 0 0")
                        .child("Running order: " + join(lists.second, " → ")),
                });
}

}  // namespace

Node InputsPage() {
    return {ButtonDemo(),       ButtonGroupDemo(),      IconButtonDemo(), FabDemo(),          ToggleButtonDemo(),
            TextFieldDemo(),    TextareaAutosizeDemo(), SelectDemo(),     AutocompleteDemo(), TransferListDemo()};
}

}  // namespace gallery
