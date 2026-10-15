use dioxus::prelude::*;

use crate::classes;

/// How many columns a grid item spans, mirroring the values of MUI's `size`
/// (and, for `Columns` / `Auto`, `offset`).
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum GridSize {
    /// Span this many of the container's columns.
    Columns(u16),
    /// As wide as the content (MUI `"auto"`).
    Auto,
    /// Take the remaining space (MUI `"grow"`).
    Grow,
}

impl From<u16> for GridSize {
    fn from(n: u16) -> Self {
        GridSize::Columns(n)
    }
}

impl From<i32> for GridSize {
    fn from(n: i32) -> Self {
        GridSize::Columns(n.max(0) as u16)
    }
}

/// A value per breakpoint (xs 0 · sm 600 · md 900 · lg 1200 · xl 1536 px),
/// like MUI's responsive objects `{ xs: 12, md: 6 }`. A plain value converts
/// into `xs` (all widths).
///
/// ```rust
/// # use skeuo_ui::prelude::*;
/// let size = Responsive::xs(12).md(6).lg(4);
/// # let _: Responsive<GridSize> = size;
/// ```
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Responsive<T> {
    pub xs: Option<T>,
    pub sm: Option<T>,
    pub md: Option<T>,
    pub lg: Option<T>,
    pub xl: Option<T>,
}

impl<T> Default for Responsive<T> {
    fn default() -> Self {
        Self {
            xs: None,
            sm: None,
            md: None,
            lg: None,
            xl: None,
        }
    }
}

impl<T> Responsive<T> {
    fn get(&self, bp: usize) -> Option<&T> {
        [&self.xs, &self.sm, &self.md, &self.lg, &self.xl][bp].as_ref()
    }
}

impl Responsive<GridSize> {
    pub fn xs(v: impl Into<GridSize>) -> Self {
        Self {
            xs: Some(v.into()),
            ..Self::default()
        }
    }
    pub fn sm(mut self, v: impl Into<GridSize>) -> Self {
        self.sm = Some(v.into());
        self
    }
    pub fn md(mut self, v: impl Into<GridSize>) -> Self {
        self.md = Some(v.into());
        self
    }
    pub fn lg(mut self, v: impl Into<GridSize>) -> Self {
        self.lg = Some(v.into());
        self
    }
    pub fn xl(mut self, v: impl Into<GridSize>) -> Self {
        self.xl = Some(v.into());
        self
    }
}

impl From<GridSize> for Responsive<GridSize> {
    fn from(v: GridSize) -> Self {
        Responsive::xs(v)
    }
}

impl From<u16> for Responsive<GridSize> {
    fn from(v: u16) -> Self {
        Responsive::xs(v)
    }
}

impl From<i32> for Responsive<GridSize> {
    fn from(v: i32) -> Self {
        Responsive::xs(v)
    }
}

const BREAKPOINTS: [&str; 5] = ["xs", "sm", "md", "lg", "xl"];

/// The parent container's column count and column gap, read by its items.
#[derive(Clone, Copy, PartialEq)]
struct GridCtx {
    columns: u16,
    column_gap: f32,
}

/// A responsive 12-column flexbox grid. No material. Mirrors MUI's
/// `<Grid>` (v2).
///
/// A `container` lays out its child `Grid`s with `spacing` (8px units)
/// between them; an item spans `size` of the parent's `columns` and can be
/// pushed right with `offset`. `size` and `offset` accept a number or a
/// [`Responsive`] per-breakpoint value. A grid can be both container and item.
#[derive(Props, Clone, PartialEq)]
pub struct GridProps {
    #[props(default)]
    pub container: bool,
    /// Gap between items, in 8px units.
    #[props(default)]
    pub spacing: f32,
    /// Overrides `spacing` vertically.
    pub row_spacing: Option<f32>,
    /// Overrides `spacing` horizontally.
    pub column_spacing: Option<f32>,
    /// Number of columns of a container.
    #[props(default = 12)]
    pub columns: u16,
    /// Columns spanned by this item.
    #[props(default, into)]
    pub size: Responsive<GridSize>,
    /// Columns to skip before this item (`GridSize::Auto` pushes it to the end).
    #[props(default, into)]
    pub offset: Responsive<GridSize>,
    /// CSS `flex-direction` of a container.
    #[props(into)]
    pub direction: Option<String>,
    /// CSS `align-items` of a container.
    #[props(into)]
    pub align_items: Option<String>,
    /// CSS `justify-content` of a container.
    #[props(into)]
    pub justify_content: Option<String>,
    /// CSS `flex-wrap` of a container (default `wrap`).
    #[props(into)]
    pub wrap: Option<String>,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    pub children: Element,
}

#[component]
pub fn Grid(props: GridProps) -> Element {
    // Read the parent before providing our own context.
    let parent = try_use_context::<Signal<GridCtx>>();
    let column_gap = props.column_spacing.unwrap_or(props.spacing) * 8.0;
    let row_gap = props.row_spacing.unwrap_or(props.spacing) * 8.0;
    if props.container {
        let ctx = GridCtx {
            columns: props.columns.max(1),
            column_gap,
        };
        let mut provided = use_hook(|| provide_context(Signal::new(ctx)));
        if *provided.peek() != ctx {
            provided.set(ctx);
        }
    }

    let mut class_list: Vec<String> = vec!["skeuo-grid".into()];
    let mut style = String::new();
    if props.container {
        class_list.push("skeuo-grid--container".into());
        style.push_str(&format!(
            "--grid-row-gap:{row_gap}px;--grid-column-gap:{column_gap}px;"
        ));
        for (prop, value) in [
            ("flex-direction", &props.direction),
            ("align-items", &props.align_items),
            ("justify-content", &props.justify_content),
            ("flex-wrap", &props.wrap),
        ] {
            if let Some(v) = value {
                style.push_str(&format!("{prop}:{v};"));
            }
        }
    }
    if let Some(parent) = parent.map(|s| s()) {
        style.push_str(&format!(
            "--grid-parent-columns:{};--grid-parent-gap:{}px;",
            parent.columns, parent.column_gap
        ));
    }
    for (bp, name) in BREAKPOINTS.iter().enumerate() {
        if let Some(size) = props.size.get(bp) {
            match size {
                GridSize::Columns(n) => {
                    class_list.push(format!("skeuo-grid--{name}-n"));
                    style.push_str(&format!("--grid-size-{name}:{n};"));
                }
                GridSize::Auto => class_list.push(format!("skeuo-grid--{name}-auto")),
                GridSize::Grow => class_list.push(format!("skeuo-grid--{name}-grow")),
            }
        }
        if let Some(offset) = props.offset.get(bp) {
            match offset {
                GridSize::Columns(n) => {
                    class_list.push(format!("skeuo-grid--{name}-offset"));
                    style.push_str(&format!("--grid-offset-{name}:{n};"));
                }
                GridSize::Auto | GridSize::Grow => {
                    class_list.push(format!("skeuo-grid--{name}-offset-auto"))
                }
            }
        }
    }
    style.push_str(&props.style);
    class_list.push(props.class.clone());
    let class = classes(class_list.iter().map(String::as_str));

    rsx! {
        div { class, style, {props.children} }
    }
}
