use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

/// Page-local helpers for the demos (not part of skeuo-ui).
const SURFACES_CSS: &str = r#"
.sd-scroll-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(260px, 1fr)); gap: 20px; }
.sd-log-row { display: flex; gap: 12px; padding: 6px 4px; font-size: 13px; border-bottom: 1px solid var(--sk-cut); box-shadow: 0 1px 0 var(--sk-catch); }
.sd-log-row:last-child { border-bottom: 0; box-shadow: none; }
.sd-log-time { font-variant-numeric: tabular-nums; color: var(--sk-ink-2); }
.sd-scroll-strip { display: flex; gap: 8px; padding: 6px 2px 10px; width: max-content; }
.sd-scroll-chip { display: flex; flex-direction: column; gap: 2px; padding: 8px 10px; font-size: 12px; white-space: nowrap; }
.sd-plate { width: 132px; height: 92px; padding: 12px; display: flex; flex-direction: column; justify-content: flex-end; }
.sd-plate b { font-size: 13px; font-weight: 600; }
.sd-avatar { width: 36px; height: 36px; display: inline-flex; align-items: center; justify-content: center; font: 600 13px/1 var(--skeuo-font); }
.sd-title { margin: 0 0 4px; font-size: 16px; font-weight: 600; letter-spacing: -.01em; }
.sd-text { margin: 0; font-size: 13px; color: var(--sk-ink-2); }
.sd-icon-btn { min-width: 0 !important; padding: 0 8px !important; font-size: 18px !important; }
.sd-bar-title { flex: 1; font-size: 16px; font-weight: 600; letter-spacing: -.01em; }
.sd-bars { display: flex; flex-direction: column; gap: 14px; width: 100%; }
.sd-bars > .skeuo-appbar { overflow: hidden; }
.sd-window { position: relative; height: 300px; flex: 1; min-width: 300px; overflow: hidden;
  border-radius: var(--sk-radius-chassis); border: 0.5px solid var(--sk-chassis-edge);
  background: linear-gradient(180deg, var(--sk-page-hi), var(--sk-page-lo)); }
.sd-window__content { padding: 18px 20px; display: flex; flex-direction: column; gap: 10px; }
.sd-line { height: 10px; border-radius: 999px; background: var(--sk-shallow); }
.sd-docked { display: flex; height: 300px; flex: 1; min-width: 300px; overflow: hidden;
  border-radius: var(--sk-radius-chassis); border: 0.5px solid var(--sk-chassis-edge); }
.sd-docked > .sd-window__content { flex: 1; background: linear-gradient(180deg, var(--sk-page-hi), var(--sk-page-lo)); }
.sd-nav { display: flex; flex-direction: column; gap: 2px; padding: 12px 8px; }
.sd-nav .skeuo-btn { justify-content: flex-start; }
.sd-nav-head { padding: 6px 10px 10px; }
.sd-tile { display: flex; align-items: center; justify-content: center; height: 48px; font: 600 13px/1 var(--skeuo-font); color: var(--sk-ink-2); }
.sd-sub { margin: 18px 0 10px; }
.sd-container-demo { display: flex; flex-direction: column; gap: 10px; width: 100%; }
.sd-container-demo .skeuo-container > div { height: 34px; display: flex; align-items: center; padding: 0 12px; }
"#;

/// A gradient landscape as an SVG data URI, used as card media.
const MEDIA_IMAGE: &str = "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='600' height='300' viewBox='0 0 600 300'><defs><linearGradient id='s' x1='0' y1='0' x2='0' y2='1'><stop offset='0' stop-color='%230a84ff'/><stop offset='1' stop-color='%23bf5af2'/></linearGradient><linearGradient id='h' x1='0' y1='0' x2='0' y2='1'><stop offset='0' stop-color='%23ffffff' stop-opacity='.55'/><stop offset='1' stop-color='%23ffffff' stop-opacity='.15'/></linearGradient></defs><rect width='600' height='300' fill='url(%23s)'/><circle cx='450' cy='95' r='38' fill='%23ffd60a' fill-opacity='.9'/><path d='M0 220 Q120 150 240 200 T480 180 T600 190 V300 H0z' fill='url(%23h)'/><path d='M0 250 Q150 200 300 240 T600 230 V300 H0z' fill='%23ffffff' fill-opacity='.35'/></svg>";

#[component]
pub fn SurfacesSection() -> Element {
    rsx! {
        style { {SURFACES_CSS} }
        PaperDemo {}
        CardDemo {}
        AccordionDemo {}
        AppBarDemo {}
        DrawerDemo {}
        ScrollAreaDemo {}
        LayoutDemo {}
    }
}

/// Broadcast log lines for the ScrollArea demo.
const LOG: [(&str, &str); 14] = [
    ("06:00:00", "Station ID · jingle 12"),
    ("06:00:08", "Morning Drive · live"),
    ("06:14:30", "Traffic report · Studio B"),
    ("06:15:10", "Bed · Sunrise loop"),
    ("06:29:45", "Weather · automated"),
    ("06:30:00", "News headlines"),
    ("06:34:20", "Interview · phone line 2"),
    ("06:44:55", "Ad break · 3 spots"),
    ("06:47:30", "Music · Tom Misch"),
    ("06:51:02", "Listener call-in"),
    ("06:58:40", "Sweeper · jingle 4"),
    ("07:00:00", "News on the hour"),
    ("07:05:00", "Sports desk"),
    ("07:12:30", "Music · Nils Frahm"),
];

#[component]
fn ScrollAreaDemo() -> Element {
    rsx! {
        Demo {
            title: "ScrollArea · scrollbars",
            description: "every scrollbar is a groove with a small keycap thumb · ScrollArea gives a region its own scroll; in a well, content slides under the lip with a shade at the edge",
            div { class: "sd-scroll-grid",
                div {
                    p { class: "skeuo-label", style: "margin: 0 0 6px", "Well · vertical" }
                    ScrollArea { variant: ScrollAreaVariant::Well, height: "180px", aria_label: "Broadcast log",
                        for (time , item) in LOG {
                            div { class: "sd-log-row",
                                span { class: "sd-log-time", "{time}" }
                                span { "{item}" }
                            }
                        }
                    }
                }
                div {
                    p { class: "skeuo-label", style: "margin: 0 0 6px", "Plain · vertical" }
                    ScrollArea { max_height: "180px",
                        for (time , item) in LOG {
                            div { class: "sd-log-row",
                                span { class: "sd-log-time", "{time}" }
                                span { "{item}" }
                            }
                        }
                    }
                }
                div {
                    p { class: "skeuo-label", style: "margin: 0 0 6px", "Well · horizontal" }
                    ScrollArea { variant: ScrollAreaVariant::Well, direction: ScrollDirection::Horizontal,
                        div { class: "sd-scroll-strip",
                            for (time , item) in LOG {
                                div { class: "sd-scroll-chip skeuo-key", b { "{time}" } span { "{item}" } }
                            }
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn PaperDemo() -> Element {
    rsx! {
        Demo {
            title: "Paper",
            description: "aluminium chassis · MUI elevation 0–24 maps onto chassis levels 0–4 · outlined = flush with a machined edge",
            DemoRow { label: "elevation",
                for e in [0u8, 1, 4, 8, 16] {
                    Paper { elevation: e, class: "sd-plate",
                        b { "elevation {e}" }
                        span { class: "skeuo-caption", "level {chassis_level(e)}" }
                    }
                }
            }
            DemoRow { label: "variants",
                Paper { variant: PaperVariant::Outlined, class: "sd-plate",
                    b { "outlined" }
                    span { class: "skeuo-caption", "flush, 1px edge" }
                }
                Paper { square: true, class: "sd-plate",
                    b { "square" }
                    span { class: "skeuo-caption", "no corner radius" }
                }
                Paper { elevation: 3, square: true, variant: PaperVariant::Outlined, class: "sd-plate",
                    b { "outlined square" }
                    span { class: "skeuo-caption", "elevation ignored" }
                }
            }
        }
    }
}

#[component]
fn CardDemo() -> Element {
    let mut liked = use_signal(|| false);
    let mut presses = use_signal(|| 0u32);
    rsx! {
        Demo {
            title: "Card",
            description: "Paper with header, media, content and actions · CardActionArea turns the whole card into one large key",
            DemoGrid {
                Card {
                    CardHeader {
                        avatar: rsx! {
                            span { class: "skeuo-key skeuo-key--filled skeuo-key--round skeuo-color-secondary sd-avatar", "MK" }
                        },
                        title: "Morning show",
                        subheader: "Studio A · 06:00",
                        action: rsx! {
                            Button { variant: ButtonVariant::Text, color: Color::Neutral, class: "sd-icon-btn", aria_label: "More",
                                Icon { name: IconName::MoreVert }
                            }
                        },
                    }
                    CardMedia { image: MEDIA_IMAGE, height: "140px", title: "Gradient landscape" }
                    CardContent {
                        p { class: "sd-text",
                            "Wake-up mix with traffic at :15 and :45, weather on the hour and the listener call-in segment."
                        }
                    }
                    CardActions {
                        Button {
                            variant: ButtonVariant::Text,
                            color: if liked() { Color::Error } else { Color::Neutral },
                            class: "sd-icon-btn",
                            aria_label: "Like",
                            onclick: move |_| liked.toggle(),
                            Icon { name: if liked() { IconName::Favorite } else { IconName::FavoriteBorder } }
                        }
                        Button { variant: ButtonVariant::Text, "Share" }
                        span { style: "flex: 1" }
                        Button { size: Size::Small, "Go live" }
                    }
                }
                Card {
                    CardActionArea { onclick: move |_| presses += 1,
                        CardMedia { image: MEDIA_IMAGE, height: "120px", title: "Gradient landscape" }
                        CardContent {
                            h3 { class: "sd-title", "Press the whole card" }
                            p { class: "sd-text", "Pressed {presses} times. The plate sinks slightly, like a large key." }
                        }
                    }
                }
                div { style: "display: flex; flex-direction: column; gap: 20px",
                    Card { variant: PaperVariant::Outlined,
                        CardContent {
                            h3 { class: "sd-title", "Outlined card" }
                            p { class: "sd-text", "Flush with the desk, a crisp machined edge." }
                        }
                        CardActions { disable_spacing: true,
                            Button { variant: ButtonVariant::Text, "Open" }
                            Button { variant: ButtonVariant::Text, "Dismiss" }
                        }
                    }
                    Card { raised: true,
                        CardContent {
                            h3 { class: "sd-title", "Raised card" }
                            p { class: "sd-text", "raised = chassis level 2." }
                        }
                    }
                }
            }
        }
    }
}

#[component]
fn AccordionDemo() -> Element {
    // Exclusive group: only one section open at a time (controlled).
    let mut open = use_signal(|| Some(1usize));
    let sections = [
        (
            "Microphones",
            "Two dynamic mics on boom arms; the guest mic is muted until the host opens it.",
        ),
        (
            "Playout",
            "Jingles and beds are on the cart wall. Hold a cart to cue, release to fire.",
        ),
        (
            "Streaming",
            "The encoder runs at 128 kbps AAC with a 30 second delay buffer.",
        ),
    ];
    rsx! {
        Demo {
            title: "Accordion",
            description: "stacked chassis sections · the expanded one separates and its chevron turns · controlled (exclusive) and uncontrolled",
            div { style: "display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 24px; align-items: start",
                div {
                    for (i , (title , body)) in sections.into_iter().enumerate() {
                        Accordion {
                            expanded: open() == Some(i),
                            onchange: move |e: bool| open.set(if e { Some(i) } else { None }),
                            AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } }, "{title}" }
                            AccordionDetails {
                                p { class: "sd-text", "{body}" }
                            }
                            if i == 1 {
                                AccordionActions {
                                    Button { variant: ButtonVariant::Text, "Reset" }
                                    Button { size: Size::Small, "Save" }
                                }
                            }
                        }
                    }
                    Accordion { disabled: true,
                        AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } }, "Disabled section" }
                        AccordionDetails { p { class: "sd-text", "Unavailable." } }
                    }
                }
                div {
                    Accordion { default_expanded: true, disable_gutters: true, square: true,
                        AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } }, "Square, no gutters (uncontrolled)" }
                        AccordionDetails { p { class: "sd-text", "disable_gutters keeps the stack together when a section opens." } }
                    }
                    Accordion { disable_gutters: true, square: true,
                        AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } }, "Collapsed" }
                        AccordionDetails { p { class: "sd-text", "Hidden until opened." } }
                    }
                    Accordion { disable_gutters: true, square: true,
                        AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } },
                            Icon { name: IconName::Settings }
                            "With an icon"
                        }
                        AccordionDetails { p { class: "sd-text", "Summary content can hold any lettering." } }
                    }
                }
            }
        }
    }
}

#[component]
fn BarControls(title: String) -> Element {
    rsx! {
        Button { variant: ButtonVariant::Text, class: "sd-icon-btn", aria_label: "Open menu",
            Icon { name: IconName::Menu }
        }
        span { class: "sd-bar-title", "{title}" }
        Button { variant: ButtonVariant::Text, class: "sd-icon-btn", aria_label: "Search",
            Icon { name: IconName::Search }
        }
        Button { variant: ButtonVariant::Text, class: "sd-icon-btn", aria_label: "More",
            Icon { name: IconName::MoreVert }
        }
    }
}

#[component]
fn AppBarDemo() -> Element {
    rsx! {
        Demo {
            title: "AppBar · Toolbar",
            description: "default = square aluminium chassis · palette colors = matte colored plate (a keycap top without travel) · transparent = lettering only",
            div { class: "sd-bars",
                AppBar { position: AppBarPosition::Static,
                    Toolbar { BarControls { title: "On Air" } }
                }
                AppBar { position: AppBarPosition::Static, color: AppBarColor::Default,
                    Toolbar {
                        BarControls { title: "Default (aluminium)" }
                        Button { size: Size::Small, "Go live" }
                    }
                }
                AppBar { position: AppBarPosition::Static, color: Color::Secondary,
                    Toolbar { variant: ToolbarVariant::Dense, BarControls { title: "Secondary · dense" } }
                }
                AppBar { position: AppBarPosition::Static, color: Color::Success,
                    Toolbar { variant: ToolbarVariant::Dense, BarControls { title: "Success · dense" } }
                }
                AppBar { position: AppBarPosition::Static, color: AppBarColor::Transparent,
                    Toolbar { variant: ToolbarVariant::Dense, BarControls { title: "Transparent" } }
                }
            }
        }
    }
}

#[component]
fn NavContents() -> Element {
    rsx! {
        div { class: "sd-nav",
            div { class: "sd-nav-head",
                b { "iAnnouncer" }
                div { class: "skeuo-caption", "Studio A" }
            }
            for (icon , label) in [
                (IconName::Home, "Home"),
                (IconName::Mic, "Live"),
                (IconName::Folder, "Library"),
                (IconName::Calendar, "Schedule"),
                (IconName::Settings, "Settings"),
            ] {
                Button {
                    variant: ButtonVariant::Text,
                    color: Color::Neutral,
                    full_width: true,
                    start_icon: rsx! { Icon { name: icon } },
                    "{label}"
                }
            }
        }
    }
}

#[component]
fn FakePage() -> Element {
    rsx! {
        div { class: "sd-window__content",
            b { "Today" }
            div { class: "sd-line", style: "width: 80%" }
            div { class: "sd-line", style: "width: 65%" }
            div { class: "sd-line", style: "width: 72%" }
            div { class: "sd-line", style: "width: 40%" }
            div { class: "sd-line", style: "width: 58%" }
        }
    }
}

#[component]
fn DrawerDemo() -> Element {
    let mut open = use_signal(|| false);
    let mut anchor = use_signal(|| DrawerAnchor::Left);
    rsx! {
        Demo {
            title: "Drawer",
            description: "temporary = floating level-4 chassis glued to an edge over a scrim (Escape / scrim click closes) · permanent = flush square plate docked in the layout",
            DemoRow {
                div { class: "sd-window",
                    FakePage {}
                    Drawer { open: true, inline: true, size: "200px", aria_label: "Preview navigation",
                        NavContents {}
                    }
                }
                div { class: "sd-docked",
                    Drawer { variant: DrawerVariant::Permanent, size: "180px", aria_label: "Docked navigation",
                        NavContents {}
                    }
                    FakePage {}
                }
            }
            DemoRow { label: "interactive",
                for (a , label) in [
                    (DrawerAnchor::Left, "Left"),
                    (DrawerAnchor::Right, "Right"),
                    (DrawerAnchor::Top, "Top"),
                    (DrawerAnchor::Bottom, "Bottom"),
                ] {
                    Button {
                        variant: ButtonVariant::Outlined,
                        onclick: move |_| {
                            anchor.set(a);
                            open.set(true);
                        },
                        "Open {label}"
                    }
                }
            }
            Drawer {
                open: open(),
                anchor: anchor(),
                onclose: move |_| open.set(false),
                aria_label: "Navigation",
                NavContents {}
                div { style: "padding: 0 16px 16px",
                    Button { full_width: true, onclick: move |_| open.set(false), "Close" }
                }
            }
        }
    }
}

#[component]
fn LayoutDemo() -> Element {
    rsx! {
        Demo {
            title: "Stack · Grid · Container · SkeuoBox",
            description: "invisible structure, no material · spacing in 8px units · Grid v2 with 12 columns, responsive sizes and offsets",
            h3 { class: "skeuo-label sd-sub", "Stack direction=row spacing=2 with a divider" }
            Stack {
                direction: StackDirection::Row,
                spacing: 2.0,
                align_items: "center",
                divider: rsx! { hr { class: "skeuo-rule skeuo-rule--vertical" } },
                for label in ["Mic 1", "Mic 2", "Phone", "Cart"] {
                    Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "{label}" }
                }
            }
            h3 { class: "skeuo-label sd-sub", "Stack column spacing=1 with a divider" }
            Paper { variant: PaperVariant::Outlined, style: "max-width: 320px; padding: 12px 16px",
                Stack { spacing: 1.0, divider: rsx! { hr { class: "skeuo-rule" } },
                    for (k , v) in [("Frequency", "101.7 MHz"), ("Bitrate", "128 kbps"), ("Delay", "30 s")] {
                        SkeuoBox { style: "display: flex; justify-content: space-between",
                            span { class: "skeuo-caption", "{k}" }
                            b { style: "font-size: 13px", "{v}" }
                        }
                    }
                }
            }
            h3 { class: "skeuo-label sd-sub", "Grid container spacing=2 · sizes 8/4, 4×3, 6 offset 3, auto + grow" }
            Grid { container: true, spacing: 2.0,
                Grid { size: 8, Tile { n: "size 8" } }
                Grid { size: 4, Tile { n: "size 4" } }
                for i in 1..=4 {
                    Grid { size: Responsive::xs(6).md(3), Tile { n: "{i} · xs 6 / md 3" } }
                }
                Grid { size: 6, offset: 3, Tile { n: "size 6 · offset 3" } }
                Grid { size: GridSize::Auto, Tile { n: "auto" } }
                Grid { size: GridSize::Grow, Tile { n: "grow" } }
            }
            h3 { class: "skeuo-label sd-sub", "Container max_width xs / sm / md" }
            div { class: "sd-container-demo",
                for (mw , label) in [
                    (ContainerMaxWidth::Xs, "xs · 444px"),
                    (ContainerMaxWidth::Sm, "sm · 600px"),
                    (ContainerMaxWidth::Md, "md · 900px"),
                ] {
                    Container { max_width: mw, disable_gutters: true,
                        div { class: "skeuo-well skeuo-well--shallow skeuo-caption", "{label}" }
                    }
                }
            }
        }
    }
}

#[component]
fn Tile(n: String) -> Element {
    rsx! {
        div { class: "skeuo-well skeuo-well--shallow sd-tile", "{n}" }
    }
}
