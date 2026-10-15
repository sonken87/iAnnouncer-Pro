use dioxus::prelude::*;

use crate::{Color, Icon, IconName, classes, use_unique_id};

/// Which way the actions fan out (MUI's `direction`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum SpeedDialDirection {
    #[default]
    Up,
    Down,
    Left,
    Right,
}

impl SpeedDialDirection {
    fn as_str(self) -> &'static str {
        match self {
            SpeedDialDirection::Up => "up",
            SpeedDialDirection::Down => "down",
            SpeedDialDirection::Left => "left",
            SpeedDialDirection::Right => "right",
        }
    }
}

#[derive(Clone, Copy, PartialEq)]
struct SpeedDialCtx {
    open: Signal<bool>,
}

/// A large round colored keycap (like a FAB) whose actions fan out as small
/// round white keycaps. Mirrors MUI's `<SpeedDial>`.
///
/// Controlled with `open` + `onopen` / `onclose`; uncontrolled otherwise
/// (pressing the key toggles it). `Escape` closes it.
#[derive(Props, Clone, PartialEq)]
pub struct SpeedDialProps {
    /// Accessible name of the main key, e.g. "Quick actions".
    #[props(into)]
    pub aria_label: String,
    pub open: Option<bool>,
    pub onopen: Option<EventHandler<()>>,
    pub onclose: Option<EventHandler<()>>,
    #[props(default)]
    pub direction: SpeedDialDirection,
    #[props(default)]
    pub color: Color,
    /// Hide the whole dial (it shrinks away).
    #[props(default)]
    pub hidden: bool,
    /// Legend of the main key; a [`SpeedDialIcon`] by default.
    pub icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
    /// The [`SpeedDialAction`]s.
    pub children: Element,
}

#[component]
pub fn SpeedDial(props: SpeedDialProps) -> Element {
    let id = use_unique_id("skeuo-speed-dial");
    let actions_id = format!("{id}-actions");
    let mut internal = use_signal(|| false);
    let controlled = props.open.is_some();
    let open = props.open.unwrap_or(*internal.read());
    let mut shared = use_signal(|| open);
    if *shared.peek() != open {
        shared.set(open);
    }
    use_context_provider(|| SpeedDialCtx { open: shared });

    let (onopen, onclose) = (props.onopen, props.onclose);
    let mut set_open = move |v: bool| {
        if !controlled {
            internal.set(v);
        }
        let h = if v { onopen } else { onclose };
        if let Some(h) = h {
            h.call(());
        }
    };
    let direction = format!("skeuo-speed-dial--{}", props.direction.as_str());
    rsx! {
        div {
            class: classes([
                "skeuo-speed-dial",
                &direction,
                if open { "skeuo-speed-dial--open" } else { "" },
                if props.hidden { "skeuo-speed-dial--hidden" } else { "" },
                props.color.class(),
                &props.class,
            ]),
            role: "presentation",
            onkeydown: move |e: KeyboardEvent| {
                if e.key() == Key::Escape && open {
                    set_open(false);
                }
            },
            button {
                class: "skeuo-speed-dial__fab skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--round",
                r#type: "button",
                id: id.clone(),
                aria_label: props.aria_label,
                aria_haspopup: "true",
                aria_expanded: if open { "true" } else { "false" },
                aria_controls: actions_id.clone(),
                onclick: move |_| set_open(!open),
                if let Some(icon) = props.icon {
                    {icon}
                } else {
                    SpeedDialIcon {}
                }
            }
            div {
                class: "skeuo-speed-dial__actions",
                id: actions_id,
                role: "menu",
                aria_orientation: match props.direction {
                    SpeedDialDirection::Up | SpeedDialDirection::Down => "vertical",
                    _ => "horizontal",
                },
                aria_labelledby: id,
                {props.children}
            }
        }
    }
}

/// The legend of a [`SpeedDial`] key: a plus that turns into a cross as
/// the dial opens (or swaps to `open_icon`). Mirrors MUI's
/// `<SpeedDialIcon>`.
#[derive(Props, Clone, PartialEq)]
pub struct SpeedDialIconProps {
    pub icon: Option<Element>,
    pub open_icon: Option<Element>,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn SpeedDialIcon(props: SpeedDialIconProps) -> Element {
    let open = try_use_context::<SpeedDialCtx>()
        .map(|c| *c.open.read())
        .unwrap_or(false);
    let has_open_icon = props.open_icon.is_some();
    rsx! {
        span {
            class: classes([
                "skeuo-speed-dial-icon",
                if open { "skeuo-speed-dial-icon--open" } else { "" },
                if has_open_icon { "skeuo-speed-dial-icon--swap" } else { "" },
                &props.class,
            ]),
            aria_hidden: "true",
            span { class: "skeuo-speed-dial-icon__icon",
                if let Some(icon) = props.icon {
                    {icon}
                } else {
                    Icon { name: IconName::Add }
                }
            }
            if let Some(open_icon) = props.open_icon {
                span { class: "skeuo-speed-dial-icon__open-icon", {open_icon} }
            }
        }
    }
}

/// One action of a [`SpeedDial`]: a small round white keycap with a
/// printed tooltip label. Mirrors MUI's `<SpeedDialAction>`.
#[derive(Props, Clone, PartialEq)]
pub struct SpeedDialActionProps {
    pub icon: Element,
    /// The action's name: shown as a tooltip and used as its accessible name.
    #[props(into)]
    pub tooltip_title: String,
    /// Always show the tooltip label (MUI's `tooltipOpen`).
    #[props(default)]
    pub tooltip_open: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
}

#[component]
pub fn SpeedDialAction(props: SpeedDialActionProps) -> Element {
    let open = try_use_context::<SpeedDialCtx>()
        .map(|c| *c.open.read())
        .unwrap_or(true);
    rsx! {
        span {
            class: classes([
                "skeuo-speed-dial-action",
                if props.tooltip_open { "skeuo-speed-dial-action--tooltip-open" } else { "" },
                &props.class,
            ]),
            button {
                class: "skeuo-speed-dial-action__key skeuo-key skeuo-key--pressable skeuo-key--round",
                r#type: "button",
                role: "menuitem",
                aria_label: props.tooltip_title.clone(),
                tabindex: if open { "0" } else { "-1" },
                onclick: move |e| {
                    if let Some(h) = &props.onclick {
                        h.call(e);
                    }
                },
                {props.icon}
            }
            span { class: "skeuo-speed-dial-action__tooltip skeuo-chassis skeuo-elev-3 skeuo-chassis--float", aria_hidden: "true",
                "{props.tooltip_title}"
            }
        }
    }
}
