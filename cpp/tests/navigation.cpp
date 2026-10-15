// Port of crates/skeuo-ui/tests/navigation.rs, plus handler wiring through Runtime.
#include <optional>

#include "skeuo/json.hpp"
#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

namespace {

size_t count(const std::string& hay, std::string_view needle) {
    size_t n = 0;
    for (size_t pos = hay.find(needle); pos != std::string::npos; pos = hay.find(needle, pos + needle.size())) ++n;
    return n;
}

void all(const std::string& html, std::initializer_list<const char*> needles) {
    for (const char* n : needles) CONTAINS(html, n);
}

/// The handler id of `type` on the tag containing `marker`. -1 when missing.
int nav_handler(const std::string& html, std::string_view marker, std::string_view type) {
    size_t at = html.find(marker);
    if (at == std::string::npos) return -1;
    size_t end = html.find('>', at);
    size_t on = html.find("data-sk-on=\"", at);
    if (on == std::string::npos || on > end) return -1;
    size_t start = on + 12;
    std::string spec = "," + html.substr(start, html.find('"', start) - start);
    std::string want = "," + std::string(type) + ":";
    size_t pos = spec.find(want);
    if (pos == std::string::npos) return -1;
    return std::stoi(spec.substr(pos + want.size()));
}

/// Sends one event and returns the re-rendered HTML.
std::string nav_fire(Runtime& rt, int id, std::string_view type, std::string_view key = "") {
    std::string reply = rt.handle(R"({"t":"event","h":[)" + std::to_string(id) + R"(],"e":{"type":")" +
                                  std::string(type) + R"(","key":")" + std::string(key) + R"("}})");
    return json::parse(reply).str("html");
}

/// The opening tag that contains `marker`.
std::string tag_of(const std::string& html, std::string_view marker) {
    size_t at = html.find(marker);
    if (at == std::string::npos) return {};
    size_t start = html.rfind('<', at);
    return html.substr(start, html.find('>', at) - start + 1);
}

}  // namespace

// ------------------------------------------------------------------ Tabs

TEST(navigation_tabs_roles_and_selected_keycap) {
    std::string html = render_static([] {
        return Tabs({.value = "b", .aria_label = "Views", .cls = "mine"},
                    {Tab({.value = "a", .label = "Alpha"}), Tab({.value = "b", .label = "Beta"}),
                     Tab({.value = "c", .label = "Gamma", .disabled = true})});
    });
    all(html, {"skeuo-tabs skeuo-tabs--standard skeuo-tabs--horizontal skeuo-tabs--medium skeuo-color-neutral mine",
               R"(role="tablist")", R"(aria-orientation="horizontal")", R"(aria-label="Views")",
               "skeuo-tabs__tray skeuo-well skeuo-well--shallow", R"(class="skeuo-tab skeuo-key skeuo-tab--selected")",
               "Alpha", "Gamma"});
    CHECK(count(html, R"(role="tab")") == 3) << html;
    CHECK(count(html, R"(aria-selected="true")") == 1) << html;
    CHECK(count(html, R"(aria-selected="false")") == 2) << html;
    CHECK(count(html, R"(tabindex="0")") == 1) << html;
    CHECK(count(html, R"(tabindex="-1")") == 2) << html;
    CHECK(count(html, " disabled") == 1) << html;
    LACKS(html, "data-sk-tab=");
}

TEST(navigation_tabs_uncontrolled_default_value_and_variants) {
    std::string html = render_static([] {
        return Tabs({.default_value = "two",
                     .variant = TabsVariant::FullWidth,
                     .orientation = Orientation::Vertical,
                     .centered = true,
                     .color = Color::Primary,
                     .size = Size::Small},
                    {Tab({.value = "one", .label = "One"}),
                     Tab({.value = "two", .label = "Two", .icon = Icon(IconName::Home),
                          .icon_position = IconPosition::Start})});
    });
    all(html, {"skeuo-tabs--full-width", "skeuo-tabs--vertical", "skeuo-tabs--centered", "skeuo-tabs--small",
               "skeuo-color-primary", R"(aria-orientation="vertical")",
               "skeuo-tab skeuo-key skeuo-tab--selected skeuo-tab--icon-start", "skeuo-tab__icon"});
    std::string scrollable = render_static(
        [] { return Tabs({.variant = TabsVariant::Scrollable}, Tab({.value = "x", .label = "X"})); });
    CONTAINS(scrollable, "skeuo-tabs--scrollable");
    CONTAINS(scrollable, R"(tabindex="0")");
    LACKS(scrollable, "skeuo-tab--selected");
}

TEST(navigation_tab_context_links_tabs_and_panels) {
    std::string html = render_static([] {
        return TabContext({.value = "script", .id = "studio"},
                          {
                              Tabs({}, {Tab({.value = "rundown", .label = "Rundown"}),
                                        Tab({.value = "script", .label = "Script"})}),
                              TabPanel({.value = "rundown"}, "Rundown body"),
                              TabPanel({.value = "script"}, "Script body"),
                          });
    });
    all(html, {R"(id="studio-tab-script")", R"(aria-controls="studio-panel-script")", R"(role="tabpanel")",
               R"(id="studio-panel-script")", R"(aria-labelledby="studio-tab-script")", "Script body"});
    LACKS(html, "Rundown body");
    CHECK(count(html, "hidden") == 1) << html;
    CONTAINS(html, R"(class="skeuo-tab skeuo-key skeuo-tab--selected" type="button" role="tab" id="studio-tab-script")");
    LACKS(html, "data-sk-panel");
}

TEST(navigation_tabs_click_selects_uncontrolled_and_calls_onchange) {
    std::string changed;
    Runtime rt([&] {
        return Tabs({.default_value = "a", .onchange = [&](const std::string& v) { changed = v; }},
                    {Tab({.value = "a", .label = "A"}), Tab({.value = "b", .label = "B"})});
    });
    std::string html = rt.render();
    int click = nav_handler(html, "-tab-b\"", "click");
    CHECK(click >= 0) << html;
    html = nav_fire(rt, click, "click");
    CHECK(changed == "b");
    CONTAINS(tag_of(html, "-tab-b\""), "skeuo-tab--selected");
    LACKS(tag_of(html, "-tab-a\""), "skeuo-tab--selected");
}

TEST(navigation_tabs_tab_onclick_runs_after_select) {
    std::string order;
    Runtime rt([&] {
        return Tabs({.onchange = [&](const std::string&) { order += "select;"; }},
                    Tab({.value = "a", .label = "A", .onclick = [&] { order += "tab;"; }}));
    });
    std::string html = rt.render();
    CHECK(count(html, "click:") == 1) << html;  // one merged listener
    nav_fire(rt, nav_handler(html, "-tab-a\"", "click"), "click");
    CHECK(order == "select;tab;") << order;
}

TEST(navigation_tabs_arrow_keys_move_focus_with_fresh_marks) {
    Runtime rt([&] {
        return Tabs({.default_value = "a"}, {Tab({.value = "a", .label = "A"}), Tab({.value = "b", .label = "B"}),
                                             Tab({.value = "c", .label = "C", .disabled = true}),
                                             Tab({.value = "d", .label = "D"})});
    });
    std::string html = rt.render();
    CONTAINS(tag_of(html, "-tab-a\""), R"(data-sk-keys="ArrowLeft ArrowRight Home End")");
    LACKS(html, "data-sk-autofocus");
    html = nav_fire(rt, nav_handler(html, "-tab-a\"", "keydown"), "keydown", "ArrowRight");
    CONTAINS(tag_of(html, "-tab-b\""), "data-sk-autofocus");
    CHECK(count(html, "data-sk-autofocus") == 1) << html;
    // From the first move on, each tab sits in a slot with an empty comment;
    // a first visit marks the existing element.
    CONTAINS(html, "</button><!---->");
    LACKS(html, "<!----><!----><button");
    // Focus moves; the selection does not.
    CONTAINS(tag_of(html, "-tab-a\""), "skeuo-tab--selected");
    // Disabled tabs are skipped.
    html = nav_fire(rt, nav_handler(html, "-tab-b\"", "keydown"), "keydown", "ArrowRight");
    CONTAINS(tag_of(html, "-tab-d\""), "data-sk-autofocus");
    // Wraps around.
    html = nav_fire(rt, nav_handler(html, "-tab-d\"", "keydown"), "keydown", "ArrowRight");
    CONTAINS(tag_of(html, "-tab-a\""), "data-sk-autofocus");
    html = nav_fire(rt, nav_handler(html, "-tab-a\"", "keydown"), "keydown", "End");
    CONTAINS(tag_of(html, "-tab-d\""), "data-sk-autofocus");
    // A revisit swaps the tab with its comment, so the page creates a fresh
    // element for it (the runtime focuses an element only once).
    size_t swapped = html.find("<!----><!----><button");
    CHECK(swapped != std::string::npos && html.substr(swapped, 300).find("-tab-d\"") != std::string::npos) << html;
    html = nav_fire(rt, nav_handler(html, "-tab-d\"", "keydown"), "keydown", "Home");
    CONTAINS(tag_of(html, "-tab-a\""), "data-sk-autofocus");
    html = nav_fire(rt, nav_handler(html, "-tab-a\"", "keydown"), "keydown", "ArrowLeft");
    CONTAINS(tag_of(html, "-tab-d\""), "data-sk-autofocus");
    // The mark is consumed: the next render (any event) does not carry it.
    html = nav_fire(rt, nav_handler(html, "-tab-b\"", "click"), "click");
    LACKS(html, "data-sk-autofocus");
}

TEST(navigation_tabs_selection_follows_focus_vertical) {
    std::string value = "a";
    Runtime rt([&] {
        return Tabs({.value = value,
                     .onchange = [&](const std::string& v) { value = v; },
                     .orientation = Orientation::Vertical,
                     .selection_follows_focus = true},
                    {Tab({.value = "a", .label = "A"}), Tab({.value = "b", .label = "B"})});
    });
    std::string html = rt.render();
    CONTAINS(tag_of(html, "-tab-a\""), R"(data-sk-keys="ArrowUp ArrowDown Home End")");
    html = nav_fire(rt, nav_handler(html, "-tab-a\"", "keydown"), "keydown", "ArrowDown");
    CHECK(value == "b");
    CONTAINS(tag_of(html, "-tab-b\""), "skeuo-tab--selected");
}

TEST(navigation_tab_context_follows_controlled_value) {
    std::string tab = "one";
    Runtime rt([&] {
        return TabContext({.value = tab, .id = "ctx"},
                          {Tabs({.onchange = [&](const std::string& v) { tab = v; }},
                                {Tab({.value = "one", .label = "1"}), Tab({.value = "two", .label = "2"})}),
                           TabPanel({.value = "one"}, "first panel"), TabPanel({.value = "two"}, "second panel")});
    });
    std::string html = rt.render();
    CONTAINS(html, "first panel");
    html = nav_fire(rt, nav_handler(html, R"(id="ctx-tab-two")", "click"), "click");
    CHECK(tab == "two");
    CONTAINS(html, "second panel");
    LACKS(html, "first panel");
    CONTAINS(tag_of(html, R"(id="ctx-tab-two")"), "skeuo-tab--selected");
}

TEST(navigation_tab_panel_keep_mounted) {
    std::string html = render_static([] {
        return TabContext({.value = "a"}, {TabPanel({.value = "a"}, "A body"),
                                           TabPanel({.value = "b", .keep_mounted = true}, "B body")});
    });
    CONTAINS(html, "A body");
    CONTAINS(html, "B body");
    CHECK(count(html, "hidden") == 1) << html;
}

// ------------------------------------------------------------ Breadcrumbs

TEST(navigation_breadcrumbs_marks_current_page) {
    std::string html = render_static([] {
        return Breadcrumbs({.items = {Link({.href = "/"}, "Home"), Link({.href = "/shows"}, "Shows"),
                                      el("span").child("Today")}});
    });
    all(html, {R"(<nav class="skeuo-breadcrumbs" aria-label="breadcrumb")", "<ol", R"(aria-current="page")", "Today"});
    CHECK(count(html, R"(aria-current="page")") == 1) << html;
    CHECK(count(html, "skeuo-breadcrumbs__separator") == 2) << html;
    LACKS(html, "skeuo-breadcrumbs__collapsed");
}

TEST(navigation_breadcrumbs_collapse_into_keycap_and_expand) {
    Runtime rt([] {
        std::vector<Node> items;
        for (int i = 1; i <= 6; ++i) items.push_back(el("span").child("crumb" + std::to_string(i)));
        return Breadcrumbs({.items = std::move(items),
                            .separator = Node("/"),
                            .max_items = 4,
                            .items_before_collapse = 1,
                            .items_after_collapse = 2});
    });
    std::string html = rt.render();
    all(html, {"crumb1", "crumb5", "crumb6", "skeuo-key skeuo-key--pressable skeuo-breadcrumbs__collapsed",
               R"(aria-label="Show path")"});
    for (const char* hidden : {"crumb2", "crumb3", "crumb4"}) LACKS(html, hidden);
    CHECK(count(html, "skeuo-breadcrumbs__separator") == 3) << html;
    html = nav_fire(rt, nav_handler(html, "skeuo-breadcrumbs__collapsed", "click"), "click");
    for (const char* shown : {"crumb2", "crumb3", "crumb4"}) CONTAINS(html, shown);
    LACKS(html, "skeuo-breadcrumbs__collapsed");
    CHECK(count(html, "skeuo-breadcrumbs__separator") == 5) << html;
}

// ------------------------------------------------------------------- Link

TEST(navigation_link_anchor_and_button_forms) {
    std::string html = render_static([] {
        return Node{Link({.href = "/docs", .underline = LinkUnderline::Hover, .color = Color::Secondary,
                          .target = "_blank"},
                         "Docs"),
                    Link({.onclick = [] {}}, "Act")};
    });
    all(html, {R"(<a class="skeuo-link skeuo-link--underline-hover skeuo-color-secondary" href="/docs" target="_blank")",
               R"(<button class="skeuo-link skeuo-link--underline-always skeuo-color-primary skeuo-link--button" type="button")",
               "Docs", "Act"});
}

TEST(navigation_link_button_click) {
    int clicks = 0;
    Runtime rt([&] { return Link({.onclick = [&] { ++clicks; }}, "Act"); });
    std::string html = rt.render();
    nav_fire(rt, nav_handler(html, "skeuo-link--button", "click"), "click");
    CHECK(clicks == 1);
}

// -------------------------------------------------------- Menu & Popover

TEST(navigation_menu_closed_renders_only_anchor) {
    std::string html = render_static([] {
        return Menu({.open = false, .anchor = el("button").child("Trigger")}, MenuItem({}, "Hidden"));
    });
    CONTAINS(html, "Trigger");
    CONTAINS(html, "skeuo-popover-anchor");
    LACKS(html, "Hidden");
    LACKS(html, R"(role="menu")");
}

TEST(navigation_menu_open_has_backdrop_floating_chassis_and_items) {
    std::string html = render_static([] {
        return Menu({.open = true,
                     .anchor = el("button").attr("id", "t").child("Edit"),
                     .placement = Placement::Top,
                     .dense = true,
                     .id = "m",
                     .aria_labelledby = "t"},
                    {
                        MenuItem({.icon = Icon(IconName::Copy), .shortcut = "⌘C"}, "Copy"),
                        MenuItem({.selected = true, .divider = true}, "Paste"),
                        MenuItem({.disabled = true}, "Delete"),
                    });
    });
    all(html, {"skeuo-popover-backdrop",
               "skeuo-popover skeuo-chassis skeuo-elev-3 skeuo-chassis--float skeuo-popover--top skeuo-menu",
               R"(role="menu")", R"(id="m")", R"(aria-labelledby="t")", "skeuo-menu-list skeuo-menu-list--dense",
               R"(<kbd class="skeuo-menu-item__shortcut">⌘C</kbd>)",
               "skeuo-menu-item skeuo-menu-item--selected skeuo-menu-item--dense", R"(aria-disabled="true")",
               R"(<hr class="skeuo-rule skeuo-menu-item__divider" role="separator")", "skeuo-menu-item__icon"});
    CHECK(count(html, R"(role="menuitem")") == 3) << html;
    CHECK(count(html, R"(role="none")") == 3) << html;
    CHECK(count(html, "<path d=\"M5 12.5l4.5 4.5L19 7.5\"") == 1) << html;
    // Focus goes to the first item when the menu opens.
    CHECK(count(html, "data-sk-autofocus") == 1) << html;
    LACKS(html, "data-sk-dense-auto");
}

TEST(navigation_menu_inline_preview_has_no_backdrop) {
    std::string html = render_static([] { return Menu({.open = true, .inline_ = true}, MenuItem({}, "One")); });
    CONTAINS(html, "skeuo-popover--inline");
    CONTAINS(html, "skeuo-popover-anchor--inline");
    LACKS(html, "skeuo-popover-backdrop");
    LACKS(html, "data-sk-autofocus");
}

TEST(navigation_menu_list_standalone) {
    std::string html = render_static([] { return MenuList({.aria_label = "Speed"}, MenuItem({}, "1×")); });
    CONTAINS(html, R"(<ul class="skeuo-menu-list" role="menu" tabindex="-1" aria-label="Speed")");
    CONTAINS(html, R"(role="menuitem")");
}

TEST(navigation_menu_keyboard_navigation_and_close) {
    bool open = true;
    std::string chosen;
    Runtime rt([&] {
        return Menu({.open = open, .onclose = [&] { open = false; }, .anchor = el("button").child("T")},
                    {
                        MenuItem({.onclick = [&] { chosen = "one"; }}, "one"),
                        MenuItem({.disabled = true}, "two"),
                        MenuItem({.onclick = [&] { chosen = "three"; }}, "three"),
                    });
    });
    std::string html = rt.render();
    // The first item is marked for focus once.
    size_t first = html.find("data-sk-autofocus");
    CHECK(first != std::string::npos && first < html.find(">one<")) << html;
    int focus_one = nav_handler(html, R"(role="menuitem" tabindex="-1" data-sk-autofocus)", "focus");
    CHECK(focus_one >= 0) << html;
    html = nav_fire(rt, focus_one, "focus");
    LACKS(html, "data-sk-autofocus");
    // ArrowDown skips the disabled item.
    html = nav_fire(rt, nav_handler(html, "skeuo-menu-list", "keydown"), "keydown", "ArrowDown");
    CHECK(count(html, "data-sk-autofocus") == 1) << html;
    CHECK(html.find("data-sk-autofocus") > html.find(">two<")) << html;
    CONTAINS(html, "</li><!---->");
    LACKS(html, "<!----><!----><li");
    // The page reports the focus change; the next arrow wraps to the first
    // item, whose element took focus on open: it is swapped for a fresh one.
    html = nav_fire(rt, nav_handler(html, "data-sk-autofocus", "focus"), "focus");
    html = nav_fire(rt, nav_handler(html, "skeuo-menu-list", "keydown"), "keydown", "ArrowDown");
    CHECK(html.find("data-sk-autofocus") < html.find(">one<")) << html;
    CONTAINS(html, R"("><!----><li)");
    // Enter on an item is a native click.
    nav_fire(rt, nav_handler(html, "data-sk-autofocus", "click"), "click");
    CHECK(chosen == "one");
    // Escape on the anchor wrapper closes.
    html = rt.render();
    int esc = nav_handler(html, R"(class="skeuo-popover-anchor")", "keydown");
    CONTAINS(tag_of(html, R"(class="skeuo-popover-anchor")"), R"(data-sk-keys="Escape")");
    CONTAINS(html, "keydown:" + std::to_string(esc) + "!");
    html = nav_fire(rt, esc, "keydown", "Escape");
    CHECK(!open);
    LACKS(html, R"(role="menu")");
}

TEST(navigation_menu_mouseenter_moves_focus_once) {
    Runtime rt([&] {
        return Menu({.open = true, .onclose = [] {}}, {MenuItem({}, "a"), MenuItem({}, "b"), MenuItem({.disabled = true}, "c")});
    });
    std::string html = rt.render();
    // The disabled item does not take focus on hover.
    LACKS(tag_of(html, R"(disabled aria-disabled="true")"), "mouseenter");
    int enter_b = nav_handler(html, R"(tabindex="-1" data-sk-on)", "mouseenter");  // first item without the mark
    CHECK(enter_b >= 0) << html;
    html = nav_fire(rt, enter_b, "mouseenter");
    CHECK(html.find("data-sk-autofocus") > html.find(">a<") && html.find("data-sk-autofocus") < html.find(">b<"))
        << html;
    // Hovering the same item again (the page made a fresh element) does nothing.
    int again = nav_handler(html, "data-sk-autofocus", "mouseenter");
    html = nav_fire(rt, again, "mouseenter");
    LACKS(html, "data-sk-autofocus");
}

TEST(navigation_menu_backdrop_click_closes) {
    bool open = true;
    Runtime rt([&] {
        return Menu({.open = open, .onclose = [&] { open = false; }, .anchor = el("button").child("T")},
                    MenuItem({}, "x"));
    });
    std::string html = rt.render();
    html = nav_fire(rt, nav_handler(html, "skeuo-popover-backdrop", "click"), "click");
    CHECK(!open);
    LACKS(html, "skeuo-popover-backdrop");
}

TEST(navigation_menu_list_dense_follows_list_unless_set) {
    std::string html = render_static([] {
        return MenuList({.dense = true}, {MenuItem({}, "a"), MenuItem({.dense = false}, "b")});
    });
    CHECK(count(html, "skeuo-menu-item--dense") == 1) << html;
}

TEST(navigation_popover_open_and_closed) {
    std::string open = render_static([] {
        return Popover({.open = true, .anchor = el("button").child("i"), .placement = Placement::Right,
                        .aria_label = "Info"},
                       "Details");
    });
    all(open, {"skeuo-popover-backdrop", "skeuo-popover--right", "skeuo-chassis--float", R"(aria-label="Info")",
               R"(tabindex="-1")", "Details", "data-sk-autofocus"});
    LACKS(open, R"(role="menu")");
    std::string closed = render_static([] { return Popover({.anchor = el("button").child("i")}, "Details"); });
    LACKS(closed, "Details");
}

// ------------------------------------------------------------- Pagination

TEST(navigation_pagination_items_match_mui) {
    using T = PaginationItemType;
    CHECK((pagination_items(10, 1, 1, 1, false, false, false, false) ==
           std::vector<T>{T::previous(), T::page(1), T::page(2), T::page(3), T::page(4), T::page(5),
                          T::end_ellipsis(), T::page(10), T::next()}));
    CHECK((pagination_items(10, 5, 1, 1, true, true, false, false) ==
           std::vector<T>{T::first(), T::previous(), T::page(1), T::start_ellipsis(), T::page(4), T::page(5),
                          T::page(6), T::end_ellipsis(), T::page(10), T::next(), T::last()}));
    CHECK((pagination_items(3, 2, 1, 1, false, false, true, true) == std::vector<T>{T::page(1), T::page(2), T::page(3)}));
}

TEST(navigation_pagination_controlled_page_is_latched_colored_key) {
    std::string html = render_static([] {
        return Pagination({.count = 10, .page = 5, .variant = PaginationVariant::Outlined,
                           .shape = PaginationShape::Rounded, .color = Color::Success, .size = Size::Small});
    });
    all(html, {R"(<nav class="skeuo-pagination" aria-label="pagination navigation")",
               "skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--down skeuo-pagination-item "
               "skeuo-pagination-item--outlined skeuo-pagination-item--rounded skeuo-pagination-item--small "
               "skeuo-color-success skeuo-pagination-item--selected",
               R"(aria-label="page 5")", R"(aria-current="page")", R"(aria-label="Go to page 4")",
               R"(aria-label="Go to previous page")", "skeuo-pagination-item--ellipsis"});
    CHECK(count(html, R"(aria-current="page")") == 1) << html;
    CHECK(count(html, "…") == 2) << html;
}

TEST(navigation_pagination_uncontrolled_default_and_disabled_ends) {
    std::string html = render_static(
        [] { return Pagination({.count = 4, .default_page = 1, .show_first_button = true, .show_last_button = true}); });
    CONTAINS(html, R"(aria-label="page 1")");
    CHECK(count(html, "disabled") == 2) << html;
    CONTAINS(html, R"(class="skeuo-pagination-item skeuo-pagination-item--text skeuo-pagination-item--circular skeuo-pagination-item--medium skeuo-color-primary" type="button" aria-label="Go to page 2")");
    std::string disabled = render_static([] { return Pagination({.count = 3, .disabled = true}); });
    CHECK(count(disabled, "disabled") == 5) << disabled;
}

TEST(navigation_pagination_clicks_change_page) {
    int changed = 0;
    Runtime rt([&] { return Pagination({.count = 10, .default_page = 5, .onchange = [&](int p) { changed = p; }}); });
    std::string html = rt.render();
    html = nav_fire(rt, nav_handler(html, R"(aria-label="Go to next page")", "click"), "click");
    CHECK(changed == 6);
    CONTAINS(html, R"(aria-label="page 6")");
    html = nav_fire(rt, nav_handler(html, R"(aria-label="Go to previous page")", "click"), "click");
    CONTAINS(html, R"(aria-label="page 5")");
    html = nav_fire(rt, nav_handler(html, R"(aria-label="Go to page 10")", "click"), "click");
    CHECK(changed == 10);
    CONTAINS(html, R"(aria-label="page 10")");
    CONTAINS(tag_of(html, R"(aria-label="Go to next page")"), "disabled");
}

// ---------------------------------------------------------------- Stepper

TEST(navigation_stepper_linear_states) {
    std::string html = render_static([] {
        return Stepper({.active_step = 1}, {
                                               Step({}, StepLabel({}, "One")),
                                               Step({}, StepLabel({.optional = Node("Optional")}, "Two")),
                                               Step({}, StepLabel({}, "Three")),
                                           });
    });
    all(html, {"skeuo-stepper skeuo-stepper--horizontal skeuo-color-primary", R"(class="skeuo-step skeuo-step--completed")",
               R"(class="skeuo-step skeuo-step--joined skeuo-step--active")",
               R"(class="skeuo-step skeuo-step--joined skeuo-step--disabled")",
               "skeuo-step-icon skeuo-key skeuo-key--round skeuo-key--filled skeuo-step-icon--active",
               "skeuo-step-label__optional skeuo-caption", ">3<"});
    CHECK(count(html, "skeuo-step-connector skeuo-groove") == 2) << html;
    CHECK(count(html, "skeuo-step-connector__fill--on") == 1) << html;
    LACKS(html, ">1<");
    CONTAINS(html, ">2<");
    LACKS(html, "data-sk-step");
}

TEST(navigation_stepper_vertical_content_alternative_and_error) {
    std::string html = render_static([] {
        return Stepper({.active_step = 0, .orientation = Orientation::Vertical},
                       {
                           Step({}, {StepLabel({}, "A"), StepContent({}, "Body A")}),
                           Step({}, {StepLabel({.error = true}, "B"), StepContent({}, "Body B")}),
                           Step({.expanded = true}, {StepLabel({}, "C"), StepContent({}, "Body C")}),
                       });
    });
    all(html, {"skeuo-stepper--vertical", "Body A", "Body C", "skeuo-step-content__line skeuo-groove",
               "skeuo-color-error skeuo-step-icon--error", "skeuo-step-label--error"});
    LACKS(html, "Body B");
    std::string alt = render_static([] {
        return Stepper({.alternative_label = true}, Step({}, {StepLabel({}, "A"), StepContent({}, "never")}));
    });
    CONTAINS(alt, "skeuo-stepper--alternative-label");
    LACKS(alt, "never");
}

TEST(navigation_stepper_non_linear_step_buttons) {
    int picked = -1;
    Runtime rt([&] {
        return Stepper({.active_step = 2, .non_linear = true},
                       {
                           Step({.completed = true}, StepButton({}, "A")),
                           Step({}, StepButton({.onclick = [&] { picked = 1; }}, "B")),
                           Step({}, StepButton({}, "C")),
                       });
    });
    std::string html = rt.render();
    CHECK(count(html, "skeuo-step-button") == 3) << html;
    LACKS(html, "disabled");
    CONTAINS(html, R"(aria-current="step")");
    CHECK(count(html, "skeuo-step--completed") == 1) << html;
    // The second button is the one that leads to "B".
    size_t b = html.find(">B<");
    size_t button = html.rfind("<button", b);
    nav_fire(rt, nav_handler(html.substr(button), "skeuo-step-button", "click"), "click");
    CHECK(picked == 1);
}

TEST(navigation_stepper_numbers_steps_in_order_and_by_index) {
    std::string html = render_static([] {
        return Stepper({.active_step = 0, .non_linear = true},
                       {Step({}, StepLabel({}, "a")), Step({.index = 5}, StepLabel({}, "b")),
                        Step({}, StepLabel({}, "c")), Step({.active = true}, StepLabel({.icon = Node("★")}, "d"))});
    });
    // a = 1, b = explicit index 5 -> 6, c = third in order -> 3, d has a custom icon.
    all(html, {">1<", ">6<", ">3<", ">★<"});
    LACKS(html, ">2<");
    LACKS(html, ">4<");
}

TEST(navigation_stepper_next_back_reset_through_buttons) {
    int active = 0;
    Runtime rt([&] {
        return Node{Stepper({.active_step = active}, {Step({}, StepLabel({}, "A")), Step({}, StepLabel({}, "B"))}),
                    Button({.onclick = [&] { ++active; }, .attrs = {{"id", "next"}}}, "Next"),
                    Button({.onclick = [&] { active = 0; }, .attrs = {{"id", "reset"}}}, "Reset")};
    });
    std::string html = rt.render();
    html = nav_fire(rt, nav_handler(html, R"(id="next")", "click"), "click");
    CONTAINS(html, R"(class="skeuo-step skeuo-step--completed")");
    CONTAINS(html, R"(class="skeuo-step skeuo-step--joined skeuo-step--active")");
    html = nav_fire(rt, nav_handler(html, R"(id="reset")", "click"), "click");
    CONTAINS(html, R"(class="skeuo-step skeuo-step--active")");
}

TEST(navigation_mobile_stepper_variants) {
    std::string dots = render_static([] {
        return MobileStepper({.steps = 4, .active_step = 1, .position = MobileStepperPosition::Static,
                              .back_button = el("button").child("Back"), .next_button = el("button").child("Next")});
    });
    all(dots, {"skeuo-mobile-stepper skeuo-chassis skeuo-elev-1 skeuo-mobile-stepper--dots skeuo-mobile-stepper--static",
               R"(aria-label="Step 2 of 4")", "Back", "Next"});
    CHECK(count(dots, "skeuo-led--off") == 3) << dots;
    std::string progress = render_static(
        [] { return MobileStepper({.steps = 5, .active_step = 2, .variant = MobileStepperVariant::Progress}); });
    all(progress, {"skeuo-mobile-stepper--bottom", R"(role="progressbar")", R"(aria-valuenow="50")", "width: 50%"});
    std::string third = render_static(
        [] { return MobileStepper({.steps = 4, .active_step = 1, .variant = MobileStepperVariant::Progress}); });
    all(third, {R"(aria-valuenow="33")", "width: 33.333333333333336%"});
    std::string text = render_static(
        [] { return MobileStepper({.steps = 6, .active_step = 1, .variant = MobileStepperVariant::Text}); });
    CONTAINS(text, "2 / 6");
}

// ------------------------------------------------------- BottomNavigation

TEST(navigation_bottom_navigation_selected_and_labels) {
    std::string html = render_static([] {
        return BottomNavigation({.default_value = "b", .color = Color::Error},
                                {BottomNavigationAction({.value = "a", .label = "Alpha", .icon = Icon(IconName::Home)}),
                                 BottomNavigationAction({.value = "b", .label = "Beta", .icon = Icon(IconName::Star)})});
    });
    all(html, {R"(<nav class="skeuo-bottom-nav skeuo-chassis skeuo-elev-1 skeuo-color-error" aria-label="Navigation")",
               "skeuo-key--down skeuo-bottom-nav__action--selected", R"(aria-current="page")", "Beta",
               "skeuo-bottom-nav__action--icon-only", R"(aria-label="Alpha")"});
    LACKS(html, ">Alpha<");
    LACKS(html, "data-sk-bn");
    std::string labelled = render_static([] {
        return BottomNavigation({.value = "a", .show_labels = true},
                                {BottomNavigationAction({.value = "a", .label = "Alpha"}),
                                 BottomNavigationAction({.value = "b", .label = "Beta"})});
    });
    CONTAINS(labelled, ">Alpha<");
    CONTAINS(labelled, ">Beta<");
    LACKS(labelled, "icon-only");
    CHECK(count(labelled, "aria-current") == 1) << labelled;
}

TEST(navigation_bottom_navigation_click_selects) {
    std::string changed;
    Runtime rt([&] {
        return BottomNavigation({.default_value = "a", .onchange = [&](const std::string& v) { changed = v; }},
                                {BottomNavigationAction({.value = "a", .label = "Alpha"}),
                                 BottomNavigationAction({.value = "b", .label = "Beta"})});
    });
    std::string html = rt.render();
    html = nav_fire(rt, nav_handler(html, R"(aria-label="Beta")", "click"), "click");
    CHECK(changed == "b");
    CONTAINS(html, ">Beta<");
    CONTAINS(html, R"(aria-label="Alpha")");
}

// -------------------------------------------------------------- SpeedDial

TEST(navigation_speed_dial_open_and_closed) {
    std::string open = render_static([] {
        return SpeedDial({.aria_label = "Actions", .open = true, .direction = SpeedDialDirection::Left,
                          .color = Color::Secondary},
                         {SpeedDialAction({.icon = Icon(IconName::Copy), .tooltip_title = "Copy", .tooltip_open = true}),
                          SpeedDialAction({.icon = Icon(IconName::Edit), .tooltip_title = "Edit"})});
    });
    all(open, {"skeuo-speed-dial skeuo-speed-dial--left skeuo-speed-dial--open skeuo-color-secondary",
               "skeuo-speed-dial__fab skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--round",
               R"(aria-label="Actions")", R"(aria-expanded="true")", R"(aria-haspopup="true")", R"(role="menu")",
               R"(aria-orientation="horizontal")", "skeuo-speed-dial-icon skeuo-speed-dial-icon--open",
               "skeuo-speed-dial-action skeuo-speed-dial-action--tooltip-open", R"(aria-label="Copy")",
               "skeuo-speed-dial-action__key skeuo-key skeuo-key--pressable skeuo-key--round"});
    CHECK(count(open, R"(role="menuitem")") == 2) << open;
    std::string closed = render_static([] {
        return SpeedDial({.aria_label = "Actions", .hidden = true}, SpeedDialAction({.icon = Node("x"), .tooltip_title = "X"}));
    });
    all(closed, {R"(aria-expanded="false")", "skeuo-speed-dial--hidden", "skeuo-speed-dial--up",
                 R"(aria-orientation="vertical")", R"(tabindex="-1")"});
    LACKS(closed, "skeuo-speed-dial--open");
    LACKS(closed, "skeuo-speed-dial-icon--open");
}

TEST(navigation_speed_dial_toggles_and_escape_closes) {
    Runtime rt([] { return SpeedDial({.aria_label = "Actions"}, SpeedDialAction({.icon = Node("x"), .tooltip_title = "X"})); });
    std::string html = rt.render();
    html = nav_fire(rt, nav_handler(html, "skeuo-speed-dial__fab", "click"), "click");
    CONTAINS(html, "skeuo-speed-dial--open");
    CONTAINS(html, "skeuo-speed-dial-icon--open");
    CONTAINS(tag_of(html, R"(aria-label="X")"), R"(tabindex="0")");
    html = nav_fire(rt, nav_handler(html, R"(role="presentation")", "keydown"), "keydown", "Escape");
    LACKS(html, "skeuo-speed-dial--open");
}

// --------------------------------------------------------------- Timeline

TEST(navigation_timeline_structure_and_dots) {
    std::string html = render_static([] {
        return Timeline({.position = TimelinePosition::Alternate},
                        {
                            TimelineItem({}, {TimelineOppositeContent({}, "06:00"),
                                              TimelineSeparator({}, {TimelineDot({.color = Color::Success}),
                                                                     TimelineConnector()}),
                                              TimelineContent({}, "News")}),
                            TimelineItem({.position = TimelinePosition::Left},
                                         {TimelineSeparator({}, TimelineDot({.variant = TimelineDotVariant::Outlined})),
                                          TimelineContent({}, "Close")}),
                        });
    });
    all(html, {R"(<ul class="skeuo-timeline skeuo-timeline--alternate")", R"(<li class="skeuo-timeline-item")",
               R"(class="skeuo-timeline-item skeuo-timeline-item--left")", "skeuo-timeline-opposite skeuo-caption",
               "skeuo-timeline-dot skeuo-led skeuo-timeline-dot--filled skeuo-color-success",
               "skeuo-timeline-dot skeuo-led skeuo-led--off skeuo-timeline-dot--outlined skeuo-timeline-dot--grey",
               "skeuo-timeline-connector skeuo-groove", "skeuo-timeline-content", "News"});
}
