#include "skeuo/json.hpp"
#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

TEST(json_roundtrip) {
    auto v = json::parse(R"({"t":"event","h":[3,7],"e":{"type":"input","value":"a\"bé","checked":true}})");
    CHECK(v.str("t") == "event");
    CHECK(v.get("h")->array.size() == 2);
    CHECK(v.get("e")->str("value") == "a\"b\xc3\xa9");
    CHECK(v.get("e")->flag("checked"));
    CHECK(json::quote("a\"\n<") == R"("a\"\n<")");
    CHECK(json::parse("{bad").type == json::Value::Type::Null);
}

TEST(html_escaping_and_void_elements) {
    std::string html = to_html(el("div").attr("title", "a\"<b>").child("x < y").child(el("input").flag("disabled", true)));
    CHECK(html == R"(<div title="a&quot;&lt;b&gt;">x &lt; y<input disabled></div>)") << html;
}

TEST(fragments_flatten_and_classes_join) {
    Node n = el("p").cls("a").cls("").cls("b").child(Node{"x", Node{}, Node{"y", "z"}});
    CHECK(to_html(n) == R"(<p class="a b">xyz</p>)") << to_html(n);
    CHECK(classes({"a", "", "b"}) == "a b");
}

TEST(runtime_dispatches_events_and_rerenders) {
    int count = 0;
    std::string typed;
    Runtime rt([&] {
        return el("div").child(el("button").on("click", [&] { ++count; }).child(std::to_string(count)))
                        .child(el("input").on("input", [&](const Event& e) { typed = e.value; }));
    });
    std::string first = rt.render();
    CONTAINS(first, R"(data-sk-on="click:0")");
    CONTAINS(first, R"(data-sk-on="input:1")");
    std::string reply = rt.handle(R"({"t":"event","seq":4,"h":[0],"e":{"type":"click"}})");
    CHECK(count == 1);
    CONTAINS(reply, R"("seq":4)");
    CONTAINS(reply, ">1</button>");
    rt.handle(R"({"t":"event","h":[1],"e":{"type":"input","value":"hello"}})");
    CHECK(typed == "hello");
    // Unknown ids are ignored.
    rt.handle(R"({"t":"event","h":[99],"e":{"type":"click"}})");
    CHECK(count == 1);
}

TEST(listener_flags_are_serialized) {
    std::vector<Handler> handlers;
    std::string html = to_html(el("div").on("click", [] {}, StopPropagation | PreventDefault), &handlers);
    CONTAINS(html, R"(data-sk-on="click:0!~")");
    CHECK(handlers.size() == 1);
}

TEST(hook_state_persists_and_is_dropped_when_unused) {
    bool show = true;
    int* seen = nullptr;
    Runtime rt([&] {
        if (show) {
            int& n = use_state<int>("counter", [] { return 5; });
            seen = &n;
            return el("b").on("click", [&n] { ++n; }).child(std::to_string(n));
        }
        return el("i");
    });
    CONTAINS(rt.render(), ">5<");
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"click"}})");
    CHECK(*seen == 6);
    CONTAINS(rt.render(), ">6<");
    show = false;
    rt.render();
    show = true;
    CONTAINS(rt.render(), ">5<");  // state was dropped while unused
}

TEST(unique_ids_are_stable_across_renders) {
    std::string a1, b1, a2;
    int pass = 0;
    Runtime rt([&] {
        std::string a = use_unique_id("x"), b = use_unique_id("x");
        if (pass == 0) { a1 = a; b1 = b; } else { a2 = a; }
        ++pass;
        return Node{};
    });
    rt.render();
    rt.render();
    CHECK(a1 != b1);
    CHECK(a1 == a2);
}

TEST(page_contains_stylesheet_runtime_and_body) {
    Runtime rt([] { return el("p").child("hi"); }, "T<itle");
    std::string page = rt.page();
    CONTAINS(page, "<title>T&lt;itle</title>");
    CONTAINS(page, ".skeuo-key--filled");
    CONTAINS(page, R"(.skeuo-root[data-skeuo-mode="dark"])");
    CONTAINS(page, R"(<div id="sk-root"><p>hi</p></div>)");
    CONTAINS(page, "data-sk-on");  // the runtime script
    LACKS(stylesheet(), "SKEUO_DARK");
}

TEST(theme_provider_sets_mode_and_both_palettes) {
    std::string html = to_html(ThemeProvider({.theme = Theme::make_dark()}, "x"));
    CONTAINS(html, R"(data-skeuo-mode="dark")");
    CONTAINS(html, "--skeuo-light-primary-main:#0071e3");
    CONTAINS(html, "--skeuo-dark-primary-main:#0a84ff");
    Theme t;
    t.dark.get(Color::Primary).main = "#ff00aa";
    t.radius = 4;
    std::string custom = to_html(ThemeProvider({.theme = t}, "x"));
    CONTAINS(custom, "--skeuo-dark-primary-main:#ff00aa");
    CONTAINS(custom, "--skeuo-radius:4px");
}

TEST(icons_render_svg) {
    std::string html = to_html(Icon({.name = IconName::Star, .title = "Rated"}));
    CONTAINS(html, R"(fill="currentColor")");
    CONTAINS(html, R"(role="img")");
    CONTAINS(html, "<title>Rated</title>");
    CONTAINS(to_html(Icon(IconName::Close)), R"(aria-hidden="true")");
}
