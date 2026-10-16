use dioxus::prelude::*;

use crate::{Color, classes};

/// Type style, mirroring MUI's Typography `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TypographyVariant {
    H1,
    H2,
    H3,
    H4,
    H5,
    H6,
    Subtitle1,
    Subtitle2,
    #[default]
    Body1,
    Body2,
    Caption,
    Overline,
    /// The legend style of a button.
    Button,
    /// Inherits the font of the surrounding lettering.
    Inherit,
}

impl TypographyVariant {
    pub const ALL: [TypographyVariant; 14] = [
        TypographyVariant::H1,
        TypographyVariant::H2,
        TypographyVariant::H3,
        TypographyVariant::H4,
        TypographyVariant::H5,
        TypographyVariant::H6,
        TypographyVariant::Subtitle1,
        TypographyVariant::Subtitle2,
        TypographyVariant::Body1,
        TypographyVariant::Body2,
        TypographyVariant::Caption,
        TypographyVariant::Overline,
        TypographyVariant::Button,
        TypographyVariant::Inherit,
    ];

    pub fn as_str(self) -> &'static str {
        match self {
            TypographyVariant::H1 => "h1",
            TypographyVariant::H2 => "h2",
            TypographyVariant::H3 => "h3",
            TypographyVariant::H4 => "h4",
            TypographyVariant::H5 => "h5",
            TypographyVariant::H6 => "h6",
            TypographyVariant::Subtitle1 => "subtitle1",
            TypographyVariant::Subtitle2 => "subtitle2",
            TypographyVariant::Body1 => "body1",
            TypographyVariant::Body2 => "body2",
            TypographyVariant::Caption => "caption",
            TypographyVariant::Overline => "overline",
            TypographyVariant::Button => "button",
            TypographyVariant::Inherit => "inherit",
        }
    }

    /// The HTML element MUI uses for this variant (`variantMapping`).
    fn default_tag(self) -> &'static str {
        match self {
            TypographyVariant::H1 => "h1",
            TypographyVariant::H2 => "h2",
            TypographyVariant::H3 => "h3",
            TypographyVariant::H4 => "h4",
            TypographyVariant::H5 => "h5",
            TypographyVariant::H6 => "h6",
            TypographyVariant::Subtitle1 | TypographyVariant::Subtitle2 => "h6",
            TypographyVariant::Body1 | TypographyVariant::Body2 => "p",
            _ => "span",
        }
    }
}

/// Ink of the lettering, as in MUI's Typography `color` prop
/// (`"text.primary"`, `"text.secondary"`, `"primary"`, …).
/// A [`Color`] converts into `Palette(color)`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TypographyColor {
    /// `text.primary`: `--sk-ink`.
    #[default]
    TextPrimary,
    /// `text.secondary`: `--sk-ink-2`.
    TextSecondary,
    /// `text.disabled`: `--sk-ink-3`.
    TextDisabled,
    /// Keeps the color of the surrounding text.
    Inherit,
    /// A palette color, printed as `--c-ink`.
    Palette(Color),
}

impl From<Color> for TypographyColor {
    fn from(color: Color) -> Self {
        TypographyColor::Palette(color)
    }
}

/// Text alignment, as in MUI's `align` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TypographyAlign {
    #[default]
    Inherit,
    Left,
    Center,
    Right,
    Justify,
}

impl TypographyAlign {
    fn as_str(self) -> &'static str {
        match self {
            TypographyAlign::Inherit => "inherit",
            TypographyAlign::Left => "left",
            TypographyAlign::Center => "center",
            TypographyAlign::Right => "right",
            TypographyAlign::Justify => "justify",
        }
    }
}

/// Text in the type scale. Mirrors MUI's `<Typography>`.
///
/// Physically it is plain lettering printed on the aluminium, like the
/// legends silk-screened on a Magic Keyboard: SF Pro with Apple's tight
/// heading tracking, no shadows, no embossing.
///
/// `component` overrides the HTML element (`"h1"`…`"h6"`, `"p"`, `"span"`,
/// `"div"`, `"label"`, `"legend"`, `"strong"`, `"em"`, `"small"`, `"li"`,
/// `"figcaption"`, `"blockquote"`, `"code"`); otherwise the element follows
/// MUI's default variant mapping, and `paragraph` forces a `<p>`.
#[derive(Props, Clone, PartialEq)]
pub struct TypographyProps {
    #[props(default)]
    pub variant: TypographyVariant,
    #[props(default, into)]
    pub color: TypographyColor,
    #[props(default)]
    pub align: TypographyAlign,
    /// Adds a bottom margin.
    #[props(default)]
    pub gutter_bottom: bool,
    /// Keeps the text on one line, truncated with an ellipsis.
    #[props(default)]
    pub no_wrap: bool,
    /// Renders a `<p>` with a paragraph bottom margin.
    #[props(default)]
    pub paragraph: bool,
    /// HTML element to render instead of the variant's default.
    #[props(into)]
    pub component: Option<String>,
    #[props(into)]
    pub id: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

macro_rules! tag {
    ($tag:ident, $class:expr, $style:expr, $id:expr, $children:expr) => {
        rsx! { $tag { class: $class, style: $style, id: $id, {$children} } }
    };
}

#[component]
pub fn Typography(props: TypographyProps) -> Element {
    let variant = format!("skeuo-typography--{}", props.variant.as_str());
    let (color_class, palette) = match props.color {
        TypographyColor::TextPrimary => ("skeuo-typography--text-primary", ""),
        TypographyColor::TextSecondary => ("skeuo-typography--text-secondary", ""),
        TypographyColor::TextDisabled => ("skeuo-typography--text-disabled", ""),
        TypographyColor::Inherit => ("skeuo-typography--color-inherit", ""),
        TypographyColor::Palette(c) => ("skeuo-typography--ink", c.class()),
    };
    let align = match props.align {
        TypographyAlign::Inherit => String::new(),
        a => format!("skeuo-typography--align-{}", a.as_str()),
    };
    let class = classes([
        "skeuo-typography",
        &variant,
        color_class,
        palette,
        &align,
        if props.gutter_bottom {
            "skeuo-typography--gutter-bottom"
        } else {
            ""
        },
        if props.no_wrap {
            "skeuo-typography--no-wrap"
        } else {
            ""
        },
        if props.paragraph {
            "skeuo-typography--paragraph"
        } else {
            ""
        },
        &props.class,
    ]);
    let tag = props.component.clone().unwrap_or_else(|| {
        if props.paragraph {
            "p".to_string()
        } else {
            props.variant.default_tag().to_string()
        }
    });
    let style = super::non_empty(props.style);
    let id = props.id;
    let children = props.children;

    match tag.as_str() {
        "h1" => tag!(h1, class, style, id, children),
        "h2" => tag!(h2, class, style, id, children),
        "h3" => tag!(h3, class, style, id, children),
        "h4" => tag!(h4, class, style, id, children),
        "h5" => tag!(h5, class, style, id, children),
        "h6" => tag!(h6, class, style, id, children),
        "p" => tag!(p, class, style, id, children),
        "div" => tag!(div, class, style, id, children),
        "label" => tag!(label, class, style, id, children),
        "legend" => tag!(legend, class, style, id, children),
        "strong" => tag!(strong, class, style, id, children),
        "em" => tag!(em, class, style, id, children),
        "small" => tag!(small, class, style, id, children),
        "li" => tag!(li, class, style, id, children),
        "figcaption" => tag!(figcaption, class, style, id, children),
        "blockquote" => tag!(blockquote, class, style, id, children),
        "code" => tag!(code, class, style, id, children),
        _ => tag!(span, class, style, id, children),
    }
}
