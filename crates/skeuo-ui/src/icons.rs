use dioxus::prelude::*;

/// Built-in line icons (24×24 grid, round caps, drawn to sit beside SF Pro
/// legends). The counterpart of `@mui/icons-material` for the icons that the
/// components themselves need, plus common app icons.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum IconName {
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
    Map,
    Menu,
    Mic,
    MoreHoriz,
    MoreVert,
    Notifications,
    Pause,
    Person,
    Plane,
    Play,
    Remove,
    Search,
    Settings,
    Star,
    StarBorder,
    Volume,
    Warning,
}

impl IconName {
    pub const ALL: [IconName; 42] = [
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
        IconName::Map,
        IconName::Menu,
        IconName::Mic,
        IconName::MoreHoriz,
        IconName::MoreVert,
        IconName::Notifications,
        IconName::Pause,
        IconName::Person,
        IconName::Plane,
        IconName::Play,
        IconName::Remove,
        IconName::Search,
        IconName::Settings,
        IconName::Star,
        IconName::StarBorder,
        IconName::Volume,
        IconName::Warning,
    ];

    /// SVG path data on a 24×24 grid.
    pub fn path(self) -> &'static str {
        match self {
            IconName::Add => "M12 5v14M5 12h14",
            IconName::ArrowBack => "M19 12H5M11 6l-6 6 6 6",
            IconName::ArrowForward => "M5 12h14M13 6l6 6-6 6",
            IconName::Bookmark => "M6.5 4h11v16l-5.5-4-5.5 4z",
            IconName::Calendar => "M4 6h16v14H4zM4 10.5h16M8 3.5v4M16 3.5v4",
            IconName::Check => "M5 12.5l4.5 4.5L19 7.5",
            IconName::CheckCircle => "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM8 12.3l2.8 2.8 5.2-5.6",
            IconName::ChevronDown => "M6 9l6 6 6-6",
            IconName::ChevronLeft => "M15 6l-6 6 6 6",
            IconName::ChevronRight => "M9 6l6 6-6 6",
            IconName::ChevronUp => "M6 15l6-6 6 6",
            IconName::Close => "M6 6l12 12M18 6L6 18",
            IconName::Copy => "M8.5 8.5h11v11h-11zM5 15.5V4.5h11",
            IconName::Delete => "M5 7h14M10 7V4.5h4V7M7 7l1 13h8l1-13M10.5 11v5.5M13.5 11v5.5",
            IconName::Dot => "M12 7a5 5 0 1 0 0 10 5 5 0 0 0 0-10z",
            IconName::Edit => "M4 20h4L19 9l-4-4L4 16zM13.5 6.5l4 4",
            IconName::Error => "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM12 7.5v6M12 16.5v.5",
            IconName::Favorite | IconName::FavoriteBorder => {
                "M12 20s-7.5-4.6-7.5-10.2A4.3 4.3 0 0 1 12 7.2a4.3 4.3 0 0 1 7.5 2.6C19.5 15.4 12 20 12 20z"
            }
            IconName::FirstPage => "M17 6l-6 6 6 6M7 6v12",
            IconName::Folder => "M3.5 6.5h6l2 2h9v10h-17z",
            IconName::Home => "M4 11l8-7 8 7M6.5 9v11h11V9",
            IconName::Info => "M12 21a9 9 0 1 0 0-18 9 9 0 0 0 0 18zM12 11v6M12 7.5v.5",
            IconName::LastPage => "M7 6l6 6-6 6M17 6v12",
            IconName::Mail => "M3.5 6h17v12h-17zM4 6.5l8 6.5 8-6.5",
            IconName::Map => "M9 4.5L3.5 6.5v13l5.5-2 6 2 5.5-2v-13l-5.5 2zM9 4.5v13M15 6.5v13",
            IconName::Menu => "M4 7h16M4 12h16M4 17h16",
            IconName::Mic => {
                "M12 14a3 3 0 0 0 3-3V6a3 3 0 0 0-6 0v5a3 3 0 0 0 3 3zM6 11a6 6 0 0 0 12 0M12 17v3.5"
            }
            IconName::MoreHoriz => "M6 12h.01M12 12h.01M18 12h.01",
            IconName::MoreVert => "M12 6v.01M12 12v.01M12 18v.01",
            IconName::Notifications => "M6 16.5V11a6 6 0 0 1 12 0v5.5l1.5 1.5h-15zM10 20.5h4",
            IconName::Pause => "M9 5.5v13M15 5.5v13",
            IconName::Person => "M12 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8zM4.5 20a7.5 7.5 0 0 1 15 0",
            IconName::Plane => {
                "M12 2.5c.9 0 1.5.8 1.5 1.8v5.2l7.5 4.3v2l-7.5-2.3v4.6l2.2 1.6v1.6L12 20.4l-3.7.9v-1.6l2.2-1.6v-4.6L3 15.8v-2l7.5-4.3V4.3c0-1 .6-1.8 1.5-1.8z"
            }
            IconName::Play => "M8 5.5v13l10-6.5z",
            IconName::Remove => "M5 12h14",
            IconName::Search => "M10.5 17a6.5 6.5 0 1 0 0-13 6.5 6.5 0 0 0 0 13zM15.5 15.5L20 20",
            IconName::Settings => "M4 7h9M17 7h3M4 17h3M11 17h9M15 4.5v5M9 14.5v5",
            IconName::Star | IconName::StarBorder => {
                "M12 3.8l2.5 5.1 5.6.8-4.05 3.95.95 5.6L12 16.6l-5 2.65.95-5.6L3.9 9.7l5.6-.8z"
            }
            IconName::Volume => {
                "M4 9.5h3.5L12 5.5v13l-4.5-4H4zM15.5 9a4 4 0 0 1 0 6M18 6.5a7.5 7.5 0 0 1 0 11"
            }
            IconName::Warning => "M12 4L21 19.5H3zM12 10v4.5M12 17v.3",
        }
    }

    /// Whether the shape is drawn solid rather than as an outline.
    pub fn filled(self) -> bool {
        matches!(
            self,
            IconName::Dot | IconName::Favorite | IconName::Plane | IconName::Play | IconName::Star
        )
    }
}

/// An SVG icon, sized to the surrounding font (`1em`) and colored with
/// `currentColor`. Mirrors MUI's `<SvgIcon>`.
#[component]
pub fn Icon(
    name: IconName,
    /// CSS size, e.g. `"20px"`; defaults to `1em`.
    #[props(default = "1em".to_string(), into)]
    size: String,
    #[props(default, into)] class: String,
    /// Accessible name. Without it the icon is decorative (`aria-hidden`).
    #[props(into)]
    title: Option<String>,
) -> Element {
    let filled = name.filled();
    let decorative = title.is_none();
    rsx! {
        svg {
            class: crate::classes(["skeuo-icon", class.as_str()]),
            view_box: "0 0 24 24",
            width: "{size}",
            height: "{size}",
            fill: if filled { "currentColor" } else { "none" },
            stroke: "currentColor",
            stroke_width: match name {
                // Dots are zero-length strokes; they need a thick pen.
                IconName::MoreHoriz | IconName::MoreVert => "3",
                _ if filled => "1.2",
                _ => "1.8",
            },
            stroke_linecap: "round",
            stroke_linejoin: "round",
            role: if decorative { "presentation" } else { "img" },
            "aria-hidden": if decorative { "true" } else { "false" },
            if let Some(t) = title {
                title { "{t}" }
            }
            path { d: name.path() }
        }
    }
}
