//! Component gallery for skeuo-ui. Kept in a library so it can be launched
//! as an app (`main.rs`) or rendered to static HTML (`examples/snapshot.rs`).

use dioxus::prelude::*;
use skeuo_ui::prelude::*;

pub mod sections;

/// The window background (the "desk") and the gallery's own layout. The
/// components themselves are styled entirely by skeuo-ui.
const PAGE_CSS: &str = r#"
html { min-height: 100%; background: linear-gradient(180deg, #e6e7ea 0%, #d6d7db 100%); }
body { margin: 0; }
.gallery { display: flex; min-height: 100vh; }
.gallery__nav {
  position: sticky; top: 0; align-self: flex-start;
  flex: none; width: 216px; height: 100vh; padding: 22px 12px;
  overflow-y: auto;
  border-right: 1px solid rgba(0,0,0,.1);
  box-shadow: 1px 0 0 rgba(255,255,255,.6);
}
.gallery__brand { padding: 0 10px 18px; }
.gallery__brand b { display: block; font-size: 17px; letter-spacing: -.01em; }
.gallery__brand span { font-size: 12px; color: var(--sk-ink-2); }
.gallery__nav-item {
  display: flex; align-items: center; gap: 10px; width: 100%;
  margin-bottom: 4px; padding: 8px 10px; border: 0; border-radius: 8px;
  background: transparent; color: var(--sk-ink); text-align: left;
  font: 500 13px/1.2 var(--skeuo-font); cursor: pointer;
}
.gallery__nav-item:hover { background: var(--sk-hover); }
.gallery__nav-item[aria-current="page"] {
  background: rgba(255,255,255,.7);
  box-shadow: 0 0 0 .5px rgba(0,0,0,.1), 0 1px 2px rgba(0,0,0,.08);
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
"#;

/// A gallery page: one component category.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Page {
    Inputs,
    Selection,
    DataDisplay,
    Feedback,
    Surfaces,
    Navigation,
    Icons,
}

impl Page {
    pub const ALL: [Page; 7] = [
        Page::Inputs,
        Page::Selection,
        Page::DataDisplay,
        Page::Feedback,
        Page::Surfaces,
        Page::Navigation,
        Page::Icons,
    ];

    pub fn title(self) -> &'static str {
        match self {
            Page::Inputs => "Inputs",
            Page::Selection => "Selection & forms",
            Page::DataDisplay => "Data display",
            Page::Feedback => "Feedback",
            Page::Surfaces => "Surfaces & layout",
            Page::Navigation => "Navigation",
            Page::Icons => "Icons",
        }
    }

    pub fn slug(self) -> &'static str {
        match self {
            Page::Inputs => "inputs",
            Page::Selection => "selection",
            Page::DataDisplay => "data-display",
            Page::Feedback => "feedback",
            Page::Surfaces => "surfaces",
            Page::Navigation => "navigation",
            Page::Icons => "icons",
        }
    }

    fn icon(self) -> IconName {
        match self {
            Page::Inputs => IconName::Edit,
            Page::Selection => IconName::CheckCircle,
            Page::DataDisplay => IconName::Menu,
            Page::Feedback => IconName::Notifications,
            Page::Surfaces => IconName::Copy,
            Page::Navigation => IconName::ArrowForward,
            Page::Icons => IconName::Star,
        }
    }

    fn intro(self) -> &'static str {
        match self {
            Page::Inputs => "Keycaps and wells: buttons you press and fields you type into.",
            Page::Selection => {
                "Checkboxes, radios, switches, sliders and ratings, with form wiring."
            }
            Page::DataDisplay => "Lettering, lists, tables, chips, avatars and indicator LEDs.",
            Page::Feedback => "Alerts, progress, dialogs and transient messages.",
            Page::Surfaces => "Aluminium plates that hold content, and the layout primitives.",
            Page::Navigation => "Moving between views: tabs, menus, steppers, pagination and more.",
            Page::Icons => "The built-in icon set, drawn to sit beside SF Pro legends.",
        }
    }
}

/// The interactive gallery app.
#[component]
pub fn App() -> Element {
    let page = use_signal(|| Page::Inputs);
    rsx! {
        Shell { page }
    }
}

/// The gallery fixed on one page (used for static snapshots).
#[component]
pub fn GalleryShell(page: Page) -> Element {
    let page = use_signal(|| page);
    rsx! {
        Shell { page }
    }
}

#[component]
fn Shell(page: Signal<Page>) -> Element {
    let current = page();
    rsx! {
        style { {PAGE_CSS} }
        ThemeProvider {
            div { class: "gallery",
                nav { class: "gallery__nav", aria_label: "Component categories",
                    div { class: "gallery__brand",
                        b { "skeuo-ui" }
                        span { "Magic Keyboard × MUI" }
                    }
                    for p in Page::ALL {
                        button {
                            class: "gallery__nav-item",
                            aria_current: if p == current { "page" } else { "false" },
                            onclick: move |_| page.set(p),
                            Icon { name: p.icon(), size: "16px" }
                            "{p.title()}"
                        }
                    }
                }
                main { class: "gallery__main",
                    h1 { "{current.title()}" }
                    p { "{current.intro()}" }
                    match current {
                        Page::Inputs => rsx! { sections::InputsSection {} },
                        Page::Selection => rsx! { sections::SelectionSection {} },
                        Page::DataDisplay => rsx! { sections::DataDisplaySection {} },
                        Page::Feedback => rsx! { sections::FeedbackSection {} },
                        Page::Surfaces => rsx! { sections::SurfacesSection {} },
                        Page::Navigation => rsx! { sections::NavigationSection {} },
                        Page::Icons => rsx! { sections::IconsSection {} },
                    }
                }
            }
        }
    }
}

/// One demo plate: a titled aluminium panel holding examples of a component.
#[component]
pub fn Demo(
    #[props(into)] title: String,
    #[props(into)] description: Option<String>,
    children: Element,
) -> Element {
    rsx! {
        section { class: "demo skeuo-chassis skeuo-elev-2",
            h2 { "{title}" }
            if let Some(d) = description {
                p { class: "demo__desc", "{d}" }
            }
            {children}
        }
    }
}

/// A labelled row of examples inside a [`Demo`].
#[component]
pub fn DemoRow(#[props(into)] label: Option<String>, children: Element) -> Element {
    rsx! {
        div { class: "demo__row",
            if let Some(l) = label {
                span { class: "demo__row-label", "{l}" }
            }
            {children}
        }
    }
}

/// A responsive grid of examples inside a [`Demo`].
#[component]
pub fn DemoGrid(children: Element) -> Element {
    rsx! {
        div { class: "demo__grid", {children} }
    }
}
