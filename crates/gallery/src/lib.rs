//! Showcase page for skeuo-ui. Kept in a library so it can be launched as
//! an app (`main.rs`) or rendered to static HTML (`examples/snapshot.rs`).

use dioxus::prelude::*;
use skeuo_ui::prelude::*;

/// Page background: the bead-blasted aluminium of a Magic Keyboard.
const PAGE_CSS: &str = r#"
html { min-height: 100%; background: linear-gradient(180deg, #e6e7ea 0%, #d6d7db 100%); }
body { margin: 0; }
.gallery { max-width: 960px; margin: 0 auto; padding: 32px 16px 64px; }
.gallery h1 {
  margin: 0 0 4px; font-size: 26px; color: #3a3a3a;
  text-shadow: 0 1px 0 rgba(255,255,255,.85);
}
.gallery > p { margin: 0 0 28px; color: #666; text-shadow: 0 1px 0 rgba(255,255,255,.7); }
.plate {
  margin-bottom: 24px; padding: 20px 22px 24px;
  border-radius: 18px; border: 0.5px solid rgba(0,0,0,.14);
  background: linear-gradient(180deg, #eff0f2, #e3e4e7);
  box-shadow: inset 0 1px 0 rgba(255,255,255,.9), 0 1px 2px rgba(0,0,0,.12), 0 12px 28px -14px rgba(0,0,0,.35);
}
.keys { display: flex; flex-wrap: wrap; gap: 6px; }
.keys .skeuo-btn { min-width: 46px; height: 46px; padding: 0 10px; font-weight: 400; }
.keys .wide { min-width: 84px; justify-content: flex-start; align-items: flex-end; padding-bottom: 8px; }
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
                p { "Skeuomorphic Dioxus components in the Magic Keyboard design language, with an MUI-style API." }

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
                    div { class: "row",
                        span { class: "row-label", "keyboard" }
                        div { class: "keys",
                            Button { variant: ButtonVariant::Outlined, color: Color::Neutral, class: "wide", "esc" }
                            for k in ["F1", "F2", "F3", "F4"] {
                                Button { variant: ButtonVariant::Outlined, color: Color::Neutral, "{k}" }
                            }
                            Button { variant: ButtonVariant::Outlined, color: Color::Neutral, class: "wide", "⌘ command" }
                            Button { variant: ButtonVariant::Outlined, color: Color::Neutral, class: "wide", "return" }
                        }
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
