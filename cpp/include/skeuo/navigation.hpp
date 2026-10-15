// skeuo-ui for C++ — Navigation: Tabs, Breadcrumbs, Link, Menu, Pagination, Stepper, BottomNavigation, SpeedDial, Popover, Timeline.
//
// Containers configure the children they are given (C++ evaluates children
// before the parent, so there is no top-down context as in Dioxus): `Tabs`
// marks the selected `Tab` and wires its clicks and arrow keys, `TabContext`
// links tabs and panels, `Stepper` numbers its `Step`s in order and fills in
// their labels, `MenuList` wires its `MenuItem`s, `BottomNavigation` and
// `SpeedDial` style their actions.
//
// Props are aggregates; use designated initializers in declaration order.
#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/inputs.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ------------------------------------------------------------------ Tabs

/// How the tabs share the available width, mirroring MUI's `variant`.
enum class TabsVariant {
    Standard,    // the tray hugs its tabs
    Scrollable,  // the tray scrolls when the tabs overflow
    FullWidth,   // the tray spans the container; tabs share the width
};

/// Where a tab's icon sits relative to its label (MUI `iconPosition`).
enum class IconPosition { Top, Bottom, Start, End };

struct TabContextProps {
    /// The value of the selected tab.
    std::string value;
    /// Prefix of the tab and panel ids. Generated when omitted.
    std::optional<std::string> id = {};
};

/// Holds the selected value for a `Tabs` + `TabPanel` group: links tabs and
/// panels by id and shows the matching panel. Mirrors MUI lab's
/// `<TabContext>`; renders no element of its own.
Node TabContext(const TabContextProps& props, Node children);

struct TabsProps {
    /// Value of the selected `Tab` (controlled, pair with `onchange`).
    std::optional<std::string> value = {};
    /// Uncontrolled initial value, read once.
    std::optional<std::string> default_value = {};
    std::function<void(const std::string&)> onchange = {};
    TabsVariant variant = TabsVariant::Standard;
    Orientation orientation = Orientation::Horizontal;
    /// Center the tray in its container.
    bool centered = false;
    /// Ink of the selected tab's legend; neutral prints it black.
    Color color = Color::Neutral;
    Size size = Size::Medium;
    /// Select a tab as soon as the arrow keys focus it.
    bool selection_follows_focus = false;
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    std::string cls = {};
};

/// A segmented control: a shallow well (the tray) holding the tabs, the
/// selected tab a raised white keycap. Mirrors MUI's `<Tabs>`. Arrow keys
/// move focus between tabs (Home/End jump to the ends).
Node Tabs(const TabsProps& props, Node children);

struct TabProps {
    /// Identifies the tab; compared with the `value` of `Tabs`.
    std::string value;
    std::optional<std::string> label = {};
    std::optional<Node> icon = {};
    IconPosition icon_position = IconPosition::Top;
    bool disabled = false;
    /// Let a long label wrap onto two lines.
    bool wrapped = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// One segment of `Tabs`. Mirrors MUI's `<Tab>`. Must be a child of `Tabs`.
Node Tab(const TabProps& props, Node children = {});

struct TabPanelProps {
    /// The value of the `Tab` this panel belongs to.
    std::string value;
    /// Keep the children in the DOM (hidden) while the panel is not shown.
    bool keep_mounted = false;
    std::string cls = {};
};

/// The content shown for one tab. Must sit inside a `TabContext`. Mirrors
/// MUI lab's `<TabPanel>`.
Node TabPanel(const TabPanelProps& props, Node children);

// ------------------------------------------------------------ Breadcrumbs

struct BreadcrumbsProps {
    /// The crumbs (usually `Link`s, plain text for the current page).
    std::vector<Node> items = {};
    /// Printed between items; a chevron by default.
    std::optional<Node> separator = {};
    /// Collapse the path when it has more items than this.
    std::size_t max_items = 8;
    std::size_t items_before_collapse = 1;
    std::size_t items_after_collapse = 1;
    /// Accessible name of the "…" keycap.
    std::string expand_text = "Show path";
    std::string aria_label = "breadcrumb";
    std::string cls = {};
};

/// A path of printed links; the middle collapses into a "…" keycap that
/// expands the path. Mirrors MUI's `<Breadcrumbs>`.
Node Breadcrumbs(const BreadcrumbsProps& props);

// ------------------------------------------------------------------- Link

enum class LinkUnderline { None, Hover, Always };

struct LinkProps {
    /// Without `href` the link is a `<button>` (MUI's `component="button"`).
    std::optional<std::string> href = {};
    LinkUnderline underline = LinkUnderline::Always;
    Color color = Color::Primary;
    std::optional<std::string> target = {};
    std::optional<std::string> rel = {};
    /// Marks the link as the current page (`aria-current="page"`).
    bool current = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A hyperlink printed in the palette ink. Mirrors MUI's `<Link>`.
Node Link(const LinkProps& props, Node children);

// -------------------------------------------------------- Popover & Menu

struct PopoverProps {
    bool open = false;
    /// Called on a click outside the surface or Escape.
    std::function<void()> onclose = {};
    /// The element the surface is attached to.
    std::optional<Node> anchor = {};
    Placement placement = Placement::Bottom;
    /// Show the surface statically in the page flow (preview, no backdrop).
    /// (Rust: `inline`, a C++ keyword.)
    bool inline_ = false;
    /// Move focus into the surface when it opens.
    bool auto_focus = true;
    std::optional<std::string> id = {};
    std::optional<std::string> aria_label = {};
    std::optional<std::string> aria_labelledby = {};
    /// Classes for the floating surface.
    std::string cls = {};
};

/// A floating aluminium plate next to its anchor. Mirrors MUI's `<Popover>`.
Node Popover(const PopoverProps& props, Node children);

struct MenuProps {
    bool open = false;
    /// Called on Escape or a click outside.
    std::function<void()> onclose = {};
    /// The trigger the menu is attached to.
    std::optional<Node> anchor = {};
    Placement placement = Placement::Bottom;
    /// Compact rows.
    bool dense = false;
    /// Show the menu in the page flow (preview).
    bool inline_ = false;
    /// Id of the menu list (point the trigger's `aria-controls` at it).
    std::optional<std::string> id = {};
    /// Id of the trigger that labels the menu.
    std::optional<std::string> aria_labelledby = {};
    std::string cls = {};
};

/// A pop-up menu: a `Popover` holding a `MenuList`. Focus moves to the
/// first item on open; arrows / Home / End move, Enter / Space activate,
/// Escape or a click outside calls `onclose`. Mirrors MUI's `<Menu>`.
Node Menu(const MenuProps& props, Node children);

struct MenuListProps {
    bool dense = false;
    /// Focus the first enabled item when it appears.
    bool auto_focus_item = false;
    std::optional<std::string> id = {};
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_label = {};
    std::string cls = {};
};

/// The list of `MenuItem`s (`role="menu"`) with arrow-key navigation.
/// Mirrors MUI's `<MenuList>`.
Node MenuList(const MenuListProps& props, Node children);

struct MenuItemProps {
    bool selected = false;
    bool disabled = false;
    /// Engrave a rule below this item.
    bool divider = false;
    /// Compact row; defaults to the list's `dense`.
    std::optional<bool> dense = {};
    /// Leading icon (MUI's `ListItemIcon`).
    std::optional<Node> icon = {};
    /// Keyboard shortcut legend, e.g. "⌘C".
    std::optional<std::string> shortcut = {};
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// One row of a menu. Mirrors MUI's `<MenuItem>`.
Node MenuItem(const MenuItemProps& props, Node children);

// ------------------------------------------------------------- Pagination

enum class PaginationVariant {
    Text,      // printed numbers; only the current page is a keycap
    Outlined,  // every page is a small white keycap
};

enum class PaginationShape { Circular, Rounded };

/// What a `PaginationItem` stands for (MUI's `type`).
struct PaginationItemType {
    enum class Kind { Page, First, Previous, Next, Last, StartEllipsis, EndEllipsis };
    Kind kind = Kind::Page;
    /// The page number (only for `Kind::Page`).
    int number = 1;

    static constexpr PaginationItemType page(int n) { return {Kind::Page, n}; }
    static constexpr PaginationItemType first() { return {Kind::First, 0}; }
    static constexpr PaginationItemType previous() { return {Kind::Previous, 0}; }
    static constexpr PaginationItemType next() { return {Kind::Next, 0}; }
    static constexpr PaginationItemType last() { return {Kind::Last, 0}; }
    static constexpr PaginationItemType start_ellipsis() { return {Kind::StartEllipsis, 0}; }
    static constexpr PaginationItemType end_ellipsis() { return {Kind::EndEllipsis, 0}; }
    bool operator==(const PaginationItemType&) const = default;
};

/// The items a pagination bar shows, as MUI's `usePagination` computes them.
std::vector<PaginationItemType> pagination_items(int count, int page, int siblings_count, int boundary_count,
                                                 bool show_first_button, bool show_last_button,
                                                 bool hide_prev_button, bool hide_next_button);

struct PaginationProps {
    int count = 1;
    /// Current page, 1-based (controlled, pair with `onchange`).
    std::optional<int> page = {};
    int default_page = 1;
    std::function<void(int)> onchange = {};
    int siblings_count = 1;
    int boundary_count = 1;
    PaginationVariant variant = PaginationVariant::Text;
    PaginationShape shape = PaginationShape::Circular;
    Color color = Color::Primary;
    Size size = Size::Medium;
    bool show_first_button = false;
    bool show_last_button = false;
    bool hide_prev_button = false;
    bool hide_next_button = false;
    bool disabled = false;
    std::string aria_label = "pagination navigation";
    std::string cls = {};
};

/// A row of small page keycaps. Mirrors MUI's `<Pagination>`.
Node Pagination(const PaginationProps& props);

struct PaginationItemProps {
    PaginationItemType type = PaginationItemType::page(1);
    /// The page this item leads to.
    int page = 0;
    bool selected = false;
    bool disabled = false;
    PaginationVariant variant = PaginationVariant::Text;
    PaginationShape shape = PaginationShape::Circular;
    Color color = Color::Primary;
    Size size = Size::Medium;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// One key of a `Pagination` bar. Mirrors MUI's `<PaginationItem>`.
Node PaginationItem(const PaginationItemProps& props);

// ---------------------------------------------------------------- Stepper

struct StepperProps {
    /// Index of the current step (0-based).
    int active_step = 0;
    Orientation orientation = Orientation::Horizontal;
    /// Put labels under the indicators (horizontal only).
    bool alternative_label = false;
    /// Steps can be visited in any order.
    bool non_linear = false;
    /// Palette color of the active / completed indicators.
    Color color = Color::Primary;
    std::string cls = {};
};

/// A row (or column) of steps joined by grooves. Steps are numbered in
/// order (or by their `index`). Mirrors MUI's `<Stepper>`.
Node Stepper(const StepperProps& props, Node children);

struct StepProps {
    /// Position in the stepper; assigned in order when omitted.
    std::optional<int> index = {};
    std::optional<bool> active = {};
    std::optional<bool> completed = {};
    std::optional<bool> disabled = {};
    /// Keep the `StepContent` open even when the step is not active.
    bool expanded = false;
    std::string cls = {};
};

/// One step of a `Stepper`. Mirrors MUI's `<Step>`. Must be in a `Stepper`.
Node Step(const StepProps& props, Node children);

struct StepLabelProps {
    /// Caption under the label, e.g. "Optional".
    std::optional<Node> optional = {};
    /// Show the step in the error state.
    bool error = false;
    /// Replace the number legend of the indicator.
    std::optional<Node> icon = {};
    std::string cls = {};
};

/// The indicator keycap and printed label of a `Step`. Mirrors MUI's `<StepLabel>`.
Node StepLabel(const StepLabelProps& props, Node children);

struct StepButtonProps {
    std::optional<Node> optional = {};
    std::optional<Node> icon = {};
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A `StepLabel` you can press (non-linear steppers). Mirrors MUI's `<StepButton>`.
Node StepButton(const StepButtonProps& props, Node children);

struct StepContentProps {
    std::string cls = {};
};

/// The body of a step in a vertical `Stepper`, shown while the step is
/// active (or expanded). Mirrors MUI's `<StepContent>`.
Node StepContent(const StepContentProps& props, Node children);

enum class MobileStepperVariant { Dots, Progress, Text };
enum class MobileStepperPosition { Bottom, Top, Static };

struct MobileStepperProps {
    int steps = 1;
    int active_step = 0;
    MobileStepperVariant variant = MobileStepperVariant::Dots;
    MobileStepperPosition position = MobileStepperPosition::Bottom;
    Color color = Color::Primary;
    std::optional<Node> back_button = {};
    std::optional<Node> next_button = {};
    std::string cls = {};
};

/// A compact stepper bar. Mirrors MUI's `<MobileStepper>`.
Node MobileStepper(const MobileStepperProps& props);

// ------------------------------------------------------- BottomNavigation

struct BottomNavigationProps {
    std::optional<std::string> value = {};
    std::optional<std::string> default_value = {};
    std::function<void(const std::string&)> onchange = {};
    /// Show every label (otherwise only the selected action's).
    bool show_labels = false;
    Color color = Color::Primary;
    std::string aria_label = "Navigation";
    std::string cls = {};
};

/// A bar of flat keys, one per destination. Mirrors MUI's `<BottomNavigation>`.
Node BottomNavigation(const BottomNavigationProps& props, Node children);

struct BottomNavigationActionProps {
    std::string value;
    std::optional<std::string> label = {};
    std::optional<Node> icon = {};
    /// Always show the label, whatever the bar's `show_labels`.
    std::optional<bool> show_label = {};
    bool disabled = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// One key of a `BottomNavigation` bar. Mirrors MUI's `<BottomNavigationAction>`.
Node BottomNavigationAction(const BottomNavigationActionProps& props);

// -------------------------------------------------------------- SpeedDial

enum class SpeedDialDirection { Up, Down, Left, Right };

struct SpeedDialProps {
    /// Accessible name of the main key.
    std::string aria_label;
    /// Controlled open state (pair with `onopen` / `onclose`).
    std::optional<bool> open = {};
    std::function<void()> onopen = {};
    std::function<void()> onclose = {};
    SpeedDialDirection direction = SpeedDialDirection::Up;
    Color color = Color::Primary;
    /// Hide the whole dial.
    bool hidden = false;
    /// Legend of the main key; a `SpeedDialIcon` by default.
    std::optional<Node> icon = {};
    std::string cls = {};
};

/// A large round colored keycap whose actions fan out. Mirrors MUI's `<SpeedDial>`.
Node SpeedDial(const SpeedDialProps& props, Node children);

struct SpeedDialIconProps {
    std::optional<Node> icon = {};
    std::optional<Node> open_icon = {};
    std::string cls = {};
};

/// A plus that turns into a cross as the dial opens. Mirrors MUI's `<SpeedDialIcon>`.
Node SpeedDialIcon(const SpeedDialIconProps& props = {});

struct SpeedDialActionProps {
    Node icon = {};
    /// The action's name: tooltip and accessible name.
    std::string tooltip_title;
    /// Always show the tooltip label.
    bool tooltip_open = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// One action of a `SpeedDial`. Mirrors MUI's `<SpeedDialAction>`.
Node SpeedDialAction(const SpeedDialActionProps& props);

// --------------------------------------------------------------- Timeline

enum class TimelinePosition { Left, Right, Alternate, AlternateReverse };

struct TimelineProps {
    TimelinePosition position = TimelinePosition::Right;
    std::string cls = {};
};
/// A vertical sequence of events. Mirrors MUI lab's `<Timeline>`.
Node Timeline(const TimelineProps& props, Node children);

struct TimelineItemProps {
    std::optional<TimelinePosition> position = {};
    std::string cls = {};
};
Node TimelineItem(const TimelineItemProps& props, Node children);

struct TimelineSeparatorProps {
    std::string cls = {};
};
Node TimelineSeparator(const TimelineSeparatorProps& props, Node children);

enum class TimelineDotVariant { Filled, Outlined };

struct TimelineDotProps {
    /// Without a color the LED is grey.
    std::optional<Color> color = {};
    TimelineDotVariant variant = TimelineDotVariant::Filled;
    std::string cls = {};
};
Node TimelineDot(const TimelineDotProps& props, Node children = {});

struct TimelineConnectorProps {
    std::string cls = {};
};
Node TimelineConnector(const TimelineConnectorProps& props = {});

struct TimelineContentProps {
    std::string cls = {};
};
Node TimelineContent(const TimelineContentProps& props, Node children);

struct TimelineOppositeContentProps {
    std::string cls = {};
};
Node TimelineOppositeContent(const TimelineOppositeContentProps& props, Node children);

}  // namespace skeuo
