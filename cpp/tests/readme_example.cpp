// Keeps the README usage example compiling and working.
#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

TEST(readme_example_renders_and_reacts) {
    std::string name;
    bool on_air = false;
    int saved = 0;

    Runtime app([&] {
        return ThemeProvider({.theme = Theme::make_system()}, {
            TextField({.label = "Name", .value = name,
                       .oninput = [&](const std::string& v) { name = v; }}),
            FormControlLabel({.control = Switch({.checked = on_air,
                                                 .onchange = [&](bool v) { on_air = v; }}),
                              .label = "On air"}),
            Button({.variant = ButtonVariant::Contained, .onclick = [&] { ++saved; }}, "Save"),
        });
    });
    std::string html = app.render();
    CONTAINS(html, R"(data-skeuo-mode="system")");
    CONTAINS(html, R"(role="switch")");
    CONTAINS(html, "On air");
    // input:0, switch change:1, button click:2 in document order.
    app.handle(R"({"t":"event","h":[0],"e":{"type":"input","value":"Ken"}})");
    CHECK(name == "Ken");
    std::string after = app.render();
    CONTAINS(after, R"(value="Ken")");
}
