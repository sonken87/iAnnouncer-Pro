use dioxus::prelude::*;

use crate::classes;

/// Layout of an image list, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ImageListVariant {
    /// A regular grid.
    #[default]
    Standard,
    /// A grid where items span several `cols` / `rows`.
    Quilted,
    /// Columns of images with their natural heights.
    Masonry,
    /// A grid where every other item is shorter, centered in its row.
    Woven,
}

impl ImageListVariant {
    fn as_str(self) -> &'static str {
        match self {
            ImageListVariant::Standard => "standard",
            ImageListVariant::Quilted => "quilted",
            ImageListVariant::Masonry => "masonry",
            ImageListVariant::Woven => "woven",
        }
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
struct ImageListContext {
    variant: ImageListVariant,
}

/// A grid of images. Mirrors MUI's `<ImageList>`.
///
/// Each image is inset into a well cut into the aluminium, so photos read as
/// recessed panes rather than stickers on the surface.
#[derive(Props, Clone, PartialEq)]
pub struct ImageListProps {
    #[props(default)]
    pub variant: ImageListVariant,
    /// Number of columns.
    #[props(default = 2)]
    pub cols: u32,
    /// Space between images in px.
    #[props(default = 4)]
    pub gap: u32,
    /// Height of one grid row in px; `None` sizes rows to the images
    /// (MUI's `"auto"`). Ignored by `Masonry`.
    pub row_height: Option<u32>,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn ImageList(props: ImageListProps) -> Element {
    use_context_provider(|| ImageListContext {
        variant: props.variant,
    });
    let variant = format!("skeuo-image-list--{}", props.variant.as_str());
    let class = classes(["skeuo-image-list", &variant, &props.class]);
    let rows = props
        .row_height
        .map(|h| format!("grid-auto-rows: {h}px;"))
        .unwrap_or_default();
    let style = format!(
        "--image-list-cols: {}; --image-list-gap: {}px; {rows}{}",
        props.cols.max(1),
        props.gap,
        props.style
    );
    rsx! {
        ul { class, style, aria_label: props.aria_label, {props.children} }
    }
}

/// One image of an [`ImageList`]. Mirrors MUI's `<ImageListItem>`.
///
/// Put an `img` (and optionally an [`ImageListItemBar`]) inside; the image
/// fills the item and a well rim is drawn over its edge.
#[derive(Props, Clone, PartialEq)]
pub struct ImageListItemProps {
    /// Columns spanned (quilted / standard grids).
    #[props(default = 1)]
    pub cols: u32,
    /// Rows spanned (quilted / standard grids).
    #[props(default = 1)]
    pub rows: u32,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn ImageListItem(props: ImageListItemProps) -> Element {
    let ctx = try_use_context::<ImageListContext>().unwrap_or(ImageListContext {
        variant: ImageListVariant::Standard,
    });
    let span = if ctx.variant == ImageListVariant::Masonry {
        String::new()
    } else {
        format!(
            "grid-column-end: span {}; grid-row-end: span {};",
            props.cols.max(1),
            props.rows.max(1)
        )
    };
    let style = format!("{span}{}", props.style);
    let class = classes(["skeuo-image-list-item", &props.class]);
    rsx! {
        li { class, style: super::non_empty(style),
            {props.children}
            span { class: "skeuo-well skeuo-image-list-item__rim", aria_hidden: "true" }
        }
    }
}

/// Where the caption bar of an image sits, as in MUI's `position`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ImageListItemBarPosition {
    /// Over the bottom of the image.
    #[default]
    Bottom,
    /// Over the top of the image.
    Top,
    /// Printed on the aluminium under the image.
    Below,
}

impl ImageListItemBarPosition {
    fn as_str(self) -> &'static str {
        match self {
            ImageListItemBarPosition::Bottom => "bottom",
            ImageListItemBarPosition::Top => "top",
            ImageListItemBarPosition::Below => "below",
        }
    }
}

/// Title, subtitle and an action for an image. Mirrors MUI's
/// `<ImageListItemBar>`. Over the image it is a dark smoked strip with white
/// lettering; `Below` prints the lettering on the aluminium.
#[derive(Props, Clone, PartialEq)]
pub struct ImageListItemBarProps {
    #[props(into)]
    pub title: Option<String>,
    #[props(into)]
    pub subtitle: Option<String>,
    /// E.g. an icon button.
    pub action_icon: Option<Element>,
    /// Put the action before the title.
    #[props(default)]
    pub action_position_left: bool,
    #[props(default)]
    pub position: ImageListItemBarPosition,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn ImageListItemBar(props: ImageListItemBarProps) -> Element {
    let position = format!("skeuo-image-list-item-bar--{}", props.position.as_str());
    let class = classes([
        "skeuo-image-list-item-bar",
        &position,
        if props.action_position_left {
            "skeuo-image-list-item-bar--action-left"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div { class,
            span { class: "skeuo-image-list-item-bar__text",
                if let Some(title) = props.title {
                    span { class: "skeuo-image-list-item-bar__title", "{title}" }
                }
                if let Some(subtitle) = props.subtitle {
                    span { class: "skeuo-image-list-item-bar__subtitle", "{subtitle}" }
                }
            }
            if let Some(action) = props.action_icon {
                span { class: "skeuo-image-list-item-bar__action", {action} }
            }
        }
    }
}
