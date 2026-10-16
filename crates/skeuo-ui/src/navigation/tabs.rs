use dioxus::prelude::*;

use super::roving::Roving;
use crate::{Color, Orientation, Size, classes, use_unique_id};

/// How the tabs share the available width, mirroring MUI's `variant`.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum TabsVariant {
    /// The tray hugs its tabs.
    #[default]
    Standard,
    /// The tray scrolls horizontally (or vertically) when the tabs overflow.
    Scrollable,
    /// The tray spans the container; tabs share the width equally.
    FullWidth,
}

impl TabsVariant {
    fn as_str(self) -> &'static str {
        match self {
            TabsVariant::Standard => "standard",
            TabsVariant::Scrollable => "scrollable",
            TabsVariant::FullWidth => "full-width",
        }
    }
}

/// Where a tab's icon sits relative to its label (MUI `iconPosition`).
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum IconPosition {
    #[default]
    Top,
    Bottom,
    Start,
    End,
}

impl IconPosition {
    fn as_str(self) -> &'static str {
        match self {
            IconPosition::Top => "top",
            IconPosition::Bottom => "bottom",
            IconPosition::Start => "start",
            IconPosition::End => "end",
        }
    }
}

/// Turns a tab value into something safe inside a DOM id.
fn id_part(value: &str) -> String {
    value
        .chars()
        .map(|c| if c.is_ascii_alphanumeric() { c } else { '-' })
        .collect()
}

/// Shared by [`TabContext`], [`Tabs`] and [`TabPanel`].
#[derive(Clone, Copy, PartialEq)]
struct TabContextHandle {
    value: Signal<String>,
    id: Signal<String>,
}

/// Holds the selected tab value for a [`Tabs`] + [`TabPanel`] group, so the
/// panels know which one is shown and how they are labelled. Mirrors MUI
/// lab's `<TabContext>`; renders no element of its own.
#[derive(Props, Clone, PartialEq)]
pub struct TabContextProps {
    /// The value of the selected tab.
    #[props(into)]
    pub value: String,
    /// Prefix of the tab and panel ids. Generated when omitted.
    #[props(into)]
    pub id: Option<String>,
    pub children: Element,
}

#[component]
pub fn TabContext(props: TabContextProps) -> Element {
    let generated = use_unique_id("skeuo-tabs");
    let id = props.id.clone().unwrap_or(generated);
    let mut ctx = use_context_provider(|| TabContextHandle {
        value: Signal::new(props.value.clone()),
        id: Signal::new(id.clone()),
    });
    if *ctx.value.peek() != props.value {
        ctx.value.set(props.value.clone());
    }
    if *ctx.id.peek() != id {
        ctx.id.set(id);
    }
    rsx! {
        {props.children}
    }
}

#[derive(Clone, PartialEq)]
struct TabsState {
    selected: Option<String>,
    id: String,
    orientation: Orientation,
    selection_follows_focus: bool,
    controlled: bool,
    onchange: Option<EventHandler<String>>,
}

#[derive(Clone, Copy, PartialEq)]
struct TabsCtx {
    state: Signal<TabsState>,
    internal: Signal<Option<String>>,
    roving: Roving,
}

impl TabsCtx {
    fn select(mut self, value: String) {
        let st = self.state.peek().clone();
        if st.selected.as_deref() == Some(value.as_str()) {
            return;
        }
        if !st.controlled {
            self.internal.set(Some(value.clone()));
        }
        if let Some(h) = st.onchange {
            h.call(value);
        }
    }
}

/// A segmented control: a shallow well (the tray) holding the tabs, with the
/// selected tab a raised white keycap inside it and the others printed
/// legends. Mirrors MUI's `<Tabs>` (`role="tablist"`).
///
/// Controlled with `value` + `onchange`, uncontrolled with `default_value`;
/// inside a [`TabContext`] the context's value is used when neither is set.
/// Arrow keys move focus between tabs (Home/End jump to the ends); with
/// `selection_follows_focus` they also select.
#[derive(Props, Clone, PartialEq)]
pub struct TabsProps {
    /// Value of the selected [`Tab`].
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    pub onchange: Option<EventHandler<String>>,
    #[props(default)]
    pub variant: TabsVariant,
    #[props(default)]
    pub orientation: Orientation,
    /// Center the tray in its container.
    #[props(default)]
    pub centered: bool,
    /// Ink of the selected tab's legend. Neutral (default) prints it black,
    /// as on a macOS segmented control.
    #[props(default = Color::Neutral)]
    pub color: Color,
    #[props(default)]
    pub size: Size,
    /// Select a tab as soon as the arrow keys focus it.
    #[props(default)]
    pub selection_follows_focus: bool,
    #[props(into)]
    pub aria_label: Option<String>,
    #[props(into)]
    pub aria_labelledby: Option<String>,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn Tabs(props: TabsProps) -> Element {
    let tab_ctx = try_use_context::<TabContextHandle>();
    let generated = use_unique_id("skeuo-tabs");
    let id = tab_ctx.map(|c| c.id.read().clone()).unwrap_or(generated);
    let default_value = use_hook(|| props.default_value.clone());
    let internal = use_signal(|| default_value);
    let controlled = props.value.is_some() || (tab_ctx.is_some() && internal.read().is_none());
    let selected = props
        .value
        .clone()
        .or_else(|| internal.read().clone())
        .or_else(|| tab_ctx.map(|c| c.value.read().clone()));

    let next = TabsState {
        selected,
        id,
        orientation: props.orientation,
        selection_follows_focus: props.selection_follows_focus,
        controlled,
        onchange: props.onchange,
    };
    let roving = use_hook(Roving::new);
    let mut state = use_signal(|| next.clone());
    if *state.peek() != next {
        state.set(next);
    }
    use_context_provider(|| TabsCtx {
        state,
        internal,
        roving,
    });

    let variant = format!("skeuo-tabs--{}", props.variant.as_str());
    let orientation = format!("skeuo-tabs--{}", props.orientation.as_str());
    let size = format!("skeuo-tabs--{}", props.size.as_str());
    let class = classes([
        "skeuo-tabs",
        &variant,
        &orientation,
        &size,
        props.color.class(),
        if props.centered {
            "skeuo-tabs--centered"
        } else {
            ""
        },
        &props.class,
    ]);
    rsx! {
        div { class,
            div {
                class: "skeuo-tabs__tray skeuo-well skeuo-well--shallow",
                role: "tablist",
                aria_orientation: props.orientation.as_str(),
                aria_label: props.aria_label,
                aria_labelledby: props.aria_labelledby,
                {props.children}
            }
        }
    }
}

/// One segment of [`Tabs`]: a printed legend in the tray that becomes a
/// raised white keycap when selected. Mirrors MUI's `<Tab>`.
#[derive(Props, Clone, PartialEq)]
pub struct TabProps {
    /// Identifies the tab; compared with the `value` of [`Tabs`].
    #[props(into)]
    pub value: String,
    #[props(into)]
    pub label: Option<String>,
    pub icon: Option<Element>,
    #[props(default)]
    pub icon_position: IconPosition,
    #[props(default)]
    pub disabled: bool,
    /// Let a long label wrap onto two lines.
    #[props(default)]
    pub wrapped: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
    pub children: Element,
}

#[component]
pub fn Tab(props: TabProps) -> Element {
    let ctx = use_context::<TabsCtx>();
    let st = ctx.state.read().clone();
    let selected = st.selected.as_deref() == Some(props.value.as_str());
    let tab_id = format!("{}-tab-{}", st.id, id_part(&props.value));
    let panel_id = format!("{}-panel-{}", st.id, id_part(&props.value));
    // The selected tab is the tab stop; if none is selected, every tab is.
    let tabindex = if selected || st.selected.is_none() {
        "0"
    } else {
        "-1"
    };

    let key = props.value.clone();
    use_drop({
        let key = key.clone();
        move || ctx.roving.unregister(&key)
    });

    let has_icon = props.icon.is_some();
    let icon_pos = format!("skeuo-tab--icon-{}", props.icon_position.as_str());
    let class = classes([
        "skeuo-tab",
        if selected {
            "skeuo-key skeuo-tab--selected"
        } else {
            ""
        },
        if props.icon.is_some() { &icon_pos } else { "" },
        if props.wrapped {
            "skeuo-tab--wrapped"
        } else {
            ""
        },
        &props.class,
    ]);

    let value = props.value.clone();
    let onclick = move |e: MouseEvent| {
        ctx.select(value.clone());
        if let Some(h) = &props.onclick {
            h.call(e);
        }
    };
    let disabled = props.disabled;
    let onmounted = {
        let key = key.clone();
        move |e: MountedEvent| ctx.roving.register(&key, disabled, e.data())
    };
    let onkeydown = {
        let key = key.clone();
        move |e: KeyboardEvent| {
            let (prev, next) = match st.orientation {
                Orientation::Horizontal => (Key::ArrowLeft, Key::ArrowRight),
                Orientation::Vertical => (Key::ArrowUp, Key::ArrowDown),
            };
            let k = e.key();
            let target = if k == next {
                ctx.roving.neighbor(Some(&key), 1)
            } else if k == prev {
                ctx.roving.neighbor(Some(&key), -1)
            } else if k == Key::Home {
                ctx.roving.first()
            } else if k == Key::End {
                ctx.roving.last()
            } else {
                return;
            };
            e.prevent_default();
            if let Some(t) = target {
                ctx.roving.focus(&t);
                if ctx.state.peek().selection_follows_focus {
                    ctx.select(t);
                }
            }
        }
    };

    rsx! {
        button {
            class,
            r#type: "button",
            role: "tab",
            id: tab_id,
            aria_selected: if selected { "true" } else { "false" },
            aria_controls: panel_id,
            tabindex,
            disabled: props.disabled,
            onclick,
            onkeydown,
            onmounted,
            if let Some(icon) = props.icon {
                span { class: "skeuo-tab__icon", aria_hidden: "true", {icon} }
            }
            if props.label.is_some() || !has_icon {
                span { class: "skeuo-tab__label",
                    if let Some(label) = props.label {
                        "{label}"
                    }
                    {props.children}
                }
            }
        }
    }
}

/// The content shown for one tab. Must sit inside a [`TabContext`]; it is
/// rendered only while its `value` is the context's value. Mirrors MUI lab's
/// `<TabPanel>` (`role="tabpanel"`, labelled by its tab).
#[derive(Props, Clone, PartialEq)]
pub struct TabPanelProps {
    /// The value of the [`Tab`] this panel belongs to.
    #[props(into)]
    pub value: String,
    /// Keep the children in the DOM (hidden) while the panel is not shown.
    #[props(default)]
    pub keep_mounted: bool,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn TabPanel(props: TabPanelProps) -> Element {
    let ctx = use_context::<TabContextHandle>();
    let id = ctx.id.read().clone();
    let shown = *ctx.value.read() == props.value;
    let panel_id = format!("{id}-panel-{}", id_part(&props.value));
    let tab_id = format!("{id}-tab-{}", id_part(&props.value));
    rsx! {
        div {
            class: classes(["skeuo-tab-panel", &props.class]),
            role: "tabpanel",
            id: panel_id,
            aria_labelledby: tab_id,
            hidden: !shown,
            tabindex: "0",
            if shown || props.keep_mounted {
                {props.children}
            }
        }
    }
}
