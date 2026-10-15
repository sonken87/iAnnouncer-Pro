use dioxus::prelude::*;

use crate::{Color, Icon, IconName, classes};

/// Alert severity, as in MUI's `severity` prop. Picks the palette color and
/// the default icon.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum AlertSeverity {
    #[default]
    Success,
    Info,
    Warning,
    Error,
}

impl AlertSeverity {
    pub const ALL: [AlertSeverity; 4] = [
        AlertSeverity::Success,
        AlertSeverity::Info,
        AlertSeverity::Warning,
        AlertSeverity::Error,
    ];

    pub fn as_str(self) -> &'static str {
        match self {
            AlertSeverity::Success => "success",
            AlertSeverity::Info => "info",
            AlertSeverity::Warning => "warning",
            AlertSeverity::Error => "error",
        }
    }

    /// The palette color used for this severity.
    pub fn color(self) -> Color {
        match self {
            AlertSeverity::Success => Color::Success,
            AlertSeverity::Info => Color::Info,
            AlertSeverity::Warning => Color::Warning,
            AlertSeverity::Error => Color::Error,
        }
    }

    fn icon(self) -> IconName {
        match self {
            AlertSeverity::Success => IconName::CheckCircle,
            AlertSeverity::Info => IconName::Info,
            AlertSeverity::Warning => IconName::Warning,
            AlertSeverity::Error => IconName::Error,
        }
    }
}

/// Alert style, mirroring MUI's `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum AlertVariant {
    /// A flush aluminium plate washed with the color's tint; colored icon.
    #[default]
    Standard,
    /// A plate in the palette color, like the flat top of a colored keycap.
    Filled,
    /// A flush aluminium plate with a colored hairline edge.
    Outlined,
}

impl AlertVariant {
    pub const ALL: [AlertVariant; 3] = [
        AlertVariant::Standard,
        AlertVariant::Filled,
        AlertVariant::Outlined,
    ];

    pub fn as_str(self) -> &'static str {
        match self {
            AlertVariant::Standard => "standard",
            AlertVariant::Filled => "filled",
            AlertVariant::Outlined => "outlined",
        }
    }
}

/// A short, important message. Mirrors MUI's `<Alert>`.
///
/// Physically: a flush plate set into the aluminium. `standard` is the
/// chassis washed with the severity tint, `filled` is a flat colored plate
/// (a colored keycap top without travel), `outlined` is plain aluminium with
/// a colored hairline. Passing `onclose` adds a close button (unless an
/// `action` is given, as in MUI).
#[derive(Props, Clone, PartialEq)]
pub struct AlertProps {
    #[props(default)]
    pub severity: AlertSeverity,
    #[props(default)]
    pub variant: AlertVariant,
    /// Overrides the color derived from `severity`.
    pub color: Option<Color>,
    /// Replaces the severity icon.
    pub icon: Option<Element>,
    /// Hides the icon (MUI `icon={false}`).
    #[props(default)]
    pub hide_icon: bool,
    /// Element at the end of the alert, e.g. a button. Replaces the close
    /// button.
    pub action: Option<Element>,
    /// Shows a close button and is called when it is pressed.
    pub onclose: Option<EventHandler<MouseEvent>>,
    /// Accessible label of the close button.
    #[props(default = "Close".to_string(), into)]
    pub close_text: String,
    /// ARIA role of the root, `"alert"` by default.
    #[props(default = "alert".to_string(), into)]
    pub role: String,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Alert(props: AlertProps) -> Element {
    let color = props.color.unwrap_or(props.severity.color());
    let variant = format!("skeuo-alert--{}", props.variant.as_str());
    let material = match props.variant {
        AlertVariant::Filled => "skeuo-key skeuo-key--filled",
        AlertVariant::Standard | AlertVariant::Outlined => "skeuo-chassis skeuo-elev-0",
    };
    let class = classes([
        material,
        "skeuo-alert",
        &variant,
        color.class(),
        &props.class,
    ]);
    let severity = props.severity;

    rsx! {
        div { class, role: props.role,
            if !props.hide_icon {
                div { class: "skeuo-alert__icon", aria_hidden: "true",
                    if let Some(icon) = props.icon {
                        {icon}
                    } else {
                        Icon { name: severity.icon() }
                    }
                }
            }
            div { class: "skeuo-alert__message", {props.children} }
            if let Some(action) = props.action {
                div { class: "skeuo-alert__action", {action} }
            } else if let Some(onclose) = props.onclose {
                div { class: "skeuo-alert__action",
                    button {
                        class: "skeuo-alert__close",
                        r#type: "button",
                        aria_label: props.close_text.clone(),
                        title: props.close_text.clone(),
                        onclick: move |e| onclose.call(e),
                        Icon { name: IconName::Close }
                    }
                }
            }
        }
    }
}

/// The bold first line of an [`Alert`]. Mirrors MUI's `<AlertTitle>`:
/// lettering in the alert's ink, slightly heavier than the message.
#[derive(Props, Clone, PartialEq)]
pub struct AlertTitleProps {
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn AlertTitle(props: AlertTitleProps) -> Element {
    rsx! {
        div { class: classes(["skeuo-alert__title", &props.class]), {props.children} }
    }
}
