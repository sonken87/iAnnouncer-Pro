#include "../gallery.hpp"

namespace gallery {

namespace {

/// Page-local helpers for the demos (not part of skeuo-ui).
const char* const kSurfacesCss = R"CSS(
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
)CSS";

/// A gradient landscape as an SVG data URI, used as card media.
const char* const kMediaImage =
    "data:image/svg+xml;utf8,<svg xmlns='http://www.w3.org/2000/svg' width='600' height='300' viewBox='0 0 600 "
    "300'><defs><linearGradient id='s' x1='0' y1='0' x2='0' y2='1'><stop offset='0' stop-color='%230a84ff'/><stop "
    "offset='1' stop-color='%23bf5af2'/></linearGradient><linearGradient id='h' x1='0' y1='0' x2='0' y2='1'><stop "
    "offset='0' stop-color='%23ffffff' stop-opacity='.55'/><stop offset='1' stop-color='%23ffffff' "
    "stop-opacity='.15'/></linearGradient></defs><rect width='600' height='300' fill='url(%23s)'/><circle cx='450' "
    "cy='95' r='38' fill='%23ffd60a' fill-opacity='.9'/><path d='M0 220 Q120 150 240 200 T480 180 T600 190 V300 "
    "H0z' fill='url(%23h)'/><path d='M0 250 Q150 200 300 240 T600 230 V300 H0z' fill='%23ffffff' "
    "fill-opacity='.35'/></svg>";

Node sd_caption(std::string text) { return el("span").cls("skeuo-caption").child(std::move(text)); }
Node sd_text(std::string text) { return el("p").cls("sd-text").child(std::move(text)); }
Node sd_title(std::string text) { return el("h3").cls("sd-title").child(std::move(text)); }

Node PaperDemo() {
    Node plates;
    for (int e : {0, 1, 4, 8, 16})
        plates.add_child(Paper({.elevation = e, .cls = "sd-plate"},
                               {el("b").child("elevation " + std::to_string(e)),
                                sd_caption("level " + std::to_string(chassis_level(e)))}));
    return Demo("Paper",
                "aluminium chassis · MUI elevation 0–24 maps onto chassis levels 0–4 · outlined = flush with a "
                "machined edge",
                {
                    DemoRow("elevation", std::move(plates)),
                    DemoRow("variants", {
                        Paper({.variant = PaperVariant::Outlined, .cls = "sd-plate"},
                              {el("b").child("outlined"), sd_caption("flush, 1px edge")}),
                        Paper({.square = true, .cls = "sd-plate"},
                              {el("b").child("square"), sd_caption("no corner radius")}),
                        Paper({.elevation = 3, .variant = PaperVariant::Outlined, .square = true, .cls = "sd-plate"},
                              {el("b").child("outlined square"), sd_caption("elevation ignored")}),
                    }),
                });
}

Node CardDemo() {
    bool& liked = use_state<bool>("demo.liked", [] { return false; });
    int& presses = use_state<int>("demo.presses", [] { return 0; });
    return Demo(
        "Card",
        "Paper with header, media, content and actions · CardActionArea turns the whole card into one large key",
        DemoGrid({
            Card({}, {
                CardHeader({
                    .avatar = el("span")
                                  .cls("skeuo-key skeuo-key--filled skeuo-key--round skeuo-color-secondary sd-avatar")
                                  .child("MK"),
                    .title = "Morning show",
                    .subheader = "Studio A · 06:00",
                    .action = Button({.variant = ButtonVariant::Text, .color = Color::Neutral, .cls = "sd-icon-btn",
                                      .attrs = {{"aria-label", "More"}}},
                                     Icon(IconName::MoreVert)),
                }),
                CardMedia({.image = kMediaImage, .title = "Gradient landscape", .height = "140px"}),
                CardContent({}, sd_text("Wake-up mix with traffic at :15 and :45, weather on the hour and the "
                                     "listener call-in segment.")),
                CardActions({}, {
                    Button({.variant = ButtonVariant::Text,
                            .color = liked ? Color::Error : Color::Neutral,
                            .cls = "sd-icon-btn",
                            .onclick = [&liked] { liked = !liked; },
                            .attrs = {{"aria-label", "Like"}}},
                           Icon(liked ? IconName::Favorite : IconName::FavoriteBorder)),
                    Button({.variant = ButtonVariant::Text}, "Share"),
                    el("span").attr("style", "flex: 1"),
                    Button({.size = Size::Small}, "Go live"),
                }),
            }),
            Card({}, CardActionArea({.onclick = [&presses] { ++presses; }}, {
                CardMedia({.image = kMediaImage, .title = "Gradient landscape", .height = "120px"}),
                CardContent({}, {
                    sd_title("Press the whole card"),
                    sd_text("Pressed " + std::to_string(presses) + " times. The plate sinks slightly, like a large key."),
                }),
            })),
            el("div").attr("style", "display: flex; flex-direction: column; gap: 20px").child({
                Card({.variant = PaperVariant::Outlined}, {
                    CardContent({}, {sd_title("Outlined card"), sd_text("Flush with the desk, a crisp machined edge.")}),
                    CardActions({.disable_spacing = true}, {
                        Button({.variant = ButtonVariant::Text}, "Open"),
                        Button({.variant = ButtonVariant::Text}, "Dismiss"),
                    }),
                }),
                Card({.raised = true}, CardContent({}, {sd_title("Raised card"), sd_text("raised = chassis level 2.")})),
            }),
        }));
}

Node chevron() { return Icon(IconName::ChevronDown); }

Node AccordionDemo() {
    // Exclusive group: only one section open at a time (controlled).
    auto& open = use_state<std::optional<size_t>>("demo.accordion", [] { return std::optional<size_t>(1); });
    const std::pair<const char*, const char*> sections[] = {
        {"Microphones", "Two dynamic mics on boom arms; the guest mic is muted until the host opens it."},
        {"Playout", "Jingles and beds are on the cart wall. Hold a cart to cue, release to fire."},
        {"Streaming", "The encoder runs at 128 kbps AAC with a 30 second delay buffer."},
    };
    Node exclusive = el("div");
    for (size_t i = 0; i < 3; ++i) {
        auto [title, body] = sections[i];
        Node parts = {
            AccordionSummary({.expand_icon = chevron()}, title),
            AccordionDetails({}, sd_text(body)),
        };
        if (i == 1)
            parts.add_child(AccordionActions({}, {
                Button({.variant = ButtonVariant::Text}, "Reset"),
                Button({.size = Size::Small}, "Save"),
            }));
        exclusive.child(Accordion({.expanded = open == i,
                                   .onchange = [&open, i](bool e) { open = e ? std::optional<size_t>(i) : std::nullopt; }},
                                  std::move(parts)));
    }
    exclusive.child(Accordion({.disabled = true}, {
        AccordionSummary({.expand_icon = chevron()}, "Disabled section"),
        AccordionDetails({}, sd_text("Unavailable.")),
    }));

    return Demo(
        "Accordion",
        "stacked chassis sections · the expanded one separates and its chevron turns · controlled (exclusive) and "
        "uncontrolled",
        el("div")
            .attr("style", "display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 24px; "
                           "align-items: start")
            .child({
                std::move(exclusive),
                el("div").child({
                    Accordion({.default_expanded = true, .disable_gutters = true, .square = true}, {
                        AccordionSummary({.expand_icon = chevron()}, "Square, no gutters (uncontrolled)"),
                        AccordionDetails({}, sd_text("disable_gutters keeps the stack together when a section opens.")),
                    }),
                    Accordion({.disable_gutters = true, .square = true}, {
                        AccordionSummary({.expand_icon = chevron()}, "Collapsed"),
                        AccordionDetails({}, sd_text("Hidden until opened.")),
                    }),
                    Accordion({.disable_gutters = true, .square = true}, {
                        AccordionSummary({.expand_icon = chevron()}, {Icon(IconName::Settings), "With an icon"}),
                        AccordionDetails({}, sd_text("Summary content can hold any lettering.")),
                    }),
                }),
            }));
}

Node BarControls(std::string title) {
    auto icon_button = [](const char* label, IconName icon) {
        return Button({.variant = ButtonVariant::Text, .cls = "sd-icon-btn", .attrs = {{"aria-label", label}}},
                      Icon(icon));
    };
    return {
        icon_button("Open menu", IconName::Menu),
        el("span").cls("sd-bar-title").child(std::move(title)),
        icon_button("Search", IconName::Search),
        icon_button("More", IconName::MoreVert),
    };
}

Node AppBarDemo() {
    return Demo(
        "AppBar · Toolbar",
        "default = square aluminium chassis · palette colors = matte colored plate (a keycap top without travel) · "
        "transparent = lettering only",
        el("div").cls("sd-bars").child({
            AppBar({.position = AppBarPosition::Static}, Toolbar({}, BarControls("On Air"))),
            AppBar({.position = AppBarPosition::Static, .color = AppBarColor::Default},
                   Toolbar({}, {BarControls("Default (aluminium)"), Button({.size = Size::Small}, "Go live")})),
            AppBar({.position = AppBarPosition::Static, .color = Color::Secondary},
                   Toolbar({.variant = ToolbarVariant::Dense}, BarControls("Secondary · dense"))),
            AppBar({.position = AppBarPosition::Static, .color = Color::Success},
                   Toolbar({.variant = ToolbarVariant::Dense}, BarControls("Success · dense"))),
            AppBar({.position = AppBarPosition::Static, .color = AppBarColor::Transparent},
                   Toolbar({.variant = ToolbarVariant::Dense}, BarControls("Transparent"))),
        }));
}

Node NavContents() {
    Node nav = el("div").cls("sd-nav").child(
        el("div").cls("sd-nav-head").child({el("b").child("iAnnouncer"), el("div").cls("skeuo-caption").child("Studio A")}));
    for (auto [icon, label] : {std::pair{IconName::Home, "Home"}, std::pair{IconName::Mic, "Live"},
                               std::pair{IconName::Folder, "Library"}, std::pair{IconName::Calendar, "Schedule"},
                               std::pair{IconName::Settings, "Settings"}})
        nav.child(Button({.variant = ButtonVariant::Text, .color = Color::Neutral, .full_width = true,
                          .start_icon = Icon(icon)},
                         label));
    return nav;
}

Node FakePage() {
    Node page = el("div").cls("sd-window__content").child(el("b").child("Today"));
    for (const char* w : {"80%", "65%", "72%", "40%", "58%"})
        page.child(el("div").cls("sd-line").attr("style", std::string("width: ") + w));
    return page;
}

Node DrawerDemo() {
    bool& open = use_state<bool>("demo.drawer.open", [] { return false; });
    DrawerAnchor& anchor = use_state<DrawerAnchor>("demo.drawer.anchor", [] { return DrawerAnchor::Left; });
    Node buttons;
    for (auto [a, label] : {std::pair{DrawerAnchor::Left, "Left"}, std::pair{DrawerAnchor::Right, "Right"},
                            std::pair{DrawerAnchor::Top, "Top"}, std::pair{DrawerAnchor::Bottom, "Bottom"}})
        buttons.add_child(Button({.variant = ButtonVariant::Outlined,
                                  .onclick = [&open, &anchor, a = a] {
                                      anchor = a;
                                      open = true;
                                  }},
                                 std::string("Open ") + label));
    return Demo(
        "Drawer",
        "temporary = floating level-4 chassis glued to an edge over a scrim (Escape / scrim click closes) · "
        "permanent = flush square plate docked in the layout",
        {
            DemoRow("", {
                el("div").cls("sd-window").child({
                    FakePage(),
                    Drawer({.open = true, .size = "200px", .inline_ = true,
                            .attrs = {{"aria-label", "Preview navigation"}}},
                           NavContents()),
                }),
                el("div").cls("sd-docked").child({
                    Drawer({.variant = DrawerVariant::Permanent, .size = "180px",
                            .attrs = {{"aria-label", "Docked navigation"}}},
                           NavContents()),
                    FakePage(),
                }),
            }),
            DemoRow("interactive", std::move(buttons)),
            Drawer({.anchor = anchor, .open = open, .onclose = [&open] { open = false; },
                    .attrs = {{"aria-label", "Navigation"}}},
                   {
                       NavContents(),
                       el("div").attr("style", "padding: 0 16px 16px")
                           .child(Button({.full_width = true, .onclick = [&open] { open = false; }}, "Close")),
                   }),
        });
}

Node sd_tile(std::string n) { return el("div").cls("skeuo-well skeuo-well--shallow sd-tile").child(std::move(n)); }

Node sd_sub(std::string text) { return el("h3").cls("skeuo-label sd-sub").child(std::move(text)); }

Node LayoutDemo() {
    Node mics;
    for (const char* label : {"Mic 1", "Mic 2", "Phone", "Cart"})
        mics.add_child(Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral}, label));
    Node facts;
    for (auto [k, v] : {std::pair{"Frequency", "101.7 MHz"}, std::pair{"Bitrate", "128 kbps"},
                        std::pair{"Delay", "30 s"}})
        facts.add_child(SkeuoBox({.attrs = {{"style", "display: flex; justify-content: space-between"}}},
                                 {sd_caption(k), el("b").attr("style", "font-size: 13px").child(v)}));
    Node quarters;
    for (int i = 1; i <= 4; ++i)
        quarters.add_child(Grid({.size = Responsive::xs(6).md(3)}, sd_tile(std::to_string(i) + " · xs 6 / md 3")));
    Node containers;
    for (auto [mw, label] : {std::pair{ContainerMaxWidth::Xs, "xs · 444px"}, std::pair{ContainerMaxWidth::Sm, "sm · 600px"},
                             std::pair{ContainerMaxWidth::Md, "md · 900px"}})
        containers.add_child(Container({.max_width = mw, .disable_gutters = true},
                                       el("div").cls("skeuo-well skeuo-well--shallow skeuo-caption").child(label)));

    return Demo(
        "Stack · Grid · Container · SkeuoBox",
        "invisible structure, no material · spacing in 8px units · Grid v2 with 12 columns, responsive sizes and "
        "offsets",
        {
            sd_sub("Stack direction=row spacing=2 with a divider"),
            Stack({.direction = StackDirection::Row, .spacing = 2.0,
                   .divider = el("hr").cls("skeuo-rule skeuo-rule--vertical"), .align_items = "center"},
                  std::move(mics)),
            sd_sub("Stack column spacing=1 with a divider"),
            Paper({.variant = PaperVariant::Outlined, .attrs = {{"style", "max-width: 320px; padding: 12px 16px"}}},
                  Stack({.spacing = 1.0, .divider = el("hr").cls("skeuo-rule")}, std::move(facts))),
            sd_sub("Grid container spacing=2 · sizes 8/4, 4×3, 6 offset 3, auto + grow"),
            Grid({.container = true, .spacing = 2.0}, {
                Grid({.size = 8}, sd_tile("size 8")),
                Grid({.size = 4}, sd_tile("size 4")),
                std::move(quarters),
                Grid({.size = 6, .offset = 3}, sd_tile("size 6 · offset 3")),
                Grid({.size = GridSize::Auto}, sd_tile("auto")),
                Grid({.size = GridSize::Grow}, sd_tile("grow")),
            }),
            sd_sub("Container max_width xs / sm / md"),
            el("div").cls("sd-container-demo").child(std::move(containers)),
        });
}

}  // namespace

Node SurfacesPage() {
    return {
        el("style").child(raw_html(kSurfacesCss)),
        PaperDemo(),
        CardDemo(),
        AccordionDemo(),
        AppBarDemo(),
        DrawerDemo(),
        LayoutDemo(),
    };
}

}  // namespace gallery
