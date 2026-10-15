#include "../gallery.hpp"

namespace gallery {

namespace {

Node ButtonDemo() {
    int& clicks = use_state<int>("demo.clicks", [] { return 0; });
    Node rows;
    for (auto [variant, label] : {std::pair{ButtonVariant::Contained, "contained"},
                                  std::pair{ButtonVariant::Outlined, "outlined"},
                                  std::pair{ButtonVariant::Text, "text"}}) {
        Node buttons;
        for (Color c : kAllColors)
            buttons.add_child(Button({.variant = variant, .color = c, .onclick = [&clicks] { ++clicks; }},
                                     std::string(as_str(c))));
        rows.add_child(DemoRow(label, std::move(buttons)));
    }
    return Demo("Button", "contained = colored keycap · outlined = white keycap · text = printed legend",
                {
                    std::move(rows),
                    DemoRow("sizes", {
                        Button({.size = Size::Small}, "Small"),
                        Button({.size = Size::Medium}, "Medium"),
                        Button({.size = Size::Large}, "Large"),
                        Button({.disabled = true}, "Disabled"),
                        Button({.variant = ButtonVariant::Outlined, .disabled = true}, "Disabled"),
                    }),
                    DemoRow("icons", {
                        Button({.color = Color::Error, .start_icon = Icon(IconName::Dot)}, "Record"),
                        Button({.variant = ButtonVariant::Outlined, .color = Color::Neutral,
                                .end_icon = Icon(IconName::Play)}, "Play"),
                        Button({.color = Color::Success, .end_icon = Icon(IconName::ArrowForward)}, "On Air"),
                    }),
                    DemoRow("loading", {
                        Button({.loading = true}, "Submit"),
                        Button({.color = Color::Success, .loading = true,
                                .loading_position = LoadingPosition::Start}, "Saving"),
                    }),
                    DemoRow("clicks", el("span").cls("skeuo-caption")
                                          .child("Clicked " + std::to_string(clicks) + " times")),
                });
}

Node TextFieldDemo() {
    std::string& name = use_state<std::string>("demo.name", [] { return std::string(); });
    std::string& email = use_state<std::string>("demo.email", [] { return std::string("operator@station"); });
    bool email_invalid = email.find('@') == std::string::npos;
    return Demo(
        "TextField", "outlined = well · filled = shallow recess · standard = engraved line",
        {
            DemoGrid({
                TextField({.label = "Announcer",
                           .value = name,
                           .placeholder = "Your name",
                           .helper_text = name.empty() ? std::string("Required") : "Hello, " + name,
                           .required = true,
                           .oninput = [&name](const std::string& v) { name = v; }}),
                TextField({.label = "Email",
                           .value = email,
                           .helper_text = email_invalid ? "Must contain @" : "Looks good",
                           .error = email_invalid,
                           .type = "email",
                           .oninput = [&email](const std::string& v) { email = v; }}),
                TextField({.label = "Password", .default_value = "secret", .type = "password"}),
                TextField({.variant = TextFieldVariant::Filled, .label = "Filled", .placeholder = "Shallow recess"}),
                TextField({.variant = TextFieldVariant::Filled, .label = "Frequency", .default_value = "101.7",
                           .end_adornment = Node("MHz")}),
                TextField({.label = "Disabled", .default_value = "Read only", .disabled = true}),
                TextField({.variant = TextFieldVariant::Standard, .label = "Standard", .placeholder = "Engraved line"}),
                TextField({.variant = TextFieldVariant::Standard, .label = "Search", .placeholder = "Find a clip",
                           .start_adornment = Icon(IconName::Search)}),
                TextField({.size = Size::Small, .label = "Small", .placeholder = "size=small"}),
            }),
            el("div").style("margin-top: 20px").child(
                TextField({.label = "Script", .placeholder = "Type the announcement…", .full_width = true,
                           .multiline = true, .rows = 4})),
        });
}

}  // namespace

Node InputsPage() { return {ButtonDemo(), TextFieldDemo()}; }

}  // namespace gallery
