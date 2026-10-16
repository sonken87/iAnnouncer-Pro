#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {

size_t count(const std::string& hay, const std::string& needle) {
    size_t n = 0;
    for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + 1)) ++n;
    return n;
}

std::string attr_value(const std::string& html, const std::string& name, size_t from = 0) {
    size_t start = html.find(name + "=\"", from);
    if (start == std::string::npos) return {};
    start += name.size() + 2;
    return html.substr(start, html.find('"', start) - start);
}

/// The id of the first handler registered for `type` on the element that
/// carries `marker` (a class or attribute text) in the rendered html.
std::string handler_after(const std::string& html, const std::string& marker, const std::string& type) {
    size_t at = html.find(marker);
    if (at == std::string::npos) return {};
    size_t tag_end = html.find('>', at);
    size_t on = html.find("data-sk-on=\"", at);
    if (on == std::string::npos || on > tag_end) return {};
    std::string spec = attr_value(html, "data-sk-on", at);
    size_t p = spec.find(type + ":");
    if (p == std::string::npos) return {};
    p += type.size() + 1;
    size_t end = p;
    while (end < spec.size() && isdigit(static_cast<unsigned char>(spec[end]))) ++end;
    return spec.substr(p, end - p);
}

std::string event(const std::string& id, const std::string& type, const std::string& extra = "") {
    return R"({"t":"event","h":[)" + id + R"(],"e":{"type":")" + type + "\"" + extra + "}}";
}

}  // namespace

// ---------------------------------------------------------------- Paper

TEST(surfaces_elevation_maps_onto_chassis_levels) {
    const int pairs[][2] = {{0, 0}, {1, 1}, {2, 1}, {3, 2}, {6, 2}, {7, 3}, {12, 3}, {13, 4}, {24, 4}};
    for (auto [e, l] : pairs) CHECK(chassis_level(e) == l) << "elevation " << e;
}

TEST(surfaces_paper_default_classes) {
    std::string html = render_static([] { return Paper({}, "Hi"); });
    CONTAINS(html, R"(class="skeuo-chassis skeuo-elev-1 skeuo-paper skeuo-paper--elevation")");
    CONTAINS(html, "Hi");
}

TEST(surfaces_paper_floating_square_and_attributes) {
    std::string html = render_static([] {
        return Paper({.elevation = 16, .square = true, .cls = "x", .attrs = {{"id", "p1"}, {"style", "padding:4px"}}},
                     {});
    });
    CONTAINS(html, "skeuo-chassis skeuo-elev-4 skeuo-chassis--float skeuo-chassis--square");
    CONTAINS(html, " x\"");
    CONTAINS(html, R"(id="p1")");
    CONTAINS(html, "padding:4px");
}

TEST(surfaces_paper_outlined_is_flush) {
    std::string html = render_static([] { return Paper({.elevation = 8, .variant = PaperVariant::Outlined}, {}); });
    CONTAINS(html, "skeuo-elev-0");
    CONTAINS(html, "skeuo-paper--outlined");
    LACKS(html, "--float");
}

// ---------------------------------------------------------------- Card

TEST(surfaces_card_parts_render) {
    std::string html = render_static([] {
        return Card({.raised = true}, {
            CardHeader({.avatar = el("span").child("AV"), .title = "Title", .subheader = "Sub",
                        .action = el("button").child("act")}),
            CardMedia({.image = "a.png", .title = "Pic", .height = "140px"}),
            CardContent({}, "Body"),
            CardActions({}, el("button").child("OK")),
        });
    });
    for (const char* needle :
         {"skeuo-elev-2", "skeuo-card", "skeuo-card-header__avatar", R"(<span class="skeuo-card-header__title">Title</span>)",
          "skeuo-card-header__subheader", "skeuo-card-header__action", "background-image:url(", "a.png",
          "height:140px;", R"(role="img")", R"(aria-label="Pic")", R"(class="skeuo-card-content")",
          R"(class="skeuo-card-actions skeuo-card-actions--spacing")"})
        CONTAINS(html, needle);
}

TEST(surfaces_card_media_img_and_action_area) {
    std::string html = render_static([] {
        return Card({}, {
            CardActionArea({}, CardMedia({.image = "b.png", .component = "img", .alt = "Bee"})),
            CardActions({.disable_spacing = true}, "x"),
        });
    });
    for (const char* needle : {"<img", R"(src="b.png")", R"(alt="Bee")", R"(class="skeuo-card-action-area")",
                               R"(type="button")", R"(class="skeuo-card-actions")"})
        CONTAINS(html, needle);
}

TEST(surfaces_card_action_area_click_reaches_handler) {
    int presses = 0;
    Runtime rt([&] {
        return Card({}, CardActionArea({.onclick = [&] { ++presses; }}, "Pressed " + std::to_string(presses)));
    });
    std::string html = rt.render();
    std::string id = handler_after(html, "skeuo-card-action-area", "click");
    CHECK(!id.empty()) << html;
    std::string reply = rt.handle(event(id, "click"));
    CHECK(presses == 1);
    CONTAINS(reply, "Pressed 1");
}

// ---------------------------------------------------------------- Accordion

TEST(surfaces_accordion_collapsed_by_default) {
    std::string html = render_static([] {
        return Accordion({}, {
            AccordionSummary({.expand_icon = Icon(IconName::ChevronDown)}, "Head"),
            AccordionDetails({}, "Body"),
        });
    });
    CONTAINS(html, "skeuo-accordion--rounded");
    CONTAINS(html, "skeuo-accordion--gutters");
    LACKS(html, "skeuo-accordion--expanded");
    CONTAINS(html, R"(aria-expanded="false")");
    CONTAINS(html, R"(role="region")");
    LACKS(html, "skeuo-accordion__collapse--open");
    CONTAINS(html, "skeuo-accordion-summary__expand-icon");
    CONTAINS(html, "<h3");
}

TEST(surfaces_accordion_ids_link_summary_and_region) {
    std::string html = render_static([] {
        return Accordion({.default_expanded = true}, {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")});
    });
    std::string controls = attr_value(html, "aria-controls");
    std::string labelled = attr_value(html, "aria-labelledby");
    CHECK(!controls.empty() && !labelled.empty()) << html;
    CONTAINS(html, "id=\"" + controls + "\"");
    CONTAINS(html, "id=\"" + labelled + "\"");
    CHECK(controls != labelled);
}

TEST(surfaces_accordion_uncontrolled_default_expanded) {
    std::string html = render_static([] {
        return Accordion({.default_expanded = true}, {
            AccordionSummary({}, "Head"),
            AccordionDetails({}, "Body"),
            AccordionActions({}, "Act"),
        });
    });
    CONTAINS(html, "skeuo-accordion--expanded");
    CONTAINS(html, R"(aria-expanded="true")");
    CHECK(count(html, "skeuo-accordion__collapse--open") == 2) << "details + actions open: " << html;
    CONTAINS(html, "skeuo-accordion-actions skeuo-accordion-actions--spacing");
}

TEST(surfaces_accordion_controlled_overrides_default) {
    std::string html = render_static([] {
        return Node{
            Accordion({.expanded = false, .default_expanded = true},
                      {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")}),
            Accordion({.expanded = true}, {AccordionSummary({}, "Head 2"), AccordionDetails({}, "Body 2")}),
        };
    });
    size_t first = html.find("Head<");
    size_t second = html.find("Head 2");
    CONTAINS(html.substr(0, first), R"(aria-expanded="false")");
    CONTAINS(html.substr(first, second - first), R"(aria-expanded="true")");
}

TEST(surfaces_accordion_disabled_square_no_gutters) {
    std::string html = render_static([] {
        return Accordion({.disabled = true, .disable_gutters = true, .square = true},
                         {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")});
    });
    CONTAINS(html, "skeuo-chassis--square");
    LACKS(html, "skeuo-accordion--rounded");
    LACKS(html, "skeuo-accordion--gutters");
    CONTAINS(html, "skeuo-accordion--disabled");
    CONTAINS(html, "<button");
    CONTAINS(html, " disabled");
}

TEST(surfaces_accordion_uncontrolled_toggles) {
    Runtime rt([] { return Accordion({}, {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")}); });
    std::string html = rt.render();
    std::string id = handler_after(html, "skeuo-accordion-summary\"", "click");
    CHECK(!id.empty()) << html;
    std::string reply = rt.handle(event(id, "click"));
    CONTAINS(reply, "skeuo-accordion--expanded");
    CONTAINS(reply, R"(aria-expanded=\"true\")");
    id = handler_after(rt.render(), "skeuo-accordion-summary\"", "click");
    reply = rt.handle(event(id, "click"));
    LACKS(reply, "skeuo-accordion--expanded");
}

TEST(surfaces_accordion_controlled_reports_and_waits_for_parent) {
    std::vector<bool> changes;
    Runtime rt([&] {
        return Accordion({.expanded = false, .onchange = [&](bool e) { changes.push_back(e); }},
                         {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")});
    });
    std::string id = handler_after(rt.render(), "skeuo-accordion-summary\"", "click");
    std::string reply = rt.handle(event(id, "click"));
    CHECK(changes.size() == 1 && changes[0]);
    LACKS(reply, "skeuo-accordion--expanded");  // the parent didn't change `expanded`
}

TEST(surfaces_accordion_disabled_ignores_toggle) {
    int calls = 0;
    Runtime rt([&] {
        return Accordion({.onchange = [&](bool) { ++calls; }, .disabled = true},
                         {AccordionSummary({}, "Head"), AccordionDetails({}, "Body")});
    });
    std::string id = handler_after(rt.render(), "skeuo-accordion-summary\"", "click");
    if (!id.empty()) rt.handle(event(id, "click"));
    CHECK(calls == 0);
}

TEST(surfaces_accordion_nested_keeps_own_ids) {
    std::string html = render_static([] {
        return Accordion({.default_expanded = true}, {
            AccordionSummary({}, "Outer"),
            AccordionDetails({}, Accordion({}, {AccordionSummary({}, "Inner"), AccordionDetails({}, "x")})),
        });
    });
    // Outer expanded, inner collapsed: exactly one aria-expanded="true".
    CHECK(count(html, R"(aria-expanded="true")") == 1) << html;
    CHECK(count(html, R"(aria-expanded="false")") == 1) << html;
    std::string outer = attr_value(html, "aria-controls");
    std::string inner = attr_value(html, "aria-controls", html.rfind("aria-controls", html.find("Inner")));
    CHECK(outer != inner) << html;
}

// ---------------------------------------------------------------- AppBar

TEST(surfaces_appbar_primary_is_colored_plate) {
    std::string html = render_static([] { return AppBar({}, Toolbar({}, "Title")); });
    CONTAINS(html, "<header");
    CONTAINS(html, "skeuo-key skeuo-key--filled skeuo-appbar--colored skeuo-color-primary");
    CONTAINS(html, "skeuo-appbar--fixed");
    CONTAINS(html, R"(class="skeuo-toolbar skeuo-toolbar--regular skeuo-toolbar--gutters")");
}

TEST(surfaces_appbar_default_is_square_chassis) {
    std::string html = render_static([] {
        return AppBar({.position = AppBarPosition::Sticky, .color = AppBarColor::Default, .elevation = 0},
                      Toolbar({.variant = ToolbarVariant::Dense, .disable_gutters = true}, "T"));
    });
    CONTAINS(html, "skeuo-chassis skeuo-elev-0 skeuo-chassis--square skeuo-paper");
    CONTAINS(html, "skeuo-appbar--sticky");
    CONTAINS(html, "skeuo-appbar--default");
    CONTAINS(html, R"(class="skeuo-toolbar skeuo-toolbar--dense")");
}

TEST(surfaces_appbar_color_from_palette_and_transparent) {
    std::string html = render_static([] {
        return Node{
            AppBar({.position = AppBarPosition::Static, .color = Color::Success}, "a"),
            AppBar({.position = AppBarPosition::Static, .color = Color::Neutral}, "b"),
            AppBar({.position = AppBarPosition::Static, .color = AppBarColor::Transparent}, "c"),
        };
    });
    CONTAINS(html, "skeuo-color-success");
    CONTAINS(html, "skeuo-appbar--default");
    CONTAINS(html, R"(class="skeuo-appbar skeuo-appbar--static skeuo-appbar--transparent")");
}

TEST(surfaces_appbar_rounded_and_attrs) {
    std::string html = render_static([] {
        return AppBar({.color = Color::Info, .square = false, .attrs = {{"id", "bar"}}}, "x");
    });
    CONTAINS(html, "skeuo-color-info skeuo-appbar--rounded skeuo-appbar");
    CONTAINS(html, R"(id="bar")");
}

// ---------------------------------------------------------------- Drawer

TEST(surfaces_temporary_drawer_closed_renders_nothing) {
    std::string html = render_static([] { return Drawer({.open = false}, "Nav"); });
    LACKS(html, "Nav");
}

TEST(surfaces_temporary_drawer_open) {
    std::string html = render_static([] {
        return Drawer({.anchor = DrawerAnchor::Right, .open = true, .size = "320px", .attrs = {{"aria-label", "Nav"}}},
                      "Items");
    });
    for (const char* needle :
         {"skeuo-drawer skeuo-drawer--temporary skeuo-drawer--right", "--drawer-size:320px;",
          "skeuo-scrim skeuo-drawer__scrim", R"(role="dialog")", R"(aria-modal="true")", R"(aria-label="Nav")",
          "skeuo-elev-4 skeuo-chassis--float", "skeuo-drawer__paper--right", "Items", "data-sk-autofocus",
          R"(data-sk-keys="Escape")"})
        CONTAINS(html, needle);
}

TEST(surfaces_drawer_inline_preview_and_hidden_backdrop) {
    std::string html = render_static([] { return Drawer({.open = true, .hide_backdrop = true, .inline_ = true}, "x"); });
    CONTAINS(html, "skeuo-drawer--inline");
    CONTAINS(html, R"(aria-modal="false")");
    LACKS(html, "skeuo-scrim");
    LACKS(html, "data-sk-autofocus");
}

TEST(surfaces_docked_drawers) {
    std::string html = render_static([] {
        return Node{
            Drawer({.variant = DrawerVariant::Permanent}, "perm"),
            Drawer({.anchor = DrawerAnchor::Top, .variant = DrawerVariant::Persistent, .open = false}, "pers"),
        };
    });
    CONTAINS(html, "skeuo-drawer--permanent skeuo-drawer--left skeuo-drawer--docked skeuo-drawer--open");
    CONTAINS(html, "skeuo-drawer--persistent skeuo-drawer--top skeuo-drawer--docked skeuo-drawer--closed");
    CONTAINS(html, "skeuo-elev-0 skeuo-chassis--square");
    LACKS(html, R"(role="dialog")");
}

TEST(surfaces_drawer_escape_and_scrim_close) {
    bool open = true;
    int closes = 0;
    Runtime rt([&] {
        return Drawer({.open = open, .onclose = [&] { ++closes; open = false; }}, "Nav");
    });
    std::string html = rt.render();
    std::string key_id = handler_after(html, "skeuo-drawer--temporary", "keydown");
    CHECK(!key_id.empty()) << html;
    CONTAINS(html, "keydown:" + key_id + "!");  // stops propagation
    std::string reply = rt.handle(event(key_id, "keydown", R"(,"key":"Escape")"));
    CHECK(closes == 1);
    LACKS(reply, "Nav");

    open = true;
    html = rt.render();
    std::string scrim_id = handler_after(html, "skeuo-drawer__scrim", "click");
    CHECK(!scrim_id.empty()) << html;
    rt.handle(event(scrim_id, "click"));
    CHECK(closes == 2 && !open);
}

// ---------------------------------------------------------------- Layout

TEST(surfaces_skeuo_box_component_tag) {
    std::string html = render_static([] {
        return Node{
            SkeuoBox({.attrs = {{"style", "color:red"}}}, "a"),
            SkeuoBox({.component = "section", .cls = "s"}, "b"),
            SkeuoBox({.component = "script"}, "c"),
        };
    });
    CONTAINS(html, R"(<div class="skeuo-box" style="color:red">a</div>)");
    CONTAINS(html, R"(<section class="skeuo-box s">b</section>)");
    CONTAINS(html, R"(<div class="skeuo-box">c</div>)");
}

TEST(surfaces_container_classes) {
    std::string html = render_static([] {
        return Node{
            Container({}, "a"),
            Container({.max_width = ContainerMaxWidth::Sm, .fixed = true, .disable_gutters = true}, "b"),
        };
    });
    CONTAINS(html, R"(class="skeuo-container skeuo-container--max-lg skeuo-container--gutters")");
    CONTAINS(html, R"(class="skeuo-container skeuo-container--max-sm skeuo-container--fixed")");
}

TEST(surfaces_stack_spacing_and_alignment) {
    std::string html = render_static([] {
        return Stack({.direction = StackDirection::Row, .spacing = 2.0, .align_items = "center", .use_flex_gap = true},
                     el("span").child("a"));
    });
    CONTAINS(html, R"(class="skeuo-stack skeuo-stack--row skeuo-stack--gap")");
    CONTAINS(html, "--stack-spacing:16px;");
    CONTAINS(html, "align-items:center;");
}

TEST(surfaces_stack_fractional_spacing_and_component) {
    std::string html = render_static([] { return Stack({.spacing = 1.5, .component = "ul"}, {}); });
    CONTAINS(html, R"(<ul class="skeuo-stack skeuo-stack--column skeuo-stack--margin" style="--stack-spacing:12px;")");
    html = render_static([] { return Stack({.spacing = 0.3}, {}); });
    CONTAINS(html, "--stack-spacing:2.4px;");
}

TEST(surfaces_stack_divider_between_items) {
    std::string html = render_static([] {
        Node items;
        for (int n = 0; n < 3; ++n) items.add_child(el("span").child("item" + std::to_string(n)));
        return Stack({.divider = el("hr").cls("div")}, std::move(items));
    });
    CHECK(count(html, R"(<hr class="div")") == 2) << html;
    size_t from = 0;
    bool ordered = true;
    for (const char* needle : {"item0", "div\"", "item1", "div\"", "item2"}) {
        size_t at = html.find(needle, from);
        if (at == std::string::npos) ordered = false;
        else from = at + 1;
    }
    CHECK(ordered) << html;
}

TEST(surfaces_stack_divider_single_child) {
    std::string html = render_static([] { return Stack({.divider = el("hr")}, el("p").child("only")); });
    LACKS(html, "<hr");
}

TEST(surfaces_grid_container_and_items) {
    std::string html = render_static([] {
        return Grid({.container = true, .spacing = 2.0, .columns = 16}, {
            Grid({.size = 8}, "a"),
            Grid({.size = Responsive::xs(12).md(6), .offset = 2}, "b"),
            Grid({.size = GridSize::Grow}, "c"),
        });
    });
    for (const char* needle :
         {"skeuo-grid skeuo-grid--container", "--grid-row-gap:16px;--grid-column-gap:16px;",
          "--grid-parent-columns:16;--grid-parent-gap:16px;", "skeuo-grid--xs-n", "--grid-size-xs:8;",
          "skeuo-grid--md-n", "--grid-size-md:6;", "skeuo-grid--xs-offset", "--grid-offset-xs:2;",
          "skeuo-grid--xs-grow"})
        CONTAINS(html, needle);
    // Same order as the Rust markup.
    CONTAINS(html, R"(style="--grid-parent-columns:16;--grid-parent-gap:16px;--grid-size-xs:12;--grid-offset-xs:2;--grid-size-md:6;")");
}

TEST(surfaces_nested_grid_container_uses_parent_columns_for_itself) {
    std::string html = render_static([] {
        return Grid({.container = true, .spacing = 1.0}, Grid({.container = true, .spacing = 3.0, .columns = 4, .size = 6},
                                                              Grid({.size = 2}, "inner")));
    });
    // The nested container is sized against the outer grid (12 cols, 8px)…
    CONTAINS(html, "--grid-row-gap:24px;--grid-column-gap:24px;--grid-parent-columns:12;--grid-parent-gap:8px;");
    // …and its own items against it (4 cols, 24px).
    CONTAINS(html, "--grid-parent-columns:4;--grid-parent-gap:24px;");
    CHECK(count(html, "--grid-parent-columns") == 2) << html;
}

TEST(surfaces_grid_items_deeper_in_subtree_and_flex_props) {
    std::string html = render_static([] {
        return Grid({.container = true, .spacing = 1.0, .direction = "column", .wrap = "nowrap", .style = "color:red;"},
                    el("section").child(Grid({.size = GridSize::Auto, .offset = GridSize::Auto}, "x")));
    });
    CONTAINS(html, R"(style="--grid-row-gap:8px;--grid-column-gap:8px;flex-direction:column;flex-wrap:nowrap;color:red;")");
    CONTAINS(html, R"(class="skeuo-grid skeuo-grid--xs-auto skeuo-grid--xs-offset-auto")");
    CONTAINS(html, R"(style="--grid-parent-columns:12;--grid-parent-gap:8px;")");
}

TEST(surfaces_nested_container_with_flex_props_gets_parent_vars_in_order) {
    std::string html = render_static([] {
        return Grid({.container = true},
                    Grid({.container = true, .size = 6, .justify_content = "center"}, {}));
    });
    CONTAINS(html, "--grid-row-gap:0px;--grid-column-gap:0px;justify-content:center;--grid-parent-columns:12;"
                   "--grid-parent-gap:0px;--grid-size-xs:6;");
}
