// Navigation page. Mirrors crates/gallery/src/sections/navigation.rs.
#include <array>
#include <tuple>

#include "../gallery.hpp"

namespace gallery {

namespace {

/// A small column with a caption over an example.
Node Cell(std::string caption, Node children) {
    return el("div")
        .attr("style", "display: flex; flex-direction: column; gap: 10px; min-width: 0;")
        .child(el("span").cls("skeuo-label").child(std::move(caption)))
        .child(std::move(children));
}

Node styled_div(std::string style, Node children = {}) {
    return el("div").attr("style", std::move(style)).child(std::move(children));
}

Node TabsDemo() {
    std::string& tab = use_state<std::string>("nav.tab", [] { return std::string("rundown"); });
    Node size_tabs;
    for (Size size : {Size::Small, Size::Medium, Size::Large}) {
        size_tabs.add_child(Tabs({.default_value = "day", .size = size, .aria_label = "Range"},
                                 {
                                     Tab({.value = "day", .label = "Day"}),
                                     Tab({.value = "week", .label = "Week"}),
                                     Tab({.value = "month", .label = "Month"}),
                                 }));
    }
    Node days;
    const char* const day_names[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; ++i) days.add_child(Tab({.value = "d" + std::to_string(i), .label = day_names[i]}));

    return Demo(
        "Tabs",
        "segmented control: shallow well tray · selected tab = raised white keycap · others = printed legends",
        {
            TabContext({.value = tab},
                       {
                           Tabs({.value = tab,
                                 .onchange = [&tab](const std::string& v) { tab = v; },
                                 .aria_label = "Studio views"},
                                {
                                    Tab({.value = "rundown", .label = "Rundown"}),
                                    Tab({.value = "script", .label = "Script"}),
                                    Tab({.value = "clips", .label = "Clips"}),
                                    Tab({.value = "archive", .label = "Archive", .disabled = true}),
                                }),
                           TabPanel({.value = "rundown"},
                                    el("span").cls("skeuo-caption").child(
                                        "Seven items on air today. Use ← → to move between tabs.")),
                           TabPanel({.value = "script"},
                                    el("span").cls("skeuo-caption").child("Good morning, this is the 6 o'clock news…")),
                           TabPanel({.value = "clips"},
                                    el("span").cls("skeuo-caption").child("12 jingles · 4 beds · 3 sweepers")),
                       }),
            styled_div("height: 14px"),
            DemoRow("icon + label",
                    {
                        Tabs({.default_value = "live", .aria_label = "Sources"},
                             {
                                 Tab({.value = "live", .label = "Live", .icon = Icon(IconName::Mic)}),
                                 Tab({.value = "music", .label = "Music", .icon = Icon(IconName::Volume)}),
                                 Tab({.value = "alerts", .label = "Alerts", .icon = Icon(IconName::Notifications)}),
                             }),
                        Tabs({.default_value = "home", .color = Color::Primary, .aria_label = "Sections"},
                             {
                                 Tab({.value = "home", .label = "Home", .icon = Icon(IconName::Home),
                                      .icon_position = IconPosition::Start}),
                                 Tab({.value = "people", .label = "People", .icon = Icon(IconName::Person),
                                      .icon_position = IconPosition::Start}),
                                 Tab({.value = "settings", .label = "Settings", .icon = Icon(IconName::Settings),
                                      .icon_position = IconPosition::End}),
                             }),
                        Tabs({.default_value = "fav", .aria_label = "Icon tabs"},
                             {
                                 Tab({.value = "fav", .icon = Icon({.name = IconName::Favorite, .title = "Favorites"})}),
                                 Tab({.value = "star", .icon = Icon({.name = IconName::Star, .title = "Starred"})}),
                                 Tab({.value = "mark", .icon = Icon({.name = IconName::Bookmark, .title = "Bookmarks"})}),
                             }),
                    }),
            DemoRow("sizes", std::move(size_tabs)),
            styled_div(
                "display: grid; grid-template-columns: minmax(0, 1fr) minmax(0, 1fr); gap: 20px; margin-top: 6px;",
                {
                    Cell("variant = full_width",
                         Tabs({.default_value = "am", .variant = TabsVariant::FullWidth, .aria_label = "Shift"},
                              {
                                  Tab({.value = "am", .label = "Morning"}),
                                  Tab({.value = "pm", .label = "Afternoon"}),
                                  Tab({.value = "night", .label = "Night"}),
                              })),
                    Cell("centered", Tabs({.default_value = "b", .centered = true, .aria_label = "Channel"},
                                          {
                                              Tab({.value = "a", .label = "FM"}),
                                              Tab({.value = "b", .label = "DAB+"}),
                                              Tab({.value = "c", .label = "Stream"}),
                                          })),
                    Cell("variant = scrollable",
                         Tabs({.default_value = "d3", .variant = TabsVariant::Scrollable, .aria_label = "Days"},
                              std::move(days))),
                    Cell("orientation = vertical",
                         Tabs({.default_value = "out", .orientation = Orientation::Vertical, .aria_label = "Settings"},
                              {
                                  Tab({.value = "in", .label = "Input", .icon = Icon(IconName::Mic),
                                       .icon_position = IconPosition::Start}),
                                  Tab({.value = "out", .label = "Output", .icon = Icon(IconName::Volume),
                                       .icon_position = IconPosition::Start}),
                                  Tab({.value = "sched", .label = "Schedule", .icon = Icon(IconName::Calendar),
                                       .icon_position = IconPosition::Start}),
                              })),
                }),
        });
}

std::vector<Node> crumbs(std::initializer_list<const char*> names) {
    std::vector<Node> out;
    size_t last = names.size() - 1, i = 0;
    for (const char* n : names) {
        if (i++ == last) out.push_back(el("span").child(n));
        else out.push_back(Link({.href = "#", .underline = LinkUnderline::Hover, .color = Color::Neutral}, n));
    }
    return out;
}

Node BreadcrumbsDemo() {
    return Demo(
        "Breadcrumbs", "printed path · separator printed · collapsed middle = a small “…” keycap that expands",
        {
            DemoRow("default", Breadcrumbs({.items = crumbs({"Studio", "Shows", "Morning Drive"})})),
            DemoRow("separator",
                    {
                        Breadcrumbs({.items = crumbs({"Library", "Jingles", "Station ID"}), .separator = Node("/")}),
                        Breadcrumbs({.items =
                                         {
                                             Link({.href = "#", .underline = LinkUnderline::Hover},
                                                  {Icon(IconName::Home), " Home"}),
                                             Link({.href = "#", .underline = LinkUnderline::Hover}, "Settings"),
                                             el("span").child("Audio"),
                                         },
                                     .separator = Node("›")}),
                    }),
            DemoRow("max_items = 4",
                    {
                        Breadcrumbs({.items = crumbs({"Station", "2026", "October", "Week 42", "Thursday", "06:00 News"}),
                                     .max_items = 4}),
                        Breadcrumbs({.items = crumbs({"Station", "Archive", "2025", "December", "Week 51", "Friday",
                                                      "Countdown"}),
                                     .max_items = 4,
                                     .items_before_collapse = 2,
                                     .items_after_collapse = 2}),
                    }),
        });
}

Node LinkDemo() {
    int& clicks = use_state<int>("nav.link-clicks", [] { return 0; });
    Node colors;
    for (Color c : kAllColors) colors.add_child(Link({.href = "#", .color = c}, std::string(as_str(c))));
    return Demo("Link", "ink printed on the aluminium in --c-ink · underline none / hover / always",
                {
                    DemoRow("underline",
                            {
                                Link({.href = "#", .underline = LinkUnderline::Always}, "Always underlined"),
                                Link({.href = "#", .underline = LinkUnderline::Hover}, "Underline on hover"),
                                Link({.href = "#", .underline = LinkUnderline::None}, "No underline"),
                            }),
                    DemoRow("colors", std::move(colors)),
                    DemoRow("in text",
                            el("p")
                                .attr("style", "margin: 0; font-size: 13px; color: var(--sk-ink-2); max-width: 60ch;")
                                .child("Read the ")
                                .child(Link({.href = "#"}, "broadcast guidelines"))
                                .child(" before going live, or ")
                                .child(Link({.onclick = [&clicks] { ++clicks; }}, "press this button link"))
                                .child(" (" + std::to_string(clicks) + ").")),
                });
}

Node EditMenuItems() {
    return {
        MenuItem({.icon = Icon(IconName::Edit), .shortcut = "⌘X"}, "Cut"),
        MenuItem({.icon = Icon(IconName::Copy), .shortcut = "⌘C"}, "Copy"),
        MenuItem({.divider = true, .icon = Icon(IconName::Folder), .shortcut = "⌘V"}, "Paste"),
        MenuItem({.disabled = true, .icon = Icon(IconName::Delete), .shortcut = "⌫"}, "Delete"),
        MenuItem({.icon = Icon(IconName::Settings), .shortcut = "⌘,"}, "Preferences…"),
    };
}

Node MenuDemo() {
    bool& open = use_state<bool>("nav.menu-open", [] { return false; });
    std::string& last = use_state<std::string>("nav.menu-last", [] { return std::string("nothing yet"); });
    std::string& speed = use_state<std::string>("nav.menu-speed", [] { return std::string("1×"); });

    Node speeds;
    for (const char* s : {"0.5×", "1×", "1.5×", "2×"})
        speeds.add_child(MenuItem({.selected = speed == s, .onclick = [&speed, s] { speed = s; }}, s));
    Node clip_items;
    for (auto [label, icon, key] : {std::tuple{"Play", IconName::Play, "Space"},
                                    std::tuple{"Pause", IconName::Pause, "⌘."},
                                    std::tuple{"Bookmark", IconName::Bookmark, "⌘D"}}) {
        clip_items.add_child(MenuItem({.icon = Icon(icon),
                                       .shortcut = key,
                                       .onclick =
                                           [&last, &open, label] {
                                               last = label;
                                               open = false;
                                           }},
                                      label));
    }

    return Demo(
        "Menu",
        "level-3 floating chassis · hover = --c-main highlight with white text, as on macOS · selected = check · ⌘ "
        "legends on the right",
        styled_div(
            "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 32px;",
            {
                Cell("open (preview)",
                     Menu({.open = true,
                           .anchor = Button({.variant = ButtonVariant::Outlined,
                                             .color = Color::Neutral,
                                             .end_icon = Icon(IconName::ChevronDown)},
                                            "Edit"),
                           .inline_ = true},
                          EditMenuItems())),
                Cell("MenuList · selected · dense",
                     el("div")
                         .cls("skeuo-chassis skeuo-elev-3 skeuo-chassis--float")
                         .attr("style", "padding: 5px; border-radius: 10px; width: 200px;")
                         .child(MenuList({.dense = true, .aria_label = "Playback speed"}, std::move(speeds)))),
                Cell("interactive",
                     {
                         Menu({.open = open,
                               .onclose = [&open] { open = false; },
                               .anchor = Button({.end_icon = Icon(IconName::ChevronDown),
                                                 .onclick = [&open] { open = !open; },
                                                 .attrs = {{"id", "nav-demo-menu-trigger"},
                                                           {"aria-haspopup", "menu"},
                                                           {"aria-expanded", open ? "true" : "false"}}},
                                                "Clip actions"),
                               .aria_labelledby = "nav-demo-menu-trigger"},
                              std::move(clip_items)),
                         el("span").cls("skeuo-caption").child("Last action: " + last),
                     }),
            }));
}

Node PopoverDemo() {
    bool& open = use_state<bool>("nav.popover-open", [] { return false; });
    return Demo(
        "Popover", "the same floating chassis, without menu semantics · placement bottom / top / left / right",
        styled_div(
            "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 32px;",
            {
                Cell("open (preview)",
                     Popover({.open = true,
                              .anchor = Button({.variant = ButtonVariant::Outlined,
                                                .color = Color::Neutral,
                                                .start_icon = Icon(IconName::Info)},
                                               "Now playing"),
                              .inline_ = true,
                              .aria_label = "Now playing"},
                             styled_div("display: flex; flex-direction: column; gap: 4px; width: 220px;",
                                        {
                                            el("b").attr("style", "font-size: 13px;").child("Morning Drive"),
                                            el("span").cls("skeuo-caption").child("06:00 – 09:00 · Studio A"),
                                            el("span").cls("skeuo-caption").child("Next: Traffic & Weather at 06:15"),
                                        }))),
                Cell("interactive (placement = right)",
                     Popover({.open = open,
                              .onclose = [&open] { open = false; },
                              .anchor = Button({.onclick = [&open] { open = !open; }}, "Show levels"),
                              .placement = Placement::Right,
                              .aria_label = "Levels"},
                             styled_div("width: 180px; font-size: 13px;",
                                        {
                                            el("b").child("Levels"),
                                            el("p").cls("skeuo-caption").attr("style", "margin: 4px 0 0;").child(
                                                "Mic −12 dB · Music −18 dB. Click outside or press Esc to close."),
                                        }))),
            }));
}

Node PaginationDemo() {
    int& page = use_state<int>("nav.page", [] { return 4; });
    Node colors;
    for (Color c : {Color::Primary, Color::Success, Color::Warning, Color::Neutral})
        colors.add_child(Pagination(
            {.count = 3, .default_page = 2, .color = c, .hide_prev_button = true, .hide_next_button = true}));
    return Demo(
        "Pagination", "page numbers as small keycaps · current page = latched-down colored key · ellipsis printed",
        {
            DemoRow("interactive",
                    {
                        Pagination({.count = 12, .page = page, .onchange = [&page](int p) { page = p; }}),
                        el("span").cls("skeuo-caption").child("Page " + std::to_string(page) + " of 12"),
                    }),
            DemoRow("outlined", Pagination({.count = 10, .default_page = 5, .variant = PaginationVariant::Outlined})),
            DemoRow("rounded", Pagination({.count = 10,
                                           .default_page = 2,
                                           .variant = PaginationVariant::Outlined,
                                           .shape = PaginationShape::Rounded,
                                           .color = Color::Secondary})),
            DemoRow("colors", std::move(colors)),
            DemoRow("sizes",
                    {
                        Pagination({.count = 5, .default_page = 2, .size = Size::Small}),
                        Pagination({.count = 5, .default_page = 2, .size = Size::Large}),
                    }),
            DemoRow("first / last", Pagination({.count = 20,
                                                .default_page = 10,
                                                .siblings_count = 0,
                                                .variant = PaginationVariant::Outlined,
                                                .show_first_button = true,
                                                .show_last_button = true})),
            DemoRow("boundary 2", Pagination({.count = 30, .default_page = 15, .boundary_count = 2})),
            DemoRow("disabled", Pagination({.count = 6, .default_page = 3, .disabled = true})),
        });
}

constexpr std::array<const char*, 4> kSteps = {"Prepare script", "Record voice", "Mix & master", "Publish"};
constexpr int kStepCount = static_cast<int>(kSteps.size());

Node StepperDemo() {
    int& active = use_state<int>("nav.step", [] { return 1; });
    int& nl_active = use_state<int>("nav.nl-step", [] { return 2; });
    int& mobile = use_state<int>("nav.mobile-step", [] { return 2; });

    Node steps;
    for (const char* label : kSteps) steps.add_child(Step({}, StepLabel({}, label)));
    Node nl_steps;
    for (int i = 0; i < kStepCount; ++i)
        nl_steps.add_child(Step({.completed = i == 0 || i == 3},
                                StepButton({.onclick = [&nl_active, i] { nl_active = i; }}, kSteps[i])));

    auto back = [&](bool icon) {
        return Button({.variant = ButtonVariant::Text,
                       .size = Size::Small,
                       .disabled = mobile == 0,
                       .start_icon = icon ? std::optional<Node>(Icon(IconName::ChevronLeft)) : std::nullopt,
                       .onclick = [&mobile] { --mobile; }},
                      "Back");
    };
    auto next = [&](bool icon) {
        return Button({.variant = ButtonVariant::Text,
                       .size = Size::Small,
                       .disabled = mobile == 4,
                       .end_icon = icon ? std::optional<Node>(Icon(IconName::ChevronRight)) : std::nullopt,
                       .onclick = [&mobile] { ++mobile; }},
                      "Next");
    };

    return Demo(
        "Stepper",
        "round keycap indicators · active = colored key lit like an LED · completed = check · connectors = grooves "
        "that fill",
        {
            Stepper({.active_step = active}, std::move(steps)),
            styled_div("display: flex; gap: 10px; justify-content: flex-end; margin: 16px 0 26px;",
                       {
                           Button({.variant = ButtonVariant::Text,
                                   .disabled = active == 0,
                                   .onclick = [&active] { --active; }},
                                  "Back"),
                           Button({.disabled = active >= kStepCount, .onclick = [&active] { ++active; }},
                                  active + 1 >= kStepCount ? "Finish" : "Next"),
                           Button({.variant = ButtonVariant::Outlined,
                                   .color = Color::Neutral,
                                   .onclick = [&active] { active = 0; }},
                                  "Reset"),
                       }),
            Cell("alternative_label · optional · error",
                 Stepper({.active_step = 2, .alternative_label = true},
                         {
                             Step({}, StepLabel({}, "Book guest")),
                             Step({}, StepLabel({.optional = Node("Line check failed"), .error = true}, "Phone line")),
                             Step({}, StepLabel({.optional = Node("Optional")}, "Pre-record intro")),
                             Step({}, StepLabel({}, "Go live")),
                         })),
            styled_div(
                "display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 28px; margin-top: "
                "26px;",
                {
                    Cell("orientation = vertical · StepContent",
                         Stepper({.active_step = 1, .orientation = Orientation::Vertical},
                                 {
                                     Step({}, {StepLabel({}, "Select a show"),
                                               StepContent({}, "Pick the slot you want to fill.")}),
                                     Step({}, {StepLabel({.optional = Node("About 2 minutes")}, "Record the announcement"),
                                               StepContent({}, {el("p").attr("style", "margin: 0 0 10px;").child(
                                                                    "Read the script at a steady pace; the meter "
                                                                    "should stay in the green."),
                                                                Button({.color = Color::Error,
                                                                        .size = Size::Small,
                                                                        .start_icon = Icon(IconName::Dot)},
                                                                       "Record")})}),
                                     Step({}, {StepLabel({}, "Schedule"), StepContent({}, "Choose when it airs.")}),
                                 })),
                    Cell("non_linear · StepButton · color = success",
                         Stepper({.active_step = nl_active,
                                  .orientation = Orientation::Vertical,
                                  .non_linear = true,
                                  .color = Color::Success},
                                 std::move(nl_steps))),
                }),
            styled_div(
                "display: grid; grid-template-columns: repeat(auto-fill, minmax(260px, 1fr)); gap: 20px; margin-top: "
                "26px;",
                {
                    Cell("MobileStepper · dots", MobileStepper({.steps = 5,
                                                                .active_step = mobile,
                                                                .position = MobileStepperPosition::Static,
                                                                .back_button = back(true),
                                                                .next_button = next(true)})),
                    Cell("MobileStepper · progress", MobileStepper({.steps = 5,
                                                                    .active_step = mobile,
                                                                    .variant = MobileStepperVariant::Progress,
                                                                    .position = MobileStepperPosition::Static,
                                                                    .back_button = back(false),
                                                                    .next_button = next(false)})),
                    Cell("MobileStepper · text", MobileStepper({.steps = 5,
                                                                .active_step = mobile,
                                                                .variant = MobileStepperVariant::Text,
                                                                .position = MobileStepperPosition::Static,
                                                                .back_button = back(false),
                                                                .next_button = next(false)})),
                }),
        });
}

Node BottomNavigationDemo() {
    std::string& value = use_state<std::string>("nav.bottom", [] { return std::string("live"); });
    return Demo(
        "BottomNavigation", "a chassis bar of keys · selected = latched key, legend and icon in --c-ink",
        DemoGrid({
            Cell("show_labels · interactive",
                 BottomNavigation({.value = value,
                                   .onchange = [&value](const std::string& v) { value = v; },
                                   .show_labels = true},
                                  {
                                      BottomNavigationAction({.value = "live", .label = "On air", .icon = Icon(IconName::Mic)}),
                                      BottomNavigationAction({.value = "library", .label = "Library", .icon = Icon(IconName::Folder)}),
                                      BottomNavigationAction({.value = "schedule", .label = "Schedule", .icon = Icon(IconName::Calendar)}),
                                      BottomNavigationAction({.value = "me", .label = "Profile", .icon = Icon(IconName::Person)}),
                                  })),
            Cell("labels on selected only · color = error",
                 BottomNavigation({.default_value = "fav", .color = Color::Error},
                                  {
                                      BottomNavigationAction({.value = "recent", .label = "Recents", .icon = Icon(IconName::Calendar)}),
                                      BottomNavigationAction({.value = "fav", .label = "Favorites", .icon = Icon(IconName::Favorite)}),
                                      BottomNavigationAction({.value = "near", .label = "Nearby", .icon = Icon(IconName::Search)}),
                                  })),
        }));
}

Node SpeedDialDemo() {
    bool& open = use_state<bool>("nav.dial-open", [] { return false; });
    return Demo(
        "SpeedDial",
        "a large round colored keycap · actions fan out as small round white keycaps with printed tooltips",
        styled_div(
            "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 56px;",
            {
                Cell("open · direction = up · tooltip_open",
                     styled_div("display: flex; align-items: flex-end; height: 240px; padding-left: 104px;",
                                SpeedDial({.aria_label = "Studio actions", .open = true},
                                          {
                                              SpeedDialAction({.icon = Icon(IconName::Mic), .tooltip_title = "Go live", .tooltip_open = true}),
                                              SpeedDialAction({.icon = Icon(IconName::Play), .tooltip_title = "Play jingle", .tooltip_open = true}),
                                              SpeedDialAction({.icon = Icon(IconName::Bookmark), .tooltip_title = "Mark", .tooltip_open = true}),
                                          }))),
                Cell("direction = right · color = error",
                     styled_div("display: flex; align-items: flex-end; height: 240px;",
                                SpeedDial({.aria_label = "Recorder actions",
                                           .open = true,
                                           .direction = SpeedDialDirection::Right,
                                           .color = Color::Error,
                                           .icon = SpeedDialIcon({.icon = Icon(IconName::Dot),
                                                                  .open_icon = Icon(IconName::Close)})},
                                          {
                                              SpeedDialAction({.icon = Icon(IconName::Pause), .tooltip_title = "Pause"}),
                                              SpeedDialAction({.icon = Icon(IconName::Delete), .tooltip_title = "Discard"}),
                                              SpeedDialAction({.icon = Icon(IconName::Check), .tooltip_title = "Save"}),
                                          }))),
                Cell("interactive · direction = left",
                     styled_div("display: flex; align-items: flex-end; justify-content: flex-end; height: 240px; "
                                "min-width: 240px;",
                                SpeedDial({.aria_label = "Clip actions",
                                           .open = open,
                                           .onopen = [&open] { open = true; },
                                           .onclose = [&open] { open = false; },
                                           .direction = SpeedDialDirection::Left,
                                           .color = Color::Secondary},
                                          {
                                              SpeedDialAction({.icon = Icon(IconName::Copy), .tooltip_title = "Duplicate"}),
                                              SpeedDialAction({.icon = Icon(IconName::Edit), .tooltip_title = "Rename"}),
                                              SpeedDialAction({.icon = Icon(IconName::Star), .tooltip_title = "Favorite"}),
                                          }))),
            }));
}

Node TimelineDemo() {
    struct Entry {
        const char* time;
        const char* title;
        const char* note;
        std::optional<Color> color;
        TimelineDotVariant variant;
        IconName icon;
    };
    const Entry rundown[] = {
        {"06:00", "News headlines", "Top stories and the weather", Color::Success, TimelineDotVariant::Filled,
         IconName::Mic},
        {"06:05", "Morning Drive", "Live — with traffic every 15 min", Color::Error, TimelineDotVariant::Filled,
         IconName::Dot},
        {"06:30", "Sports desk", "Pre-recorded, 4:30", Color::Primary, TimelineDotVariant::Outlined, IconName::Play},
        {"06:45", "Listener calls", "Phone lines open", Color::Warning, TimelineDotVariant::Outlined,
         IconName::Notifications},
        {"07:00", "News on the hour", "Standby", std::nullopt, TimelineDotVariant::Outlined, IconName::Calendar},
    };
    const size_t n = std::size(rundown);
    Node main_items;
    for (size_t i = 0; i < n; ++i) {
        const Entry& e = rundown[i];
        main_items.add_child(TimelineItem(
            {}, {
                    TimelineOppositeContent({}, e.time),
                    TimelineSeparator({}, {TimelineDot({.color = e.color, .variant = e.variant}, Icon(e.icon)),
                                           i + 1 < n ? TimelineConnector() : Node()}),
                    TimelineContent({}, {el("b").attr("style", "font-weight: 600;").child(e.title),
                                         el("div").cls("skeuo-caption").child(e.note)}),
                }));
    }

    Node leds;
    const std::pair<const char*, std::optional<Color>> led_items[] = {
        {"Mic check", Color::Success}, {"Sound check", Color::Success}, {"On air", Color::Error}, {"Wrap", std::nullopt}};
    for (size_t i = 0; i < 4; ++i) {
        leds.add_child(TimelineItem({}, {
                                            TimelineSeparator({}, {TimelineDot({.color = led_items[i].second}),
                                                                   i < 3 ? TimelineConnector() : Node()}),
                                            TimelineContent({}, led_items[i].first),
                                        }));
    }
    Node left;
    const std::pair<const char*, const char*> left_items[] = {
        {"Uploaded", "09:12"}, {"Transcoded", "09:14"}, {"Scheduled", "09:20"}};
    for (size_t i = 0; i < 3; ++i) {
        left.add_child(TimelineItem(
            {}, {
                    TimelineOppositeContent({}, left_items[i].second),
                    TimelineSeparator({}, {TimelineDot({.color = Color::Info, .variant = TimelineDotVariant::Outlined}),
                                           i < 2 ? TimelineConnector() : Node()}),
                    TimelineContent({}, left_items[i].first),
                }));
    }
    Node colors;
    for (size_t i = 0; i < kAllColors.size(); ++i) {
        colors.add_child(TimelineItem({}, {
                                              TimelineSeparator({}, {TimelineDot({.color = kAllColors[i]}),
                                                                     i + 1 < kAllColors.size() ? TimelineConnector()
                                                                                               : Node()}),
                                              TimelineContent({}, std::string(as_str(kAllColors[i]))),
                                          }));
    }

    return Demo(
        "Timeline",
        "LEDs joined by a groove · filled = lit LED · outlined = LED off · position left / right / alternate",
        {
            Cell("broadcast run-down · position = alternate",
                 Timeline({.position = TimelinePosition::Alternate}, std::move(main_items))),
            styled_div(
                "display: grid; grid-template-columns: repeat(auto-fill, minmax(240px, 1fr)); gap: 20px; margin-top: "
                "18px;",
                {
                    Cell("position = right · LEDs", Timeline({}, std::move(leds))),
                    Cell("position = left · outlined",
                         Timeline({.position = TimelinePosition::Left}, std::move(left))),
                    Cell("colors", Timeline({}, std::move(colors))),
                }),
        });
}

}  // namespace

Node NavigationPage() {
    return {
        TabsDemo(),     BreadcrumbsDemo(),      LinkDemo(),      MenuDemo(),     PopoverDemo(),
        PaginationDemo(), StepperDemo(),        BottomNavigationDemo(), SpeedDialDemo(), TimelineDemo(),
    };
}

}  // namespace gallery
