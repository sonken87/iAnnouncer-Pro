use dioxus::core::{DynamicNode, Template, TemplateNode};
use dioxus::prelude::*;

use crate::classes;

/// A plain layout wrapper with no material. Mirrors MUI's `<Box>` (renamed
/// so it doesn't shadow Rust's `Box`).
///
/// `component` picks the HTML tag (`div` by default; also `span`, `section`,
/// `article`, `main`, `header`, `footer`, `nav`, `aside`, `ul`, `ol`, `li`,
/// `form`, `p`). Global attributes such as `style` pass through.
#[derive(Props, Clone, PartialEq)]
pub struct SkeuoBoxProps {
    #[props(into)]
    pub component: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn SkeuoBox(props: SkeuoBoxProps) -> Element {
    let class = classes(["skeuo-box", &props.class]);
    let a = props.attributes;
    let c = props.children;
    match props.component.as_deref().unwrap_or("div") {
        "span" => rsx! { span { class, ..a, {c} } },
        "section" => rsx! { section { class, ..a, {c} } },
        "article" => rsx! { article { class, ..a, {c} } },
        "main" => rsx! { main { class, ..a, {c} } },
        "header" => rsx! { header { class, ..a, {c} } },
        "footer" => rsx! { footer { class, ..a, {c} } },
        "nav" => rsx! { nav { class, ..a, {c} } },
        "aside" => rsx! { aside { class, ..a, {c} } },
        "ul" => rsx! { ul { class, ..a, {c} } },
        "ol" => rsx! { ol { class, ..a, {c} } },
        "li" => rsx! { li { class, ..a, {c} } },
        "form" => rsx! { form { class, ..a, {c} } },
        "p" => rsx! { p { class, ..a, {c} } },
        _ => rsx! { div { class, ..a, {c} } },
    }
}

/// Maximum width of a [`Container`], mirroring MUI's `maxWidth` prop
/// (breakpoints xs 444 · sm 600 · md 900 · lg 1200 · xl 1536 px).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum ContainerMaxWidth {
    Xs,
    Sm,
    Md,
    #[default]
    Lg,
    Xl,
    /// No maximum (MUI `maxWidth={false}`).
    False,
}

impl ContainerMaxWidth {
    pub fn as_str(self) -> &'static str {
        match self {
            ContainerMaxWidth::Xs => "xs",
            ContainerMaxWidth::Sm => "sm",
            ContainerMaxWidth::Md => "md",
            ContainerMaxWidth::Lg => "lg",
            ContainerMaxWidth::Xl => "xl",
            ContainerMaxWidth::False => "false",
        }
    }
}

/// Centers content horizontally with a maximum width and side gutters. No
/// material. Mirrors MUI's `<Container>`.
#[derive(Props, Clone, PartialEq)]
pub struct ContainerProps {
    #[props(default)]
    pub max_width: ContainerMaxWidth,
    /// Snap the max width to the current breakpoint's minimum width.
    #[props(default)]
    pub fixed: bool,
    /// Remove the side padding.
    #[props(default)]
    pub disable_gutters: bool,
    #[props(default, into)]
    pub class: String,
    #[props(extends = GlobalAttributes)]
    pub attributes: Vec<Attribute>,
    pub children: Element,
}

#[component]
pub fn Container(props: ContainerProps) -> Element {
    let max = format!("skeuo-container--max-{}", props.max_width.as_str());
    let class = classes([
        "skeuo-container",
        &max,
        if props.fixed {
            "skeuo-container--fixed"
        } else {
            ""
        },
        if props.disable_gutters {
            ""
        } else {
            "skeuo-container--gutters"
        },
        &props.class,
    ]);
    rsx! {
        div { class, ..props.attributes, {props.children} }
    }
}

/// Direction of a [`Stack`], mirroring MUI's `direction` prop.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum StackDirection {
    Row,
    RowReverse,
    #[default]
    Column,
    ColumnReverse,
}

impl StackDirection {
    pub fn as_str(self) -> &'static str {
        match self {
            StackDirection::Row => "row",
            StackDirection::RowReverse => "row-reverse",
            StackDirection::Column => "column",
            StackDirection::ColumnReverse => "column-reverse",
        }
    }
}

/// Lays its children out in a row or column with even spacing, optionally
/// with a divider element between them. No material. Mirrors MUI's
/// `<Stack>`.
///
/// `spacing` is in 8px units (MUI's default theme spacing). `divider` is
/// inserted between children that are components, loop items, text or
/// static elements; children mixing static markup with dynamic parts in one
/// root render without dividers.
#[derive(Props, Clone, PartialEq)]
pub struct StackProps {
    #[props(default)]
    pub direction: StackDirection,
    /// Gap between children, in 8px units.
    #[props(default)]
    pub spacing: f32,
    /// Element placed between each pair of children (e.g. a divider rule).
    pub divider: Option<Element>,
    /// CSS `align-items`.
    #[props(into)]
    pub align_items: Option<String>,
    /// CSS `justify-content`.
    #[props(into)]
    pub justify_content: Option<String>,
    /// CSS `flex-wrap`.
    #[props(into)]
    pub flex_wrap: Option<String>,
    /// Space children with the flexbox `gap` instead of margins (the margin
    /// approach, MUI's default, doesn't add space between wrapped lines).
    #[props(default)]
    pub use_flex_gap: bool,
    #[props(into)]
    pub component: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn Stack(props: StackProps) -> Element {
    let direction = format!("skeuo-stack--{}", props.direction.as_str());
    let class = classes([
        "skeuo-stack",
        &direction,
        if props.use_flex_gap {
            "skeuo-stack--gap"
        } else {
            "skeuo-stack--margin"
        },
        &props.class,
    ]);
    let mut style = format!("--stack-spacing:{}px;", props.spacing * 8.0);
    for (prop, value) in [
        ("align-items", &props.align_items),
        ("justify-content", &props.justify_content),
        ("flex-wrap", &props.flex_wrap),
    ] {
        if let Some(v) = value {
            style.push_str(&format!("{prop}:{v};"));
        }
    }
    style.push_str(&props.style);

    let content = match (props.divider, split_children(&props.children)) {
        (Some(divider), Some(items)) => rsx! {
            for (i , item) in items.into_iter().enumerate() {
                if i > 0 {
                    {divider.clone()}
                }
                {item}
            }
        },
        _ => props.children,
    };
    match props.component.as_deref() {
        Some("ul") => rsx! { ul { class, style, {content} } },
        Some("ol") => rsx! { ol { class, style, {content} } },
        Some("section") => rsx! { section { class, style, {content} } },
        Some("nav") => rsx! { nav { class, style, {content} } },
        _ => rsx! { div { class, style, {content} } },
    }
}

/// A template with a single dynamic root: used to render one child node of
/// another template on its own.
const SINGLE_DYNAMIC: Template = Template {
    roots: &[TemplateNode::Dynamic { id: 0 }],
    node_paths: &[&[0]],
    attr_paths: &[],
};

/// Splits `children` into one element per top-level child, so a separator
/// can be placed between them. Returns `None` when a root mixes static
/// markup with dynamic parts (those can't be split without re-indexing the
/// template).
pub(crate) fn split_children(children: &Element) -> Option<Vec<Element>> {
    let vnode = children.as_ref().ok()?;
    let template = vnode.template;
    let mut items = Vec::new();
    for (i, root) in template.roots.iter().enumerate() {
        match root {
            TemplateNode::Dynamic { id } => match &vnode.dynamic_nodes[*id] {
                DynamicNode::Fragment(list) => {
                    items.extend(list.iter().map(|n| Ok(n.clone())));
                }
                DynamicNode::Placeholder(_) => {}
                node => items.push(Ok(VNode::new(
                    None,
                    SINGLE_DYNAMIC,
                    Box::new([node.clone()]),
                    Box::new([]),
                ))),
            },
            _ => {
                let has_dynamic = template
                    .node_paths
                    .iter()
                    .chain(template.attr_paths.iter())
                    .any(|p| p.first() == Some(&(i as u8)));
                if has_dynamic {
                    return None;
                }
                let single = Template {
                    roots: &template.roots[i..=i],
                    node_paths: &[],
                    attr_paths: &[],
                };
                items.push(Ok(VNode::new(None, single, Box::new([]), Box::new([]))));
            }
        }
    }
    Some(items)
}
