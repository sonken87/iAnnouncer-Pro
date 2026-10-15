use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::{Demo, DemoGrid, DemoRow};

#[component]
pub fn InputsSection() -> Element {
    rsx! {
        ButtonDemo {}
        TextFieldDemo {}
    }
}

#[component]
fn ButtonDemo() -> Element {
    let mut clicks = use_signal(|| 0u32);
    rsx! {
        Demo {
            title: "Button",
            description: "contained = colored keycap · outlined = white keycap · text = printed legend",
            for (variant, label) in [
                (ButtonVariant::Contained, "contained"),
                (ButtonVariant::Outlined, "outlined"),
                (ButtonVariant::Text, "text"),
            ] {
                DemoRow { label,
                    for color in Color::ALL {
                        Button { variant, color, onclick: move |_| clicks += 1, "{color.as_str()}" }
                    }
                }
            }
            DemoRow { label: "sizes",
                Button { size: Size::Small, "Small" }
                Button { size: Size::Medium, "Medium" }
                Button { size: Size::Large, "Large" }
                Button { disabled: true, "Disabled" }
                Button { variant: ButtonVariant::Outlined, disabled: true, "Disabled" }
            }
            DemoRow { label: "icons",
                Button { color: Color::Error, start_icon: rsx! { Icon { name: IconName::Dot } }, "Record" }
                Button { variant: ButtonVariant::Outlined, color: Color::Neutral, end_icon: rsx! { Icon { name: IconName::Play } }, "Play" }
                Button { color: Color::Success, end_icon: rsx! { Icon { name: IconName::ArrowForward } }, "On Air" }
            }
            DemoRow { label: "clicks", span { class: "skeuo-caption", "Clicked {clicks} times" } }
        }
    }
}

#[component]
fn TextFieldDemo() -> Element {
    let mut name = use_signal(String::new);
    let mut email = use_signal(|| "operator@station".to_string());
    let email_invalid = !email.read().contains('@');
    rsx! {
        Demo {
            title: "TextField",
            description: "outlined = well · filled = shallow recess · standard = engraved line",
            DemoGrid {
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
                TextField { label: "Filled", variant: TextFieldVariant::Filled, placeholder: "Shallow recess" }
                TextField {
                    label: "Frequency",
                    variant: TextFieldVariant::Filled,
                    default_value: "101.7",
                    end_adornment: rsx! { "MHz" },
                }
                TextField { label: "Disabled", default_value: "Read only", disabled: true }
                TextField { label: "Standard", variant: TextFieldVariant::Standard, placeholder: "Engraved line" }
                TextField {
                    label: "Search",
                    variant: TextFieldVariant::Standard,
                    start_adornment: rsx! { Icon { name: IconName::Search } },
                    placeholder: "Find a clip",
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
