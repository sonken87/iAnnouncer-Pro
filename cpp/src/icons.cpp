#include "skeuo/icons.hpp"

namespace skeuo {

std::string_view icon_name(IconName name) {
    switch (name) {
        case IconName::Add: return "Add";
        case IconName::ArrowBack: return "ArrowBack";
        case IconName::ArrowForward: return "ArrowForward";
        case IconName::Bookmark: return "Bookmark";
        case IconName::Calendar: return "Calendar";
        case IconName::Check: return "Check";
        case IconName::CheckCircle: return "CheckCircle";
        case IconName::ChevronDown: return "ChevronDown";
        case IconName::ChevronLeft: return "ChevronLeft";
        case IconName::ChevronRight: return "ChevronRight";
        case IconName::ChevronUp: return "ChevronUp";
        case IconName::Close: return "Close";
        case IconName::Copy: return "Copy";
        case IconName::Delete: return "Delete";
        case IconName::Dot: return "Dot";
        case IconName::Edit: return "Edit";
        case IconName::Error: return "Error";
        case IconName::Favorite: return "Favorite";
        case IconName::FavoriteBorder: return "FavoriteBorder";
        case IconName::FirstPage: return "FirstPage";
        case IconName::Folder: return "Folder";
        case IconName::Home: return "Home";
        case IconName::Info: return "Info";
        case IconName::LastPage: return "LastPage";
        case IconName::Mail: return "Mail";
        case IconName::Menu: return "Menu";
        case IconName::Mic: return "Mic";
        case IconName::MoreHoriz: return "MoreHoriz";
        case IconName::MoreVert: return "MoreVert";
        case IconName::Notifications: return "Notifications";
        case IconName::Pause: return "Pause";
        case IconName::Person: return "Person";
        case IconName::Play: return "Play";
        case IconName::Remove: return "Remove";
        case IconName::Search: return "Search";
        case IconName::Settings: return "Settings";
        case IconName::Star: return "Star";
        case IconName::StarBorder: return "StarBorder";
        case IconName::Volume: return "Volume";
        case IconName::Warning: return "Warning";
    }
    return "";
}

std::string_view icon_path(IconName name) {
    switch (name) {
        case IconName::Add: return "M12 5v14M5 12h14";
        case IconName::ArrowBack: return "M19 12H5M11 6l-6 6 6 6";
        case IconName::ArrowForward: return "M5 12h14M13 6l6 6-6 6";
        case IconName::Bookmark: return "M6.5 4h11v16l-5.5-4-5.5 4z";
        case IconName::Calendar: return "M4 6h16v14H4zM4 10.5h16M8 3.5v4M16 3.5v4";
        case IconName::Check: return "M5 12.5l4.5 4.5L19 7.5";
        case IconName::CheckCircle: return "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM8 12.3l2.8 2.8 5.2-5.6";
        case IconName::ChevronDown: return "M6 9l6 6 6-6";
        case IconName::ChevronLeft: return "M15 6l-6 6 6 6";
        case IconName::ChevronRight: return "M9 6l6 6-6 6";
        case IconName::ChevronUp: return "M6 15l6-6 6 6";
        case IconName::Close: return "M6 6l12 12M18 6L6 18";
        case IconName::Copy: return "M8.5 8.5h11v11h-11zM5 15.5V4.5h11";
        case IconName::Delete: return "M5 7h14M10 7V4.5h4V7M7 7l1 13h8l1-13M10.5 11v5.5M13.5 11v5.5";
        case IconName::Dot: return "M12 7a5 5 0 1 0 0 10 5 5 0 0 0 0-10z";
        case IconName::Edit: return "M4 20h4L19 9l-4-4L4 16zM13.5 6.5l4 4";
        case IconName::Error: return "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM12 7.5v6M12 16.5v.5";
        case IconName::Favorite: return "M12 20s-7.5-4.6-7.5-10.2A4.3 4.3 0 0 1 12 7.2a4.3 4.3 0 0 1 7.5 2.6C19.5 15.4 12 20 12 20z";
        case IconName::FavoriteBorder: return "M12 20s-7.5-4.6-7.5-10.2A4.3 4.3 0 0 1 12 7.2a4.3 4.3 0 0 1 7.5 2.6C19.5 15.4 12 20 12 20z";
        case IconName::FirstPage: return "M17 6l-6 6 6 6M7 6v12";
        case IconName::Folder: return "M3.5 6.5h6l2 2h9v10h-17z";
        case IconName::Home: return "M4 11l8-7 8 7M6.5 9v11h11V9";
        case IconName::Info: return "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM12 11v6M12 7.5v.5";
        case IconName::LastPage: return "M7 6l6 6-6 6M17 6v12";
        case IconName::Mail: return "M3.5 6h17v12h-17zM4 6.5l8 6.5 8-6.5";
        case IconName::Menu: return "M4 7h16M4 12h16M4 17h16";
        case IconName::Mic: return "M12 14a3 3 0 0 0 3-3V6a3 3 0 0 0-6 0v5a3 3 0 0 0 3 3zM6 11a6 6 0 0 0 12 0M12 17v3.5";
        case IconName::MoreHoriz: return "M6 12h.01M12 12h.01M18 12h.01";
        case IconName::MoreVert: return "M12 6v.01M12 12v.01M12 18v.01";
        case IconName::Notifications: return "M6 16.5V11a6 6 0 0 1 12 0v5.5l1.5 1.5h-15zM10 20.5h4";
        case IconName::Pause: return "M9 5.5v13M15 5.5v13";
        case IconName::Person: return "M12 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8zM4.5 20a7.5 7.5 0 0 1 15 0";
        case IconName::Play: return "M8 5.5v13l10-6.5z";
        case IconName::Remove: return "M5 12h14";
        case IconName::Search: return "M10.5 17a6.5 6.5 0 1 0 0-13 6.5 6.5 0 0 0 0 13zM15.5 15.5L20 20";
        case IconName::Settings: return "M4 7h9M17 7h3M4 17h3M11 17h9M15 4.5v5M9 14.5v5";
        case IconName::Star: return "M12 3.8l2.5 5.1 5.6.8-4.05 3.95.95 5.6L12 16.6l-5 2.65.95-5.6L3.9 9.7l5.6-.8z";
        case IconName::StarBorder: return "M12 3.8l2.5 5.1 5.6.8-4.05 3.95.95 5.6L12 16.6l-5 2.65.95-5.6L3.9 9.7l5.6-.8z";
        case IconName::Volume: return "M4 9.5h3.5L12 5.5v13l-4.5-4H4zM15.5 9a4 4 0 0 1 0 6M18 6.5a7.5 7.5 0 0 1 0 11";
        case IconName::Warning: return "M12 4L21 19.5H3zM12 10v4.5M12 17v.3";
    }
    return "";
}

bool icon_filled(IconName name) {
    switch (name) {
        case IconName::Dot:
        case IconName::Favorite:
        case IconName::Play:
        case IconName::Star:
            return true;
        default:
            return false;
    }
}

Node Icon(const IconProps& props) {
    bool filled = icon_filled(props.name);
    bool decorative = !props.title.has_value();
    // Dots are zero-length strokes; they need a thick pen.
    std::string stroke_width = (props.name == IconName::MoreHoriz || props.name == IconName::MoreVert)
                                   ? "3"
                                   : (filled ? "1.2" : "1.8");
    Node svg = el("svg")
                   .cls(classes({"skeuo-icon", props.cls}))
                   .attr("viewBox", "0 0 24 24")
                   .attr("width", props.size)
                   .attr("height", props.size)
                   .attr("fill", filled ? "currentColor" : "none")
                   .attr("stroke", "currentColor")
                   .attr("stroke-width", stroke_width)
                   .attr("stroke-linecap", "round")
                   .attr("stroke-linejoin", "round")
                   .attr("role", decorative ? "presentation" : "img")
                   .attr("aria-hidden", decorative ? "true" : "false");
    if (props.title) svg.child(el("title").child(*props.title));
    svg.child(el("path").attr("d", std::string(icon_path(props.name))));
    return svg;
}

}  // namespace skeuo
