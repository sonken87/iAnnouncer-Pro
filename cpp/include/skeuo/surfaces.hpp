// skeuo-ui for C++ — Surfaces and layout: Paper, Card, Accordion, AppBar, Toolbar, Drawer, SkeuoBox, Container, Stack, Grid.
//
// Surfaces are plates of aluminium (`.skeuo-chassis`); the layout pieces
// (SkeuoBox, Container, Stack, Grid) are invisible structure with no material
// of their own.
//
// Props are aggregates used with C++20 designated initializers, which must
// follow the declaration order below:
//
//   Card({.variant = PaperVariant::Outlined}, {
//       CardContent({}, "Body"),
//       CardActions({.disable_spacing = true}, Button({}, "OK")),
//   })
//
// Containers that configure their children (Accordion, Grid, Stack) do it by
// rewriting the child tree they are given, since C++ builds children before
// the parent runs (there is no top-down context as in the Rust version).
#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/inputs.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ------------------------------------------------------------------ Paper

/// Paper style, mirroring MUI's `variant` names.
enum class PaperVariant {
    Elevation,  // a raised plate whose cast shadow follows `elevation`
    Outlined,   // a flush plate (no cast shadow) with a crisp machined edge
};

constexpr std::string_view as_str(PaperVariant v) {
    return v == PaperVariant::Outlined ? "outlined" : "elevation";
}

/// Maps MUI's elevation scale (0–24) onto the five chassis levels of
/// DESIGN.md: 0 → 0, 1–2 → 1, 3–6 → 2, 7–12 → 3, 13–24 → 4.
constexpr int chassis_level(int elevation) {
    return elevation <= 0 ? 0 : elevation <= 2 ? 1 : elevation <= 6 ? 2 : elevation <= 12 ? 3 : 4;
}

/// The material classes of a chassis plate (shared by every surface).
std::string chassis_classes(PaperVariant variant, int elevation, bool square);

struct PaperProps {
    /// MUI elevation, 0–24 (mapped onto chassis levels 0–4).
    int elevation = 1;
    PaperVariant variant = PaperVariant::Elevation;
    /// Square corners.
    bool square = false;
    std::string cls = {};
    /// Passed through to the <div> (`id`, `style`, `role`, `aria-*`, ...).
    Attrs attrs = {};
};

/// A plate of bead-blasted aluminium. Mirrors MUI's <Paper>.
Node Paper(const PaperProps& props, Node children);

// ------------------------------------------------------------------ Card

struct CardProps {
    /// Lift the card to a raised panel (chassis level 2).
    bool raised = false;
    PaperVariant variant = PaperVariant::Elevation;
    /// MUI elevation, 0–24. Ignored when `raised`.
    int elevation = 1;
    bool square = false;
    std::string cls = {};
    Attrs attrs = {};
};

/// A resting aluminium plate that holds one subject. Mirrors MUI's <Card>.
Node Card(const CardProps& props, Node children);

struct CardHeaderProps {
    /// Element before the title, e.g. an avatar.
    std::optional<Node> avatar = {};
    std::optional<std::string> title = {};
    std::optional<std::string> subheader = {};
    /// Element at the end of the row, e.g. an icon button.
    std::optional<Node> action = {};
    std::string cls = {};
};

/// The heading row of a card. Mirrors MUI's <CardHeader>.
Node CardHeader(const CardHeaderProps& props);

struct CardMediaProps {
    /// Image URL (background image, or `src` when `component` is "img").
    std::optional<std::string> image = {};
    /// "img" renders an <img>; otherwise a <div> with a cover background.
    std::optional<std::string> component = {};
    /// Alt text for `component: "img"`.
    std::optional<std::string> alt = {};
    /// Accessible name of a background-image media block.
    std::optional<std::string> title = {};
    /// CSS height, e.g. "140px".
    std::optional<std::string> height = {};
    std::string cls = {};
    std::string style = {};
};

/// A full-bleed picture set into the card. Mirrors MUI's <CardMedia>.
/// Children are laid over the media area (ignored for an <img>).
Node CardMedia(const CardMediaProps& props, Node children = {});

struct CardContentProps {
    std::string cls = {};
};

/// The padded body of a card. Mirrors MUI's <CardContent>.
Node CardContent(const CardContentProps& props, Node children);

struct CardActionsProps {
    /// Remove the gap between the actions.
    bool disable_spacing = false;
    std::string cls = {};
};

/// A row of keycaps along the bottom of a card. Mirrors MUI's <CardActions>.
Node CardActions(const CardActionsProps& props, Node children);

struct CardActionAreaProps {
    bool disabled = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// Makes the card's content one large key. Mirrors MUI's <CardActionArea>.
Node CardActionArea(const CardActionAreaProps& props, Node children);

// ------------------------------------------------------------------ Accordion

struct AccordionProps {
    /// Controlled expanded state (pair with `onchange`).
    std::optional<bool> expanded = {};
    /// Initial state when uncontrolled (read once).
    bool default_expanded = false;
    /// Called with the requested new expanded state.
    std::function<void(bool)> onchange = {};
    bool disabled = false;
    /// Keep the same spacing and corners when expanded.
    bool disable_gutters = false;
    /// Square corners.
    bool square = false;
    /// MUI elevation, 0–24.
    int elevation = 1;
    PaperVariant variant = PaperVariant::Elevation;
    std::string cls = {};
};

/// One section of a stack of aluminium plates that opens to reveal its
/// details. Mirrors MUI's <Accordion>. Put an AccordionSummary first, then
/// AccordionDetails and optionally AccordionActions; the Accordion wires them
/// (ids, aria-expanded, open state, the toggle handler).
Node Accordion(const AccordionProps& props, Node children);

struct AccordionSummaryProps {
    /// Icon at the end of the row, rotated 180° when expanded.
    std::optional<Node> expand_icon = {};
    std::string cls = {};
};

/// The always-visible header of an accordion. Mirrors MUI's <AccordionSummary>.
Node AccordionSummary(const AccordionSummaryProps& props, Node children);

struct AccordionDetailsProps {
    std::string cls = {};
};

/// The collapsible body of an accordion. Mirrors MUI's <AccordionDetails>.
Node AccordionDetails(const AccordionDetailsProps& props, Node children);

struct AccordionActionsProps {
    bool disable_spacing = false;
    std::string cls = {};
};

/// A right-aligned row of keycaps at the bottom of an expanded accordion.
/// Mirrors MUI's <AccordionActions>.
Node AccordionActions(const AccordionActionsProps& props, Node children);

// ------------------------------------------------------------------ AppBar / Toolbar

/// CSS positioning of an app bar, mirroring MUI's `position` prop.
enum class AppBarPosition { Fixed, Absolute, Sticky, Static, Relative };

constexpr std::string_view as_str(AppBarPosition p) {
    switch (p) {
        case AppBarPosition::Fixed: return "fixed";
        case AppBarPosition::Absolute: return "absolute";
        case AppBarPosition::Sticky: return "sticky";
        case AppBarPosition::Static: return "static";
        case AppBarPosition::Relative: return "relative";
    }
    return "fixed";
}

/// App bar color, mirroring MUI's `color` prop.
enum class AppBarColor {
    Primary,  // a matte plate in the palette color
    Secondary,
    Success,
    Error,
    Warning,
    Info,
    Default,      // plain aluminium chassis
    Inherit,      // aluminium chassis inheriting the surrounding text color
    Transparent,  // no plate at all
};

constexpr std::string_view as_str(AppBarColor c) {
    switch (c) {
        case AppBarColor::Primary: return "primary";
        case AppBarColor::Secondary: return "secondary";
        case AppBarColor::Success: return "success";
        case AppBarColor::Error: return "error";
        case AppBarColor::Warning: return "warning";
        case AppBarColor::Info: return "info";
        case AppBarColor::Default: return "default";
        case AppBarColor::Inherit: return "inherit";
        case AppBarColor::Transparent: return "transparent";
    }
    return "primary";
}

/// The `color` prop of an AppBar: an AppBarColor or a palette Color
/// (`Color::Neutral` is the aluminium `Default`), like Rust's `Into`.
struct AppBarColorProp {
    AppBarColor value = AppBarColor::Primary;
    constexpr AppBarColorProp(AppBarColor c) : value(c) {}
    constexpr AppBarColorProp(Color c)
        : value(c == Color::Primary     ? AppBarColor::Primary
                : c == Color::Secondary ? AppBarColor::Secondary
                : c == Color::Success   ? AppBarColor::Success
                : c == Color::Error     ? AppBarColor::Error
                : c == Color::Warning   ? AppBarColor::Warning
                : c == Color::Info      ? AppBarColor::Info
                                        : AppBarColor::Default) {}
};

struct AppBarProps {
    AppBarPosition position = AppBarPosition::Fixed;
    AppBarColorProp color = AppBarColor::Primary;
    /// MUI elevation, 0–24 (default 4 → chassis level 2).
    int elevation = 4;
    /// Square corners (default). Set false for a rounded floating bar.
    bool square = true;
    std::string cls = {};
    Attrs attrs = {};
};

/// The bar across the top of a window. Mirrors MUI's <AppBar> (a <header>).
Node AppBar(const AppBarProps& props, Node children);

/// Toolbar density, mirroring MUI's `variant` prop.
enum class ToolbarVariant { Regular, Dense };

struct ToolbarProps {
    ToolbarVariant variant = ToolbarVariant::Regular;
    /// Remove the side padding.
    bool disable_gutters = false;
    std::string cls = {};
    Attrs attrs = {};
};

/// A horizontal row of controls and lettering. Mirrors MUI's <Toolbar>.
Node Toolbar(const ToolbarProps& props, Node children);

// ------------------------------------------------------------------ Drawer

/// The window edge a drawer is glued to, mirroring MUI's `anchor` prop.
enum class DrawerAnchor { Left, Right, Top, Bottom };

constexpr std::string_view as_str(DrawerAnchor a) {
    switch (a) {
        case DrawerAnchor::Left: return "left";
        case DrawerAnchor::Right: return "right";
        case DrawerAnchor::Top: return "top";
        case DrawerAnchor::Bottom: return "bottom";
    }
    return "left";
}

/// How a drawer behaves, mirroring MUI's `variant` prop.
enum class DrawerVariant {
    Temporary,   // a modal panel over a scrim; closes on Escape / scrim click
    Persistent,  // docked in the layout; `open` slides it in and out
    Permanent,   // always docked in the layout
};

constexpr std::string_view as_str(DrawerVariant v) {
    switch (v) {
        case DrawerVariant::Temporary: return "temporary";
        case DrawerVariant::Persistent: return "persistent";
        case DrawerVariant::Permanent: return "permanent";
    }
    return "temporary";
}

struct DrawerProps {
    DrawerAnchor anchor = DrawerAnchor::Left;
    DrawerVariant variant = DrawerVariant::Temporary;
    bool open = false;
    /// Called on Escape or a scrim click (temporary drawers).
    std::function<void()> onclose = {};
    /// Width (left/right) or height (top/bottom) of the panel, as CSS.
    std::optional<std::string> size = {};
    /// MUI elevation of a temporary drawer, 0–24 (default 16 → level 4).
    int elevation = 16;
    /// Don't render the scrim behind a temporary drawer.
    bool hide_backdrop = false;
    /// Rust's `inline` (a C++ keyword): preview mode, positioned against the
    /// nearest positioned ancestor instead of the viewport, no focus grab.
    bool inline_ = false;
    std::string cls = {};
    /// Passed through to the panel element (e.g. `aria-label`).
    Attrs attrs = {};
};

/// A panel glued to one edge of the window. Mirrors MUI's <Drawer>.
Node Drawer(const DrawerProps& props, Node children);

// ------------------------------------------------------------------ SkeuoBox / Container / Stack

struct SkeuoBoxProps {
    /// HTML tag: div (default), span, section, article, main, header, footer,
    /// nav, aside, ul, ol, li, form or p.
    std::optional<std::string> component = {};
    std::string cls = {};
    Attrs attrs = {};
};

/// A plain layout wrapper with no material. Mirrors MUI's <Box>.
Node SkeuoBox(const SkeuoBoxProps& props, Node children);

/// Maximum width of a Container (xs 444 · sm 600 · md 900 · lg 1200 · xl 1536 px).
enum class ContainerMaxWidth {
    Xs,
    Sm,
    Md,
    Lg,
    Xl,
    False,  // no maximum (MUI `maxWidth={false}`)
};

constexpr std::string_view as_str(ContainerMaxWidth m) {
    switch (m) {
        case ContainerMaxWidth::Xs: return "xs";
        case ContainerMaxWidth::Sm: return "sm";
        case ContainerMaxWidth::Md: return "md";
        case ContainerMaxWidth::Lg: return "lg";
        case ContainerMaxWidth::Xl: return "xl";
        case ContainerMaxWidth::False: return "false";
    }
    return "lg";
}

struct ContainerProps {
    ContainerMaxWidth max_width = ContainerMaxWidth::Lg;
    /// Snap the max width to the current breakpoint's minimum width.
    bool fixed = false;
    /// Remove the side padding.
    bool disable_gutters = false;
    std::string cls = {};
    Attrs attrs = {};
};

/// Centers content horizontally with a maximum width. Mirrors MUI's <Container>.
Node Container(const ContainerProps& props, Node children);

/// Direction of a Stack, mirroring MUI's `direction` prop.
enum class StackDirection { Row, RowReverse, Column, ColumnReverse };

constexpr std::string_view as_str(StackDirection d) {
    switch (d) {
        case StackDirection::Row: return "row";
        case StackDirection::RowReverse: return "row-reverse";
        case StackDirection::Column: return "column";
        case StackDirection::ColumnReverse: return "column-reverse";
    }
    return "column";
}

struct StackProps {
    StackDirection direction = StackDirection::Column;
    /// Gap between children, in 8px units.
    double spacing = 0;
    /// Element placed between each pair of children (e.g. a divider rule).
    std::optional<Node> divider = {};
    /// CSS `align-items`.
    std::optional<std::string> align_items = {};
    /// CSS `justify-content`.
    std::optional<std::string> justify_content = {};
    /// CSS `flex-wrap`.
    std::optional<std::string> flex_wrap = {};
    /// Space children with the flexbox `gap` instead of margins.
    bool use_flex_gap = false;
    /// "ul", "ol", "section" or "nav"; a <div> otherwise.
    std::optional<std::string> component = {};
    std::string cls = {};
    std::string style = {};
};

/// Lays its children out in a row or column with even spacing, optionally
/// with a divider between them. Mirrors MUI's <Stack>. The children are the
/// top-level nodes of `children` (a fragment) — each gets a divider between.
Node Stack(const StackProps& props, Node children);

// ------------------------------------------------------------------ Grid

/// How many columns a grid item spans (MUI's `size` / `offset` values):
/// a number, `GridSize::Auto` (as wide as the content) or `GridSize::Grow`
/// (the remaining space).
struct GridSize {
    enum Keyword { Auto, Grow };
    enum class Kind { Columns, Auto, Grow };

    Kind kind = Kind::Columns;
    int columns = 0;

    constexpr GridSize(int n) : kind(Kind::Columns), columns(n < 0 ? 0 : n) {}
    constexpr GridSize(Keyword k) : kind(k == Auto ? Kind::Auto : Kind::Grow) {}
    friend constexpr bool operator==(const GridSize&, const GridSize&) = default;
};

/// A GridSize per breakpoint (xs 0 · sm 600 · md 900 · lg 1200 · xl 1536 px),
/// like MUI's `{ xs: 12, md: 6 }`. A plain value applies from `xs` up:
/// `.size = 6`, `.size = GridSize::Grow`, `.size = Responsive::xs(12).md(6)`.
/// (The Rust version is generic, `Responsive<GridSize>`.)
struct Responsive {
    /// Values for xs, sm, md, lg, xl.
    std::array<std::optional<GridSize>, 5> values = {};

    Responsive() = default;
    Responsive(GridSize v) { values[0] = v; }
    Responsive(int n) { values[0] = GridSize(n); }
    Responsive(GridSize::Keyword k) { values[0] = GridSize(k); }

    static Responsive xs(GridSize v) { return Responsive(v); }
    Responsive sm(GridSize v) const { Responsive r = *this; r.values[1] = v; return r; }
    Responsive md(GridSize v) const { Responsive r = *this; r.values[2] = v; return r; }
    Responsive lg(GridSize v) const { Responsive r = *this; r.values[3] = v; return r; }
    Responsive xl(GridSize v) const { Responsive r = *this; r.values[4] = v; return r; }
};

struct GridProps {
    bool container = false;
    /// Gap between items, in 8px units.
    double spacing = 0;
    /// Overrides `spacing` vertically.
    std::optional<double> row_spacing = {};
    /// Overrides `spacing` horizontally.
    std::optional<double> column_spacing = {};
    /// Number of columns of a container.
    int columns = 12;
    /// Columns spanned by this item.
    Responsive size = {};
    /// Columns to skip before this item (`GridSize::Auto` pushes it to the end).
    Responsive offset = {};
    /// CSS `flex-direction` of a container.
    std::optional<std::string> direction = {};
    /// CSS `align-items` of a container.
    std::optional<std::string> align_items = {};
    /// CSS `justify-content` of a container.
    std::optional<std::string> justify_content = {};
    /// CSS `flex-wrap` of a container (default `wrap`).
    std::optional<std::string> wrap = {};
    std::string cls = {};
    std::string style = {};
};

/// A responsive 12-column flexbox grid. Mirrors MUI's <Grid> (v2). A
/// container passes its columns and column gap to the Grid items in its
/// subtree (up to the next nested container).
Node Grid(const GridProps& props, Node children);

// ------------------------------------------------------------------ ScrollArea

/// Which way a ScrollArea scrolls.
enum class ScrollDirection { Vertical, Horizontal, Both };

/// What a ScrollArea is made of.
enum class ScrollAreaVariant {
    Plain,  // no material: the content scrolls in place
    Well,   // a well; content slides under its lip, with edge shades
};

struct ScrollAreaProps {
    ScrollDirection direction = ScrollDirection::Vertical;
    ScrollAreaVariant variant = ScrollAreaVariant::Plain;
    /// CSS height, e.g. "240px" (fixed height).
    std::optional<std::string> height = {};
    /// CSS max-height, e.g. "40vh" (grows with content up to this).
    std::optional<std::string> max_height = {};
    /// CSS max-width, for horizontal scrolling.
    std::optional<std::string> max_width = {};
    /// Accessible name; makes the region keyboard-focusable.
    std::optional<std::string> aria_label = {};
    std::string cls = {};
    std::string style = {};
};

/// A region with its own scrollbar (a groove with a keycap thumb). Not part
/// of MUI. Every scrollbar in the theme is styled the same way; this adds the
/// sizing, the well and the edge shades.
Node ScrollArea(const ScrollAreaProps& props, Node children);

}  // namespace skeuo
