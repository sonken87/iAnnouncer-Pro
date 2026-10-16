use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

fn assert_all(html: &str, needles: &[&str]) {
    for needle in needles {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
}

// ---------------------------------------------------------------- Typography

#[test]
fn typography_default_is_body1_paragraph() {
    let html = render(|| rsx! { Typography { "Hello" } });
    assert!(
        html.starts_with(
            r#"<p class="skeuo-typography skeuo-typography--body1 skeuo-typography--text-primary">Hello</p>"#
        ),
        "{html}"
    );
}

#[test]
fn typography_variant_mapping_and_props() {
    let html = render(|| {
        rsx! {
            Typography { variant: TypographyVariant::H2, gutter_bottom: true, "Title" }
            Typography { variant: TypographyVariant::Subtitle1, "Sub" }
            Typography { variant: TypographyVariant::Caption, color: TypographyColor::TextSecondary, "Cap" }
            Typography { variant: TypographyVariant::Overline, no_wrap: true, align: TypographyAlign::Center, "Over" }
            Typography { variant: TypographyVariant::Caption, paragraph: true, "Para" }
            Typography { color: Color::Error, class: "mine", id: "t1", "Ink" }
        }
    });
    assert_all(
        &html,
        &[
            r#"<h2 class="skeuo-typography skeuo-typography--h2 skeuo-typography--text-primary skeuo-typography--gutter-bottom">Title</h2>"#,
            r#"<h6 class="skeuo-typography skeuo-typography--subtitle1"#,
            r#"<span class="skeuo-typography skeuo-typography--caption skeuo-typography--text-secondary">Cap</span>"#,
            "skeuo-typography--align-center skeuo-typography--no-wrap",
            r#"<p class="skeuo-typography skeuo-typography--caption skeuo-typography--text-primary skeuo-typography--paragraph">Para</p>"#,
            r#"class="skeuo-typography skeuo-typography--body1 skeuo-typography--ink skeuo-color-error mine""#,
            r#"id="t1""#,
        ],
    );
}

#[test]
fn typography_component_overrides_tag() {
    let html = render(|| {
        rsx! {
            Typography { variant: TypographyVariant::H1, component: "div", "A" }
            Typography { variant: TypographyVariant::Body2, component: "label", "B" }
            Typography { component: "nonsense", "C" }
        }
    });
    assert_all(
        &html,
        &[
            r#"<div class="skeuo-typography skeuo-typography--h1"#,
            r#"<label class="skeuo-typography skeuo-typography--body2"#,
            r#"<span class="skeuo-typography skeuo-typography--body1"#,
        ],
    );
    assert!(!html.contains("<h1"), "{html}");
}

// -------------------------------------------------------------------- Avatar

#[test]
fn avatar_letter_is_a_keycap() {
    let html = render(|| {
        rsx! {
            Avatar { alt: "Jamie", "JA" }
            Avatar { color: Color::Primary, variant: AvatarVariant::Rounded, size: Size::Large, "MK" }
        }
    });
    assert_all(
        &html,
        &[
            r#"class="skeuo-key skeuo-avatar--letter skeuo-avatar skeuo-avatar--circular skeuo-avatar--medium""#,
            r#"role="img""#,
            r#"aria-label="Jamie""#,
            ">JA<",
            r#"class="skeuo-key skeuo-key--filled skeuo-avatar--letter skeuo-avatar skeuo-avatar--rounded skeuo-avatar--large skeuo-color-primary""#,
            ">MK<",
        ],
    );
    // No accessible name given: no role.
    assert_eq!(html.matches("role=").count(), 1, "{html}");
}

#[test]
fn avatar_image_and_fallbacks() {
    let html = render(|| {
        rsx! {
            Avatar { src: "a.png", alt: "Photo", variant: AvatarVariant::Square }
            Avatar { alt: "zoe" }
            Avatar {}
        }
    });
    assert_all(
        &html,
        &[
            r#"class="skeuo-well skeuo-avatar--img skeuo-avatar skeuo-avatar--square skeuo-avatar--medium""#,
            r#"src="a.png""#,
            r#"alt="Photo""#,
            ">Z<",
            "skeuo-avatar__fallback",
        ],
    );
}

#[test]
fn avatar_group_max_and_surplus() {
    fn avatars(n: usize) -> Vec<Element> {
        (0..n).map(|i| rsx! { Avatar { "{i}" } }).collect()
    }
    let html = render(|| rsx! { AvatarGroup { max: 4, avatars: avatars(6) } });
    assert!(html.contains(r#"role="group""#), "{html}");
    // 3 shown + the surplus cap.
    assert_eq!(
        html.matches("skeuo-avatar-group__item").count(),
        4,
        "{html}"
    );
    assert!(html.contains(">+3<"), "{html}");
    assert!(html.contains(r#"aria-label="3 more""#), "{html}");

    let html = render(|| rsx! { AvatarGroup { avatars: avatars(3), total: 20 } });
    assert_eq!(
        html.matches("skeuo-avatar-group__item").count(),
        4,
        "{html}"
    );
    assert!(html.contains(">+17<"), "{html}");

    let html = render(|| rsx! { AvatarGroup { max: 4, avatars: avatars(4) } });
    assert_eq!(
        html.matches("skeuo-avatar-group__item").count(),
        4,
        "{html}"
    );
    assert!(!html.contains("+"), "{html}");

    let html = render(|| {
        rsx! { AvatarGroup { spacing: AvatarGroupSpacing::Px(12.0), avatars: avatars(2) } }
    });
    assert!(html.contains("--avatar-overlap: 12px"), "{html}");
}

// --------------------------------------------------------------------- Badge

#[test]
fn badge_is_an_led_with_capped_count() {
    let html = render(|| {
        rsx! {
            Badge { badge_content: 4, "A" }
            Badge { badge_content: 120, color: Color::Error, "B" }
            Badge { badge_content: 1200, max: 999, "C" }
            Badge { badge_content: "new", "D" }
        }
    });
    assert_all(
        &html,
        &[
            r#"<span class="skeuo-badge">A<span class="skeuo-led skeuo-badge__badge skeuo-badge__badge--standard skeuo-badge__badge--top-right skeuo-badge__badge--rectangular skeuo-color-primary">4</span></span>"#,
            "skeuo-color-error\">99+<",
            ">999+<",
            ">new<",
        ],
    );
    assert!(!html.contains("invisible"), "{html}");
}

#[test]
fn badge_visibility_rules() {
    let html = render(|| {
        rsx! {
            Badge { badge_content: 0, "zero" }
            Badge { badge_content: 0, show_zero: true, "shown" }
            Badge { badge_content: 5, invisible: true, "hidden" }
            Badge { "empty" }
            Badge { variant: BadgeVariant::Dot, "dot" }
        }
    });
    // zero, hidden and empty are invisible; show_zero and dot are not.
    assert_eq!(
        html.matches("skeuo-badge__badge--invisible").count(),
        3,
        "{html}"
    );
    assert_eq!(html.matches(r#"aria-hidden="true""#).count(), 3, "{html}");
    assert!(html.contains("shown<span"), "{html}");
    assert!(html.contains(">0</span>"), "{html}");
    assert!(
        html.contains("skeuo-badge__badge--dot skeuo-badge__badge--top-right"),
        "{html}"
    );
}

#[test]
fn badge_anchor_and_overlap() {
    let html = render(|| {
        rsx! {
            Badge {
                badge_content: 1,
                anchor_origin: BadgeAnchorOrigin::BOTTOM_LEFT,
                overlap: BadgeOverlap::Circular,
                class: "mine",
                "x"
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"class="skeuo-badge mine""#,
            "skeuo-badge__badge--bottom-left skeuo-badge__badge--circular",
        ],
    );
}

// ---------------------------------------------------------------------- Chip

#[test]
fn chip_static_variants() {
    let html = render(|| {
        rsx! {
            Chip { label: "Plain" }
            Chip { label: "Blue", color: Color::Primary }
            Chip { label: "Out", variant: ChipVariant::Outlined, color: Color::Error, size: Size::Small }
        }
    });
    assert_all(
        &html,
        &[
            r#"<div class="skeuo-key skeuo-chip skeuo-chip--filled skeuo-color-neutral skeuo-chip--medium"><span class="skeuo-chip__label">Plain</span></div>"#,
            r#"class="skeuo-key skeuo-key--filled skeuo-chip skeuo-chip--filled skeuo-color-primary skeuo-chip--medium""#,
            r#"class="skeuo-key skeuo-chip skeuo-chip--outlined skeuo-color-error skeuo-chip--small""#,
        ],
    );
    assert!(!html.contains("<button"), "{html}");
}

#[test]
fn chip_clickable_and_deletable() {
    let html = render(|| {
        rsx! {
            Chip { label: "Tap", onclick: move |_| {} }
            Chip { label: "Tag", on_delete: move |_| {}, icon: rsx! { Icon { name: IconName::Star } } }
            Chip { label: "Off", onclick: move |_| {}, on_delete: move |_| {}, disabled: true }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-key--pressable skeuo-key--focus-within skeuo-chip--clickable",
            r#"<button class="skeuo-chip__action" type="button"><span class="skeuo-chip__label">Tap</span></button>"#,
            "skeuo-chip--deletable",
            r#"aria-label="Delete Tag""#,
            "skeuo-chip__icon",
            "skeuo-chip--disabled",
            r#"aria-disabled="true""#,
        ],
    );
    assert_eq!(html.matches("disabled=true").count(), 2, "{html}");
}

#[test]
fn chip_with_avatar() {
    let html = render(|| {
        rsx! { Chip { label: "Mika", avatar: rsx! { Avatar { "M" } } } }
    });
    assert!(
        html.contains(
            r#"<span class="skeuo-chip__avatar"><span class="skeuo-key skeuo-avatar--letter"#
        ),
        "{html}"
    );
}

// ------------------------------------------------------------------- Divider

#[test]
fn divider_is_an_engraved_rule() {
    let html = render(|| {
        rsx! {
            Divider {}
            Divider { variant: DividerVariant::Inset }
            Divider { orientation: Orientation::Vertical, flex_item: true, variant: DividerVariant::Middle }
        }
    });
    assert_all(
        &html,
        &[
            r#"<hr class="skeuo-rule skeuo-divider skeuo-divider--full-width skeuo-divider--horizontal"/>"#,
            "skeuo-divider--inset",
            r#"<hr class="skeuo-rule skeuo-rule--vertical skeuo-divider skeuo-divider--middle skeuo-divider--vertical skeuo-divider--flex-item" aria-orientation="vertical"/>"#,
        ],
    );
}

#[test]
fn divider_with_text() {
    let html = render(|| rsx! { Divider { text_align: DividerTextAlign::Left, "OR" } });
    assert_all(
        &html,
        &[
            r#"role="separator""#,
            "skeuo-divider--with-children skeuo-divider--text-left",
            r#"<span class="skeuo-divider__text skeuo-label">OR</span>"#,
        ],
    );
    assert_eq!(
        html.matches("skeuo-rule skeuo-divider__line").count(),
        2,
        "{html}"
    );
    assert!(!html.contains("<hr"), "{html}");
}

// ---------------------------------------------------------------------- List

#[test]
fn list_family_markup() {
    let html = render(|| {
        rsx! {
            List {
                dense: true,
                aria_label: "Nav",
                subheader: rsx! { ListSubheader { "Section" } },
                ListItem { disable_padding: true,
                    ListItemButton { selected: true,
                        ListItemIcon { Icon { name: IconName::Home } }
                        ListItemText { primary: "Home", secondary: "Start" }
                    }
                }
                ListItem { divider: true, secondary_action: rsx! { "⋮" },
                    ListItemAvatar { Avatar { "A" } }
                    ListItemText { "Child primary" }
                }
                ListItem { ListItemButton { disabled: true, ListItemText { inset: true, primary: "Off" } } }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<ul class="skeuo-list skeuo-color-primary skeuo-list--dense skeuo-list--with-subheader" aria-label="Nav">"#,
            r#"<li class="skeuo-list-subheader skeuo-label skeuo-list-subheader--sticky">Section</li>"#,
            r#"class="skeuo-list-item skeuo-list-item--dense skeuo-list-item--no-padding""#,
            r#"class="skeuo-list-item-button skeuo-list-item--dense skeuo-list-item-button--selected""#,
            r#"aria-current="true""#,
            r#"<span class="skeuo-list-item-icon">"#,
            r#"<span class="skeuo-list-item-text skeuo-list-item-text--multiline"><span class="skeuo-list-item-text__primary">Home</span><span class="skeuo-list-item-text__secondary skeuo-caption">Start</span></span>"#,
            "skeuo-list-item--divider skeuo-list-item--with-action",
            r#"<span class="skeuo-list-item__secondary-action">⋮</span>"#,
            r#"<span class="skeuo-list-item-avatar">"#,
            ">Child primary<",
            "skeuo-list-item-text--inset",
            "disabled=true",
        ],
    );
    assert_eq!(html.matches("aria-current").count(), 1, "{html}");
}

#[test]
fn list_subheader_color_and_flags() {
    let html = render(|| {
        rsx! {
            ListSubheader { color: Color::Success, disable_sticky: true, inset: true, id: "s", "H" }
        }
    });
    assert!(
        html.contains(r#"<li class="skeuo-list-subheader skeuo-label skeuo-color-success skeuo-list-subheader--colored skeuo-list-subheader--inset" id="s">H</li>"#),
        "{html}"
    );
}

// --------------------------------------------------------------------- Table

#[test]
fn table_family_markup() {
    let html = render(|| {
        rsx! {
            TableContainer { max_height: "200px",
                Table { size: Size::Small, sticky_header: true, aria_label: "Clips",
                    TableHead {
                        TableRow {
                            TableCell { sort_direction: SortDirection::Desc,
                                TableSortLabel { active: true, direction: SortDirection::Desc, "Name" }
                            }
                            TableCell { align: TableCellAlign::Right, "Plays" }
                        }
                    }
                    TableBody {
                        TableRow { hover: true, selected: true,
                            TableCell { variant: TableCellVariant::Head, "Intro" }
                            TableCell { align: TableCellAlign::Right, "12" }
                        }
                        TableRow {
                            TableCell { padding: TableCellPadding::Checkbox, "x" }
                            TableCell { col_span: 2, padding: TableCellPadding::None, "y" }
                        }
                    }
                    TableFooter {
                        TableRow { TableCell { "Total" } }
                    }
                }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<div class="skeuo-chassis skeuo-elev-1 skeuo-table-container" style="max-height: 200px;">"#,
            r#"<table class="skeuo-table skeuo-color-primary skeuo-table--small skeuo-table--sticky-header" aria-label="Clips">"#,
            r#"<thead class="skeuo-table-head">"#,
            r#"<tr class="skeuo-table-row skeuo-table-row--head">"#,
            r#"<th class="skeuo-table-cell skeuo-table-cell--head skeuo-table-cell--small skeuo-table-cell--sticky" scope="col" aria-sort="descending">"#,
            "skeuo-table-sort-label skeuo-table-sort-label--active skeuo-table-sort-label--desc",
            "skeuo-table-sort-label__icon",
            "skeuo-table-cell--head skeuo-table-cell--align-right",
            r#"<tr class="skeuo-table-row skeuo-table-row--hover skeuo-table-row--selected" aria-selected="true">"#,
            r#"<th class="skeuo-table-cell skeuo-table-cell--head skeuo-table-cell--small" scope="row">Intro</th>"#,
            r#"<td class="skeuo-table-cell skeuo-table-cell--body skeuo-table-cell--align-right skeuo-table-cell--small">12</td>"#,
            "skeuo-table-cell--padding-checkbox",
            r#"colspan="2""#,
            "skeuo-table-cell--padding-none",
            r#"<tfoot class="skeuo-table-footer"><tr class="skeuo-table-row skeuo-table-row--footer"><td class="skeuo-table-cell skeuo-table-cell--footer skeuo-table-cell--small">Total</td>"#,
        ],
    );
}

#[test]
fn table_sort_label_inactive_hides_icon_when_asked() {
    let html = render(|| {
        rsx! {
            TableSortLabel { "A" }
            TableSortLabel { hide_sort_icon: true, "B" }
            TableSortLabel { hide_sort_icon: true, active: true, "C" }
        }
    });
    assert_eq!(
        html.matches("skeuo-table-sort-label__icon").count(),
        2,
        "{html}"
    );
    assert_eq!(html.matches(r#"type="button""#).count(), 3, "{html}");
    assert!(html.contains("skeuo-table-sort-label--asc"), "{html}");
}

#[test]
fn table_container_elevation() {
    let html = render(|| rsx! { TableContainer { elevation: 0, Table { "" } } });
    assert!(html.contains("skeuo-chassis skeuo-elev-0"), "{html}");
    assert!(html.contains("skeuo-table--medium"), "{html}");
}

// ------------------------------------------------------------------- Tooltip

#[test]
fn tooltip_uncontrolled_and_forced_open() {
    let html = render(|| {
        rsx! {
            Tooltip { title: "Delete", id: "tip", button { aria_describedby: "tip", "x" } }
            Tooltip { title: "Shown", placement: Placement::Top, arrow: true, open: true, span { "y" } }
            Tooltip { title: "Never", open: false, disable_interactive: true, span { "z" } }
            Tooltip { title: "", span { "empty" } }
        }
    });
    assert_all(
        &html,
        &[
            r#"<span class="skeuo-tooltip skeuo-tooltip--auto skeuo-tooltip--bottom skeuo-tooltip--interactive">"#,
            r#"<span class="skeuo-tooltip__popper" role="tooltip" id="tip"><span class="skeuo-tooltip__bubble skeuo-chassis skeuo-chassis--float skeuo-elev-3">Delete</span></span>"#,
            "skeuo-tooltip skeuo-tooltip--open skeuo-tooltip--top skeuo-tooltip--arrow skeuo-tooltip--interactive",
            r#"<span class="skeuo-tooltip__arrow" aria-hidden="true"></span>"#,
            r#"class="skeuo-tooltip skeuo-tooltip--closed skeuo-tooltip--bottom""#,
        ],
    );
    // An empty title renders no bubble.
    assert_eq!(html.matches(r#"role="tooltip""#).count(), 3, "{html}");
    // Generated ids are used when none is given.
    assert!(html.contains(r#"id="skeuo-tooltip-"#), "{html}");
}

// ----------------------------------------------------------------- ImageList

#[test]
fn image_list_layout_and_items() {
    let html = render(|| {
        rsx! {
            ImageList { variant: ImageListVariant::Quilted, cols: 4, gap: 6, row_height: 80,
                ImageListItem { cols: 2, rows: 2,
                    img { src: "a.png", alt: "A" }
                    ImageListItemBar { title: "A", subtitle: "by B", action_icon: rsx! { "★" } }
                }
                ImageListItem {
                    img { src: "b.png", alt: "B" }
                    ImageListItemBar { title: "B", position: ImageListItemBarPosition::Below, action_position_left: true }
                }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<ul class="skeuo-image-list skeuo-image-list--quilted" style="--image-list-cols: 4; --image-list-gap: 6px; grid-auto-rows: 80px;">"#,
            r#"<li class="skeuo-image-list-item" style="grid-column-end: span 2; grid-row-end: span 2;">"#,
            r#"<span class="skeuo-well skeuo-image-list-item__rim" aria-hidden="true"></span>"#,
            r#"class="skeuo-image-list-item-bar skeuo-image-list-item-bar--bottom""#,
            r#"<span class="skeuo-image-list-item-bar__title">A</span><span class="skeuo-image-list-item-bar__subtitle">by B</span>"#,
            r#"<span class="skeuo-image-list-item-bar__action">★</span>"#,
            "skeuo-image-list-item-bar--below skeuo-image-list-item-bar--action-left",
        ],
    );
}

#[test]
fn image_list_masonry_items_do_not_span() {
    let html = render(|| {
        rsx! {
            ImageList { variant: ImageListVariant::Masonry, cols: 3,
                ImageListItem { cols: 2, img { src: "a.png" } }
            }
        }
    });
    assert!(html.contains("skeuo-image-list--masonry"), "{html}");
    assert!(
        html.contains("--image-list-cols: 3; --image-list-gap: 4px; \""),
        "{html}"
    );
    assert!(!html.contains("span 2"), "{html}");
}
