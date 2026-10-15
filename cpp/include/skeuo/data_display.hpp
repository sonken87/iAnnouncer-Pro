// skeuo-ui for C++ — Data display: Typography, Avatar, Badge, Chip, Divider, List, Table, Tooltip, ImageList.
//
// Ported from crates/skeuo-ui/src/data_display; every component emits the same
// markup and classes as its Rust counterpart, so the shared stylesheet draws it
// identically.
//
// Props are aggregates: call sites use designated initializers, which C++
// requires in declaration order (the order below).
//
// Where the Rust components share state through a Dioxus context (List →
// rows, Table → sections → rows → cells, ImageList → items), the C++
// containers rewrite the child tree they were given instead (see the notes on
// List, Table and ImageList).
#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/inputs.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ================================================================ Typography

/// Type style, mirroring MUI's Typography `variant` names.
enum class TypographyVariant {
    H1, H2, H3, H4, H5, H6,
    Subtitle1, Subtitle2,
    Body1, Body2,
    Caption, Overline,
    /// The legend style of a button.
    Button,
    /// Inherits the font of the surrounding lettering.
    Inherit,
};

inline constexpr std::array<TypographyVariant, 14> kAllTypographyVariants = {
    TypographyVariant::H1,        TypographyVariant::H2,        TypographyVariant::H3,
    TypographyVariant::H4,        TypographyVariant::H5,        TypographyVariant::H6,
    TypographyVariant::Subtitle1, TypographyVariant::Subtitle2, TypographyVariant::Body1,
    TypographyVariant::Body2,     TypographyVariant::Caption,   TypographyVariant::Overline,
    TypographyVariant::Button,    TypographyVariant::Inherit,
};

constexpr std::string_view as_str(TypographyVariant v) {
    switch (v) {
        case TypographyVariant::H1: return "h1";
        case TypographyVariant::H2: return "h2";
        case TypographyVariant::H3: return "h3";
        case TypographyVariant::H4: return "h4";
        case TypographyVariant::H5: return "h5";
        case TypographyVariant::H6: return "h6";
        case TypographyVariant::Subtitle1: return "subtitle1";
        case TypographyVariant::Subtitle2: return "subtitle2";
        case TypographyVariant::Body1: return "body1";
        case TypographyVariant::Body2: return "body2";
        case TypographyVariant::Caption: return "caption";
        case TypographyVariant::Overline: return "overline";
        case TypographyVariant::Button: return "button";
        case TypographyVariant::Inherit: return "inherit";
    }
    return "body1";
}

/// Ink of the lettering, as in MUI's Typography `color` prop
/// (`"text.primary"`, `"text.secondary"`, `"primary"`, ...). A `Color`
/// converts implicitly into a palette ink: `.color = Color::Error`.
struct TypographyColor {
    enum class Kind { TextPrimary, TextSecondary, TextDisabled, Inherit, Palette };
    Kind kind = Kind::TextPrimary;
    Color palette = Color::Primary;

    constexpr TypographyColor() = default;
    constexpr TypographyColor(Color c) : kind(Kind::Palette), palette(c) {}
    constexpr explicit TypographyColor(Kind k) : kind(k) {}

    /// `text.primary`: `--sk-ink`.
    static const TypographyColor TextPrimary;
    /// `text.secondary`: `--sk-ink-2`.
    static const TypographyColor TextSecondary;
    /// `text.disabled`: `--sk-ink-3`.
    static const TypographyColor TextDisabled;
    /// Keeps the color of the surrounding text.
    static const TypographyColor Inherit;

    friend constexpr bool operator==(const TypographyColor&, const TypographyColor&) = default;
};
inline constexpr TypographyColor TypographyColor::TextPrimary{TypographyColor::Kind::TextPrimary};
inline constexpr TypographyColor TypographyColor::TextSecondary{TypographyColor::Kind::TextSecondary};
inline constexpr TypographyColor TypographyColor::TextDisabled{TypographyColor::Kind::TextDisabled};
inline constexpr TypographyColor TypographyColor::Inherit{TypographyColor::Kind::Inherit};

/// Text alignment, as in MUI's `align` prop.
enum class TypographyAlign { Inherit, Left, Center, Right, Justify };

struct TypographyProps {
    TypographyVariant variant = TypographyVariant::Body1;
    TypographyColor color = TypographyColor::TextPrimary;
    TypographyAlign align = TypographyAlign::Inherit;
    /// Adds a bottom margin.
    bool gutter_bottom = false;
    /// Keeps the text on one line, truncated with an ellipsis.
    bool no_wrap = false;
    /// Renders a `<p>` with a paragraph bottom margin.
    bool paragraph = false;
    /// HTML element to render instead of the variant's default ("h1".."h6",
    /// "p", "span", "div", "label", "legend", "strong", "em", "small", "li",
    /// "figcaption", "blockquote", "code"; anything else renders a span).
    std::optional<std::string> component = {};
    std::optional<std::string> id = {};
    std::string cls = {};
    std::string style = {};
};

/// Text in the type scale, printed on the aluminium. Mirrors MUI's <Typography>.
Node Typography(const TypographyProps& props, Node children);

// ================================================================ Avatar

/// Avatar shape, mirroring MUI's `variant` names.
enum class AvatarVariant { Circular, Rounded, Square };

struct AvatarProps {
    /// Image URL.
    std::optional<std::string> src = {};
    std::optional<std::string> src_set = {};
    /// Alternative text; also the accessible name of a letter avatar, and its
    /// first letter is the fallback legend.
    std::optional<std::string> alt = {};
    AvatarVariant variant = AvatarVariant::Circular;
    /// Filled keycap color. Empty is the white keycap.
    std::optional<Color> color = {};
    /// small 28px · medium 40px · large 56px.
    Size size = Size::Medium;
    std::string cls = {};
    std::string style = {};
};

/// A person or thing at a glance: a small keycap with initials or an icon, or
/// a photo inset into a thin well ring. Falls back to the initials / first
/// letter of `alt` / a person glyph when the image is missing or fails to
/// load. Mirrors MUI's <Avatar>.
Node Avatar(const AvatarProps& props = {}, Node children = {});

/// Overlap between grouped avatars, as in MUI's AvatarGroup `spacing`:
/// `AvatarGroupSpacing::Medium`, `::Small` or `AvatarGroupSpacing::Px(12)`.
struct AvatarGroupSpacing {
    /// Overlap in px.
    double overlap = 8.0;

    static const AvatarGroupSpacing Medium;
    static const AvatarGroupSpacing Small;
    static constexpr AvatarGroupSpacing Px(double px) { return {px}; }
};
inline constexpr AvatarGroupSpacing AvatarGroupSpacing::Medium{8.0};
inline constexpr AvatarGroupSpacing AvatarGroupSpacing::Small{16.0};

struct AvatarGroupProps {
    /// The avatars (as in the Rust version, a list rather than children).
    std::vector<Node> avatars = {};
    /// At most this many caps are shown, the last one being the surplus.
    size_t max = 5;
    /// Overrides the count used for the surplus.
    std::optional<size_t> total = {};
    AvatarGroupSpacing spacing = AvatarGroupSpacing::Medium;
    /// Shape of the surplus avatar.
    AvatarVariant variant = AvatarVariant::Circular;
    /// Size of the surplus avatar (match it to the grouped ones).
    Size size = Size::Medium;
    std::string cls = {};
};

/// A stack of overlapping avatars with a "+N" surplus cap. Mirrors MUI's <AvatarGroup>.
Node AvatarGroup(const AvatarGroupProps& props);

// ================================================================ Badge

/// What a badge shows, as in MUI's `badgeContent`: nothing, a number (capped
/// by `max`, hidden at zero unless `show_zero`) or a short text. Converts
/// implicitly from integers and strings: `.badge_content = 4`, `= "new"`.
struct BadgeContent {
    enum class Kind { Empty, Number, Text };
    Kind kind = Kind::Empty;
    std::uint64_t number = 0;
    std::string text = {};

    BadgeContent() = default;
    BadgeContent(int n) : kind(Kind::Number), number(n < 0 ? 0 : std::uint64_t(n)) {}
    BadgeContent(long n) : kind(Kind::Number), number(n < 0 ? 0 : std::uint64_t(n)) {}
    BadgeContent(long long n) : kind(Kind::Number), number(n < 0 ? 0 : std::uint64_t(n)) {}
    BadgeContent(unsigned n) : kind(Kind::Number), number(n) {}
    BadgeContent(unsigned long n) : kind(Kind::Number), number(n) {}
    BadgeContent(unsigned long long n) : kind(Kind::Number), number(n) {}
    BadgeContent(const char* s) : kind(Kind::Text), text(s ? s : "") {}
    BadgeContent(std::string s) : kind(Kind::Text), text(std::move(s)) {}
};

/// Badge style, mirroring MUI's `variant`.
enum class BadgeVariant {
    /// An LED pill carrying a count or short text.
    Standard,
    /// A bare status LED.
    Dot,
};

enum class BadgeVertical { Top, Bottom };
enum class BadgeHorizontal { Left, Right };

/// Corner the badge sits on, as in MUI's `anchorOrigin`.
struct BadgeAnchorOrigin {
    BadgeVertical vertical = BadgeVertical::Top;
    BadgeHorizontal horizontal = BadgeHorizontal::Right;

    static const BadgeAnchorOrigin TOP_RIGHT;
    static const BadgeAnchorOrigin TOP_LEFT;
    static const BadgeAnchorOrigin BOTTOM_RIGHT;
    static const BadgeAnchorOrigin BOTTOM_LEFT;
};
inline constexpr BadgeAnchorOrigin BadgeAnchorOrigin::TOP_RIGHT{BadgeVertical::Top, BadgeHorizontal::Right};
inline constexpr BadgeAnchorOrigin BadgeAnchorOrigin::TOP_LEFT{BadgeVertical::Top, BadgeHorizontal::Left};
inline constexpr BadgeAnchorOrigin BadgeAnchorOrigin::BOTTOM_RIGHT{BadgeVertical::Bottom, BadgeHorizontal::Right};
inline constexpr BadgeAnchorOrigin BadgeAnchorOrigin::BOTTOM_LEFT{BadgeVertical::Bottom, BadgeHorizontal::Left};

/// Shape the badge is anchored to, as in MUI's `overlap`.
enum class BadgeOverlap { Rectangular, Circular };

struct BadgeProps {
    /// A number or short text (`4`, `"new"`).
    BadgeContent badge_content = {};
    /// Numbers above `max` show as `max+`.
    std::uint64_t max = 99;
    /// Show the badge when the count is 0.
    bool show_zero = false;
    /// Hide the badge (it shrinks away).
    bool invisible = false;
    BadgeVariant variant = BadgeVariant::Standard;
    Color color = Color::Primary;
    BadgeAnchorOrigin anchor_origin = {};
    BadgeOverlap overlap = BadgeOverlap::Rectangular;
    std::string cls = {};
};

/// A small count or status LED on the corner of its child. Mirrors MUI's <Badge>.
Node Badge(const BadgeProps& props, Node children);

// ================================================================ Chip

/// Chip style, mirroring MUI's `variant`.
enum class ChipVariant {
    /// A small keycap pill in the palette color.
    Filled,
    /// A white keycap pill with colored ink and edge.
    Outlined,
};

struct ChipProps {
    std::string label = {};
    ChipVariant variant = ChipVariant::Filled;
    /// `Neutral` (MUI's `default`) is a light grey / white cap.
    Color color = Color::Neutral;
    /// small 24px · medium 32px (large is treated as medium).
    Size size = Size::Medium;
    /// An Avatar shown at the start.
    std::optional<Node> avatar = {};
    /// An icon shown at the start.
    std::optional<Node> icon = {};
    /// Force the pressable look even without `onclick`.
    std::optional<bool> clickable = {};
    bool disabled = false;
    std::function<void()> onclick = {};
    /// Shows a delete button that calls this.
    std::function<void()> on_delete = {};
    /// Replaces the default delete glyph.
    std::optional<Node> delete_icon = {};
    std::string cls = {};
};

/// A compact token: a tag, a filter, a contact, drawn as a small pill-shaped
/// keycap. Mirrors MUI's <Chip>.
Node Chip(const ChipProps& props);

// ================================================================ Divider

/// How far the divider runs, mirroring MUI's `variant`.
enum class DividerVariant {
    /// Edge to edge.
    FullWidth,
    /// Indented at the start (to line up with list text after an icon/avatar).
    Inset,
    /// Indented on both sides.
    Middle,
};

/// Where the text of a divider with children sits.
enum class DividerTextAlign { Left, Center, Right };

struct DividerProps {
    Orientation orientation = Orientation::Horizontal;
    DividerVariant variant = DividerVariant::FullWidth;
    /// Vertical divider inside a flex container: stretch to its height.
    bool flex_item = false;
    DividerTextAlign text_align = DividerTextAlign::Center;
    std::string cls = {};
};

/// The engraved hairline cut into the aluminium; with children the cut is
/// interrupted by printed lettering. Mirrors MUI's <Divider>.
Node Divider(const DividerProps& props = {}, Node children = {});

// ================================================================ List
//
// `List` applies its `dense` setting to the ListItem / ListItemButton rows in
// its children (Rust: ListContext), stopping at nested lists.

struct ListProps {
    /// Compact rows for every item.
    bool dense = false;
    /// Drop the top/bottom padding.
    bool disable_padding = false;
    /// Rendered first, usually a ListSubheader.
    std::optional<Node> subheader = {};
    /// Palette color of selected rows.
    Color color = Color::Primary;
    std::optional<std::string> aria_label = {};
    std::string cls = {};
    std::string style = {};
};

/// A vertical list of rows. Mirrors MUI's <List>.
Node List(const ListProps& props, Node children);

struct ListItemProps {
    bool dense = false;
    bool disable_gutters = false;
    bool disable_padding = false;
    /// Engraved rule under the row.
    bool divider = false;
    /// Align the icon/avatar to the top (multi-line rows).
    bool align_items_start = false;
    /// Placed at the end of the row, e.g. an icon button or a switch.
    std::optional<Node> secondary_action = {};
    std::string cls = {};
};

/// A row of a List. Wrap a ListItemButton in `ListItem({.disable_padding = true}, ...)`
/// for a clickable row. Mirrors MUI's <ListItem>.
Node ListItem(const ListItemProps& props, Node children);

struct ListItemButtonProps {
    bool selected = false;
    bool disabled = false;
    bool dense = false;
    bool disable_gutters = false;
    /// Engraved rule under the row.
    bool divider = false;
    bool autofocus = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A pressable row (a native <button>; `selected` sets aria-current). Mirrors MUI's <ListItemButton>.
Node ListItemButton(const ListItemButtonProps& props, Node children);

struct ListItemIconProps {
    std::string cls = {};
};

/// The leading icon of a row, in secondary ink. Mirrors MUI's <ListItemIcon>.
Node ListItemIcon(const ListItemIconProps& props, Node children);

struct ListItemAvatarProps {
    std::string cls = {};
};

/// The leading avatar of a row. Mirrors MUI's <ListItemAvatar>.
Node ListItemAvatar(const ListItemAvatarProps& props, Node children);

struct ListItemTextProps {
    std::optional<std::string> primary = {};
    std::optional<std::string> secondary = {};
    /// Indent as if there were a leading icon.
    bool inset = false;
    std::string cls = {};
};

/// Primary and secondary lettering of a row; children, if given, replace
/// `primary`. Mirrors MUI's <ListItemText>.
Node ListItemText(const ListItemTextProps& props, Node children = {});

struct ListSubheaderProps {
    /// Palette ink for the lettering; empty is the secondary ink.
    std::optional<Color> color = {};
    bool disable_sticky = false;
    bool disable_gutters = false;
    bool inset = false;
    std::optional<std::string> id = {};
    std::string cls = {};
};

/// A sticky section title inside a list. Mirrors MUI's <ListSubheader>.
Node ListSubheader(const ListSubheaderProps& props, Node children);

// ================================================================ Table
//
// Rust shares the table's size / padding / sticky header and the section
// (head, body, footer) with its cells through contexts. In C++ a TableCell
// renders for a body cell of a default table and remembers its own explicit
// props in a `data-sk-cell` attribute; TableHead / TableFooter turn their
// rows and cells into head / footer ones, and Table applies its settings and
// removes the helper attribute. A TableCell used outside a Table keeps it.

/// Cell padding, as in MUI's `padding`.
enum class TableCellPadding {
    Normal,
    /// Tight cell for a leading checkbox.
    Checkbox,
    None,
};

struct TableContainerProps {
    /// Chassis elevation 0–4.
    int elevation = 1;
    /// CSS max-height, e.g. "320px", for a scrolling (sticky header) table.
    std::optional<std::string> max_height = {};
    std::string cls = {};
    std::string style = {};
};

/// The aluminium plate a table sits on (scrolls its table when it overflows).
/// Mirrors MUI's <TableContainer component={Paper}>.
Node TableContainer(const TableContainerProps& props, Node children);

struct TableProps {
    /// `Small` gives compact rows; `Large` is treated as medium.
    Size size = Size::Medium;
    /// Keep the head row visible while the container scrolls.
    bool sticky_header = false;
    /// Default padding of every cell.
    TableCellPadding padding = TableCellPadding::Normal;
    /// Palette color of selected rows.
    Color color = Color::Primary;
    std::optional<std::string> aria_label = {};
    std::string cls = {};
};

/// A data table. Mirrors MUI's <Table>.
Node Table(const TableProps& props, Node children);

struct TableHeadProps {
    std::string cls = {};
};
struct TableBodyProps {
    std::string cls = {};
};
struct TableFooterProps {
    std::string cls = {};
};

/// The head rows of a table; its cells are <th scope="col">. Mirrors MUI's <TableHead>.
Node TableHead(const TableHeadProps& props, Node children);
/// The body rows of a table. Mirrors MUI's <TableBody>.
Node TableBody(const TableBodyProps& props, Node children);
/// The footer rows of a table (totals, pagination). Mirrors MUI's <TableFooter>.
Node TableFooter(const TableFooterProps& props, Node children);

struct TableRowProps {
    /// Wash the row on hover.
    bool hover = false;
    /// Tint the row and set aria-selected.
    bool selected = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A table row. Mirrors MUI's <TableRow>.
Node TableRow(const TableRowProps& props, Node children);

/// Text alignment of a cell, as in MUI's `align`.
enum class TableCellAlign {
    Inherit,
    Left,
    Center,
    /// Use for numbers.
    Right,
    Justify,
};

/// Cell style, as in MUI's `variant`.
enum class TableCellVariant { Head, Body, Footer };

/// Sort order, as in MUI's `sortDirection` / `direction`.
enum class SortDirection { Asc, Desc };

/// The other direction.
constexpr SortDirection toggled(SortDirection d) {
    return d == SortDirection::Asc ? SortDirection::Desc : SortDirection::Asc;
}

struct TableCellProps {
    TableCellAlign align = TableCellAlign::Inherit;
    /// Overrides the table's padding.
    std::optional<TableCellPadding> padding = {};
    /// Overrides the style implied by the section.
    std::optional<TableCellVariant> variant = {};
    /// Overrides the table's size.
    std::optional<Size> size = {};
    /// Sets aria-sort on the head cell of the sorted column.
    std::optional<SortDirection> sort_direction = {};
    /// HTML `scope`; head cells default to "col" (in the head) or "row".
    std::optional<std::string> scope = {};
    std::optional<unsigned> col_span = {};
    std::optional<unsigned> row_span = {};
    std::string cls = {};
    std::string style = {};
};

/// A table cell: <th> in the head (or with `variant = Head`), <td> otherwise.
/// Mirrors MUI's <TableCell>.
Node TableCell(const TableCellProps& props = {}, Node children = {});

struct TableSortLabelProps {
    bool active = false;
    SortDirection direction = SortDirection::Asc;
    bool hide_sort_icon = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A sortable column heading: lettering plus a direction arrow. Mirrors MUI's <TableSortLabel>.
Node TableSortLabel(const TableSortLabelProps& props, Node children);

// ================================================================ Tooltip

struct TooltipProps {
    /// The tooltip text. An empty title disables the tooltip, as in MUI.
    std::string title = {};
    /// MUI's default is below the child.
    Placement placement = Placement::Bottom;
    /// Draw a small arrow pointing at the child.
    bool arrow = false;
    /// Controlled visibility. Empty shows it on hover and focus (pure CSS).
    std::optional<bool> open = {};
    /// The bubble ignores the pointer (hovering it does not keep it open).
    bool disable_interactive = false;
    bool disable_hover_listener = false;
    bool disable_focus_listener = false;
    /// Called when the pointer enters or the child gains focus.
    std::function<void()> on_open = {};
    /// Called when the pointer leaves, focus leaves, or Escape is pressed.
    std::function<void()> on_close = {};
    /// Id of the role="tooltip" element (generated when omitted).
    std::optional<std::string> id = {};
    std::string cls = {};
};

/// A short label that floats next to its child on hover and keyboard focus
/// (CSS :hover / :focus-visible); Escape dismisses it until the pointer or
/// focus leaves. Mirrors MUI's <Tooltip>.
Node Tooltip(const TooltipProps& props, Node children);

// ================================================================ ImageList
//
// A masonry ImageList removes the grid spans its ImageListItems render
// (Rust: ImageListContext), stopping at nested image lists.

/// Layout of an image list, mirroring MUI's `variant`.
enum class ImageListVariant {
    /// A regular grid.
    Standard,
    /// A grid where items span several `cols` / `rows`.
    Quilted,
    /// Columns of images with their natural heights.
    Masonry,
    /// A grid where every other item is shorter, centered in its row.
    Woven,
};

struct ImageListProps {
    ImageListVariant variant = ImageListVariant::Standard;
    /// Number of columns.
    unsigned cols = 2;
    /// Space between images in px.
    unsigned gap = 4;
    /// Height of one grid row in px; empty sizes rows to the images
    /// (MUI's "auto"). Ignored by Masonry.
    std::optional<unsigned> row_height = {};
    std::optional<std::string> aria_label = {};
    std::string cls = {};
    std::string style = {};
};

/// A grid of images, each inset into a well. Mirrors MUI's <ImageList>.
Node ImageList(const ImageListProps& props, Node children);

struct ImageListItemProps {
    /// Columns spanned (quilted / standard grids).
    unsigned cols = 1;
    /// Rows spanned (quilted / standard grids).
    unsigned rows = 1;
    std::string cls = {};
    std::string style = {};
};

/// One image of an ImageList: put an <img> (and optionally an
/// ImageListItemBar) inside. Mirrors MUI's <ImageListItem>.
Node ImageListItem(const ImageListItemProps& props, Node children);

/// Where the caption bar of an image sits, as in MUI's `position`.
enum class ImageListItemBarPosition {
    /// Over the bottom of the image.
    Bottom,
    /// Over the top of the image.
    Top,
    /// Printed on the aluminium under the image.
    Below,
};

struct ImageListItemBarProps {
    std::optional<std::string> title = {};
    std::optional<std::string> subtitle = {};
    /// E.g. an icon button.
    std::optional<Node> action_icon = {};
    /// Put the action before the title.
    bool action_position_left = false;
    ImageListItemBarPosition position = ImageListItemBarPosition::Bottom;
    std::string cls = {};
};

/// Title, subtitle and an action for an image. Mirrors MUI's <ImageListItemBar>.
Node ImageListItemBar(const ImageListItemBarProps& props);

}  // namespace skeuo
