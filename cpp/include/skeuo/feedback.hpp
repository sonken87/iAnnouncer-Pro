// skeuo-ui for C++ — Feedback: Alert, Backdrop, Modal, Dialog, Progress, Skeleton, Snackbar.
//
// Mirrors crates/skeuo-ui/src/feedback. Props are aggregates; call sites use
// designated initializers in declaration order:
//
//   Alert({.severity = AlertSeverity::Warning, .onclose = [&] { hide(); }}, "Careful")
//   Dialog({.open = open, .onclose = [&](ModalCloseReason) { open = false; }},
//          {DialogTitle({}, "Discard?"), DialogActions({}, Button({}, "OK"))})
#pragma once

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "skeuo/core.hpp"
#include "skeuo/inputs.hpp"
#include "skeuo/node.hpp"

namespace skeuo {

// ------------------------------------------------------------------ Alert

/// Alert severity, as in MUI's `severity` prop. Picks the palette color and
/// the default icon.
enum class AlertSeverity { Success, Info, Warning, Error };

inline constexpr std::array<AlertSeverity, 4> kAllAlertSeverities = {
    AlertSeverity::Success, AlertSeverity::Info, AlertSeverity::Warning, AlertSeverity::Error};

constexpr std::string_view as_str(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::Success: return "success";
        case AlertSeverity::Info: return "info";
        case AlertSeverity::Warning: return "warning";
        case AlertSeverity::Error: return "error";
    }
    return "success";
}

/// The palette color used for this severity.
constexpr Color severity_color(AlertSeverity s) {
    switch (s) {
        case AlertSeverity::Success: return Color::Success;
        case AlertSeverity::Info: return Color::Info;
        case AlertSeverity::Warning: return Color::Warning;
        case AlertSeverity::Error: return Color::Error;
    }
    return Color::Success;
}

/// Alert style, mirroring MUI's `variant` names.
enum class AlertVariant {
    Standard,  // a flush aluminium plate washed with the color's tint; colored icon
    Filled,    // a plate in the palette color, like the flat top of a colored keycap
    Outlined,  // a flush aluminium plate with a colored hairline edge
};

inline constexpr std::array<AlertVariant, 3> kAllAlertVariants = {AlertVariant::Standard, AlertVariant::Filled,
                                                                  AlertVariant::Outlined};

constexpr std::string_view as_str(AlertVariant v) {
    switch (v) {
        case AlertVariant::Standard: return "standard";
        case AlertVariant::Filled: return "filled";
        case AlertVariant::Outlined: return "outlined";
    }
    return "standard";
}

struct AlertProps {
    AlertSeverity severity = AlertSeverity::Success;
    AlertVariant variant = AlertVariant::Standard;
    /// Overrides the color derived from `severity`.
    std::optional<Color> color = {};
    /// Replaces the severity icon.
    std::optional<Node> icon = {};
    /// Hides the icon (MUI `icon={false}`).
    bool hide_icon = false;
    /// Element at the end of the alert, e.g. a button. Replaces the close button.
    std::optional<Node> action = {};
    /// Shows a close button and is called when it is pressed.
    std::function<void()> onclose = {};
    /// Accessible label of the close button.
    std::string close_text = "Close";
    /// ARIA role of the root.
    std::string role = "alert";
    std::string cls = {};
};

/// A short, important message on a flush plate. Mirrors MUI's <Alert>.
Node Alert(const AlertProps& props, Node children);

struct AlertTitleProps {
    std::string cls = {};
};

/// The bold first line of an Alert. Mirrors MUI's <AlertTitle>.
Node AlertTitle(const AlertTitleProps& props, Node children);

// ------------------------------------------------------------------ Backdrop

struct BackdropProps {
    bool open = false;
    /// Transparent: catches clicks without dimming.
    bool invisible = false;
    /// Preview mode: fills the nearest positioned ancestor instead of the window.
    bool inline_ = false;
    std::string cls = {};
    std::function<void()> onclick = {};
};

/// A dimming scrim over the window with its children centered. Mirrors
/// MUI's <Backdrop>. Renders nothing while closed.
Node Backdrop(const BackdropProps& props, Node children = {});

// ------------------------------------------------------------------ Modal

/// Why a Modal or Dialog asks to close (MUI's `reason` argument of `onClose`).
enum class ModalCloseReason {
    BackdropClick,  // the scrim behind the surface was clicked
    EscapeKeyDown,  // Escape was pressed while focus was inside
};

constexpr std::string_view as_str(ModalCloseReason r) {
    return r == ModalCloseReason::BackdropClick ? "backdropClick" : "escapeKeyDown";
}

struct ModalProps {
    bool open = false;
    std::function<void(ModalCloseReason)> onclose = {};
    bool disable_escape_key_down = false;
    /// Do not render the scrim (clicks outside then do nothing).
    bool hide_backdrop = false;
    /// Keep the children in the DOM (hidden) while closed.
    bool keep_mounted = false;
    /// ARIA role of the focusable content wrapper.
    std::string role = "dialog";
    std::optional<std::string> aria_labelledby = {};
    std::optional<std::string> aria_describedby = {};
    std::string cls = {};
};

/// The bare overlay dialogs are built on: scrim, focus on open, Escape and
/// scrim click call `onclose`. Mirrors MUI's <Modal>.
Node Modal(const ModalProps& props, Node children);

// ------------------------------------------------------------------ Dialog

/// Width cap of a Dialog, as in MUI's `maxWidth` prop.
enum class DialogMaxWidth { Xs, Sm, Md, Lg, Xl, False };

constexpr std::string_view as_str(DialogMaxWidth w) {
    switch (w) {
        case DialogMaxWidth::Xs: return "xs";
        case DialogMaxWidth::Sm: return "sm";
        case DialogMaxWidth::Md: return "md";
        case DialogMaxWidth::Lg: return "lg";
        case DialogMaxWidth::Xl: return "xl";
        case DialogMaxWidth::False: return "false";
    }
    return "sm";
}

/// What scrolls when a Dialog is taller than the window (MUI's `scroll`).
enum class DialogScroll { Paper, Body };

constexpr std::string_view as_str(DialogScroll s) { return s == DialogScroll::Body ? "body" : "paper"; }

struct DialogProps {
    bool open = false;
    std::function<void(ModalCloseReason)> onclose = {};
    /// Stretch to `max_width`.
    bool full_width = false;
    DialogMaxWidth max_width = DialogMaxWidth::Sm;
    /// Cover the whole window, with square corners.
    bool full_screen = false;
    DialogScroll scroll = DialogScroll::Paper;
    bool disable_escape_key_down = false;
    /// Preview mode: render in flow instead of over the window.
    bool inline_ = false;
    /// Id of the element labelling the dialog. Defaults to the DialogTitle.
    std::optional<std::string> aria_labelledby = {};
    /// Id of the element describing the dialog, e.g. a DialogContentText.
    std::optional<std::string> aria_describedby = {};
    std::string cls = {};
};

/// A modal window: a level-4 floating chassis over the scrim. Mirrors MUI's
/// <Dialog>. Its DialogTitle (one without its own id) gets the id the
/// dialog is labelled by.
Node Dialog(const DialogProps& props, Node children);

struct DialogTitleProps {
    /// Overrides the id provided by the surrounding Dialog.
    std::optional<std::string> id = {};
    std::string cls = {};
};
Node DialogTitle(const DialogTitleProps& props, Node children);

struct DialogContentProps {
    /// Engrave rules above and below the content.
    bool dividers = false;
    std::string cls = {};
};
Node DialogContent(const DialogContentProps& props, Node children);

struct DialogContentTextProps {
    std::optional<std::string> id = {};
    std::string cls = {};
};
Node DialogContentText(const DialogContentTextProps& props, Node children);

struct DialogActionsProps {
    /// Remove the gap between the actions.
    bool disable_spacing = false;
    std::string cls = {};
};
Node DialogActions(const DialogActionsProps& props, Node children);

// ------------------------------------------------------------------ Progress

/// Mode of a CircularProgress, as in MUI's `variant`.
enum class CircularProgressVariant { Determinate, Indeterminate };

constexpr std::string_view as_str(CircularProgressVariant v) {
    return v == CircularProgressVariant::Determinate ? "determinate" : "indeterminate";
}

struct CircularProgressProps {
    CircularProgressVariant variant = CircularProgressVariant::Indeterminate;
    /// Percent complete (0–100) for the determinate variant.
    double value = 0.0;
    /// Diameter in px.
    unsigned size = 40;
    /// Ring thickness in units of a 44-unit-wide box (MUI's default 3.6).
    double thickness = 3.6;
    Color color = Color::Primary;
    /// Spin without the arc growing and shrinking.
    bool disable_shrink = false;
    std::optional<std::string> aria_label = {};
    std::string cls = {};
};

/// A ring groove with a colored arc. Mirrors MUI's <CircularProgress>.
Node CircularProgress(const CircularProgressProps& props);

/// Mode of a LinearProgress, as in MUI's `variant`.
enum class LinearProgressVariant { Determinate, Indeterminate, Buffer, Query };

constexpr std::string_view as_str(LinearProgressVariant v) {
    switch (v) {
        case LinearProgressVariant::Determinate: return "determinate";
        case LinearProgressVariant::Indeterminate: return "indeterminate";
        case LinearProgressVariant::Buffer: return "buffer";
        case LinearProgressVariant::Query: return "query";
    }
    return "indeterminate";
}

struct LinearProgressProps {
    LinearProgressVariant variant = LinearProgressVariant::Indeterminate;
    /// Percent complete (0–100) for the determinate and buffer variants.
    double value = 0.0;
    /// Percent buffered (0–100) for the buffer variant.
    double value_buffer = 0.0;
    Color color = Color::Primary;
    std::optional<std::string> aria_label = {};
    std::string cls = {};
};

/// A groove filled with the palette color. Mirrors MUI's <LinearProgress>.
Node LinearProgress(const LinearProgressProps& props);

// ------------------------------------------------------------------ Skeleton

/// Shape of a Skeleton, as in MUI's `variant`.
enum class SkeletonVariant { Text, Rectangular, Rounded, Circular };

constexpr std::string_view as_str(SkeletonVariant v) {
    switch (v) {
        case SkeletonVariant::Text: return "text";
        case SkeletonVariant::Rectangular: return "rectangular";
        case SkeletonVariant::Rounded: return "rounded";
        case SkeletonVariant::Circular: return "circular";
    }
    return "text";
}

/// Loading animation of a Skeleton, as in MUI's `animation`.
enum class SkeletonAnimation { Pulse, Wave, False };

constexpr std::string_view as_str(SkeletonAnimation a) {
    switch (a) {
        case SkeletonAnimation::Pulse: return "pulse";
        case SkeletonAnimation::Wave: return "wave";
        case SkeletonAnimation::False: return "false";
    }
    return "pulse";
}

struct SkeletonProps {
    SkeletonVariant variant = SkeletonVariant::Text;
    SkeletonAnimation animation = SkeletonAnimation::Pulse;
    /// CSS width, e.g. "120px" or "60%".
    std::optional<std::string> width = {};
    /// CSS height.
    std::optional<std::string> height = {};
    std::string cls = {};
};

/// A shallow well where content will land. Mirrors MUI's <Skeleton>.
/// Children (if any) are hidden but give the skeleton its size.
Node Skeleton(const SkeletonProps& props, Node children = {});

// ------------------------------------------------------------------ Snackbar

/// Vertical edge a Snackbar is anchored to.
enum class SnackbarVertical { Top, Bottom };
/// Horizontal position of a Snackbar.
enum class SnackbarHorizontal { Left, Center, Right };

/// Where a Snackbar appears, as in MUI's `anchorOrigin`.
struct SnackbarOrigin {
    SnackbarVertical vertical = SnackbarVertical::Bottom;
    SnackbarHorizontal horizontal = SnackbarHorizontal::Left;
    bool operator==(const SnackbarOrigin&) const = default;
};

/// Why a Snackbar asks to close (MUI's `reason` argument).
enum class SnackbarCloseReason {
    Timeout,        // auto_hide_duration elapsed
    EscapeKeyDown,  // Escape was pressed while focus was inside the snackbar
};

constexpr std::string_view as_str(SnackbarCloseReason r) {
    return r == SnackbarCloseReason::Timeout ? "timeout" : "escapeKeyDown";
}

struct SnackbarProps {
    bool open = false;
    std::optional<std::string> message = {};
    /// Element after the message, e.g. an "Undo" button.
    std::optional<Node> action = {};
    std::function<void(SnackbarCloseReason)> onclose = {};
    /// Milliseconds before `onclose(Timeout)`; unset never hides. The
    /// countdown is a CSS animation (paused on hover/focus), no timers.
    std::optional<unsigned> auto_hide_duration = {};
    SnackbarOrigin anchor_origin = {};
    /// Preview mode: render in flow instead of fixed to the window edge.
    bool inline_ = false;
    std::string cls = {};
};

/// A brief message at the edge of the window. Mirrors MUI's <Snackbar>.
/// Pass `message` / `action` for the standard content, or children for a
/// custom surface (e.g. an Alert).
Node Snackbar(const SnackbarProps& props, Node children = {});

struct SnackbarContentProps {
    std::string message = {};
    std::optional<Node> action = {};
    std::string role = "alert";
    std::string cls = {};
};

/// The plate of a Snackbar. Mirrors MUI's <SnackbarContent>.
Node SnackbarContent(const SnackbarContentProps& props);

}  // namespace skeuo
