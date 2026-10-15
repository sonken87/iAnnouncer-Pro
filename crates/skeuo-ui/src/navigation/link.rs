use dioxus::prelude::*;

use crate::{Color, classes};

/// When a link is underlined, mirroring MUI's `underline` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum LinkUnderline {
    None,
    Hover,
    #[default]
    Always,
}

impl LinkUnderline {
    fn as_str(self) -> &'static str {
        match self {
            LinkUnderline::None => "none",
            LinkUnderline::Hover => "hover",
            LinkUnderline::Always => "always",
        }
    }
}

/// A hyperlink printed on the aluminium in the palette ink (`--c-ink`).
/// Mirrors MUI's `<Link>`. Without `href` it renders a `<button>` styled as a
/// link (MUI's `component="button"`).
#[derive(Props, Clone, PartialEq)]
pub struct LinkProps {
    #[props(into)]
    pub href: Option<String>,
    #[props(default)]
    pub underline: LinkUnderline,
    #[props(default)]
    pub color: Color,
    #[props(into)]
    pub target: Option<String>,
    #[props(into)]
    pub rel: Option<String>,
    /// Marks the link as the current page (`aria-current="page"`).
    #[props(default)]
    pub current: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn Link(props: LinkProps) -> Element {
    let underline = format!("skeuo-link--underline-{}", props.underline.as_str());
    let class = classes(["skeuo-link", &underline, props.color.class(), &props.class]);
    let onclick = move |e: MouseEvent| {
        if let Some(h) = &props.onclick {
            h.call(e);
        }
    };
    let current = props.current.then_some("page");
    match props.href {
        Some(href) => rsx! {
            a {
                class,
                href,
                target: props.target,
                rel: props.rel,
                aria_current: current,
                onclick,
                {props.children}
            }
        },
        None => rsx! {
            button {
                class: "{class} skeuo-link--button",
                r#type: "button",
                aria_current: current,
                onclick,
                {props.children}
            }
        },
    }
}
