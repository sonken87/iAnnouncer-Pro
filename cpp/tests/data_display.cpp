// Port of crates/skeuo-ui/tests/data_display.rs plus handler wiring tests.
#include <regex>

#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {

size_t count(const std::string& hay, const std::string& needle) {
    size_t n = 0;
    for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + needle.size())) ++n;
    return n;
}

/// The handler id of the `type` listener on the element whose start tag
/// contains `needle` (the `nth` such tag), or -1.
int handler_id(const std::string& html, const std::string& needle, const std::string& type, size_t nth = 0) {
    size_t pos = html.find(needle);
    for (size_t i = 0; i < nth && pos != std::string::npos; ++i) pos = html.find(needle, pos + 1);
    if (pos == std::string::npos) return -1;
    size_t start = html.rfind('<', pos);
    size_t end = html.find('>', pos);
    std::string tag = html.substr(start, end - start);
    std::smatch m;
    if (std::regex_search(tag, m, std::regex("[\",]" + type + ":(\\d+)"))) return std::stoi(m[1]);
    return -1;
}

std::string event(int id, const std::string& type, const std::string& extra = "") {
    return R"({"t":"event","h":[)" + std::to_string(id) + R"(],"e":{"type":")" + type + "\"" + extra + "}}";
}

}  // namespace

// ---------------------------------------------------------------- Typography

TEST(data_display_typography_default_is_body1_paragraph) {
    std::string html = render_static([] { return Typography({}, "Hello"); });
    CHECK(html.rfind(R"(<p class="skeuo-typography skeuo-typography--body1 skeuo-typography--text-primary">Hello</p>)", 0) == 0) << html;
}

TEST(data_display_typography_variant_mapping_and_props) {
    std::string html = render_static([] {
        return Node{
            Typography({.variant = TypographyVariant::H2, .gutter_bottom = true}, "Title"),
            Typography({.variant = TypographyVariant::Subtitle1}, "Sub"),
            Typography({.variant = TypographyVariant::Caption, .color = TypographyColor::TextSecondary}, "Cap"),
            Typography({.variant = TypographyVariant::Overline, .align = TypographyAlign::Center, .no_wrap = true}, "Over"),
            Typography({.variant = TypographyVariant::Caption, .paragraph = true}, "Para"),
            Typography({.color = Color::Error, .id = "t1", .cls = "mine"}, "Ink"),
        };
    });
    for (const char* needle : {
             R"(<h2 class="skeuo-typography skeuo-typography--h2 skeuo-typography--text-primary skeuo-typography--gutter-bottom">Title</h2>)",
             R"(<h6 class="skeuo-typography skeuo-typography--subtitle1)",
             R"(<span class="skeuo-typography skeuo-typography--caption skeuo-typography--text-secondary">Cap</span>)",
             "skeuo-typography--align-center skeuo-typography--no-wrap",
             R"(<p class="skeuo-typography skeuo-typography--caption skeuo-typography--text-primary skeuo-typography--paragraph">Para</p>)",
             R"(class="skeuo-typography skeuo-typography--body1 skeuo-typography--ink skeuo-color-error mine")",
             R"(id="t1")",
         })
        CONTAINS(html, needle);
}

TEST(data_display_typography_component_overrides_tag) {
    std::string html = render_static([] {
        return Node{
            Typography({.variant = TypographyVariant::H1, .component = "div"}, "A"),
            Typography({.variant = TypographyVariant::Body2, .component = "label"}, "B"),
            Typography({.component = "nonsense"}, "C"),
        };
    });
    CONTAINS(html, R"(<div class="skeuo-typography skeuo-typography--h1)");
    CONTAINS(html, R"(<label class="skeuo-typography skeuo-typography--body2)");
    CONTAINS(html, R"(<span class="skeuo-typography skeuo-typography--body1)");
    LACKS(html, "<h1");
}

TEST(data_display_typography_colors_and_style) {
    std::string html = render_static([] {
        return Node{
            Typography({.color = TypographyColor::TextDisabled}, "a"),
            Typography({.color = TypographyColor::Inherit, .style = "margin: 0"}, "b"),
        };
    });
    CONTAINS(html, "skeuo-typography--text-disabled");
    CONTAINS(html, R"(skeuo-typography--color-inherit" style="margin: 0")");
    CHECK(TypographyColor(Color::Info) == TypographyColor(Color::Info));
    CHECK(!(TypographyColor(Color::Info) == TypographyColor::TextPrimary));
}

// -------------------------------------------------------------------- Avatar

TEST(data_display_avatar_letter_is_a_keycap) {
    std::string html = render_static([] {
        return Node{
            Avatar({.alt = "Jamie"}, "JA"),
            Avatar({.variant = AvatarVariant::Rounded, .color = Color::Primary, .size = Size::Large}, "MK"),
        };
    });
    for (const char* needle : {
             R"(class="skeuo-key skeuo-avatar--letter skeuo-avatar skeuo-avatar--circular skeuo-avatar--medium")",
             R"(role="img")",
             R"(aria-label="Jamie")",
             ">JA<",
             R"(class="skeuo-key skeuo-key--filled skeuo-avatar--letter skeuo-avatar skeuo-avatar--rounded skeuo-avatar--large skeuo-color-primary")",
             ">MK<",
         })
        CONTAINS(html, needle);
    // No accessible name given: no role.
    CHECK(count(html, "role=") == 1) << html;
}

TEST(data_display_avatar_image_and_fallbacks) {
    std::string html = render_static([] {
        return Node{
            Avatar({.src = "a.png", .alt = "Photo", .variant = AvatarVariant::Square}),
            Avatar({.alt = "zoe"}),
            Avatar(),
        };
    });
    for (const char* needle : {
             R"(class="skeuo-well skeuo-avatar--img skeuo-avatar skeuo-avatar--square skeuo-avatar--medium")",
             R"(src="a.png")",
             R"(alt="Photo")",
             ">Z<",
             "skeuo-avatar__fallback",
         })
        CONTAINS(html, needle);
}

TEST(data_display_avatar_image_error_falls_back_to_letter) {
    Runtime rt([] { return Avatar({.src = "missing.png", .alt = "dawn"}); });
    std::string html = rt.render();
    CONTAINS(html, "onerror=");
    int id = handler_id(html, "skeuo-avatar__img", "animationend");
    CHECK(id >= 0) << html;
    CONTAINS(html, "animationend:" + std::to_string(id) + "!");  // stops at the <img>
    rt.handle(event(id, "animationend"));
    html = rt.render();
    CONTAINS(html, ">D<");
    CONTAINS(html, "skeuo-avatar--letter");
    LACKS(html, "<img");
}

TEST(data_display_avatar_group_max_and_surplus) {
    auto avatars = [](size_t n) {
        std::vector<Node> out;
        for (size_t i = 0; i < n; ++i) out.push_back(Avatar({}, std::to_string(i)));
        return out;
    };
    std::string html = render_static([&] { return AvatarGroup({.avatars = avatars(6), .max = 4}); });
    CONTAINS(html, R"(role="group")");
    // 3 shown + the surplus cap.
    CHECK(count(html, "skeuo-avatar-group__item") == 4) << html;
    CONTAINS(html, ">+3<");
    CONTAINS(html, R"(aria-label="3 more")");
    CONTAINS(html, "skeuo-avatar--surplus");

    html = render_static([&] { return AvatarGroup({.avatars = avatars(3), .total = 20}); });
    CHECK(count(html, "skeuo-avatar-group__item") == 4) << html;
    CONTAINS(html, ">+17<");

    html = render_static([&] { return AvatarGroup({.avatars = avatars(4), .max = 4}); });
    CHECK(count(html, "skeuo-avatar-group__item") == 4) << html;
    LACKS(html, "+");

    html = render_static([&] { return AvatarGroup({.avatars = avatars(2), .spacing = AvatarGroupSpacing::Px(12.0)}); });
    CONTAINS(html, "--avatar-overlap: 12px");
    html = render_static([&] { return AvatarGroup({.avatars = avatars(2), .spacing = AvatarGroupSpacing::Small}); });
    CONTAINS(html, "--avatar-overlap: 16px");
    html = render_static([&] { return AvatarGroup({.avatars = avatars(2), .spacing = AvatarGroupSpacing::Px(2.5)}); });
    CONTAINS(html, "--avatar-overlap: 2.5px");
}

// --------------------------------------------------------------------- Badge

TEST(data_display_badge_is_an_led_with_capped_count) {
    std::string html = render_static([] {
        return Node{
            Badge({.badge_content = 4}, "A"),
            Badge({.badge_content = 120, .color = Color::Error}, "B"),
            Badge({.badge_content = 1200, .max = 999}, "C"),
            Badge({.badge_content = "new"}, "D"),
        };
    });
    for (const char* needle : {
             R"(<span class="skeuo-badge">A<span class="skeuo-led skeuo-badge__badge skeuo-badge__badge--standard skeuo-badge__badge--top-right skeuo-badge__badge--rectangular skeuo-color-primary">4</span></span>)",
             "skeuo-color-error\">99+<",
             ">999+<",
             ">new<",
         })
        CONTAINS(html, needle);
    LACKS(html, "invisible");
}

TEST(data_display_badge_visibility_rules) {
    std::string html = render_static([] {
        return Node{
            Badge({.badge_content = 0}, "zero"),
            Badge({.badge_content = 0, .show_zero = true}, "shown"),
            Badge({.badge_content = 5, .invisible = true}, "hidden"),
            Badge({}, "empty"),
            Badge({.variant = BadgeVariant::Dot}, "dot"),
        };
    });
    // zero, hidden and empty are invisible; show_zero and dot are not.
    CHECK(count(html, "skeuo-badge__badge--invisible") == 3) << html;
    CHECK(count(html, R"(aria-hidden="true")") == 3) << html;
    CONTAINS(html, "shown<span");
    CONTAINS(html, ">0</span>");
    CONTAINS(html, "skeuo-badge__badge--dot skeuo-badge__badge--top-right");
}

TEST(data_display_badge_anchor_and_overlap) {
    std::string html = render_static([] {
        return Badge({.badge_content = 1, .anchor_origin = BadgeAnchorOrigin::BOTTOM_LEFT,
                      .overlap = BadgeOverlap::Circular, .cls = "mine"},
                     "x");
    });
    CONTAINS(html, R"(class="skeuo-badge mine")");
    CONTAINS(html, "skeuo-badge__badge--bottom-left skeuo-badge__badge--circular");
}

TEST(data_display_badge_content_conversions) {
    CHECK(BadgeContent(-3).kind == BadgeContent::Kind::Number && BadgeContent(-3).number == 0);
    CHECK(BadgeContent(std::string("hi")).kind == BadgeContent::Kind::Text);
    CHECK(BadgeContent(7ull).number == 7);
    CHECK(BadgeContent().kind == BadgeContent::Kind::Empty);
}

// ---------------------------------------------------------------------- Chip

TEST(data_display_chip_static_variants) {
    std::string html = render_static([] {
        return Node{
            Chip({.label = "Plain"}),
            Chip({.label = "Blue", .color = Color::Primary}),
            Chip({.label = "Out", .variant = ChipVariant::Outlined, .color = Color::Error, .size = Size::Small}),
        };
    });
    CONTAINS(html, R"(<div class="skeuo-key skeuo-chip skeuo-chip--filled skeuo-color-neutral skeuo-chip--medium"><span class="skeuo-chip__label">Plain</span></div>)");
    CONTAINS(html, R"(class="skeuo-key skeuo-key--filled skeuo-chip skeuo-chip--filled skeuo-color-primary skeuo-chip--medium")");
    CONTAINS(html, R"(class="skeuo-key skeuo-chip skeuo-chip--outlined skeuo-color-error skeuo-chip--small")");
    LACKS(html, "<button");
}

TEST(data_display_chip_clickable_and_deletable) {
    std::string html = render_static([] {
        return Node{
            Chip({.label = "Tap", .onclick = [] {}}),
            Chip({.label = "Tag", .icon = Icon(IconName::Star), .on_delete = [] {}}),
            Chip({.label = "Off", .disabled = true, .onclick = [] {}, .on_delete = [] {}}),
        };
    });
    for (const char* needle : {
             "skeuo-key--pressable skeuo-key--focus-within skeuo-chip--clickable",
             R"(<button class="skeuo-chip__action" type="button"><span class="skeuo-chip__label">Tap</span></button>)",
             "skeuo-chip--deletable",
             R"(aria-label="Delete Tag")",
             "skeuo-chip__icon",
             "skeuo-chip--disabled",
             R"(aria-disabled="true")",
         })
        CONTAINS(html, needle);
    CHECK(count(html, " disabled>") == 2) << html;
}

TEST(data_display_chip_with_avatar) {
    std::string html = render_static([] { return Chip({.label = "Mika", .avatar = Avatar({}, "M")}); });
    CONTAINS(html, R"(<span class="skeuo-chip__avatar"><span class="skeuo-key skeuo-avatar--letter)");
}

TEST(data_display_chip_click_and_delete_reach_handlers) {
    int clicks = 0, deletes = 0;
    Runtime rt([&] {
        return Chip({.label = "Tag", .onclick = [&] { ++clicks; }, .on_delete = [&] { ++deletes; }});
    });
    std::string html = rt.render();
    int click = handler_id(html, "skeuo-chip__action", "click");
    int del = handler_id(html, "skeuo-chip__delete", "click");
    CHECK(click >= 0 && del >= 0) << html;
    CONTAINS(html, "click:" + std::to_string(del) + "!");  // delete stops propagation
    rt.handle(event(click, "click"));
    rt.handle(event(del, "click"));
    CHECK(clicks == 1 && deletes == 1);
}

// ------------------------------------------------------------------- Divider

TEST(data_display_divider_is_an_engraved_rule) {
    std::string html = render_static([] {
        return Node{
            Divider(),
            Divider({.variant = DividerVariant::Inset}),
            Divider({.orientation = Orientation::Vertical, .variant = DividerVariant::Middle, .flex_item = true}),
        };
    });
    CONTAINS(html, R"(<hr class="skeuo-rule skeuo-divider skeuo-divider--full-width skeuo-divider--horizontal">)");
    CONTAINS(html, "skeuo-divider--inset");
    CONTAINS(html, R"(<hr class="skeuo-rule skeuo-rule--vertical skeuo-divider skeuo-divider--middle skeuo-divider--vertical skeuo-divider--flex-item" aria-orientation="vertical">)");
}

TEST(data_display_divider_with_text) {
    std::string html = render_static([] { return Divider({.text_align = DividerTextAlign::Left}, "OR"); });
    CONTAINS(html, R"(role="separator")");
    CONTAINS(html, "skeuo-divider--with-children skeuo-divider--text-left");
    CONTAINS(html, R"(<span class="skeuo-divider__text skeuo-label">OR</span>)");
    CHECK(count(html, "skeuo-rule skeuo-divider__line") == 2) << html;
    LACKS(html, "<hr");
}

// ---------------------------------------------------------------------- List

TEST(data_display_list_family_markup) {
    std::string html = render_static([] {
        return List({.dense = true, .subheader = ListSubheader({}, "Section"), .aria_label = "Nav"},
                    {
                        ListItem({.disable_padding = true},
                                 ListItemButton({.selected = true}, {
                                                    ListItemIcon({}, Icon(IconName::Home)),
                                                    ListItemText({.primary = "Home", .secondary = "Start"}),
                                                })),
                        ListItem({.divider = true, .secondary_action = Node("⋮")},
                                 {
                                     ListItemAvatar({}, Avatar({}, "A")),
                                     ListItemText({}, "Child primary"),
                                 }),
                        ListItem({}, ListItemButton({.disabled = true}, ListItemText({.primary = "Off", .inset = true}))),
                    });
    });
    for (const char* needle : {
             R"(<ul class="skeuo-list skeuo-color-primary skeuo-list--dense skeuo-list--with-subheader" aria-label="Nav">)",
             R"(<li class="skeuo-list-subheader skeuo-label skeuo-list-subheader--sticky">Section</li>)",
             R"(class="skeuo-list-item skeuo-list-item--dense skeuo-list-item--no-padding")",
             R"(class="skeuo-list-item-button skeuo-list-item--dense skeuo-list-item-button--selected")",
             R"(aria-current="true")",
             R"(<span class="skeuo-list-item-icon">)",
             R"(<span class="skeuo-list-item-text skeuo-list-item-text--multiline"><span class="skeuo-list-item-text__primary">Home</span><span class="skeuo-list-item-text__secondary skeuo-caption">Start</span></span>)",
             "skeuo-list-item--divider skeuo-list-item--with-action",
             R"(<span class="skeuo-list-item__secondary-action">⋮</span>)",
             R"(<span class="skeuo-list-item-avatar">)",
             ">Child primary<",
             "skeuo-list-item-text--inset",
             " disabled",
         })
        CONTAINS(html, needle);
    CHECK(count(html, "aria-current") == 1) << html;
    // Every row is dense, exactly once.
    CHECK(count(html, "skeuo-list-item--dense") == 5) << html;
}

TEST(data_display_list_dense_stops_at_nested_lists) {
    std::string html = render_static([] {
        return List({.dense = true}, ListItem({}, List({}, ListItem({.cls = "inner"}, "x"))));
    });
    CONTAINS(html, R"(<li class="skeuo-list-item skeuo-list-item--dense">)");
    CONTAINS(html, R"(<li class="skeuo-list-item inner">)");
}

TEST(data_display_list_subheader_color_and_flags) {
    std::string html = render_static([] {
        return ListSubheader({.color = Color::Success, .disable_sticky = true, .inset = true, .id = "s"}, "H");
    });
    CONTAINS(html, R"(<li class="skeuo-list-subheader skeuo-label skeuo-color-success skeuo-list-subheader--colored skeuo-list-subheader--inset" id="s">H</li>)");
}

TEST(data_display_list_item_button_click_and_autofocus) {
    int selected = 0;
    Runtime rt([&] {
        return List({}, {
                            ListItemButton({.selected = selected == 0, .onclick = [&] { selected = 0; }}, "A"),
                            ListItemButton({.selected = selected == 1, .autofocus = true, .onclick = [&] { selected = 1; }}, "B"),
                        });
    });
    std::string html = rt.render();
    CONTAINS(html, "data-sk-autofocus");
    int b = handler_id(html, "data-sk-autofocus", "click");
    rt.handle(event(b, "click"));
    html = rt.render();
    CHECK(selected == 1);
    CHECK(count(html, "aria-current") == 1) << html;
}

// --------------------------------------------------------------------- Table

TEST(data_display_table_family_markup) {
    std::string html = render_static([] {
        return TableContainer(
            {.max_height = "200px"},
            Table({.size = Size::Small, .sticky_header = true, .aria_label = "Clips"},
                  {
                      TableHead({}, TableRow({}, {
                                                     TableCell({.sort_direction = SortDirection::Desc},
                                                               TableSortLabel({.active = true, .direction = SortDirection::Desc}, "Name")),
                                                     TableCell({.align = TableCellAlign::Right}, "Plays"),
                                                 })),
                      TableBody({}, {
                                        TableRow({.hover = true, .selected = true},
                                                 {
                                                     TableCell({.variant = TableCellVariant::Head}, "Intro"),
                                                     TableCell({.align = TableCellAlign::Right}, "12"),
                                                 }),
                                        TableRow({}, {
                                                         TableCell({.padding = TableCellPadding::Checkbox}, "x"),
                                                         TableCell({.padding = TableCellPadding::None, .col_span = 2}, "y"),
                                                     }),
                                    }),
                      TableFooter({}, TableRow({}, TableCell({}, "Total"))),
                  }));
    });
    for (const char* needle : {
             R"(<div class="skeuo-chassis skeuo-elev-1 skeuo-table-container" style="max-height: 200px;">)",
             R"(<table class="skeuo-table skeuo-color-primary skeuo-table--small skeuo-table--sticky-header" aria-label="Clips">)",
             R"(<thead class="skeuo-table-head">)",
             R"(<tr class="skeuo-table-row skeuo-table-row--head">)",
             R"(<th class="skeuo-table-cell skeuo-table-cell--head skeuo-table-cell--small skeuo-table-cell--sticky" scope="col" aria-sort="descending">)",
             "skeuo-table-sort-label skeuo-table-sort-label--active skeuo-table-sort-label--desc",
             "skeuo-table-sort-label__icon",
             "skeuo-table-cell--head skeuo-table-cell--align-right",
             R"(<tr class="skeuo-table-row skeuo-table-row--hover skeuo-table-row--selected" aria-selected="true">)",
             R"(<th class="skeuo-table-cell skeuo-table-cell--head skeuo-table-cell--small" scope="row">Intro</th>)",
             R"(<td class="skeuo-table-cell skeuo-table-cell--body skeuo-table-cell--align-right skeuo-table-cell--small">12</td>)",
             "skeuo-table-cell--padding-checkbox",
             R"(colspan="2")",
             "skeuo-table-cell--padding-none",
             R"(<tfoot class="skeuo-table-footer"><tr class="skeuo-table-row skeuo-table-row--footer"><td class="skeuo-table-cell skeuo-table-cell--footer skeuo-table-cell--small">Total</td>)",
         })
        CONTAINS(html, needle);
    // The helper attribute is gone once a Table has resolved its cells.
    LACKS(html, "data-sk-cell");
}

TEST(data_display_table_cell_overrides_and_padding) {
    std::string html = render_static([] {
        return Table({.padding = TableCellPadding::None},
                     TableBody({}, TableRow({}, {
                                                    TableCell({}, "a"),
                                                    TableCell({.padding = TableCellPadding::Normal, .size = Size::Small,
                                                               .scope = "rowgroup", .style = "width: 1px"},
                                                              "b"),
                                                })));
    });
    CONTAINS(html, R"(<td class="skeuo-table-cell skeuo-table-cell--body skeuo-table-cell--padding-none">a</td>)");
    CONTAINS(html, R"(<td class="skeuo-table-cell skeuo-table-cell--body skeuo-table-cell--small" style="width: 1px" scope="rowgroup">b</td>)");
    // Standalone cells render as body cells of a default table.
    std::string lone = render_static([] { return TableCell({.cls = "x"}, "c"); });
    CONTAINS(lone, R"(<td class="skeuo-table-cell skeuo-table-cell--body x")");
}

TEST(data_display_table_sort_label_inactive_hides_icon_when_asked) {
    std::string html = render_static([] {
        return Node{
            TableSortLabel({}, "A"),
            TableSortLabel({.hide_sort_icon = true}, "B"),
            TableSortLabel({.active = true, .hide_sort_icon = true}, "C"),
        };
    });
    CHECK(count(html, "skeuo-table-sort-label__icon") == 2) << html;
    CHECK(count(html, R"(type="button")") == 3) << html;
    CONTAINS(html, "skeuo-table-sort-label--asc");
}

TEST(data_display_table_container_elevation) {
    std::string html = render_static([] { return TableContainer({.elevation = 0}, Table({}, "")); });
    CONTAINS(html, "skeuo-chassis skeuo-elev-0");
    CONTAINS(html, "skeuo-table--medium");
}

TEST(data_display_table_sort_and_row_clicks_reach_handlers) {
    SortDirection dir = SortDirection::Asc;
    bool picked = false;
    Runtime rt([&] {
        return Table({}, {
                             TableHead({}, TableRow({}, TableCell({.sort_direction = dir},
                                                                  TableSortLabel({.active = true, .direction = dir,
                                                                                  .onclick = [&] { dir = toggled(dir); }},
                                                                                 "Name")))),
                             TableBody({}, TableRow({.selected = picked, .onclick = [&] { picked = !picked; }},
                                                    TableCell({}, "x"))),
                         });
    });
    std::string html = rt.render();
    CONTAINS(html, "skeuo-table-row--clickable");
    rt.handle(event(handler_id(html, "skeuo-table-sort-label", "click"), "click"));
    html = rt.render();
    CHECK(dir == SortDirection::Desc);
    CONTAINS(html, R"(aria-sort="descending")");
    CONTAINS(html, "skeuo-table-sort-label--desc");
    rt.handle(event(handler_id(html, "skeuo-table-row--clickable", "click"), "click"));
    html = rt.render();
    CHECK(picked);
    CONTAINS(html, "skeuo-table-row--selected");
}

// ------------------------------------------------------------------- Tooltip

TEST(data_display_tooltip_uncontrolled_and_forced_open) {
    std::string html = render_static([] {
        return Node{
            Tooltip({.title = "Delete", .id = "tip"}, el("button").attr("aria-describedby", "tip").child("x")),
            Tooltip({.title = "Shown", .placement = Placement::Top, .arrow = true, .open = true}, el("span").child("y")),
            Tooltip({.title = "Never", .open = false, .disable_interactive = true}, el("span").child("z")),
            Tooltip({.title = ""}, el("span").child("empty")),
        };
    });
    for (const char* needle : {
             R"(<span class="skeuo-tooltip skeuo-tooltip--auto skeuo-tooltip--bottom skeuo-tooltip--interactive")",
             R"(<span class="skeuo-tooltip__popper" role="tooltip" id="tip"><span class="skeuo-tooltip__bubble skeuo-chassis skeuo-chassis--float skeuo-elev-3">Delete</span></span>)",
             "skeuo-tooltip skeuo-tooltip--open skeuo-tooltip--top skeuo-tooltip--arrow skeuo-tooltip--interactive",
             R"(<span class="skeuo-tooltip__arrow" aria-hidden="true"></span>)",
             R"(class="skeuo-tooltip skeuo-tooltip--closed skeuo-tooltip--bottom")",
         })
        CONTAINS(html, needle);
    // An empty title renders no bubble.
    CHECK(count(html, R"(role="tooltip")") == 3) << html;
    // Generated ids are used when none is given.
    CONTAINS(html, R"(id="skeuo-tooltip-)");
}

TEST(data_display_tooltip_escape_dismisses_until_pointer_leaves) {
    int closes = 0;
    Runtime rt([&] { return Tooltip({.title = "Tip", .on_close = [&] { ++closes; }}, el("button").child("x")); });
    std::string html = rt.render();
    CONTAINS(html, R"(data-sk-keys="Escape")");
    // Pure CSS while nothing needs C++: no hover/focus listeners without callbacks.
    LACKS(html, "mouseenter");
    rt.handle(event(handler_id(html, "skeuo-tooltip ", "keydown"), "keydown", R"(,"key":"Escape")"));
    html = rt.render();
    CHECK(closes == 1);
    CONTAINS(html, "skeuo-tooltip--closed");
    rt.handle(event(handler_id(html, "skeuo-tooltip ", "mouseleave"), "mouseleave"));
    html = rt.render();
    CHECK(closes == 2);
    CONTAINS(html, "skeuo-tooltip--auto");
}

TEST(data_display_tooltip_without_callbacks_has_only_escape) {
    Runtime rt([] { return Tooltip({.title = "Tip"}, "x"); });
    std::string html = rt.render();
    CONTAINS(html, R"(data-sk-on="keydown:0")");
    CHECK(rt.handler_count() == 1);
}

// ----------------------------------------------------------------- ImageList

TEST(data_display_image_list_layout_and_items) {
    std::string html = render_static([] {
        return ImageList({.variant = ImageListVariant::Quilted, .cols = 4, .gap = 6, .row_height = 80},
                         {
                             ImageListItem({.cols = 2, .rows = 2},
                                           {
                                               el("img").attr("src", "a.png").attr("alt", "A"),
                                               ImageListItemBar({.title = "A", .subtitle = "by B", .action_icon = Node("★")}),
                                           }),
                             ImageListItem({}, {
                                                   el("img").attr("src", "b.png").attr("alt", "B"),
                                                   ImageListItemBar({.title = "B", .action_position_left = true,
                                                                     .position = ImageListItemBarPosition::Below}),
                                               }),
                         });
    });
    for (const char* needle : {
             R"(<ul class="skeuo-image-list skeuo-image-list--quilted" style="--image-list-cols: 4; --image-list-gap: 6px; grid-auto-rows: 80px;">)",
             R"(<li class="skeuo-image-list-item" style="grid-column-end: span 2; grid-row-end: span 2;">)",
             R"(<span class="skeuo-well skeuo-image-list-item__rim" aria-hidden="true"></span>)",
             R"(class="skeuo-image-list-item-bar skeuo-image-list-item-bar--bottom")",
             R"(<span class="skeuo-image-list-item-bar__title">A</span><span class="skeuo-image-list-item-bar__subtitle">by B</span>)",
             R"(<span class="skeuo-image-list-item-bar__action">★</span>)",
             "skeuo-image-list-item-bar--below skeuo-image-list-item-bar--action-left",
         })
        CONTAINS(html, needle);
}

TEST(data_display_image_list_masonry_items_do_not_span) {
    std::string html = render_static([] {
        return ImageList({.variant = ImageListVariant::Masonry, .cols = 3},
                         {
                             ImageListItem({.cols = 2}, el("img").attr("src", "a.png")),
                             ImageListItem({.style = "opacity: .5"}, el("img").attr("src", "b.png")),
                         });
    });
    CONTAINS(html, "skeuo-image-list--masonry");
    CONTAINS(html, "--image-list-cols: 3; --image-list-gap: 4px; \"");
    LACKS(html, "span 2");
    CONTAINS(html, R"(<li class="skeuo-image-list-item"><img)");
    CONTAINS(html, R"(<li class="skeuo-image-list-item" style="opacity: .5">)");
}
