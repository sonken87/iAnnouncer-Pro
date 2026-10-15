use dioxus::prelude::*;

use super::paper::{PaperVariant, chassis_classes};
use crate::classes;

/// A card: a resting aluminium plate that holds one subject. Mirrors MUI's
/// `<Card>` (a `Paper` with `overflow: hidden`).
///
/// `raised` lifts it to a raised panel (chassis level 2); `variant: Outlined`
/// makes it flush with a machined edge. Global attributes pass through.
#[derive(Props, Clone, PartialEq)]
pub struct CardProps {
    /// Lift the card to a raised panel (chassis level 2).
    #[props(default)]
    pub raised: bool,
    #[props(default)]
    pub variant: PaperVariant,
    /// MUI elevation, 0–24 (default 1). Ignored when `raised`.
    #[props(default = 1)]
    pub elevation: u8,
    #[props(default)]
    pub square: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Card(props: CardProps) -> Element {
    let elevation = if props.raised { 4 } else { props.elevation };
    let class = classes([
        chassis_classes(props.variant, elevation, props.square).as_str(),
        "skeuo-card",
        &props.class,
    ]);
    rsx! {
        div { class, ..props.attributes, {props.children} }
    }
}

/// The heading row of a card: avatar, title, subheader and an action
/// (e.g. a "more" button) printed on the plate. Mirrors MUI's `<CardHeader>`.
#[derive(Props, Clone, PartialEq)]
pub struct CardHeaderProps {
    /// Element before the title, e.g. an avatar.
    pub avatar: Option<Element>,
    #[props(into)]
    pub title: Option<String>,
    #[props(into)]
    pub subheader: Option<String>,
    /// Element at the end of the row, e.g. an icon button.
    pub action: Option<Element>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn CardHeader(props: CardHeaderProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-card-header", &props.class]),
            if let Some(avatar) = props.avatar {
                div { class: "skeuo-card-header__avatar", {avatar} }
            }
            div { class: "skeuo-card-header__content",
                if let Some(title) = props.title {
                    span { class: "skeuo-card-header__title", "{title}" }
                }
                if let Some(sub) = props.subheader {
                    span { class: "skeuo-card-header__subheader skeuo-caption", "{sub}" }
                }
            }
            if let Some(action) = props.action {
                div { class: "skeuo-card-header__action", {action} }
            }
        }
    }
}

/// A full-bleed picture set into the card. Mirrors MUI's `<CardMedia>`.
///
/// With `image` and no `component`, renders a `div` with the image as a
/// cover background (labelled by `title`); with `component: "img"` renders an
/// `<img>` (use `alt`). Children are laid over the media area, so a media
/// block can also be a plain colored/gradient panel set by `style`/`class`.
#[derive(Props, Clone, PartialEq)]
pub struct CardMediaProps {
    /// Image URL (background image, or `src` when `component: "img"`).
    #[props(into)]
    pub image: Option<String>,
    /// `"img"` to render an `<img>` element; otherwise a `div`.
    #[props(into)]
    pub component: Option<String>,
    /// Alt text for `component: "img"`.
    #[props(into)]
    pub alt: Option<String>,
    /// Accessible name of a background-image media block.
    #[props(into)]
    pub title: Option<String>,
    /// CSS height, e.g. `"140px"`.
    #[props(into)]
    pub height: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn CardMedia(props: CardMediaProps) -> Element {
    let height = props
        .height
        .as_ref()
        .map(|h| format!("height:{h};"))
        .unwrap_or_default();
    if props.component.as_deref() == Some("img") {
        let class = classes(["skeuo-card-media", "skeuo-card-media--img", &props.class]);
        return rsx! {
            img {
                class,
                src: props.image.unwrap_or_default(),
                alt: props.alt.unwrap_or_default(),
                style: "{height}{props.style}",
            }
        };
    }
    let bg = props
        .image
        .as_ref()
        .map(|i| format!("background-image:url(\"{i}\");"))
        .unwrap_or_default();
    let class = classes(["skeuo-card-media", &props.class]);
    let labelled = props.title.is_some();
    rsx! {
        div {
            class,
            style: "{bg}{height}{props.style}",
            role: labelled.then_some("img"),
            aria_label: props.title,
            {props.children}
        }
    }
}

/// The padded body of a card. Mirrors MUI's `<CardContent>`.
#[derive(Props, Clone, PartialEq)]
pub struct CardContentProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn CardContent(props: CardContentProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-card-content", &props.class]), {props.children} }
    }
}

/// A row of keycaps along the bottom of a card. Mirrors MUI's `<CardActions>`.
#[derive(Props, Clone, PartialEq)]
pub struct CardActionsProps {
    /// Remove the gap between the actions.
    #[props(default)]
    pub disable_spacing: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn CardActions(props: CardActionsProps) -> Element {
    let class = classes([
        "skeuo-card-actions",
        if props.disable_spacing {
            ""
        } else {
            "skeuo-card-actions--spacing"
        },
        &props.class,
    ]);
    rsx! {
        div { class, {props.children} }
    }
}

/// Makes the card's content one large key: hovering lightens the plate and
/// pressing pushes the whole card down slightly. Mirrors MUI's
/// `<CardActionArea>` (a `<button>`).
#[derive(Props, Clone, PartialEq)]
pub struct CardActionAreaProps {
    #[props(default)]
    pub disabled: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn CardActionArea(props: CardActionAreaProps) -> Element {
    rsx! {
        button {
            class: classes(["skeuo-card-action-area", &props.class]),
            r#type: "button",
            disabled: props.disabled,
            onclick: move |e| {
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            {props.children}
        }
    }
}
