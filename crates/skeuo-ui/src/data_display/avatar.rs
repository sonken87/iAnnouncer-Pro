use dioxus::prelude::*;

use super::is_empty;
use crate::{Color, Icon, IconName, Size, classes};

/// Avatar shape, mirroring MUI's `variant` names.
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
pub enum AvatarVariant {
    #[default]
    Circular,
    Rounded,
    Square,
}

impl AvatarVariant {
    fn as_str(self) -> &'static str {
        match self {
            AvatarVariant::Circular => "circular",
            AvatarVariant::Rounded => "rounded",
            AvatarVariant::Square => "square",
        }
    }
}

/// A person or thing at a glance. Mirrors MUI's `<Avatar>`.
///
/// Without an image it is a small keycap with initials or an icon for a
/// legend (white, or filled with `color`). With `src` it is a photo inset
/// into a thin well ring. Falls back to the initials / first letter of `alt`
/// / a person glyph when the image is missing or fails to load.
#[derive(Props, Clone, PartialEq)]
pub struct AvatarProps {
    /// Image URL.
    #[props(into)]
    pub src: Option<String>,
    #[props(into)]
    pub src_set: Option<String>,
    /// Alternative text; also the accessible name of a letter avatar, and
    /// its first letter is the fallback legend.
    #[props(into)]
    pub alt: Option<String>,
    #[props(default)]
    pub variant: AvatarVariant,
    /// Filled keycap color. `None` is the white keycap.
    pub color: Option<Color>,
    /// small 28px · medium 40px · large 56px.
    #[props(default)]
    pub size: Size,
    #[props(default, into)]
    pub class: String,
    #[props(default, into)]
    pub style: String,
    /// Initials or an icon.
    pub children: Element,
}

#[component]
pub fn Avatar(props: AvatarProps) -> Element {
    let mut failed = use_signal(|| false);
    let has_image = props.src.is_some() && !failed();
    let variant = format!("skeuo-avatar--{}", props.variant.as_str());
    let size = format!("skeuo-avatar--{}", props.size.as_str());
    let material = if has_image {
        "skeuo-well skeuo-avatar--img"
    } else if matches!(props.color, Some(c) if c != Color::Neutral) {
        "skeuo-key skeuo-key--filled skeuo-avatar--letter"
    } else {
        "skeuo-key skeuo-avatar--letter"
    };
    let class = classes([
        material,
        "skeuo-avatar",
        &variant,
        &size,
        props.color.map(Color::class).unwrap_or(""),
        &props.class,
    ]);

    if has_image {
        return rsx! {
            span { class, style: super::non_empty(props.style),
                img {
                    class: "skeuo-avatar__img",
                    src: props.src.clone(),
                    srcset: props.src_set.clone(),
                    alt: props.alt.clone().unwrap_or_default(),
                    onerror: move |_| failed.set(true),
                }
            }
        };
    }

    let fallback_letter = props
        .alt
        .as_ref()
        .and_then(|a| a.chars().next())
        .map(|c| c.to_uppercase().to_string());
    let has_children = !is_empty(&props.children);
    rsx! {
        span {
            class,
            style: super::non_empty(props.style),
            role: if props.alt.is_some() { "img" },
            aria_label: props.alt.clone(),
            if has_children {
                {props.children}
            } else if let Some(letter) = fallback_letter {
                "{letter}"
            } else {
                Icon { name: IconName::Person, class: "skeuo-avatar__fallback" }
            }
        }
    }
}

/// Overlap between grouped avatars, as in MUI's AvatarGroup `spacing`.
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub enum AvatarGroupSpacing {
    #[default]
    Medium,
    Small,
    /// Overlap in px.
    Px(f32),
}

/// A stack of overlapping avatars with a "+N" surplus cap. Mirrors MUI's
/// `<AvatarGroup>`.
///
/// Dioxus cannot count an `Element`'s children, so the avatars are passed as
/// a list in `avatars` instead of as children. At most `max` caps are shown
/// (the last one being the surplus); `total` overrides the count used for
/// the surplus.
#[derive(Props, Clone, PartialEq)]
pub struct AvatarGroupProps {
    pub avatars: Vec<Element>,
    #[props(default = 5)]
    pub max: usize,
    pub total: Option<usize>,
    #[props(default)]
    pub spacing: AvatarGroupSpacing,
    /// Shape of the surplus avatar.
    #[props(default)]
    pub variant: AvatarVariant,
    /// Size of the surplus avatar (match it to the grouped ones).
    #[props(default)]
    pub size: Size,
    #[props(default, into)]
    pub class: String,
}

#[component]
pub fn AvatarGroup(props: AvatarGroupProps) -> Element {
    // Same arithmetic as MUI.
    let count = props.avatars.len();
    let total = props.total.unwrap_or(count);
    let mut clamped_max = props.max.max(2);
    if total == clamped_max {
        clamped_max += 1;
    }
    clamped_max = clamped_max.min(total + 1);
    let shown = count.min(clamped_max - 1);
    let extra = total
        .saturating_sub(clamped_max)
        .max(total.saturating_sub(shown));

    let overlap = match props.spacing {
        AvatarGroupSpacing::Medium => 8.0,
        AvatarGroupSpacing::Small => 16.0,
        AvatarGroupSpacing::Px(px) => px,
    };
    let style = format!("--avatar-overlap: {overlap}px");
    let class = classes(["skeuo-avatar-group", &props.class]);
    rsx! {
        div { class, style, role: "group",
            for (i, avatar) in props.avatars.iter().take(shown).enumerate() {
                span { key: "{i}", class: "skeuo-avatar-group__item", {avatar.clone()} }
            }
            if extra > 0 {
                span { class: "skeuo-avatar-group__item",
                    Avatar {
                        variant: props.variant,
                        size: props.size,
                        alt: format!("{extra} more"),
                        class: "skeuo-avatar--surplus",
                        "+{extra}"
                    }
                }
            }
        }
    }
}
