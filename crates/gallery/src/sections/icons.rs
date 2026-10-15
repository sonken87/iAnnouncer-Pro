use dioxus::prelude::*;
use skeuo_ui::prelude::*;

use crate::Demo;

const ICONS_CSS: &str = r#"
.icon-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(104px, 1fr)); gap: 10px; }
.icon-cell { display: flex; flex-direction: column; align-items: center; gap: 8px; padding: 14px 6px 10px; }
.icon-cell span { font-size: 11px; color: var(--sk-ink-2); }
"#;

#[component]
pub fn IconsSection() -> Element {
    rsx! {
        style { {ICONS_CSS} }
        Demo {
            title: "Icon",
            description: "Icon {{ name: IconName::… }} — 1em, currentColor; pass title for an accessible name.",
            div { class: "icon-grid",
                for name in IconName::ALL {
                    div { class: "icon-cell skeuo-key",
                        Icon { name, size: "22px" }
                        span { "{name:?}" }
                    }
                }
            }
        }
    }
}
