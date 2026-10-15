use dioxus::prelude::*;

use crate::{Color, classes};

#[derive(Clone, PartialEq)]
struct BottomNavState {
    value: Option<String>,
    show_labels: bool,
    controlled: bool,
    onchange: Option<EventHandler<String>>,
}

#[derive(Clone, Copy, PartialEq)]
struct BottomNavCtx {
    state: Signal<BottomNavState>,
    internal: Signal<Option<String>>,
}

/// A bar of aluminium with a row of flat keys, one per destination; the
/// selected key is latched down and its icon and legend take the palette
/// ink. Mirrors MUI's `<BottomNavigation>`.
///
/// Controlled with `value` + `onchange`, uncontrolled with `default_value`.
/// Without `show_labels` only the selected action shows its label.
#[derive(Props, Clone, PartialEq)]
pub struct BottomNavigationProps {
    #[props(into)]
    pub value: Option<String>,
    #[props(into)]
    pub default_value: Option<String>,
    pub onchange: Option<EventHandler<String>>,
    #[props(default)]
    pub show_labels: bool,
    #[props(default)]
    pub color: Color,
    #[props(default = "Navigation".to_string(), into)]
    pub aria_label: String,
    #[props(default, into)]
    pub class: String,
    pub children: Element,
}

#[component]
pub fn BottomNavigation(props: BottomNavigationProps) -> Element {
    let default_value = use_hook(|| props.default_value.clone());
    let internal = use_signal(|| default_value);
    let next = BottomNavState {
        value: props.value.clone().or_else(|| internal.read().clone()),
        show_labels: props.show_labels,
        controlled: props.value.is_some(),
        onchange: props.onchange,
    };
    let mut state = use_signal(|| next.clone());
    if *state.peek() != next {
        state.set(next);
    }
    use_context_provider(|| BottomNavCtx { state, internal });
    rsx! {
        nav {
            class: classes([
                "skeuo-bottom-nav skeuo-chassis skeuo-elev-1",
                props.color.class(),
                &props.class,
            ]),
            aria_label: props.aria_label,
            {props.children}
        }
    }
}

/// One key of a [`BottomNavigation`] bar: an icon over a printed label.
/// Mirrors MUI's `<BottomNavigationAction>`.
#[derive(Props, Clone, PartialEq)]
pub struct BottomNavigationActionProps {
    #[props(into)]
    pub value: String,
    #[props(into)]
    pub label: Option<String>,
    pub icon: Option<Element>,
    /// Always show the label, whatever the bar's `show_labels`.
    pub show_label: Option<bool>,
    #[props(default)]
    pub disabled: bool,
    #[props(default, into)]
    pub class: String,
    pub onclick: Option<EventHandler<MouseEvent>>,
}

#[component]
pub fn BottomNavigationAction(props: BottomNavigationActionProps) -> Element {
    let mut ctx = use_context::<BottomNavCtx>();
    let st = ctx.state.read().clone();
    let selected = st.value.as_deref() == Some(props.value.as_str());
    let show_label = props.show_label.unwrap_or(st.show_labels) || selected;
    let value = props.value.clone();
    let hidden_label = if show_label {
        None
    } else {
        props.label.clone()
    };
    rsx! {
        button {
            class: classes([
                "skeuo-key skeuo-key--pressable skeuo-bottom-nav__action",
                if selected { "skeuo-key--down skeuo-bottom-nav__action--selected" } else { "" },
                if show_label { "" } else { "skeuo-bottom-nav__action--icon-only" },
                &props.class,
            ]),
            r#type: "button",
            aria_current: selected.then_some("page"),
            aria_label: hidden_label,
            disabled: props.disabled,
            onclick: move |e| {
                let st = ctx.state.peek().clone();
                if st.value.as_deref() != Some(value.as_str()) {
                    if !st.controlled {
                        ctx.internal.set(Some(value.clone()));
                    }
                    if let Some(h) = st.onchange {
                        h.call(value.clone());
                    }
                }
                if let Some(h) = &props.onclick {
                    h.call(e);
                }
            },
            if let Some(icon) = props.icon {
                span { class: "skeuo-bottom-nav__icon", aria_hidden: "true", {icon} }
            }
            if show_label {
                if let Some(label) = props.label {
                    span { class: "skeuo-bottom-nav__label", "{label}" }
                }
            }
        }
    }
}
