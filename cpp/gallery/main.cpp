// Gallery shell: sidebar with appearance switch and categories, one page per
// category. Mirrors crates/gallery/src/lib.rs.
//
//   gallery                     open the window
//   gallery --stdio             headless host (tools/harness.js drives it)
//   gallery --snapshot DIR      write every page (light + dark) as static HTML
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "gallery.hpp"

namespace gallery {

namespace {

/// The window background (the "desk") and the gallery's own layout. The
/// components themselves are styled entirely by skeuo-ui.
const char* const kPageCss = R"CSS(
html, body { margin: 0; min-height: 100%; }
html { background: #d6d7db; }
html:has(.skeuo-root[data-skeuo-mode="dark"]) { background: #18181a; }
@media (prefers-color-scheme: dark) { html:has(.skeuo-root[data-skeuo-mode="system"]) { background: #18181a; } }
.gallery {
  display: flex; min-height: 100vh;
  background: linear-gradient(180deg, var(--sk-page-hi) 0%, var(--sk-page-lo) 100%);
}
.gallery__nav {
  position: sticky; top: 0; align-self: flex-start;
  flex: none; width: 216px; height: 100vh; padding: 22px 12px;
  overflow-y: auto;
  border-right: 1px solid var(--sk-cut);
  box-shadow: 1px 0 0 var(--sk-catch);
}
.gallery__brand { padding: 0 10px 18px; }
.gallery__brand b { display: block; font-size: 17px; letter-spacing: -.01em; }
.gallery__brand span { font-size: 12px; color: var(--sk-ink-2); }
.gallery__mode { padding: 0 4px 18px; }
.gallery__mode .skeuo-tabs__tray { width: 100%; }
.gallery__mode .skeuo-tab { flex: 1; }
.gallery__nav-item {
  display: flex; align-items: center; gap: 10px; width: 100%;
  margin-bottom: 4px; padding: 8px 10px; border: 0; border-radius: 8px;
  background: transparent; color: var(--sk-ink); text-align: left;
  font: 500 13px/1.2 var(--skeuo-font); cursor: pointer;
}
.gallery__nav-item:hover { background: var(--sk-hover); }
.gallery__nav-item[aria-current="page"] {
  background: var(--sk-key);
  box-shadow: 0 0 0 .5px var(--sk-key-edge), 0 1px 2px var(--sk-ambient);
}
.gallery__main { flex: 1; min-width: 0; padding: 30px 32px 64px; }
.gallery__main > h1 { margin: 0 0 4px; font-size: 26px; letter-spacing: -.02em; }
.gallery__main > p { margin: 0 0 26px; color: var(--sk-ink-2); max-width: 70ch; }
.demo { margin-bottom: 22px; padding: 18px 22px 22px; }
.demo > h2 { margin: 0 0 4px; font-size: 15px; letter-spacing: -.01em; }
.demo > .demo__desc { margin: 0 0 16px; font-size: 12.5px; color: var(--sk-ink-2); }
.demo__row { display: flex; flex-wrap: wrap; align-items: center; gap: 14px; margin-bottom: 14px; }
.demo__row:last-child { margin-bottom: 0; }
.demo__row-label { width: 92px; flex: none; font-size: 12px; color: var(--sk-ink-2); }
.demo__grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(240px, 1fr)); gap: 20px; }
@media (max-width: 720px) {
  .gallery { flex-direction: column; }
  .gallery__nav { position: static; width: auto; height: auto; display: flex; flex-wrap: wrap; gap: 4px; }
  .gallery__nav-item { width: auto; }
  .gallery__main { padding: 20px 16px 48px; }
}
)CSS";

IconName page_icon(Page p) {
    switch (p) {
        case Page::Inputs: return IconName::Edit;
        case Page::Selection: return IconName::CheckCircle;
        case Page::DataDisplay: return IconName::Menu;
        case Page::Feedback: return IconName::Notifications;
        case Page::Surfaces: return IconName::Copy;
        case Page::Navigation: return IconName::ArrowForward;
        case Page::Icons: return IconName::Star;
    }
    return IconName::Star;
}

std::string_view page_intro(Page p) {
    switch (p) {
        case Page::Inputs: return "Keycaps and wells: buttons you press and fields you type into.";
        case Page::Selection: return "Checkboxes, radios, switches, sliders and ratings, with form wiring.";
        case Page::DataDisplay: return "Lettering, lists, tables, chips, avatars and indicator LEDs.";
        case Page::Feedback: return "Alerts, progress, dialogs and transient messages.";
        case Page::Surfaces: return "Aluminium plates that hold content, and the layout primitives.";
        case Page::Navigation: return "Moving between views: tabs, menus, steppers, pagination and more.";
        case Page::Icons: return "The built-in icon set, drawn to sit beside SF Pro legends.";
    }
    return "";
}

Node page_body(Page p) {
    switch (p) {
        case Page::Inputs: return InputsPage();
        case Page::Selection: return SelectionPage();
        case Page::DataDisplay: return DataDisplayPage();
        case Page::Feedback: return FeedbackPage();
        case Page::Surfaces: return SurfacesPage();
        case Page::Navigation: return NavigationPage();
        case Page::Icons: return IconsPage();
    }
    return {};
}

struct State {
    Page page = Page::Inputs;
    ThemeMode mode = ThemeMode::System;
};

/// The Light / Dark / Auto segmented control (same markup as skeuo Tabs).
Node mode_switch(State& state) {
    Node tray = el("div")
                    .cls("skeuo-tabs__tray skeuo-well skeuo-well--shallow")
                    .attr("role", "tablist")
                    .attr("aria-orientation", "horizontal")
                    .attr("aria-label", "Appearance");
    for (auto [mode, label] : {std::pair{ThemeMode::Light, "Light"}, std::pair{ThemeMode::Dark, "Dark"},
                               std::pair{ThemeMode::System, "Auto"}}) {
        bool selected = state.mode == mode;
        tray.child(el("button")
                       .cls(selected ? "skeuo-tab skeuo-key skeuo-tab--selected" : "skeuo-tab")
                       .attr("type", "button")
                       .attr("role", "tab")
                       .attr("aria-selected", selected ? "true" : "false")
                       .attr("tabindex", selected ? "0" : "-1")
                       .on("click", [&state, mode] { state.mode = mode; })
                       .child(el("span").cls("skeuo-tab__label").child(label)));
    }
    return el("div").cls("gallery__mode").child(
        el("div")
            .cls("skeuo-tabs skeuo-tabs--standard skeuo-tabs--horizontal skeuo-tabs--small skeuo-color-neutral")
            .child(std::move(tray)));
}

Node shell(State& state) {
    Node nav = el("nav").cls("gallery__nav").attr("aria-label", "Component categories");
    nav.child(el("div").cls("gallery__brand").child(el("b").child("skeuo-ui")).child(el("span").child("Magic Keyboard × MUI · C++")));
    nav.child(mode_switch(state));
    for (Page p : kAllPages) {
        nav.child(el("button")
                      .cls("gallery__nav-item")
                      .attr("aria-current", p == state.page ? "page" : "false")
                      .on("click", [&state, p] { state.page = p; })
                      .child(Icon({.name = page_icon(p), .size = "16px"}))
                      .child(std::string(page_title(p))));
    }
    Node main = el("main").cls("gallery__main");
    main.child(el("h1").child(std::string(page_title(state.page))));
    main.child(el("p").child(std::string(page_intro(state.page))));
    // Each page keeps its component state under its own key, so nothing
    // carries over between pages and a page starts fresh when re-entered.
    main.child(keyed(page_slug(state.page), [&] { return page_body(state.page); }));

    return Node{
        el("style").child(raw_html(kPageCss)),
        ThemeProvider({.theme = Theme{}.with_mode(state.mode)},
                      el("div").cls("gallery").child(std::move(nav)).child(std::move(main))),
    };
}

}  // namespace

std::string_view page_title(Page p) {
    switch (p) {
        case Page::Inputs: return "Inputs";
        case Page::Selection: return "Selection & forms";
        case Page::DataDisplay: return "Data display";
        case Page::Feedback: return "Feedback";
        case Page::Surfaces: return "Surfaces & layout";
        case Page::Navigation: return "Navigation";
        case Page::Icons: return "Icons";
    }
    return "";
}

std::string_view page_slug(Page p) {
    switch (p) {
        case Page::Inputs: return "inputs";
        case Page::Selection: return "selection";
        case Page::DataDisplay: return "data-display";
        case Page::Feedback: return "feedback";
        case Page::Surfaces: return "surfaces";
        case Page::Navigation: return "navigation";
        case Page::Icons: return "icons";
    }
    return "";
}

Node Demo(std::string title, std::string description, Node children) {
    Node section = el("section").cls("demo skeuo-chassis skeuo-elev-2").child(el("h2").child(std::move(title)));
    if (!description.empty()) section.child(el("p").cls("demo__desc").child(std::move(description)));
    return std::move(section).child(std::move(children));
}

Node DemoRow(std::string label, Node children) {
    Node row = el("div").cls("demo__row");
    if (!label.empty()) row.child(el("span").cls("demo__row-label").child(std::move(label)));
    return std::move(row).child(std::move(children));
}

Node DemoGrid(Node children) { return el("div").cls("demo__grid").child(std::move(children)); }

}  // namespace gallery

int main(int argc, char** argv) {
    using namespace gallery;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--snapshot") == 0) {
            std::filesystem::path dir = argv[i + 1];
            std::filesystem::create_directories(dir);
            for (auto [mode, suffix] : {std::pair{ThemeMode::Light, ""}, std::pair{ThemeMode::Dark, "-dark"}}) {
                for (Page p : kAllPages) {
                    State state{p, mode};
                    skeuo::Runtime rt([&] { return shell(state); }, "skeuo-ui — " + std::string(page_title(p)));
                    std::filesystem::path file = dir / (std::string(page_slug(p)) + suffix + ".html");
                    std::ofstream(file, std::ios::binary) << rt.page();
                    std::cout << "wrote " << file.string() << "\n";
                }
            }
            return 0;
        }
    }
    State state;
    skeuo::Runtime runtime([&] { return shell(state); }, "skeuo-ui — component gallery");
    return skeuo::run_app(runtime, {.title = "skeuo-ui — component gallery (C++)", .min_width = 760, .min_height = 560},
                          argc, argv);
}
