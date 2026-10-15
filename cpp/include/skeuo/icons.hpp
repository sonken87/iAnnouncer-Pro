// skeuo-ui for C++ — built-in line icons (generated from crates/skeuo-ui/src/icons.rs
// by the same 24x24 path data, so both implementations draw identical icons).
#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>

#include "skeuo/node.hpp"

namespace skeuo {

enum class IconName {
    Add,
    ArrowBack,
    ArrowForward,
    Bookmark,
    Calendar,
    Check,
    CheckCircle,
    ChevronDown,
    ChevronLeft,
    ChevronRight,
    ChevronUp,
    Close,
    Copy,
    Delete,
    Dot,
    Edit,
    Error,
    Favorite,
    FavoriteBorder,
    FirstPage,
    Folder,
    Home,
    Info,
    LastPage,
    Mail,
    Menu,
    Mic,
    MoreHoriz,
    MoreVert,
    Notifications,
    Pause,
    Person,
    Play,
    Remove,
    Search,
    Settings,
    Star,
    StarBorder,
    Volume,
    Warning,
};

inline constexpr std::array<IconName, 40> kAllIcons = {
    IconName::Add,
    IconName::ArrowBack,
    IconName::ArrowForward,
    IconName::Bookmark,
    IconName::Calendar,
    IconName::Check,
    IconName::CheckCircle,
    IconName::ChevronDown,
    IconName::ChevronLeft,
    IconName::ChevronRight,
    IconName::ChevronUp,
    IconName::Close,
    IconName::Copy,
    IconName::Delete,
    IconName::Dot,
    IconName::Edit,
    IconName::Error,
    IconName::Favorite,
    IconName::FavoriteBorder,
    IconName::FirstPage,
    IconName::Folder,
    IconName::Home,
    IconName::Info,
    IconName::LastPage,
    IconName::Mail,
    IconName::Menu,
    IconName::Mic,
    IconName::MoreHoriz,
    IconName::MoreVert,
    IconName::Notifications,
    IconName::Pause,
    IconName::Person,
    IconName::Play,
    IconName::Remove,
    IconName::Search,
    IconName::Settings,
    IconName::Star,
    IconName::StarBorder,
    IconName::Volume,
    IconName::Warning,
};

/// The enum name, e.g. "ChevronDown".
std::string_view icon_name(IconName name);
/// SVG path data on a 24x24 grid.
std::string_view icon_path(IconName name);
/// Whether the shape is drawn solid rather than as an outline.
bool icon_filled(IconName name);

struct IconProps {
    IconName name;
    /// CSS size, e.g. "20px"; defaults to 1em.
    std::string size = "1em";
    std::string cls = {};
    /// Accessible name. Without it the icon is decorative (aria-hidden).
    std::optional<std::string> title = {};
};

/// An SVG icon colored with currentColor. Mirrors MUI's <SvgIcon>.
Node Icon(const IconProps& props);
inline Node Icon(IconName name) { return Icon(IconProps{.name = name}); }

}  // namespace skeuo
