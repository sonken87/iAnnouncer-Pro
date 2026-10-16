use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

#[component]
pub fn DataDisplaySection() -> Element {
    rsx! {
        TypographyDemo {}
        AvatarDemo {}
        BadgeDemo {}
        ChipDemo {}
        DividerDemo {}
        ListDemo {}
        TableDemo {}
        TooltipDemo {}
        ImageListDemo {}
    }
}

/// A landscape "photo" as an inline SVG data URI (the gallery ships no
/// image files). `h` sets the aspect ratio (width is 400).
fn photo(sky_top: &str, sky_bottom: &str, sun: &str, hill: &str, ground: &str, h: u32) -> String {
    let h1 = h * 70 / 100;
    let h2 = h * 85 / 100;
    let svg = format!(
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 400 {h}' preserveAspectRatio='xMidYMid slice'>\
         <defs><linearGradient id='s' x1='0' y1='0' x2='0' y2='1'>\
         <stop offset='0' stop-color='{sky_top}'/><stop offset='1' stop-color='{sky_bottom}'/>\
         </linearGradient></defs>\
         <rect width='400' height='{h}' fill='url(#s)'/>\
         <circle cx='290' cy='{sun_y}' r='34' fill='{sun}' opacity='.9'/>\
         <path d='M0 {h1} Q100 {hill_peak} 200 {h1} T400 {hill_end} V{h} H0Z' fill='{hill}' opacity='.75'/>\
         <path d='M0 {h2} Q120 {ground_peak} 240 {h2} T400 {h2} V{h} H0Z' fill='{ground}'/></svg>",
        sun_y = h * 35 / 100,
        hill_peak = h * 50 / 100,
        hill_end = h * 62 / 100,
        ground_peak = h * 72 / 100,
    );
    let encoded = svg
        .replace('%', "%25")
        .replace('#', "%23")
        .replace('<', "%3C")
        .replace('>', "%3E");
    format!("data:image/svg+xml;utf8,{encoded}")
}

const PHOTOS: [(&str, &str, &str, &str, &str, &str); 9] = [
    (
        "Dawn", "#ffb88c", "#ffe2c6", "#fff4d6", "#d9826a", "#8e5a63",
    ),
    (
        "Coast", "#5ac8fa", "#d6f0ff", "#ffffff", "#3a8fb7", "#1f5f7a",
    ),
    (
        "Meadow", "#a8e063", "#eef8d8", "#fffbe0", "#5f9c3a", "#3d6e2a",
    ),
    (
        "Dusk", "#5e5ce6", "#ff9fb2", "#ffd7a8", "#6b4a8e", "#3b2a55",
    ),
    (
        "Desert", "#ffd28a", "#fff0d1", "#fff8ea", "#d79b55", "#a0683a",
    ),
    (
        "Glacier", "#c7e6ff", "#f2f9ff", "#ffffff", "#8fb6d9", "#5d84a8",
    ),
    (
        "Forest", "#7fc8a9", "#e3f4ea", "#f8ffe8", "#3f7d5f", "#25523c",
    ),
    (
        "Night", "#1c2541", "#3a506b", "#f2f2d0", "#2c3e5c", "#141b2d",
    ),
    (
        "Studio", "#d1d1d6", "#f2f2f7", "#ffffff", "#a1a1a6", "#6e6e73",
    ),
];

fn photo_n(i: usize, h: u32) -> String {
    let (_, a, b, c, d, e) = PHOTOS[i % PHOTOS.len()];
    photo(a, b, c, d, e, h)
}

#[component]
fn TypographyDemo() -> Element {
    rsx! {
        Demo {
            title: "Typography",
            description: "printed lettering on the aluminium · SF Pro scale with tight heading tracking · no text effects",
            div { style: "display: grid; grid-template-columns: 1.4fr 1fr; gap: 28px; align-items: start; margin-bottom: 18px",
                div {
                    for variant in [
                        TypographyVariant::H1,
                        TypographyVariant::H2,
                        TypographyVariant::H3,
                        TypographyVariant::H4,
                        TypographyVariant::H5,
                        TypographyVariant::H6,
                    ] {
                        Typography { variant, gutter_bottom: true, "{variant.as_str()}. On Air" }
                    }
                }
                div {
                    Typography { variant: TypographyVariant::Overline, gutter_bottom: true, "Overline · now playing" }
                    Typography { variant: TypographyVariant::Subtitle1,
                        "subtitle1. Morning drive with the station team"
                    }
                    Typography { variant: TypographyVariant::Subtitle2, gutter_bottom: true,
                        "subtitle2. Segment 3 of 8"
                    }
                    Typography { variant: TypographyVariant::Body1, paragraph: true,
                        "body1. Every announcement is cut like a keycap legend: crisp, even and quietly confident, printed straight onto the surface."
                    }
                    Typography { variant: TypographyVariant::Body2, color: TypographyColor::TextSecondary, paragraph: true,
                        "body2 · text.secondary. Secondary copy steps back in a softer grey ink."
                    }
                    Typography { variant: TypographyVariant::Caption, component: "div", gutter_bottom: true,
                        "caption. Last edited 2 minutes ago"
                    }
                    Typography { variant: TypographyVariant::Button, component: "div", "button legend" }
                }
            }
            DemoRow { label: "palette ink",
                for color in Color::ALL {
                    Typography { variant: TypographyVariant::Subtitle2, color, component: "span", "{color.as_str()}" }
                }
                Typography { variant: TypographyVariant::Subtitle2, color: TypographyColor::TextDisabled, component: "span", "text.disabled" }
            }
            DemoRow { label: "align / wrap",
                div { style: "width: 220px",
                    Typography { variant: TypographyVariant::Body2, align: TypographyAlign::Right, "right aligned" }
                    Typography { variant: TypographyVariant::Body2, align: TypographyAlign::Center, "centered" }
                    Typography { variant: TypographyVariant::Body2, no_wrap: true,
                        "no_wrap truncates a long line with an ellipsis instead of wrapping"
                    }
                }
            }
        }
    }
}

#[component]
fn AvatarDemo() -> Element {
    let mut extra = use_signal(|| 7usize);
    let group = |n: usize| -> Vec<Element> {
        let initials = ["JA", "MK", "SO", "LT", "RB", "CN", "EV", "PD"];
        let colors = [
            Some(Color::Primary),
            Some(Color::Secondary),
            None,
            Some(Color::Success),
            Some(Color::Warning),
            Some(Color::Info),
            None,
            Some(Color::Error),
        ];
        (0..n)
            .map(|i| {
                let color = colors[i % colors.len()];
                let initial = initials[i % initials.len()];
                if i == 2 {
                    rsx! { Avatar { src: photo_n(3, 400), alt: "Sora" } }
                } else {
                    rsx! { Avatar { color, alt: initial, "{initial}" } }
                }
            })
            .collect()
    };
    rsx! {
        Demo {
            title: "Avatar",
            description: "a small keycap carrying initials or a glyph · photos inset into a thin well ring · circular / rounded / square",
            DemoRow { label: "letters",
                Avatar { alt: "Jamie Apple", "JA" }
                Avatar { color: Color::Primary, alt: "Mika", "MK" }
                Avatar { color: Color::Secondary, alt: "Sol", "SO" }
                Avatar { color: Color::Success, alt: "Lee", "LT" }
                Avatar { color: Color::Warning, alt: "Rae", "RB" }
                Avatar { color: Color::Error, alt: "Nico", "N" }
                Avatar { color: Color::Info, alt: "Eve" }
            }
            DemoRow { label: "icons",
                Avatar { Icon { name: IconName::Person } }
                Avatar { color: Color::Primary, Icon { name: IconName::Mic } }
                Avatar { color: Color::Success, variant: AvatarVariant::Rounded, Icon { name: IconName::Folder } }
                Avatar { color: Color::Error, variant: AvatarVariant::Square, Icon { name: IconName::Favorite } }
            }
            DemoRow { label: "photos",
                Avatar { src: photo_n(0, 400), alt: "Dawn", size: Size::Small }
                Avatar { src: photo_n(1, 400), alt: "Coast" }
                Avatar { src: photo_n(3, 400), alt: "Dusk", size: Size::Large }
                Avatar { src: photo_n(2, 400), alt: "Meadow", size: Size::Large, variant: AvatarVariant::Rounded }
                Avatar { src: photo_n(4, 400), alt: "Desert", size: Size::Large, variant: AvatarVariant::Square }
            }
            DemoRow { label: "sizes",
                Avatar { size: Size::Small, color: Color::Primary, "S" }
                Avatar { size: Size::Medium, color: Color::Primary, "M" }
                Avatar { size: Size::Large, color: Color::Primary, "L" }
                Avatar { size: Size::Small, variant: AvatarVariant::Rounded, "S" }
                Avatar { size: Size::Medium, variant: AvatarVariant::Rounded, "M" }
                Avatar { size: Size::Large, variant: AvatarVariant::Rounded, "L" }
            }
            DemoRow { label: "AvatarGroup",
                AvatarGroup { max: 4, avatars: group(6) }
                AvatarGroup { max: 5, total: 24, spacing: AvatarGroupSpacing::Small, avatars: group(4) }
                AvatarGroup { max: 6, avatars: group(extra()) }
                Button {
                    variant: ButtonVariant::Outlined,
                    size: Size::Small,
                    onclick: move |_| extra += 1,
                    "Add"
                }
                Button {
                    variant: ButtonVariant::Outlined,
                    color: Color::Neutral,
                    size: Size::Small,
                    disabled: extra() == 0,
                    onclick: move |_| extra -= 1,
                    "Remove"
                }
            }
        }
    }
}

#[component]
fn BadgeDemo() -> Element {
    const SPACED: &str =
        "display: flex; flex-wrap: wrap; align-items: center; gap: 22px 30px; padding: 6px 4px";
    let mut count = use_signal(|| 4i32);
    let mut hidden = use_signal(|| false);
    let icon = || rsx! { Icon { name: IconName::Mail, size: "24px" } };
    let tile = |label: &'static str| rsx! { Avatar { variant: AvatarVariant::Rounded, "{label}" } };
    rsx! {
        Demo {
            title: "Badge",
            description: "an indicator LED on the corner of its child · standard pill or dot · any palette color · four anchors",
            DemoRow { label: "colors",
                div { style: SPACED,
                for (i, color) in Color::ALL.into_iter().enumerate() {
                    Badge { color, badge_content: i as i32 + 1, {icon()} }
                }
                }
            }
            DemoRow { label: "max / zero",
                div { style: SPACED,
                Badge { badge_content: 99, {icon()} }
                Badge { badge_content: 100, {icon()} }
                Badge { badge_content: 1000, max: 999, {icon()} }
                Badge { badge_content: 0, {icon()} }
                Badge { badge_content: 0, show_zero: true, color: Color::Neutral, {icon()} }
                Badge { badge_content: "new", color: Color::Success, {icon()} }
                Badge { variant: BadgeVariant::Dot, color: Color::Error, Icon { name: IconName::Notifications, size: "24px" } }
                Badge { variant: BadgeVariant::Dot, color: Color::Success, Typography { variant: TypographyVariant::Body2, "Live" } }
                }
            }
            DemoRow { label: "anchor",
                div { style: SPACED,
                Badge { badge_content: 3, anchor_origin: BadgeAnchorOrigin::TOP_RIGHT, {tile("TR")} }
                Badge { badge_content: 3, anchor_origin: BadgeAnchorOrigin::TOP_LEFT, color: Color::Secondary, {tile("TL")} }
                Badge { badge_content: 3, anchor_origin: BadgeAnchorOrigin::BOTTOM_RIGHT, color: Color::Success, {tile("BR")} }
                Badge { badge_content: 3, anchor_origin: BadgeAnchorOrigin::BOTTOM_LEFT, color: Color::Warning, {tile("BL")} }
                Badge { overlap: BadgeOverlap::Circular, variant: BadgeVariant::Dot, color: Color::Success,
                    anchor_origin: BadgeAnchorOrigin::BOTTOM_RIGHT,
                    Avatar { src: photo_n(1, 400), alt: "Online" }
                }
                Badge { overlap: BadgeOverlap::Circular, badge_content: 12,
                    Avatar { color: Color::Info, "IA" }
                }
                }
            }
            DemoRow { label: "interactive",
                Badge { badge_content: count(), invisible: hidden(), max: 9, color: Color::Error,
                    Avatar { variant: AvatarVariant::Rounded, Icon { name: IconName::Notifications } }
                }
                Button { size: Size::Small, variant: ButtonVariant::Outlined, onclick: move |_| count -= 1, disabled: count() == 0, "−" }
                Button { size: Size::Small, variant: ButtonVariant::Outlined, onclick: move |_| count += 1, "+" }
                Button {
                    size: Size::Small,
                    variant: ButtonVariant::Text,
                    onclick: move |_| hidden.toggle(),
                    if hidden() { "Show badge" } else { "Hide badge" }
                }
            }
        }
    }
}

#[component]
fn ChipDemo() -> Element {
    let mut tags = use_signal(|| vec!["News", "Weather", "Traffic", "Sports", "Music"]);
    let mut picked = use_signal(|| 1usize);
    let filters = ["All", "Live", "Recorded", "Scheduled"];
    rsx! {
        Demo {
            title: "Chip",
            description: "filled = small colored keycap pill · outlined = white keycap with colored ink · clickable chips press like keys",
            DemoRow { label: "filled",
                for color in Color::ALL {
                    Chip { color, label: color.as_str() }
                }
            }
            DemoRow { label: "outlined",
                for color in Color::ALL {
                    Chip { color, variant: ChipVariant::Outlined, label: color.as_str() }
                }
            }
            DemoRow { label: "small",
                Chip { size: Size::Small, label: "default" }
                Chip { size: Size::Small, color: Color::Primary, label: "primary" }
                Chip { size: Size::Small, variant: ChipVariant::Outlined, color: Color::Success, label: "outlined" }
                Chip { size: Size::Small, color: Color::Error, icon: rsx! { Icon { name: IconName::Dot } }, label: "REC" }
                Chip { size: Size::Small, variant: ChipVariant::Outlined, label: "Delete me", on_delete: move |_| {} }
            }
            DemoRow { label: "avatar / icon",
                Chip { avatar: rsx! { Avatar { color: Color::Primary, "M" } }, label: "Mika" }
                Chip { variant: ChipVariant::Outlined, avatar: rsx! { Avatar { src: photo_n(5, 400), alt: "Glacier" } }, label: "Glacier" }
                Chip { color: Color::Success, icon: rsx! { Icon { name: IconName::CheckCircle } }, label: "Approved" }
                Chip { variant: ChipVariant::Outlined, color: Color::Warning, icon: rsx! { Icon { name: IconName::Warning } }, label: "Clipping" }
                Chip { label: "Disabled", disabled: true, on_delete: move |_| {} }
                Chip { variant: ChipVariant::Outlined, color: Color::Primary, label: "Disabled", disabled: true }
            }
            DemoRow { label: "clickable",
                for (i, f) in filters.into_iter().enumerate() {
                    Chip {
                        label: f,
                        color: if picked() == i { Color::Primary } else { Color::Neutral },
                        variant: if picked() == i { ChipVariant::Filled } else { ChipVariant::Outlined },
                        onclick: move |_| picked.set(i),
                    }
                }
            }
            DemoRow { label: "deletable",
                for tag in tags() {
                    Chip {
                        key: "{tag}",
                        label: tag,
                        color: Color::Info,
                        on_delete: move |_| tags.write().retain(|t| *t != tag),
                    }
                }
                if tags.read().is_empty() {
                    Button {
                        size: Size::Small,
                        variant: ButtonVariant::Text,
                        onclick: move |_| tags.set(vec!["News", "Weather", "Traffic", "Sports", "Music"]),
                        "Reset"
                    }
                }
            }
        }
    }
}

#[component]
fn DividerDemo() -> Element {
    rsx! {
        Demo {
            title: "Divider",
            description: "the engraved rule: a dark cut with a catch-light · full width / inset / middle · vertical · with text",
            DemoGrid {
                div { class: "skeuo-chassis skeuo-elev-1",
                    List { disable_padding: true,
                        ListItem { ListItemText { primary: "full_width" } }
                        Divider {}
                        ListItem {
                            ListItemAvatar { Avatar { size: Size::Small, color: Color::Primary, "A" } }
                            ListItemText { primary: "inset (lines up with text)" }
                        }
                        Divider { variant: DividerVariant::Inset }
                        ListItem {
                            ListItemAvatar { Avatar { size: Size::Small, color: Color::Success, "B" } }
                            ListItemText { primary: "middle" }
                        }
                        Divider { variant: DividerVariant::Middle }
                        ListItem { ListItemText { primary: "last row" } }
                    }
                }
                div { style: "display: flex; flex-direction: column; gap: 18px; justify-content: center",
                    Divider { "CENTER" }
                    Divider { text_align: DividerTextAlign::Left, "Left aligned" }
                    Divider { text_align: DividerTextAlign::Right,
                        Chip { size: Size::Small, label: "Chip in a rule" }
                    }
                    div { style: "display: flex; align-items: center; gap: 16px; height: 56px; padding: 0 4px; font-size: 20px; color: var(--sk-ink-2)",
                        Icon { name: IconName::Play }
                        Divider { orientation: Orientation::Vertical, flex_item: true }
                        Icon { name: IconName::Pause }
                        Divider { orientation: Orientation::Vertical, variant: DividerVariant::Middle, flex_item: true }
                        Icon { name: IconName::Volume }
                        Divider { orientation: Orientation::Vertical, flex_item: true, "OR" }
                        Icon { name: IconName::Mic }
                    }
                }
            }
        }
    }
}

#[component]
fn ListDemo() -> Element {
    let mut selected = use_signal(|| 1usize);
    let mut muted = use_signal(|| false);
    let nav = [
        (IconName::Home, "Home"),
        (IconName::Mic, "Live studio"),
        (IconName::Calendar, "Schedule"),
        (IconName::Folder, "Library"),
    ];
    rsx! {
        Demo {
            title: "List",
            description: "rows printed on the plate · selected rows get the color tint · rules between rows on request · subheaders and secondary actions",
            DemoGrid {
                div { class: "skeuo-chassis skeuo-elev-1",
                    List {
                        aria_label: "Sections",
                        subheader: rsx! { ListSubheader { "Studio" } },
                        for (i, (icon, label)) in nav.into_iter().enumerate() {
                            ListItem { disable_padding: true,
                                ListItemButton {
                                    selected: selected() == i,
                                    onclick: move |_| selected.set(i),
                                    ListItemIcon { Icon { name: icon } }
                                    ListItemText { primary: label }
                                }
                            }
                        }
                        ListItem { disable_padding: true,
                            ListItemButton { disabled: true,
                                ListItemIcon { Icon { name: IconName::Settings } }
                                ListItemText { primary: "Settings (disabled)" }
                            }
                        }
                    }
                }
                div { class: "skeuo-chassis skeuo-elev-1",
                    List {
                        subheader: rsx! { ListSubheader { color: Color::Primary, "On air today" } },
                        ListItem { divider: true,
                            secondary_action: rsx! {
                                Badge { variant: BadgeVariant::Dot, color: Color::Error, invisible: muted(),
                                    Icon { name: IconName::Mic, size: "18px" }
                                }
                            },
                            ListItemAvatar { Avatar { src: photo_n(0, 400), alt: "Morning" } }
                            ListItemText { primary: "Morning drive", secondary: "06:00 – 09:00 · Jamie & Mika" }
                        }
                        ListItem { divider: true,
                            secondary_action: rsx! { Icon { name: IconName::MoreVert, size: "18px" } },
                            ListItemAvatar { Avatar { color: Color::Secondary, "MT" } }
                            ListItemText { primary: "Midday talk", secondary: "11:00 – 13:00 · Sol" }
                        }
                        ListItem {
                            secondary_action: rsx! { Icon { name: IconName::MoreVert, size: "18px" } },
                            ListItemAvatar { Avatar { color: Color::Success, Icon { name: IconName::Volume } } }
                            ListItemText { primary: "Evening mix", secondary: "19:00 – 22:00 · Automated" }
                        }
                    }
                }
                div { class: "skeuo-chassis skeuo-elev-1",
                    List { dense: true, color: Color::Success,
                        subheader: rsx! { ListSubheader { "Dense · success tint" } },
                        ListItem { disable_padding: true,
                            ListItemButton { selected: !muted(), onclick: move |_| muted.set(false),
                                ListItemIcon { Icon { name: IconName::Volume } }
                                ListItemText { primary: "Monitor on" }
                            }
                        }
                        ListItem { disable_padding: true,
                            ListItemButton { selected: muted(), onclick: move |_| muted.set(true),
                                ListItemIcon { Icon { name: IconName::Remove } }
                                ListItemText { primary: "Muted" }
                            }
                        }
                        ListItem {
                            ListItemText { inset: true, primary: "Inset text", secondary: "No icon, aligned with the rows above" }
                        }
                        ListItem {
                            ListItemIcon { Icon { name: IconName::Info } }
                            ListItemText { primary: "Plain row" }
                        }
                    }
                }
            }
        }
    }
}

#[derive(Clone, Copy, PartialEq)]
struct Clip {
    name: &'static str,
    host: &'static str,
    secs: u32,
    plays: u32,
    level: f32,
}

const CLIPS: [Clip; 5] = [
    Clip {
        name: "Station ID",
        host: "Jamie",
        secs: 12,
        plays: 1820,
        level: -14.2,
    },
    Clip {
        name: "Weather bed",
        host: "Mika",
        secs: 45,
        plays: 310,
        level: -16.0,
    },
    Clip {
        name: "Traffic sting",
        host: "Sol",
        secs: 6,
        plays: 942,
        level: -12.8,
    },
    Clip {
        name: "News intro",
        host: "Lee",
        secs: 18,
        plays: 1204,
        level: -13.5,
    },
    Clip {
        name: "Sign-off",
        host: "Rae",
        secs: 30,
        plays: 88,
        level: -18.1,
    },
];

#[component]
fn TableDemo() -> Element {
    let mut order_by = use_signal(|| 3usize);
    let mut direction = use_signal(|| SortDirection::Desc);
    let mut picked = use_signal(|| Some("News intro"));
    let mut rows = CLIPS.to_vec();
    rows.sort_by(|a, b| {
        let ord = match order_by() {
            0 => a.name.cmp(b.name),
            1 => a.host.cmp(b.host),
            2 => a.secs.cmp(&b.secs),
            3 => a.plays.cmp(&b.plays),
            _ => a.level.total_cmp(&b.level),
        };
        if direction() == SortDirection::Desc {
            ord.reverse()
        } else {
            ord
        }
    });
    let total: u32 = CLIPS.iter().map(|c| c.plays).sum();
    let columns = [
        ("Clip", TableCellAlign::Left),
        ("Host", TableCellAlign::Left),
        ("Length (s)", TableCellAlign::Right),
        ("Plays", TableCellAlign::Right),
        ("Level (LUFS)", TableCellAlign::Right),
    ];
    rsx! {
        Demo {
            title: "Table",
            description: "a chassis holding printed column heads · engraved rules between rows · hover wash, tinted selection, sortable heads",
            TableContainer {
                Table { aria_label: "Clips",
                    TableHead {
                        TableRow {
                            for (i, (label, align)) in columns.into_iter().enumerate() {
                                TableCell {
                                    align,
                                    sort_direction: if order_by() == i { Some(direction()) } else { None },
                                    TableSortLabel {
                                        active: order_by() == i,
                                        direction: if order_by() == i { direction() } else { SortDirection::Asc },
                                        onclick: move |_| {
                                            if order_by() == i {
                                                direction.set(direction().toggled());
                                            } else {
                                                order_by.set(i);
                                                direction.set(SortDirection::Asc);
                                            }
                                        },
                                        "{label}"
                                    }
                                }
                            }
                        }
                    }
                    TableBody {
                        for clip in rows {
                            TableRow {
                                key: "{clip.name}",
                                hover: true,
                                selected: picked() == Some(clip.name),
                                onclick: move |_| {
                                    picked.set(if picked() == Some(clip.name) { None } else { Some(clip.name) })
                                },
                                TableCell { variant: TableCellVariant::Head, "{clip.name}" }
                                TableCell { "{clip.host}" }
                                TableCell { align: TableCellAlign::Right, "{clip.secs}" }
                                TableCell { align: TableCellAlign::Right, "{clip.plays}" }
                                TableCell { align: TableCellAlign::Right, "{clip.level:.1}" }
                            }
                        }
                    }
                    TableFooter {
                        TableRow {
                            TableCell { col_span: 3, "Total plays" }
                            TableCell { align: TableCellAlign::Right, "{total}" }
                            TableCell {}
                        }
                    }
                }
            }
            div { style: "display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 20px; margin-top: 20px",
                TableContainer { max_height: "188px",
                    Table { size: Size::Small, sticky_header: true, aria_label: "Schedule",
                        TableHead {
                            TableRow {
                                TableCell { "Time" }
                                TableCell { "Show" }
                                TableCell { align: TableCellAlign::Right, "Min" }
                            }
                        }
                        TableBody {
                            for (h, show, m) in [
                                ("06:00", "Morning drive", 180),
                                ("09:00", "News hour", 60),
                                ("10:00", "Requests", 60),
                                ("11:00", "Midday talk", 120),
                                ("13:00", "Lunch mix", 60),
                                ("14:00", "Afternoon", 180),
                                ("17:00", "Commute", 120),
                            ] {
                                TableRow { hover: true,
                                    TableCell { "{h}" }
                                    TableCell { "{show}" }
                                    TableCell { align: TableCellAlign::Right, "{m}" }
                                }
                            }
                        }
                    }
                }
                TableContainer { elevation: 0,
                    Table { size: Size::Small, color: Color::Success, aria_label: "Checklist",
                        TableHead {
                            TableRow {
                                TableCell { padding: TableCellPadding::Checkbox, "" }
                                TableCell { "Check (small, flush, success tint)" }
                            }
                        }
                        TableBody {
                            for (done, task) in [(true, "Levels set"), (true, "Mic check"), (false, "Cue intro")] {
                                TableRow { selected: done,
                                    TableCell { padding: TableCellPadding::Checkbox,
                                        span { style: "display: inline-flex; color: var(--c-ink); padding-left: 10px",
                                            Icon { name: if done { IconName::CheckCircle } else { IconName::Dot }, size: "16px" }
                                        }
                                    }
                                    TableCell { "{task}" }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn TooltipDemo() -> Element {
    let mut open = use_signal(|| true);
    rsx! {
        Demo {
            title: "Tooltip",
            description: "a small floating chassis (level 3) that rises in on hover and keyboard focus · four placements · optional arrow",
            div { style: "display: flex; flex-wrap: wrap; align-items: center; justify-content: space-around; gap: 24px; padding: 44px 64px 52px",
                Tooltip { title: "Above, with arrow", placement: Placement::Top, arrow: true, open: true,
                    Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "Top" }
                }
                Tooltip { title: "Below (MUI's default)", open: true,
                    Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "Bottom" }
                }
                Tooltip { title: "Left", placement: Placement::Left, arrow: true, open: true,
                    Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "Left" }
                }
                Tooltip { title: "Right, with arrow", placement: Placement::Right, arrow: true, open: true,
                    Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "Right" }
                }
            }
            DemoRow { label: "hover / focus",
                Tooltip { title: "Delete clip", arrow: true,
                    Button { color: Color::Error, start_icon: rsx! { Icon { name: IconName::Delete } }, "Hover me" }
                }
                Tooltip { title: "Shows on keyboard focus too", placement: Placement::Top,
                    Chip { label: "Tab to me", onclick: move |_| {} }
                }
                Tooltip { title: "Not interactive: the bubble ignores the pointer", disable_interactive: true, placement: Placement::Right,
                    Avatar { color: Color::Info, Icon { name: IconName::Info } }
                }
            }
            DemoRow { label: "controlled",
                Tooltip { title: "Controlled by the switch", placement: Placement::Right, arrow: true, open: open(),
                    Chip {
                        label: if open() { "open: Some(true)" } else { "open: Some(false)" },
                        color: if open() { Color::Primary } else { Color::Neutral },
                        onclick: move |_| open.toggle(),
                    }
                }
            }
        }
    }
}

#[component]
fn ImageListDemo() -> Element {
    let quilted = [(2, 2), (1, 1), (1, 1), (1, 2), (1, 1), (2, 1), (1, 1)];
    let masonry_heights = [300u32, 460, 260, 380, 520, 300, 420, 280];
    rsx! {
        Demo {
            title: "ImageList",
            description: "photos inset into wells · standard / quilted / masonry / woven · caption bars over or below the image",
            div { style: "display: grid; grid-template-columns: repeat(auto-fill, minmax(340px, 1fr)); gap: 28px 32px",
                div {
                    Typography { variant: TypographyVariant::Overline, gutter_bottom: true, "standard · bars" }
                    ImageList { cols: 3, gap: 8, row_height: 96,
                        for i in 0..6usize {
                            ImageListItem { key: "{i}",
                                img { src: photo_n(i, 300), alt: PHOTOS[i].0 }
                                if i < 3 {
                                    ImageListItemBar {
                                        title: PHOTOS[i].0,
                                        position: if i == 1 { ImageListItemBarPosition::Top } else { ImageListItemBarPosition::Bottom },
                                        action_icon: rsx! { Icon { name: IconName::StarBorder } },
                                    }
                                }
                            }
                        }
                    }
                }
                div {
                    Typography { variant: TypographyVariant::Overline, gutter_bottom: true, "quilted" }
                    ImageList { variant: ImageListVariant::Quilted, cols: 4, gap: 6, row_height: 64,
                        for (i, (cols, rows)) in quilted.into_iter().enumerate() {
                            ImageListItem { key: "{i}", cols, rows,
                                img { src: photo_n(i + 2, 300), alt: PHOTOS[(i + 2) % 9].0 }
                            }
                        }
                    }
                }
                div {
                    Typography { variant: TypographyVariant::Overline, gutter_bottom: true, "masonry" }
                    ImageList { variant: ImageListVariant::Masonry, cols: 3, gap: 8,
                        for (i, h) in masonry_heights.into_iter().enumerate() {
                            ImageListItem { key: "{i}",
                                img { src: photo_n(i + 4, h), alt: PHOTOS[(i + 4) % 9].0 }
                            }
                        }
                    }
                }
                div {
                    Typography { variant: TypographyVariant::Overline, gutter_bottom: true, "woven · bar below" }
                    ImageList { variant: ImageListVariant::Woven, cols: 3, gap: 8, row_height: 150,
                        for i in 0..6usize {
                            ImageListItem { key: "{i}",
                                img { src: photo_n(i + 1, 400), alt: PHOTOS[(i + 1) % 9].0 }
                                ImageListItemBar {
                                    title: PHOTOS[(i + 1) % 9].0,
                                    subtitle: "by Studio",
                                    position: ImageListItemBarPosition::Below,
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
