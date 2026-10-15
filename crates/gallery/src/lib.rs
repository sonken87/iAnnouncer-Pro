//! Showcase page for skeuo-ui. Kept in a library so it can be launched as
//! an app (`main.rs`) or rendered to static HTML (`examples/snapshot.rs`).

use dioxus::prelude::*;
use skeuo_ui::prelude::*;

/// Panel background the components sit on: brushed aluminium.
const PAGE_CSS: &str = r#"
html, body { margin: 0; min-height: 100%; }
body {
  background:
    repeating-linear-gradient(90deg, rgba(255,255,255,0) 0 2px, rgba(255,255,255,.18) 2px 3px, rgba(0,0,0,.02) 3px 5px),
    linear-gradient(180deg, #e9e9e9 0%, #d5d5d5 100%);
  background-attachment: fixed;
}
.gallery { max-width: 960px; margin: 0 auto; padding: 32px 16px 64px; }
.gallery h1 {
  margin: 0 0 4px; font-size: 26px; color: #3a3a3a;
  text-shadow: 0 1px 0 rgba(255,255,255,.85);
}
.gallery > p { margin: 0 0 28px; color: #666; text-shadow: 0 1px 0 rgba(255,255,255,.7); }
.plate {
  margin-bottom: 24px; padding: 20px 22px 24px;
  border-radius: 12px; border: 1px solid rgba(0,0,0,.18);
  background: linear-gradient(180deg, #f4f4f4, #e4e4e4);
  box-shadow: inset 0 1px 0 #fff, 0 1px 2px rgba(0,0,0,.18), 0 6px 14px -6px rgba(0,0,0,.25);
}
.plate h2 {
  margin: 0 0 16px; font-size: 13px; letter-spacing: .08em; text-transform: uppercase;
  color: #555; text-shadow: 0 1px 0 #fff;
}
.row { display: flex; flex-wrap: wrap; align-items: center; gap: 14px; margin-bottom: 14px; }
.row:last-child { margin-bottom: 0; }
.row-label { width: 92px; font-size: 12px; color: #777; text-shadow: 0 1px 0 #fff; }
.form { display: grid; grid-template-columns: repeat(auto-fill, minmax(240px, 1fr)); gap: 20px; }
.readout { margin-top: 14px; font-size: 13px; color: #555; text-shadow: 0 1px 0 #fff; }
"#;

#[component]
pub fn Showcase() -> Element {
    let mut clicks = use_signal(|| 0u32);
    let mut name = use_signal(String::new);
    let mut email = use_signal(|| "operator@station".to_string());
    let email_invalid = !email.read().contains('@');
    let mut frequency = use_signal(|| "101.7".to_string());

    rsx! {
        style { {PAGE_CSS} }
        ThemeProvider {
            div { class: "gallery",
                h1 { "skeuo-ui" }
                p { "Skeuomorphic Dioxus components with an MUI-style API." }

                section { class: "plate",
                    h2 { "Button" }
                    for (variant, label) in [
                        (ButtonVariant::Contained, "contained"),
                        (ButtonVariant::Outlined, "outlined"),
                        (ButtonVariant::Text, "text"),
                    ] {
                        div { class: "row",
                            span { class: "row-label", "{label}" }
                            for color in Color::ALL {
                                Button {
                                    variant,
                                    color,
                                    onclick: move |_| clicks += 1,
                                    "{color.as_str()}"
                                }
                            }
                        }
                    }
                    div { class: "row",
                        span { class: "row-label", "sizes" }
                        Button { size: Size::Small, "Small" }
                        Button { size: Size::Medium, "Medium" }
                        Button { size: Size::Large, "Large" }
                        Button { disabled: true, "Disabled" }
                        Button { variant: ButtonVariant::Outlined, disabled: true, "Disabled" }
                    }
                    div { class: "row",
                        span { class: "row-label", "icons" }
                        Button { color: Color::Error, start_icon: rsx! { "●" }, "Record" }
                        Button { variant: ButtonVariant::Outlined, color: Color::Neutral, end_icon: rsx! { "▶" }, "Play" }
                        Button { color: Color::Success, end_icon: rsx! { "→" }, "On Air" }
                    }
                    p { class: "readout", "Clicked {clicks} times" }
                }

                section { class: "plate",
                    h2 { "TextField" }
                    div { class: "form",
                        TextField {
                            label: "Announcer",
                            placeholder: "Your name",
                            value: name(),
                            oninput: move |e: FormEvent| name.set(e.value()),
                            helper_text: if name.read().is_empty() { "Required".to_string() } else { format!("Hello, {}", name.read()) },
                            required: true,
                        }
                        TextField {
                            label: "Email",
                            r#type: "email",
                            value: email(),
                            oninput: move |e: FormEvent| email.set(e.value()),
                            error: email_invalid,
                            helper_text: if email_invalid { "Must contain @" } else { "Looks good" },
                        }
                        TextField { label: "Password", r#type: "password", default_value: "secret" }
                        TextField { label: "Disabled", default_value: "Read only", disabled: true }
                        TextField {
                            label: "Frequency",
                            variant: TextFieldVariant::Filled,
                            value: frequency(),
                            oninput: move |e: FormEvent| frequency.set(e.value()),
                            end_adornment: rsx! { "MHz" },
                        }
                        TextField {
                            label: "Error display",
                            variant: TextFieldVariant::Filled,
                            default_value: "SIGNAL LOST",
                            error: true,
                            helper_text: "Check the antenna",
                        }
                        TextField { label: "Standard", variant: TextFieldVariant::Standard, placeholder: "Engraved line" }
                        TextField {
                            label: "Price",
                            variant: TextFieldVariant::Standard,
                            start_adornment: rsx! { "¥" },
                            default_value: "1280",
                            color: Color::Success,
                        }
                        TextField { label: "Small", size: Size::Small, placeholder: "size=small" }
                    }
                    div { style: "margin-top: 20px",
                        TextField {
                            label: "Script",
                            multiline: true,
                            rows: 4,
                            full_width: true,
                            placeholder: "Type the announcement…",
                        }
                    }
                }
            }
        }
    }
}
