// Data display page. Mirrors crates/gallery/src/sections/data_display.rs.
#include <algorithm>
#include <array>
#include <cstdio>
#include <optional>

#include "../gallery.hpp"

namespace gallery {

namespace {

std::string replace_all(std::string s, std::string_view from, std::string_view to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

/// A landscape "photo" as an inline SVG data URI (the gallery ships no image
/// files). `h` sets the aspect ratio (width is 400).
std::string photo(const char* sky_top, const char* sky_bottom, const char* sun, const char* hill, const char* ground,
                  unsigned h) {
    auto n = [](unsigned v) { return std::to_string(v); };
    unsigned h1 = h * 70 / 100;
    unsigned h2 = h * 85 / 100;
    std::string svg =
        "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 400 " + n(h) +
        "' preserveAspectRatio='xMidYMid slice'>"
        "<defs><linearGradient id='s' x1='0' y1='0' x2='0' y2='1'>"
        "<stop offset='0' stop-color='" + sky_top + "'/><stop offset='1' stop-color='" + sky_bottom + "'/>"
        "</linearGradient></defs>"
        "<rect width='400' height='" + n(h) + "' fill='url(#s)'/>"
        "<circle cx='290' cy='" + n(h * 35 / 100) + "' r='34' fill='" + sun + "' opacity='.9'/>"
        "<path d='M0 " + n(h1) + " Q100 " + n(h * 50 / 100) + " 200 " + n(h1) + " T400 " + n(h * 62 / 100) + " V" +
        n(h) + " H0Z' fill='" + hill + "' opacity='.75'/>"
        "<path d='M0 " + n(h2) + " Q120 " + n(h * 72 / 100) + " 240 " + n(h2) + " T400 " + n(h2) + " V" + n(h) +
        " H0Z' fill='" + ground + "'/></svg>";
    svg = replace_all(svg, "%", "%25");
    svg = replace_all(svg, "#", "%23");
    svg = replace_all(svg, "<", "%3C");
    svg = replace_all(svg, ">", "%3E");
    return "data:image/svg+xml;utf8," + svg;
}

struct PhotoSpec {
    const char* name;
    const char* sky_top;
    const char* sky_bottom;
    const char* sun;
    const char* hill;
    const char* ground;
};

constexpr std::array<PhotoSpec, 9> kPhotos = {{
    {"Dawn", "#ffb88c", "#ffe2c6", "#fff4d6", "#d9826a", "#8e5a63"},
    {"Coast", "#5ac8fa", "#d6f0ff", "#ffffff", "#3a8fb7", "#1f5f7a"},
    {"Meadow", "#a8e063", "#eef8d8", "#fffbe0", "#5f9c3a", "#3d6e2a"},
    {"Dusk", "#5e5ce6", "#ff9fb2", "#ffd7a8", "#6b4a8e", "#3b2a55"},
    {"Desert", "#ffd28a", "#fff0d1", "#fff8ea", "#d79b55", "#a0683a"},
    {"Glacier", "#c7e6ff", "#f2f9ff", "#ffffff", "#8fb6d9", "#5d84a8"},
    {"Forest", "#7fc8a9", "#e3f4ea", "#f8ffe8", "#3f7d5f", "#25523c"},
    {"Night", "#1c2541", "#3a506b", "#f2f2d0", "#2c3e5c", "#141b2d"},
    {"Studio", "#d1d1d6", "#f2f2f7", "#ffffff", "#a1a1a6", "#6e6e73"},
}};

std::string photo_n(size_t i, unsigned h) {
    const PhotoSpec& p = kPhotos[i % kPhotos.size()];
    return photo(p.sky_top, p.sky_bottom, p.sun, p.hill, p.ground, h);
}

std::string str(std::string_view s) { return std::string(s); }

// ------------------------------------------------------------------ Typography

Node TypographyDemo() {
    Node headings = el("div");
    for (TypographyVariant v : {TypographyVariant::H1, TypographyVariant::H2, TypographyVariant::H3,
                                TypographyVariant::H4, TypographyVariant::H5, TypographyVariant::H6})
        headings.child(Typography({.variant = v, .gutter_bottom = true}, str(as_str(v)) + ". On Air"));

    Node inks;
    for (Color c : kAllColors)
        inks.add_child(Typography({.variant = TypographyVariant::Subtitle2, .color = c, .component = "span"},
                                  str(as_str(c))));
    inks.add_child(Typography(
        {.variant = TypographyVariant::Subtitle2, .color = TypographyColor::TextDisabled, .component = "span"},
        "text.disabled"));

    return Demo(
        "Typography",
        "printed lettering on the aluminium · SF Pro scale with tight heading tracking · no text effects",
        {
            el("div")
                .style("display: grid; grid-template-columns: 1.4fr 1fr; gap: 28px; align-items: start; "
                       "margin-bottom: 18px")
                .child(std::move(headings))
                .child(el("div").children({
                    Typography({.variant = TypographyVariant::Overline, .gutter_bottom = true},
                               "Overline · now playing"),
                    Typography({.variant = TypographyVariant::Subtitle1},
                               "subtitle1. Morning drive with the station team"),
                    Typography({.variant = TypographyVariant::Subtitle2, .gutter_bottom = true},
                               "subtitle2. Segment 3 of 8"),
                    Typography({.variant = TypographyVariant::Body1, .paragraph = true},
                               "body1. Every announcement is cut like a keycap legend: crisp, even and quietly "
                               "confident, printed straight onto the surface."),
                    Typography({.variant = TypographyVariant::Body2, .color = TypographyColor::TextSecondary,
                                .paragraph = true},
                               "body2 · text.secondary. Secondary copy steps back in a softer grey ink."),
                    Typography({.variant = TypographyVariant::Caption, .gutter_bottom = true, .component = "div"},
                               "caption. Last edited 2 minutes ago"),
                    Typography({.variant = TypographyVariant::Button, .component = "div"}, "button legend"),
                })),
            DemoRow("palette ink", std::move(inks)),
            DemoRow("align / wrap",
                    el("div").style("width: 220px").children({
                        Typography({.variant = TypographyVariant::Body2, .align = TypographyAlign::Right},
                                   "right aligned"),
                        Typography({.variant = TypographyVariant::Body2, .align = TypographyAlign::Center},
                                   "centered"),
                        Typography({.variant = TypographyVariant::Body2, .no_wrap = true},
                                   "no_wrap truncates a long line with an ellipsis instead of wrapping"),
                    })),
        });
}

// ------------------------------------------------------------------ Avatar

std::vector<Node> avatar_group(size_t n) {
    static const char* const initials[] = {"JA", "MK", "SO", "LT", "RB", "CN", "EV", "PD"};
    static const std::optional<Color> colors[] = {Color::Primary, Color::Secondary, std::nullopt, Color::Success,
                                                  Color::Warning, Color::Info,      std::nullopt, Color::Error};
    std::vector<Node> out;
    for (size_t i = 0; i < n; ++i) {
        const char* initial = initials[i % 8];
        if (i == 2) out.push_back(Avatar({.src = photo_n(3, 400), .alt = "Sora"}));
        else out.push_back(Avatar({.alt = initial, .color = colors[i % 8]}, initial));
    }
    return out;
}

Node AvatarDemo() {
    size_t& extra = use_state<size_t>("dd.avatar.extra", [] { return size_t(7); });
    return Demo(
        "Avatar",
        "a small keycap carrying initials or a glyph · photos inset into a thin well ring · circular / rounded / "
        "square",
        {
            DemoRow("letters", {
                Avatar({.alt = "Jamie Apple"}, "JA"),
                Avatar({.alt = "Mika", .color = Color::Primary}, "MK"),
                Avatar({.alt = "Sol", .color = Color::Secondary}, "SO"),
                Avatar({.alt = "Lee", .color = Color::Success}, "LT"),
                Avatar({.alt = "Rae", .color = Color::Warning}, "RB"),
                Avatar({.alt = "Nico", .color = Color::Error}, "N"),
                Avatar({.alt = "Eve", .color = Color::Info}),
            }),
            DemoRow("icons", {
                Avatar({}, Icon(IconName::Person)),
                Avatar({.color = Color::Primary}, Icon(IconName::Mic)),
                Avatar({.variant = AvatarVariant::Rounded, .color = Color::Success}, Icon(IconName::Folder)),
                Avatar({.variant = AvatarVariant::Square, .color = Color::Error}, Icon(IconName::Favorite)),
            }),
            DemoRow("photos", {
                Avatar({.src = photo_n(0, 400), .alt = "Dawn", .size = Size::Small}),
                Avatar({.src = photo_n(1, 400), .alt = "Coast"}),
                Avatar({.src = photo_n(3, 400), .alt = "Dusk", .size = Size::Large}),
                Avatar({.src = photo_n(2, 400), .alt = "Meadow", .variant = AvatarVariant::Rounded,
                        .size = Size::Large}),
                Avatar({.src = photo_n(4, 400), .alt = "Desert", .variant = AvatarVariant::Square,
                        .size = Size::Large}),
            }),
            DemoRow("sizes", {
                Avatar({.color = Color::Primary, .size = Size::Small}, "S"),
                Avatar({.color = Color::Primary, .size = Size::Medium}, "M"),
                Avatar({.color = Color::Primary, .size = Size::Large}, "L"),
                Avatar({.variant = AvatarVariant::Rounded, .size = Size::Small}, "S"),
                Avatar({.variant = AvatarVariant::Rounded, .size = Size::Medium}, "M"),
                Avatar({.variant = AvatarVariant::Rounded, .size = Size::Large}, "L"),
            }),
            DemoRow("AvatarGroup", {
                AvatarGroup({.avatars = avatar_group(6), .max = 4}),
                AvatarGroup({.avatars = avatar_group(4), .max = 5, .total = 24,
                             .spacing = AvatarGroupSpacing::Small}),
                AvatarGroup({.avatars = avatar_group(extra), .max = 6}),
                Button({.variant = ButtonVariant::Outlined, .size = Size::Small, .onclick = [&extra] { ++extra; }},
                       "Add"),
                Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral, .size = Size::Small,
                        .disabled = extra == 0,
                        .onclick = [&extra] { if (extra > 0) --extra; }},
                       "Remove"),
            }),
        });
}

// ------------------------------------------------------------------ Badge

Node BadgeDemo() {
    const char* const spaced = "display: flex; flex-wrap: wrap; align-items: center; gap: 22px 30px; padding: 6px 4px";
    int& count = use_state<int>("dd.badge.count", [] { return 4; });
    bool& hidden = use_state<bool>("dd.badge.hidden", [] { return false; });
    auto icon = [] { return Icon({.name = IconName::Mail, .size = "24px"}); };
    auto tile = [](const char* label) { return Avatar({.variant = AvatarVariant::Rounded}, label); };

    Node colors = el("div").style(spaced);
    int i = 0;
    for (Color c : kAllColors) colors.child(Badge({.badge_content = ++i, .color = c}, icon()));

    return Demo(
        "Badge",
        "an indicator LED on the corner of its child · standard pill or dot · any palette color · four anchors",
        {
            DemoRow("colors", std::move(colors)),
            DemoRow("max / zero", el("div").style(spaced).children({
                Badge({.badge_content = 99}, icon()),
                Badge({.badge_content = 100}, icon()),
                Badge({.badge_content = 1000, .max = 999}, icon()),
                Badge({.badge_content = 0}, icon()),
                Badge({.badge_content = 0, .show_zero = true, .color = Color::Neutral}, icon()),
                Badge({.badge_content = "new", .color = Color::Success}, icon()),
                Badge({.variant = BadgeVariant::Dot, .color = Color::Error},
                      Icon({.name = IconName::Notifications, .size = "24px"})),
                Badge({.variant = BadgeVariant::Dot, .color = Color::Success},
                      Typography({.variant = TypographyVariant::Body2}, "Live")),
            })),
            DemoRow("anchor", el("div").style(spaced).children({
                Badge({.badge_content = 3, .anchor_origin = BadgeAnchorOrigin::TOP_RIGHT}, tile("TR")),
                Badge({.badge_content = 3, .color = Color::Secondary, .anchor_origin = BadgeAnchorOrigin::TOP_LEFT},
                      tile("TL")),
                Badge({.badge_content = 3, .color = Color::Success,
                       .anchor_origin = BadgeAnchorOrigin::BOTTOM_RIGHT},
                      tile("BR")),
                Badge({.badge_content = 3, .color = Color::Warning,
                       .anchor_origin = BadgeAnchorOrigin::BOTTOM_LEFT},
                      tile("BL")),
                Badge({.variant = BadgeVariant::Dot, .color = Color::Success,
                       .anchor_origin = BadgeAnchorOrigin::BOTTOM_RIGHT, .overlap = BadgeOverlap::Circular},
                      Avatar({.src = photo_n(1, 400), .alt = "Online"})),
                Badge({.badge_content = 12, .overlap = BadgeOverlap::Circular},
                      Avatar({.color = Color::Info}, "IA")),
            })),
            DemoRow("interactive", {
                Badge({.badge_content = count, .max = 9, .invisible = hidden, .color = Color::Error},
                      Avatar({.variant = AvatarVariant::Rounded}, Icon(IconName::Notifications))),
                Button({.variant = ButtonVariant::Outlined, .size = Size::Small, .disabled = count == 0,
                        .onclick = [&count] { if (count > 0) --count; }},
                       "−"),
                Button({.variant = ButtonVariant::Outlined, .size = Size::Small, .onclick = [&count] { ++count; }},
                       "+"),
                Button({.variant = ButtonVariant::Text, .size = Size::Small, .onclick = [&hidden] { hidden = !hidden; }},
                       hidden ? "Show badge" : "Hide badge"),
            }),
        });
}

// ------------------------------------------------------------------ Chip

std::vector<std::string> default_tags() { return {"News", "Weather", "Traffic", "Sports", "Music"}; }

Node ChipDemo() {
    auto& tags = use_state<std::vector<std::string>>("dd.chip.tags", [] { return default_tags(); });
    size_t& picked = use_state<size_t>("dd.chip.picked", [] { return size_t(1); });
    static const char* const filters[] = {"All", "Live", "Recorded", "Scheduled"};

    Node filled, outlined;
    for (Color c : kAllColors) {
        filled.add_child(Chip({.label = str(as_str(c)), .color = c}));
        outlined.add_child(Chip({.label = str(as_str(c)), .variant = ChipVariant::Outlined, .color = c}));
    }
    Node clickable;
    for (size_t i = 0; i < 4; ++i) {
        bool on = picked == i;
        clickable.add_child(Chip({.label = filters[i],
                                  .variant = on ? ChipVariant::Filled : ChipVariant::Outlined,
                                  .color = on ? Color::Primary : Color::Neutral,
                                  .onclick = [&picked, i] { picked = i; }}));
    }
    Node deletable;
    for (const std::string& tag : tags) {
        deletable.add_child(Chip({.label = tag, .color = Color::Info, .on_delete = [&tags, tag] {
                                      tags.erase(std::remove(tags.begin(), tags.end(), tag), tags.end());
                                  }}));
    }
    if (tags.empty())
        deletable.add_child(Button({.variant = ButtonVariant::Text, .size = Size::Small,
                                    .onclick = [&tags] { tags = default_tags(); }},
                                   "Reset"));

    return Demo(
        "Chip",
        "filled = small colored keycap pill · outlined = white keycap with colored ink · clickable chips press like "
        "keys",
        {
            DemoRow("filled", std::move(filled)),
            DemoRow("outlined", std::move(outlined)),
            DemoRow("small", {
                Chip({.label = "default", .size = Size::Small}),
                Chip({.label = "primary", .color = Color::Primary, .size = Size::Small}),
                Chip({.label = "outlined", .variant = ChipVariant::Outlined, .color = Color::Success,
                      .size = Size::Small}),
                Chip({.label = "REC", .color = Color::Error, .size = Size::Small, .icon = Icon(IconName::Dot)}),
                Chip({.label = "Delete me", .variant = ChipVariant::Outlined, .size = Size::Small,
                      .on_delete = [] {}}),
            }),
            DemoRow("avatar / icon", {
                Chip({.label = "Mika", .avatar = Avatar({.color = Color::Primary}, "M")}),
                Chip({.label = "Glacier", .variant = ChipVariant::Outlined,
                      .avatar = Avatar({.src = photo_n(5, 400), .alt = "Glacier"})}),
                Chip({.label = "Approved", .color = Color::Success, .icon = Icon(IconName::CheckCircle)}),
                Chip({.label = "Clipping", .variant = ChipVariant::Outlined, .color = Color::Warning,
                      .icon = Icon(IconName::Warning)}),
                Chip({.label = "Disabled", .disabled = true, .on_delete = [] {}}),
                Chip({.label = "Disabled", .variant = ChipVariant::Outlined, .color = Color::Primary,
                      .disabled = true}),
            }),
            DemoRow("clickable", std::move(clickable)),
            DemoRow("deletable", std::move(deletable)),
        });
}

// ------------------------------------------------------------------ Divider

Node DividerDemo() {
    return Demo(
        "Divider",
        "the engraved rule: a dark cut with a catch-light · full width / inset / middle · vertical · with text",
        DemoGrid({
            el("div").cls("skeuo-chassis skeuo-elev-1").child(List({.disable_padding = true}, {
                ListItem({}, ListItemText({.primary = "full_width"})),
                Divider(),
                ListItem({}, {
                    ListItemAvatar({}, Avatar({.color = Color::Primary, .size = Size::Small}, "A")),
                    ListItemText({.primary = "inset (lines up with text)"}),
                }),
                Divider({.variant = DividerVariant::Inset}),
                ListItem({}, {
                    ListItemAvatar({}, Avatar({.color = Color::Success, .size = Size::Small}, "B")),
                    ListItemText({.primary = "middle"}),
                }),
                Divider({.variant = DividerVariant::Middle}),
                ListItem({}, ListItemText({.primary = "last row"})),
            })),
            el("div")
                .style("display: flex; flex-direction: column; gap: 18px; justify-content: center")
                .children({
                    Divider({}, "CENTER"),
                    Divider({.text_align = DividerTextAlign::Left}, "Left aligned"),
                    Divider({.text_align = DividerTextAlign::Right},
                            Chip({.label = "Chip in a rule", .size = Size::Small})),
                    el("div")
                        .style("display: flex; align-items: center; gap: 16px; height: 56px; padding: 0 4px; "
                               "font-size: 20px; color: var(--sk-ink-2)")
                        .children({
                            Icon(IconName::Play),
                            Divider({.orientation = Orientation::Vertical, .flex_item = true}),
                            Icon(IconName::Pause),
                            Divider({.orientation = Orientation::Vertical, .variant = DividerVariant::Middle,
                                     .flex_item = true}),
                            Icon(IconName::Volume),
                            Divider({.orientation = Orientation::Vertical, .flex_item = true}, "OR"),
                            Icon(IconName::Mic),
                        }),
                }),
        }));
}

// ------------------------------------------------------------------ List

Node ListDemo() {
    size_t& selected = use_state<size_t>("dd.list.selected", [] { return size_t(1); });
    bool& muted = use_state<bool>("dd.list.muted", [] { return false; });
    const std::pair<IconName, const char*> nav[] = {
        {IconName::Home, "Home"},
        {IconName::Mic, "Live studio"},
        {IconName::Calendar, "Schedule"},
        {IconName::Folder, "Library"},
    };
    Node rows;
    for (size_t i = 0; i < 4; ++i) {
        rows.add_child(ListItem({.disable_padding = true},
                                ListItemButton({.selected = selected == i, .onclick = [&selected, i] { selected = i; }},
                                               {
                                                   ListItemIcon({}, Icon(nav[i].first)),
                                                   ListItemText({.primary = nav[i].second}),
                                               })));
    }
    rows.add_child(ListItem({.disable_padding = true}, ListItemButton({.disabled = true}, {
                                                           ListItemIcon({}, Icon(IconName::Settings)),
                                                           ListItemText({.primary = "Settings (disabled)"}),
                                                       })));

    return Demo(
        "List",
        "rows printed on the plate · selected rows get the color tint · rules between rows on request · subheaders "
        "and secondary actions",
        DemoGrid({
            el("div").cls("skeuo-chassis skeuo-elev-1").child(
                List({.subheader = ListSubheader({}, "Studio"), .aria_label = "Sections"}, std::move(rows))),
            el("div").cls("skeuo-chassis skeuo-elev-1").child(List(
                {.subheader = ListSubheader({.color = Color::Primary}, "On air today")},
                {
                    ListItem({.divider = true,
                              .secondary_action = Badge({.invisible = muted, .variant = BadgeVariant::Dot,
                                                         .color = Color::Error},
                                                        Icon({.name = IconName::Mic, .size = "18px"}))},
                             {
                                 ListItemAvatar({}, Avatar({.src = photo_n(0, 400), .alt = "Morning"})),
                                 ListItemText({.primary = "Morning drive", .secondary = "06:00 – 09:00 · Jamie & Mika"}),
                             }),
                    ListItem({.divider = true, .secondary_action = Icon({.name = IconName::MoreVert, .size = "18px"})},
                             {
                                 ListItemAvatar({}, Avatar({.color = Color::Secondary}, "MT")),
                                 ListItemText({.primary = "Midday talk", .secondary = "11:00 – 13:00 · Sol"}),
                             }),
                    ListItem({.secondary_action = Icon({.name = IconName::MoreVert, .size = "18px"})},
                             {
                                 ListItemAvatar({}, Avatar({.color = Color::Success}, Icon(IconName::Volume))),
                                 ListItemText({.primary = "Evening mix", .secondary = "19:00 – 22:00 · Automated"}),
                             }),
                })),
            el("div").cls("skeuo-chassis skeuo-elev-1").child(List(
                {.dense = true, .subheader = ListSubheader({}, "Dense · success tint"), .color = Color::Success},
                {
                    ListItem({.disable_padding = true},
                             ListItemButton({.selected = !muted, .onclick = [&muted] { muted = false; }}, {
                                                ListItemIcon({}, Icon(IconName::Volume)),
                                                ListItemText({.primary = "Monitor on"}),
                                            })),
                    ListItem({.disable_padding = true},
                             ListItemButton({.selected = muted, .onclick = [&muted] { muted = true; }}, {
                                                ListItemIcon({}, Icon(IconName::Remove)),
                                                ListItemText({.primary = "Muted"}),
                                            })),
                    ListItem({}, ListItemText({.primary = "Inset text",
                                               .secondary = "No icon, aligned with the rows above",
                                               .inset = true})),
                    ListItem({}, {
                                     ListItemIcon({}, Icon(IconName::Info)),
                                     ListItemText({.primary = "Plain row"}),
                                 }),
                })),
        }));
}

// ------------------------------------------------------------------ Table

struct Clip {
    const char* name;
    const char* host;
    unsigned secs;
    unsigned plays;
    float level;
};

constexpr std::array<Clip, 5> kClips = {{
    {"Station ID", "Jamie", 12, 1820, -14.2f},
    {"Weather bed", "Mika", 45, 310, -16.0f},
    {"Traffic sting", "Sol", 6, 942, -12.8f},
    {"News intro", "Lee", 18, 1204, -13.5f},
    {"Sign-off", "Rae", 30, 88, -18.1f},
}};

Node TableDemo() {
    size_t& order_by = use_state<size_t>("dd.table.order_by", [] { return size_t(3); });
    SortDirection& direction = use_state<SortDirection>("dd.table.direction", [] { return SortDirection::Desc; });
    auto& picked =
        use_state<std::optional<std::string>>("dd.table.picked", [] { return std::optional<std::string>("News intro"); });

    std::vector<Clip> rows(kClips.begin(), kClips.end());
    auto less = [&](const Clip& a, const Clip& b) {
        switch (order_by) {
            case 0: return std::string_view(a.name) < std::string_view(b.name);
            case 1: return std::string_view(a.host) < std::string_view(b.host);
            case 2: return a.secs < b.secs;
            case 3: return a.plays < b.plays;
            default: return a.level < b.level;
        }
    };
    if (direction == SortDirection::Desc)
        std::stable_sort(rows.begin(), rows.end(), [&](const Clip& a, const Clip& b) { return less(b, a); });
    else
        std::stable_sort(rows.begin(), rows.end(), less);

    unsigned total = 0;
    for (const Clip& c : kClips) total += c.plays;
    const std::pair<const char*, TableCellAlign> columns[] = {
        {"Clip", TableCellAlign::Left},
        {"Host", TableCellAlign::Left},
        {"Length (s)", TableCellAlign::Right},
        {"Plays", TableCellAlign::Right},
        {"Level (LUFS)", TableCellAlign::Right},
    };

    Node head_row;
    for (size_t i = 0; i < 5; ++i) {
        bool active = order_by == i;
        head_row.add_child(TableCell(
            {.align = columns[i].second,
             .sort_direction = active ? std::optional<SortDirection>(direction) : std::nullopt},
            TableSortLabel({.active = active,
                            .direction = active ? direction : SortDirection::Asc,
                            .onclick =
                                [&order_by, &direction, i] {
                                    if (order_by == i) {
                                        direction = toggled(direction);
                                    } else {
                                        order_by = i;
                                        direction = SortDirection::Asc;
                                    }
                                }},
                           columns[i].first)));
    }
    Node body;
    for (const Clip& clip : rows) {
        std::string name = clip.name;
        char level[16];
        std::snprintf(level, sizeof level, "%.1f", double(clip.level));
        body.add_child(TableRow({.hover = true,
                                 .selected = picked == name,
                                 .onclick =
                                     [&picked, name] {
                                         if (picked == name) picked.reset();
                                         else picked = name;
                                     }},
                                {
                                    TableCell({.variant = TableCellVariant::Head}, name),
                                    TableCell({}, clip.host),
                                    TableCell({.align = TableCellAlign::Right}, std::to_string(clip.secs)),
                                    TableCell({.align = TableCellAlign::Right}, std::to_string(clip.plays)),
                                    TableCell({.align = TableCellAlign::Right}, level),
                                }));
    }

    const std::tuple<const char*, const char*, int> schedule[] = {
        {"06:00", "Morning drive", 180}, {"09:00", "News hour", 60}, {"10:00", "Requests", 60},
        {"11:00", "Midday talk", 120},   {"13:00", "Lunch mix", 60}, {"14:00", "Afternoon", 180},
        {"17:00", "Commute", 120},
    };
    Node schedule_rows;
    for (auto [h, show, m] : schedule)
        schedule_rows.add_child(TableRow({.hover = true}, {
                                                              TableCell({}, h),
                                                              TableCell({}, show),
                                                              TableCell({.align = TableCellAlign::Right},
                                                                        std::to_string(m)),
                                                          }));
    Node checklist_rows;
    for (auto [done, task] : {std::pair{true, "Levels set"}, std::pair{true, "Mic check"}, std::pair{false, "Cue intro"}})
        checklist_rows.add_child(TableRow(
            {.selected = done},
            {
                TableCell({.padding = TableCellPadding::Checkbox},
                          el("span")
                              .style("display: inline-flex; color: var(--c-ink); padding-left: 10px")
                              .child(Icon({.name = done ? IconName::CheckCircle : IconName::Dot, .size = "16px"}))),
                TableCell({}, task),
            }));

    return Demo(
        "Table",
        "a chassis holding printed column heads · engraved rules between rows · hover wash, tinted selection, "
        "sortable heads",
        {
            TableContainer({}, Table({.aria_label = "Clips"}, {
                                                                  TableHead({}, TableRow({}, std::move(head_row))),
                                                                  TableBody({}, std::move(body)),
                                                                  TableFooter({}, TableRow({}, {
                                                                                                   TableCell({.col_span = 3}, "Total plays"),
                                                                                                   TableCell({.align = TableCellAlign::Right}, std::to_string(total)),
                                                                                                   TableCell(),
                                                                                               })),
                                                              })),
            el("div")
                .style("display: grid; grid-template-columns: repeat(auto-fill, minmax(300px, 1fr)); gap: 20px; "
                       "margin-top: 20px")
                .children({
                    TableContainer({.max_height = "188px"},
                                   Table({.size = Size::Small, .sticky_header = true, .aria_label = "Schedule"},
                                         {
                                             TableHead({}, TableRow({}, {
                                                                            TableCell({}, "Time"),
                                                                            TableCell({}, "Show"),
                                                                            TableCell({.align = TableCellAlign::Right}, "Min"),
                                                                        })),
                                             TableBody({}, std::move(schedule_rows)),
                                         })),
                    TableContainer({.elevation = 0},
                                   Table({.size = Size::Small, .color = Color::Success, .aria_label = "Checklist"},
                                         {
                                             TableHead({}, TableRow({}, {
                                                                            TableCell({.padding = TableCellPadding::Checkbox}, ""),
                                                                            TableCell({}, "Check (small, flush, success tint)"),
                                                                        })),
                                             TableBody({}, std::move(checklist_rows)),
                                         })),
                }),
        });
}

// ------------------------------------------------------------------ Tooltip

Node TooltipDemo() {
    bool& open = use_state<bool>("dd.tooltip.open", [] { return true; });
    auto key = [](const char* label) {
        return Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral}, label);
    };
    return Demo(
        "Tooltip",
        "a small floating chassis (level 3) that rises in on hover and keyboard focus · four placements · optional "
        "arrow",
        {
            el("div")
                .style("display: flex; flex-wrap: wrap; align-items: center; justify-content: space-around; gap: "
                       "24px; padding: 44px 64px 52px")
                .children({
                    Tooltip({.title = "Above, with arrow", .placement = Placement::Top, .arrow = true, .open = true},
                            key("Top")),
                    Tooltip({.title = "Below (MUI's default)", .open = true}, key("Bottom")),
                    Tooltip({.title = "Left", .placement = Placement::Left, .arrow = true, .open = true},
                            key("Left")),
                    Tooltip({.title = "Right, with arrow", .placement = Placement::Right, .arrow = true,
                             .open = true},
                            key("Right")),
                }),
            DemoRow("hover / focus", {
                Tooltip({.title = "Delete clip", .arrow = true},
                        Button({.color = Color::Error, .start_icon = Icon(IconName::Delete)}, "Hover me")),
                Tooltip({.title = "Shows on keyboard focus too", .placement = Placement::Top},
                        Chip({.label = "Tab to me", .onclick = [] {}})),
                Tooltip({.title = "Not interactive: the bubble ignores the pointer", .placement = Placement::Right,
                         .disable_interactive = true},
                        Avatar({.color = Color::Info}, Icon(IconName::Info))),
            }),
            DemoRow("controlled",
                    Tooltip({.title = "Controlled by the switch", .placement = Placement::Right, .arrow = true,
                             .open = open},
                            Chip({.label = open ? "open: Some(true)" : "open: Some(false)",
                                  .color = open ? Color::Primary : Color::Neutral,
                                  .onclick = [&open] { open = !open; }}))),
        });
}

// ------------------------------------------------------------------ ImageList

Node ImageListDemo() {
    const std::pair<unsigned, unsigned> quilted[] = {{2, 2}, {1, 1}, {1, 1}, {1, 2}, {1, 1}, {2, 1}, {1, 1}};
    const unsigned masonry_heights[] = {300, 460, 260, 380, 520, 300, 420, 280};
    auto img = [](size_t i, unsigned h, size_t alt) {
        return el("img").attr("src", photo_n(i, h)).attr("alt", kPhotos[alt % 9].name);
    };

    Node standard;
    for (size_t i = 0; i < 6; ++i) {
        Node item = img(i, 300, i);
        if (i < 3)
            item = Node{std::move(item),
                        ImageListItemBar({.title = kPhotos[i].name,
                                          .action_icon = Icon(IconName::StarBorder),
                                          .position = i == 1 ? ImageListItemBarPosition::Top
                                                             : ImageListItemBarPosition::Bottom})};
        standard.add_child(ImageListItem({}, std::move(item)));
    }
    Node quilt;
    for (size_t i = 0; i < 7; ++i)
        quilt.add_child(ImageListItem({.cols = quilted[i].first, .rows = quilted[i].second}, img(i + 2, 300, i + 2)));
    Node masonry;
    for (size_t i = 0; i < 8; ++i) masonry.add_child(ImageListItem({}, img(i + 4, masonry_heights[i], i + 4)));
    Node woven;
    for (size_t i = 0; i < 6; ++i)
        woven.add_child(ImageListItem({}, {
                                              img(i + 1, 400, i + 1),
                                              ImageListItemBar({.title = kPhotos[(i + 1) % 9].name,
                                                                .subtitle = "by Studio",
                                                                .position = ImageListItemBarPosition::Below}),
                                          }));

    auto overline = [](const char* t) {
        return Typography({.variant = TypographyVariant::Overline, .gutter_bottom = true}, t);
    };
    return Demo(
        "ImageList",
        "photos inset into wells · standard / quilted / masonry / woven · caption bars over or below the image",
        el("div")
            .style("display: grid; grid-template-columns: repeat(auto-fill, minmax(340px, 1fr)); gap: 28px 32px")
            .children({
                el("div").children({overline("standard · bars"),
                                    ImageList({.cols = 3, .gap = 8, .row_height = 96}, std::move(standard))}),
                el("div").children({overline("quilted"),
                                    ImageList({.variant = ImageListVariant::Quilted, .cols = 4, .gap = 6,
                                               .row_height = 64},
                                              std::move(quilt))}),
                el("div").children({overline("masonry"),
                                    ImageList({.variant = ImageListVariant::Masonry, .cols = 3, .gap = 8},
                                              std::move(masonry))}),
                el("div").children({overline("woven · bar below"),
                                    ImageList({.variant = ImageListVariant::Woven, .cols = 3, .gap = 8,
                                               .row_height = 150},
                                              std::move(woven))}),
            }));
}

}  // namespace

Node DataDisplayPage() {
    return {TypographyDemo(), AvatarDemo(), BadgeDemo(), ChipDemo(), DividerDemo(),
            ListDemo(),       TableDemo(),  TooltipDemo(), ImageListDemo()};
}

}  // namespace gallery
