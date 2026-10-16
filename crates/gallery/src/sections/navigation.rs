use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

#[component]
pub fn NavigationSection() -> Element {
    rsx! {
        TabsDemo {}
        BreadcrumbsDemo {}
        LinkDemo {}
        MenuDemo {}
        PopoverDemo {}
        PaginationDemo {}
        StepperDemo {}
        BottomNavigationDemo {}
        SpeedDialDemo {}
        TimelineDemo {}
    }
}

/// A small column with a caption over an example.
#[component]
fn Cell(#[props(into)] caption: String, children: Element) -> Element {
    rsx! {
        div { style: "display: flex; flex-direction: column; gap: 10px; min-width: 0;",
            span { class: "skeuo-label", "{caption}" }
            {children}
        }
    }
}

#[component]
fn TabsDemo() -> Element {
    let mut tab = use_signal(|| "rundown".to_string());
    rsx! {
        Demo {
            title: "Tabs",
            description: "segmented control: shallow well tray · selected tab = raised white keycap · others = printed legends",
            TabContext { value: tab(),
                Tabs {
                    value: tab(),
                    onchange: move |v: String| tab.set(v),
                    aria_label: "Studio views",
                    Tab { value: "rundown", label: "Rundown" }
                    Tab { value: "script", label: "Script" }
                    Tab { value: "clips", label: "Clips" }
                    Tab { value: "archive", label: "Archive", disabled: true }
                }
                TabPanel { value: "rundown",
                    span { class: "skeuo-caption", "Seven items on air today. Use ← → to move between tabs." }
                }
                TabPanel { value: "script",
                    span { class: "skeuo-caption", "Good morning, this is the 6 o'clock news…" }
                }
                TabPanel { value: "clips",
                    span { class: "skeuo-caption", "12 jingles · 4 beds · 3 sweepers" }
                }
            }
            div { style: "height: 14px" }
            DemoRow { label: "icon + label",
                Tabs { default_value: "live", aria_label: "Sources",
                    Tab { value: "live", label: "Live", icon: rsx! { Icon { name: IconName::Mic } } }
                    Tab { value: "music", label: "Music", icon: rsx! { Icon { name: IconName::Volume } } }
                    Tab { value: "alerts", label: "Alerts", icon: rsx! { Icon { name: IconName::Notifications } } }
                }
                Tabs { default_value: "home", color: Color::Primary, aria_label: "Sections",
                    Tab { value: "home", label: "Home", icon: rsx! { Icon { name: IconName::Home } }, icon_position: IconPosition::Start }
                    Tab { value: "people", label: "People", icon: rsx! { Icon { name: IconName::Person } }, icon_position: IconPosition::Start }
                    Tab { value: "settings", label: "Settings", icon: rsx! { Icon { name: IconName::Settings } }, icon_position: IconPosition::End }
                }
                Tabs { default_value: "fav", aria_label: "Icon tabs",
                    Tab { value: "fav", icon: rsx! { Icon { name: IconName::Favorite, title: "Favorites" } } }
                    Tab { value: "star", icon: rsx! { Icon { name: IconName::Star, title: "Starred" } } }
                    Tab { value: "mark", icon: rsx! { Icon { name: IconName::Bookmark, title: "Bookmarks" } } }
                }
            }
            DemoRow { label: "sizes",
                for size in [Size::Small, Size::Medium, Size::Large] {
                    Tabs { default_value: "day", size, aria_label: "Range",
                        Tab { value: "day", label: "Day" }
                        Tab { value: "week", label: "Week" }
                        Tab { value: "month", label: "Month" }
                    }
                }
            }
            div { style: "display: grid; grid-template-columns: minmax(0, 1fr) minmax(0, 1fr); gap: 20px; margin-top: 6px;",
                Cell { caption: "variant = full_width",
                    Tabs { default_value: "am", variant: TabsVariant::FullWidth, aria_label: "Shift",
                        Tab { value: "am", label: "Morning" }
                        Tab { value: "pm", label: "Afternoon" }
                        Tab { value: "night", label: "Night" }
                    }
                }
                Cell { caption: "centered",
                    Tabs { default_value: "b", centered: true, aria_label: "Channel",
                        Tab { value: "a", label: "FM" }
                        Tab { value: "b", label: "DAB+" }
                        Tab { value: "c", label: "Stream" }
                    }
                }
                Cell { caption: "variant = scrollable",
                    Tabs { default_value: "d3", variant: TabsVariant::Scrollable, aria_label: "Days",
                        for (i, day) in ["Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"].iter().enumerate() {
                            Tab { key: "{i}", value: "d{i}", label: *day }
                        }
                    }
                }
                Cell { caption: "orientation = vertical",
                    Tabs { default_value: "out", orientation: Orientation::Vertical, aria_label: "Settings",
                        Tab { value: "in", label: "Input", icon: rsx! { Icon { name: IconName::Mic } }, icon_position: IconPosition::Start }
                        Tab { value: "out", label: "Output", icon: rsx! { Icon { name: IconName::Volume } }, icon_position: IconPosition::Start }
                        Tab { value: "sched", label: "Schedule", icon: rsx! { Icon { name: IconName::Calendar } }, icon_position: IconPosition::Start }
                    }
                }
            }
        }
    }
}

#[component]
fn BreadcrumbsDemo() -> Element {
    let crumbs = |names: &[&str]| -> Vec<Element> {
        let last = names.len() - 1;
        names
            .iter()
            .enumerate()
            .map(|(i, n)| {
                let n = n.to_string();
                if i == last {
                    rsx! { span { "{n}" } }
                } else {
                    rsx! { Link { href: "#", underline: LinkUnderline::Hover, color: Color::Neutral, "{n}" } }
                }
            })
            .collect()
    };
    rsx! {
        Demo {
            title: "Breadcrumbs",
            description: "printed path · separator printed · collapsed middle = a small “…” keycap that expands",
            DemoRow { label: "default",
                Breadcrumbs { items: crumbs(&["Studio", "Shows", "Morning Drive"]) }
            }
            DemoRow { label: "separator",
                Breadcrumbs {
                    separator: rsx! { "/" },
                    items: crumbs(&["Library", "Jingles", "Station ID"]),
                }
                Breadcrumbs {
                    separator: rsx! { "›" },
                    items: vec![
                        rsx! { Link { href: "#", underline: LinkUnderline::Hover, Icon { name: IconName::Home } " Home" } },
                        rsx! { Link { href: "#", underline: LinkUnderline::Hover, "Settings" } },
                        rsx! { span { "Audio" } },
                    ],
                }
            }
            DemoRow { label: "max_items = 4",
                Breadcrumbs {
                    max_items: 4,
                    items: crumbs(&["Station", "2026", "October", "Week 42", "Thursday", "06:00 News"]),
                }
                Breadcrumbs {
                    max_items: 4,
                    items_before_collapse: 2,
                    items_after_collapse: 2,
                    items: crumbs(&["Station", "Archive", "2025", "December", "Week 51", "Friday", "Countdown"]),
                }
            }
        }
    }
}

#[component]
fn LinkDemo() -> Element {
    let mut clicks = use_signal(|| 0u32);
    rsx! {
        Demo {
            title: "Link",
            description: "ink printed on the aluminium in --c-ink · underline none / hover / always",
            DemoRow { label: "underline",
                Link { href: "#", underline: LinkUnderline::Always, "Always underlined" }
                Link { href: "#", underline: LinkUnderline::Hover, "Underline on hover" }
                Link { href: "#", underline: LinkUnderline::None, "No underline" }
            }
            DemoRow { label: "colors",
                for color in Color::ALL {
                    Link { href: "#", color, "{color.as_str()}" }
                }
            }
            DemoRow { label: "in text",
                p { style: "margin: 0; font-size: 13px; color: var(--sk-ink-2); max-width: 60ch;",
                    "Read the "
                    Link { href: "#", "broadcast guidelines" }
                    " before going live, or "
                    Link { onclick: move |_| clicks += 1, "press this button link" }
                    " ({clicks})."
                }
            }
        }
    }
}

#[component]
fn EditMenuItems() -> Element {
    rsx! {
        MenuItem { icon: rsx! { Icon { name: IconName::Edit } }, shortcut: "⌘X", "Cut" }
        MenuItem { icon: rsx! { Icon { name: IconName::Copy } }, shortcut: "⌘C", "Copy" }
        MenuItem { icon: rsx! { Icon { name: IconName::Folder } }, shortcut: "⌘V", divider: true, "Paste" }
        MenuItem { icon: rsx! { Icon { name: IconName::Delete } }, shortcut: "⌫", disabled: true, "Delete" }
        MenuItem { icon: rsx! { Icon { name: IconName::Settings } }, shortcut: "⌘,", "Preferences…" }
    }
}

#[component]
fn MenuDemo() -> Element {
    let mut open = use_signal(|| false);
    let mut last = use_signal(|| "nothing yet".to_string());
    let mut speed = use_signal(|| "1×".to_string());
    rsx! {
        Demo {
            title: "Menu",
            description: "level-3 floating chassis · hover = --c-main highlight with white text, as on macOS · selected = check · ⌘ legends on the right",
            div { style: "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 32px;",
                Cell { caption: "open (preview)",
                    Menu {
                        open: true,
                        inline: true,
                        anchor: rsx! {
                            Button { variant: ButtonVariant::Outlined, color: Color::Neutral, end_icon: rsx! { Icon { name: IconName::ChevronDown } }, "Edit" }
                        },
                        EditMenuItems {}
                    }
                }
                Cell { caption: "MenuList · selected · dense",
                    div { class: "skeuo-chassis skeuo-elev-3 skeuo-chassis--float", style: "padding: 5px; border-radius: 10px; width: 200px;",
                        MenuList { dense: true, aria_label: "Playback speed",
                            for s in ["0.5×", "1×", "1.5×", "2×"] {
                                MenuItem {
                                    key: "{s}",
                                    selected: speed() == s,
                                    onclick: move |_| speed.set(s.to_string()),
                                    "{s}"
                                }
                            }
                        }
                    }
                }
                Cell { caption: "interactive",
                    Menu {
                        open: open(),
                        onclose: move |_| open.set(false),
                        aria_labelledby: "nav-demo-menu-trigger",
                        anchor: rsx! {
                            Button {
                                id: "nav-demo-menu-trigger",
                                aria_haspopup: "menu",
                                aria_expanded: if open() { "true" } else { "false" },
                                end_icon: rsx! { Icon { name: IconName::ChevronDown } },
                                onclick: move |_| open.toggle(),
                                "Clip actions"
                            }
                        },
                        for (label, icon, key) in [("Play", IconName::Play, "Space"), ("Pause", IconName::Pause, "⌘."), ("Bookmark", IconName::Bookmark, "⌘D")] {
                            MenuItem {
                                key: "{label}",
                                icon: rsx! { Icon { name: icon } },
                                shortcut: key,
                                onclick: move |_| {
                                    last.set(label.to_string());
                                    open.set(false);
                                },
                                "{label}"
                            }
                        }
                    }
                    span { class: "skeuo-caption", "Last action: {last}" }
                }
            }
        }
    }
}

#[component]
fn PopoverDemo() -> Element {
    let mut open = use_signal(|| false);
    rsx! {
        Demo {
            title: "Popover",
            description: "the same floating chassis, without menu semantics · placement bottom / top / left / right",
            div { style: "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 32px;",
                Cell { caption: "open (preview)",
                    Popover {
                        open: true,
                        inline: true,
                        aria_label: "Now playing",
                        anchor: rsx! {
                            Button { variant: ButtonVariant::Outlined, color: Color::Neutral, start_icon: rsx! { Icon { name: IconName::Info } }, "Now playing" }
                        },
                        div { style: "display: flex; flex-direction: column; gap: 4px; width: 220px;",
                            b { style: "font-size: 13px;", "Morning Drive" }
                            span { class: "skeuo-caption", "06:00 – 09:00 · Studio A" }
                            span { class: "skeuo-caption", "Next: Traffic & Weather at 06:15" }
                        }
                    }
                }
                Cell { caption: "interactive (placement = right)",
                    Popover {
                        open: open(),
                        onclose: move |_| open.set(false),
                        placement: Placement::Right,
                        aria_label: "Levels",
                        anchor: rsx! {
                            Button { onclick: move |_| open.toggle(), "Show levels" }
                        },
                        div { style: "width: 180px; font-size: 13px;",
                            b { "Levels" }
                            p { class: "skeuo-caption", style: "margin: 4px 0 0;", "Mic −12 dB · Music −18 dB. Click outside or press Esc to close." }
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn PaginationDemo() -> Element {
    let mut page = use_signal(|| 4usize);
    rsx! {
        Demo {
            title: "Pagination",
            description: "page numbers as small keycaps · current page = latched-down colored key · ellipsis printed",
            DemoRow { label: "interactive",
                Pagination { count: 12, page: page(), onchange: move |p| page.set(p) }
                span { class: "skeuo-caption", "Page {page} of 12" }
            }
            DemoRow { label: "outlined",
                Pagination { count: 10, default_page: 5, variant: PaginationVariant::Outlined }
            }
            DemoRow { label: "rounded",
                Pagination { count: 10, default_page: 2, variant: PaginationVariant::Outlined, shape: PaginationShape::Rounded, color: Color::Secondary }
            }
            DemoRow { label: "colors",
                for color in [Color::Primary, Color::Success, Color::Warning, Color::Neutral] {
                    Pagination { count: 3, default_page: 2, color, hide_prev_button: true, hide_next_button: true }
                }
            }
            DemoRow { label: "sizes",
                Pagination { count: 5, default_page: 2, size: Size::Small }
                Pagination { count: 5, default_page: 2, size: Size::Large }
            }
            DemoRow { label: "first / last",
                Pagination { count: 20, default_page: 10, siblings_count: 0, show_first_button: true, show_last_button: true, variant: PaginationVariant::Outlined }
            }
            DemoRow { label: "boundary 2",
                Pagination { count: 30, default_page: 15, boundary_count: 2 }
            }
            DemoRow { label: "disabled",
                Pagination { count: 6, default_page: 3, disabled: true }
            }
        }
    }
}

const STEPS: [&str; 4] = ["Prepare script", "Record voice", "Mix & master", "Publish"];

#[component]
fn StepperDemo() -> Element {
    let mut active = use_signal(|| 1usize);
    let mut nl_active = use_signal(|| 2usize);
    let mut mobile = use_signal(|| 2usize);
    rsx! {
        Demo {
            title: "Stepper",
            description: "round keycap indicators · active = colored key lit like an LED · completed = check · connectors = grooves that fill",
            Stepper { active_step: active(),
                for label in STEPS {
                    Step { key: "{label}", StepLabel { "{label}" } }
                }
            }
            div { style: "display: flex; gap: 10px; justify-content: flex-end; margin: 16px 0 26px;",
                Button { variant: ButtonVariant::Text, disabled: active() == 0, onclick: move |_| active -= 1, "Back" }
                Button {
                    disabled: active() >= STEPS.len(),
                    onclick: move |_| active += 1,
                    if active() + 1 >= STEPS.len() { "Finish" } else { "Next" }
                }
                Button { variant: ButtonVariant::Outlined, color: Color::Neutral, onclick: move |_| active.set(0), "Reset" }
            }
            Cell { caption: "alternative_label · optional · error",
                Stepper { active_step: 2, alternative_label: true,
                    Step { StepLabel { "Book guest" } }
                    Step {
                        StepLabel {
                            error: true,
                            optional: rsx! { "Line check failed" },
                            "Phone line"
                        }
                    }
                    Step {
                        StepLabel { optional: rsx! { "Optional" }, "Pre-record intro" }
                    }
                    Step { StepLabel { "Go live" } }
                }
            }
            div { style: "display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 28px; margin-top: 26px;",
                Cell { caption: "orientation = vertical · StepContent",
                    Stepper { active_step: 1, orientation: Orientation::Vertical,
                        Step {
                            StepLabel { "Select a show" }
                            StepContent { "Pick the slot you want to fill." }
                        }
                        Step {
                            StepLabel { optional: rsx! { "About 2 minutes" }, "Record the announcement" }
                            StepContent {
                                p { style: "margin: 0 0 10px;", "Read the script at a steady pace; the meter should stay in the green." }
                                Button { size: Size::Small, start_icon: rsx! { Icon { name: IconName::Dot } }, color: Color::Error, "Record" }
                            }
                        }
                        Step {
                            StepLabel { "Schedule" }
                            StepContent { "Choose when it airs." }
                        }
                    }
                }
                Cell { caption: "non_linear · StepButton · color = success",
                    Stepper { active_step: nl_active(), non_linear: true, orientation: Orientation::Vertical, color: Color::Success,
                        for (i, label) in STEPS.iter().enumerate() {
                            Step { key: "{i}", completed: i == 0 || i == 3,
                                StepButton { onclick: move |_| nl_active.set(i), "{label}" }
                            }
                        }
                    }
                }
            }
            div { style: "display: grid; grid-template-columns: repeat(auto-fill, minmax(260px, 1fr)); gap: 20px; margin-top: 26px;",
                Cell { caption: "MobileStepper · dots",
                    MobileStepper {
                        steps: 5,
                        active_step: mobile(),
                        position: MobileStepperPosition::Static,
                        back_button: rsx! {
                            Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 0, onclick: move |_| mobile -= 1, start_icon: rsx! { Icon { name: IconName::ChevronLeft } }, "Back" }
                        },
                        next_button: rsx! {
                            Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 4, onclick: move |_| mobile += 1, end_icon: rsx! { Icon { name: IconName::ChevronRight } }, "Next" }
                        },
                    }
                }
                Cell { caption: "MobileStepper · progress",
                    MobileStepper {
                        steps: 5,
                        active_step: mobile(),
                        variant: MobileStepperVariant::Progress,
                        position: MobileStepperPosition::Static,
                        back_button: rsx! { Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 0, onclick: move |_| mobile -= 1, "Back" } },
                        next_button: rsx! { Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 4, onclick: move |_| mobile += 1, "Next" } },
                    }
                }
                Cell { caption: "MobileStepper · text",
                    MobileStepper {
                        steps: 5,
                        active_step: mobile(),
                        variant: MobileStepperVariant::Text,
                        position: MobileStepperPosition::Static,
                        back_button: rsx! { Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 0, onclick: move |_| mobile -= 1, "Back" } },
                        next_button: rsx! { Button { size: Size::Small, variant: ButtonVariant::Text, disabled: mobile() == 4, onclick: move |_| mobile += 1, "Next" } },
                    }
                }
            }
        }
    }
}

#[component]
fn BottomNavigationDemo() -> Element {
    let mut value = use_signal(|| "live".to_string());
    rsx! {
        Demo {
            title: "BottomNavigation",
            description: "a chassis bar of keys · selected = latched key, legend and icon in --c-ink",
            DemoGrid {
                Cell { caption: "show_labels · interactive",
                    BottomNavigation {
                        value: value(),
                        onchange: move |v: String| value.set(v),
                        show_labels: true,
                        BottomNavigationAction { value: "live", label: "On air", icon: rsx! { Icon { name: IconName::Mic } } }
                        BottomNavigationAction { value: "library", label: "Library", icon: rsx! { Icon { name: IconName::Folder } } }
                        BottomNavigationAction { value: "schedule", label: "Schedule", icon: rsx! { Icon { name: IconName::Calendar } } }
                        BottomNavigationAction { value: "me", label: "Profile", icon: rsx! { Icon { name: IconName::Person } } }
                    }
                }
                Cell { caption: "labels on selected only · color = error",
                    BottomNavigation { default_value: "fav", color: Color::Error,
                        BottomNavigationAction { value: "recent", label: "Recents", icon: rsx! { Icon { name: IconName::Calendar } } }
                        BottomNavigationAction { value: "fav", label: "Favorites", icon: rsx! { Icon { name: IconName::Favorite } } }
                        BottomNavigationAction { value: "near", label: "Nearby", icon: rsx! { Icon { name: IconName::Search } } }
                    }
                }
            }
        }
    }
}

#[component]
fn SpeedDialActions() -> Element {
    rsx! {
        SpeedDialAction { icon: rsx! { Icon { name: IconName::Mic } }, tooltip_title: "Go live", tooltip_open: true }
        SpeedDialAction { icon: rsx! { Icon { name: IconName::Play } }, tooltip_title: "Play jingle", tooltip_open: true }
        SpeedDialAction { icon: rsx! { Icon { name: IconName::Bookmark } }, tooltip_title: "Mark", tooltip_open: true }
    }
}

#[component]
fn SpeedDialDemo() -> Element {
    let mut open = use_signal(|| false);
    rsx! {
        Demo {
            title: "SpeedDial",
            description: "a large round colored keycap · actions fan out as small round white keycaps with printed tooltips",
            div { style: "display: flex; flex-wrap: wrap; align-items: flex-start; gap: 56px;",
                Cell { caption: "open · direction = up · tooltip_open",
                    div { style: "display: flex; align-items: flex-end; height: 240px; padding-left: 104px;",
                        SpeedDial { aria_label: "Studio actions", open: true, SpeedDialActions {} }
                    }
                }
                Cell { caption: "direction = right · color = error",
                    div { style: "display: flex; align-items: flex-end; height: 240px;",
                    SpeedDial {
                        aria_label: "Recorder actions",
                        open: true,
                        direction: SpeedDialDirection::Right,
                        color: Color::Error,
                        icon: rsx! { SpeedDialIcon { icon: rsx! { Icon { name: IconName::Dot } }, open_icon: rsx! { Icon { name: IconName::Close } } } },
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Pause } }, tooltip_title: "Pause" }
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Delete } }, tooltip_title: "Discard" }
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Check } }, tooltip_title: "Save" }
                    }
                    }
                }
                Cell { caption: "interactive · direction = left",
                    div { style: "display: flex; align-items: flex-end; justify-content: flex-end; height: 240px; min-width: 240px;",
                    SpeedDial {
                        aria_label: "Clip actions",
                        open: open(),
                        onopen: move |_| open.set(true),
                        onclose: move |_| open.set(false),
                        direction: SpeedDialDirection::Left,
                        color: Color::Secondary,
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Copy } }, tooltip_title: "Duplicate" }
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Edit } }, tooltip_title: "Rename" }
                        SpeedDialAction { icon: rsx! { Icon { name: IconName::Star } }, tooltip_title: "Favorite" }
                    }
                    }
                }
            }
        }
    }
}

#[component]
fn TimelineDemo() -> Element {
    let rundown = [
        (
            "06:00",
            "News headlines",
            "Top stories and the weather",
            Some(Color::Success),
            TimelineDotVariant::Filled,
            IconName::Mic,
        ),
        (
            "06:05",
            "Morning Drive",
            "Live — with traffic every 15 min",
            Some(Color::Error),
            TimelineDotVariant::Filled,
            IconName::Dot,
        ),
        (
            "06:30",
            "Sports desk",
            "Pre-recorded, 4:30",
            Some(Color::Primary),
            TimelineDotVariant::Outlined,
            IconName::Play,
        ),
        (
            "06:45",
            "Listener calls",
            "Phone lines open",
            Some(Color::Warning),
            TimelineDotVariant::Outlined,
            IconName::Notifications,
        ),
        (
            "07:00",
            "News on the hour",
            "Standby",
            None,
            TimelineDotVariant::Outlined,
            IconName::Calendar,
        ),
    ];
    rsx! {
        Demo {
            title: "Timeline",
            description: "LEDs joined by a groove · filled = lit LED · outlined = LED off · position left / right / alternate",
            Cell { caption: "broadcast run-down · position = alternate",
                Timeline { position: TimelinePosition::Alternate,
                    for (i, (time, title, note, color, variant, icon)) in rundown.into_iter().enumerate() {
                        TimelineItem { key: "{i}",
                            TimelineOppositeContent { "{time}" }
                            TimelineSeparator {
                                TimelineDot { color, variant, Icon { name: icon } }
                                if i + 1 < rundown.len() {
                                    TimelineConnector {}
                                }
                            }
                            TimelineContent {
                                b { style: "font-weight: 600;", "{title}" }
                                div { class: "skeuo-caption", "{note}" }
                            }
                        }
                    }
                }
            }
            div { style: "display: grid; grid-template-columns: repeat(auto-fill, minmax(240px, 1fr)); gap: 20px; margin-top: 18px;",
                Cell { caption: "position = right · LEDs",
                    Timeline {
                        for (i, (label, color)) in [("Mic check", Some(Color::Success)), ("Sound check", Some(Color::Success)), ("On air", Some(Color::Error)), ("Wrap", None)].into_iter().enumerate() {
                            TimelineItem { key: "{i}",
                                TimelineSeparator {
                                    TimelineDot { color }
                                    if i < 3 { TimelineConnector {} }
                                }
                                TimelineContent { "{label}" }
                            }
                        }
                    }
                }
                Cell { caption: "position = left · outlined",
                    Timeline { position: TimelinePosition::Left,
                        for (i, (label, time)) in [("Uploaded", "09:12"), ("Transcoded", "09:14"), ("Scheduled", "09:20")].into_iter().enumerate() {
                            TimelineItem { key: "{i}",
                                TimelineOppositeContent { "{time}" }
                                TimelineSeparator {
                                    TimelineDot { color: Color::Info, variant: TimelineDotVariant::Outlined }
                                    if i < 2 { TimelineConnector {} }
                                }
                                TimelineContent { "{label}" }
                            }
                        }
                    }
                }
                Cell { caption: "colors",
                    Timeline {
                        for (i, color) in Color::ALL.into_iter().enumerate() {
                            TimelineItem { key: "{i}",
                                TimelineSeparator {
                                    TimelineDot { color: Some(color) }
                                    if i + 1 < Color::ALL.len() { TimelineConnector {} }
                                }
                                TimelineContent { "{color.as_str()}" }
                            }
                        }
                    }
                }
            }
        }
    }
}
