#include "skeuo/skeuo.hpp"
#include "test.hpp"

using namespace skeuo;

TEST(button_default_classes_and_type) {
    std::string html = render_static([] { return Button({}, "Save"); });
    CONTAINS(html, R"(class="skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-btn skeuo-btn--contained skeuo-color-primary skeuo-btn--medium")");
    CONTAINS(html, R"(type="button")");
    CONTAINS(html, "Save");
    LACKS(html, "disabled");
}

TEST(button_props_and_passthrough_attributes) {
    std::string html = render_static([] {
        return Button({.variant = ButtonVariant::Outlined, .color = Color::Error, .size = Size::Small,
                       .disabled = true, .full_width = true, .type = "submit", .start_icon = Node("✕"),
                       .cls = "mine", .attrs = {{"id", "del"}, {"aria-label", "Delete item"}}},
                      "Delete");
    });
    for (const char* needle : {"skeuo-btn--outlined", "skeuo-color-error", "skeuo-btn--small",
                               "skeuo-btn--full-width", "mine", R"(type="submit")", R"(id="del")",
                               R"(aria-label="Delete item")", " disabled", "skeuo-btn__icon--start"})
        CONTAINS(html, needle);
    LACKS(html, "skeuo-btn__icon--end");
}

TEST(button_loading_disables_and_shows_spinner) {
    std::string html = render_static([] { return Button({.loading = true}, "Go"); });
    CONTAINS(html, " disabled");
    CONTAINS(html, R"(aria-busy="true")");
    CONTAINS(html, "skeuo-spinner");
    CONTAINS(html, "skeuo-btn--loading-center");
}

TEST(button_click_reaches_handler) {
    int clicks = 0;
    Runtime rt([&] { return Button({.onclick = [&] { ++clicks; }}, "x"); });
    CONTAINS(rt.render(), R"(data-sk-on="click:0")");
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"click"}})");
    CHECK(clicks == 1);
}

TEST(text_field_links_label_and_helper) {
    std::string html = render_static([] {
        return TextField({.label = "Email", .value = "a@b.c", .helper_text = "We never share it", .required = true,
                          .type = "email", .id = "email"});
    });
    for (const char* needle : {R"(for="email")", R"(id="email")", R"(type="email")", R"(value="a@b.c")",
                               R"(aria-describedby="email-helper")", R"(id="email-helper")", "We never share it",
                               "skeuo-field__asterisk", " required"})
        CONTAINS(html, needle);
}

TEST(text_field_generates_unique_ids) {
    std::string html = render_static([] { return Node{TextField({.label = "A"}), TextField({.label = "B"})}; });
    size_t first = html.find("for=\"");
    size_t second = html.find("for=\"", first + 1);
    CHECK(first != std::string::npos && second != std::string::npos);
    auto id_at = [&](size_t pos) { pos += 5; return html.substr(pos, html.find('"', pos) - pos); };
    CHECK(id_at(first) != id_at(second)) << html;
}

TEST(text_field_variants_error_multiline_adornments) {
    std::string html = render_static([] {
        return TextField({.variant = TextFieldVariant::Filled, .default_value = "notes", .error = true,
                          .multiline = true, .rows = 4, .start_adornment = Node("¥")});
    });
    for (const char* needle : {"skeuo-field--filled", "skeuo-well--shallow", "skeuo-field--error",
                               "skeuo-field--multiline", "<textarea", R"(rows="4")", R"(aria-invalid="true")",
                               "skeuo-field__adornment--start", ">notes</textarea>"})
        CONTAINS(html, needle);
    LACKS(html, "<label");
    LACKS(html, "aria-describedby");
}

TEST(text_field_default_value_is_read_once) {
    std::string initial = "first";
    Runtime rt([&] { return TextField({.default_value = initial}); });
    CONTAINS(rt.render(), R"(value="first")");
    initial = "second";
    CONTAINS(rt.render(), R"(value="first")");
}

TEST(text_field_input_reports_value) {
    std::string got;
    Runtime rt([&] { return TextField({.oninput = [&](const std::string& v) { got = v; }}); });
    rt.render();
    rt.handle(R"({"t":"event","h":[0],"e":{"type":"input","value":"typed"}})");
    CHECK(got == "typed");
}
