use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

// ---------------------------------------------------------------- Paper

#[test]
fn elevation_maps_onto_chassis_levels() {
    let pairs = [
        (0, 0),
        (1, 1),
        (2, 1),
        (3, 2),
        (6, 2),
        (7, 3),
        (12, 3),
        (13, 4),
        (24, 4),
    ];
    for (e, l) in pairs {
        assert_eq!(chassis_level(e), l, "elevation {e}");
    }
}

#[test]
fn paper_default_classes() {
    let html = render(|| rsx! { Paper { "Hi" } });
    assert!(
        html.contains(r#"class="skeuo-chassis skeuo-elev-1 skeuo-paper skeuo-paper--elevation""#),
        "{html}"
    );
    assert!(html.contains("Hi"), "{html}");
}

#[test]
fn paper_floating_square_and_attributes() {
    let html = render(|| {
        rsx! { Paper { elevation: 16, square: true, class: "x", id: "p1", style: "padding:4px", "" } }
    });
    assert!(
        html.contains("skeuo-chassis skeuo-elev-4 skeuo-chassis--float skeuo-chassis--square"),
        "{html}"
    );
    assert!(html.contains(" x\""), "{html}");
    assert!(html.contains(r#"id="p1""#), "{html}");
    assert!(html.contains("padding:4px"), "{html}");
}

#[test]
fn paper_outlined_is_flush() {
    let html = render(|| rsx! { Paper { variant: PaperVariant::Outlined, elevation: 8, "" } });
    assert!(html.contains("skeuo-elev-0"), "{html}");
    assert!(html.contains("skeuo-paper--outlined"), "{html}");
    assert!(!html.contains("--float"), "{html}");
}

// ---------------------------------------------------------------- Card

#[test]
fn card_parts_render() {
    let html = render(|| {
        rsx! {
            Card { raised: true,
                CardHeader {
                    avatar: rsx! { span { "AV" } },
                    title: "Title",
                    subheader: "Sub",
                    action: rsx! { button { "act" } },
                }
                CardMedia { image: "a.png", height: "140px", title: "Pic" }
                CardContent { "Body" }
                CardActions { button { "OK" } }
            }
        }
    });
    assert!(html.contains("skeuo-elev-2"), "raised → level 2: {html}");
    assert!(html.contains("skeuo-card"), "{html}");
    assert!(html.contains("skeuo-card-header__avatar"), "{html}");
    assert!(
        html.contains(r#"<span class="skeuo-card-header__title">Title</span>"#),
        "{html}"
    );
    assert!(html.contains("skeuo-card-header__subheader"), "{html}");
    assert!(html.contains("skeuo-card-header__action"), "{html}");
    assert!(html.contains("background-image:url("), "{html}");
    assert!(html.contains("a.png"), "{html}");
    assert!(html.contains("height:140px;"), "{html}");
    assert!(html.contains(r#"role="img""#), "{html}");
    assert!(html.contains(r#"aria-label="Pic""#), "{html}");
    assert!(html.contains(r#"class="skeuo-card-content""#), "{html}");
    assert!(
        html.contains(r#"class="skeuo-card-actions skeuo-card-actions--spacing""#),
        "{html}"
    );
}

#[test]
fn card_media_img_and_action_area() {
    let html = render(|| {
        rsx! {
            Card {
                CardActionArea {
                    CardMedia { component: "img", image: "b.png", alt: "Bee" }
                }
                CardActions { disable_spacing: true, "x" }
            }
        }
    });
    assert!(html.contains("<img"), "{html}");
    assert!(html.contains(r#"src="b.png""#), "{html}");
    assert!(html.contains(r#"alt="Bee""#), "{html}");
    assert!(html.contains(r#"class="skeuo-card-action-area""#), "{html}");
    assert!(html.contains(r#"type="button""#), "{html}");
    assert!(html.contains(r#"class="skeuo-card-actions""#), "{html}");
}

// ---------------------------------------------------------------- Accordion

#[test]
fn accordion_collapsed_by_default() {
    let html = render(|| {
        rsx! {
            Accordion {
                AccordionSummary { expand_icon: rsx! { Icon { name: IconName::ChevronDown } }, "Head" }
                AccordionDetails { "Body" }
            }
        }
    });
    assert!(html.contains("skeuo-accordion--rounded"), "{html}");
    assert!(html.contains("skeuo-accordion--gutters"), "{html}");
    assert!(!html.contains("skeuo-accordion--expanded"), "{html}");
    assert!(html.contains(r#"aria-expanded="false""#), "{html}");
    assert!(html.contains(r#"role="region""#), "{html}");
    assert!(!html.contains("skeuo-accordion__collapse--open"), "{html}");
    assert!(
        html.contains("skeuo-accordion-summary__expand-icon"),
        "{html}"
    );
    assert!(html.contains("<h3"), "{html}");
}

#[test]
fn accordion_ids_link_summary_and_region() {
    let html = render(|| {
        rsx! {
            Accordion { default_expanded: true,
                AccordionSummary { "Head" }
                AccordionDetails { "Body" }
            }
        }
    });
    let attr = |name: &str| -> String {
        let start = html.find(&format!("{name}=\"")).expect(name) + name.len() + 2;
        html[start..start + html[start..].find('"').unwrap()].to_string()
    };
    let controls = attr("aria-controls");
    let labelled = attr("aria-labelledby");
    assert!(html.contains(&format!(r#"id="{controls}""#)), "{html}");
    assert!(html.contains(&format!(r#"id="{labelled}""#)), "{html}");
    assert_ne!(controls, labelled);
}

#[test]
fn accordion_uncontrolled_default_expanded() {
    let html = render(|| {
        rsx! {
            Accordion { default_expanded: true,
                AccordionSummary { "Head" }
                AccordionDetails { "Body" }
                AccordionActions { "Act" }
            }
        }
    });
    assert!(html.contains("skeuo-accordion--expanded"), "{html}");
    assert!(html.contains(r#"aria-expanded="true""#), "{html}");
    assert_eq!(
        html.matches("skeuo-accordion__collapse--open").count(),
        2,
        "details + actions open: {html}"
    );
    assert!(
        html.contains("skeuo-accordion-actions skeuo-accordion-actions--spacing"),
        "{html}"
    );
}

#[test]
fn accordion_controlled_overrides_default() {
    let html = render(|| {
        rsx! {
            Accordion { expanded: false, default_expanded: true,
                AccordionSummary { "Head" }
                AccordionDetails { "Body" }
            }
            Accordion { expanded: true,
                AccordionSummary { "Head 2" }
                AccordionDetails { "Body 2" }
            }
        }
    });
    let first = html.find("Head<").unwrap();
    let second = html.find("Head 2").unwrap();
    assert!(html[..first].contains(r#"aria-expanded="false""#), "{html}");
    assert!(
        html[first..second].contains(r#"aria-expanded="true""#),
        "{html}"
    );
}

#[test]
fn accordion_disabled_square_no_gutters() {
    let html = render(|| {
        rsx! {
            Accordion { disabled: true, square: true, disable_gutters: true,
                AccordionSummary { "Head" }
                AccordionDetails { "Body" }
            }
        }
    });
    assert!(html.contains("skeuo-chassis--square"), "{html}");
    assert!(!html.contains("skeuo-accordion--rounded"), "{html}");
    assert!(!html.contains("skeuo-accordion--gutters"), "{html}");
    assert!(html.contains("skeuo-accordion--disabled"), "{html}");
    assert!(html.contains("disabled"), "{html}");
    assert!(
        html.contains("<button") && html.contains(" disabled"),
        "{html}"
    );
}

// ---------------------------------------------------------------- AppBar

#[test]
fn appbar_primary_is_colored_plate() {
    let html = render(|| rsx! { AppBar { Toolbar { "Title" } } });
    assert!(html.contains("<header"), "{html}");
    assert!(
        html.contains("skeuo-key skeuo-key--filled skeuo-appbar--colored skeuo-color-primary"),
        "{html}"
    );
    assert!(html.contains("skeuo-appbar--fixed"), "{html}");
    assert!(
        html.contains(r#"class="skeuo-toolbar skeuo-toolbar--regular skeuo-toolbar--gutters""#),
        "{html}"
    );
}

#[test]
fn appbar_default_is_square_chassis() {
    let html = render(|| {
        rsx! {
            AppBar { position: AppBarPosition::Sticky, color: AppBarColor::Default, elevation: 0,
                Toolbar { variant: ToolbarVariant::Dense, disable_gutters: true, "T" }
            }
        }
    });
    assert!(
        html.contains("skeuo-chassis skeuo-elev-0 skeuo-chassis--square skeuo-paper"),
        "{html}"
    );
    assert!(html.contains("skeuo-appbar--sticky"), "{html}");
    assert!(html.contains("skeuo-appbar--default"), "{html}");
    assert!(
        html.contains(r#"class="skeuo-toolbar skeuo-toolbar--dense""#),
        "{html}"
    );
}

#[test]
fn appbar_color_from_palette_and_transparent() {
    let html = render(|| {
        rsx! {
            AppBar { position: AppBarPosition::Static, color: Color::Success, "a" }
            AppBar { position: AppBarPosition::Static, color: Color::Neutral, "b" }
            AppBar { position: AppBarPosition::Static, color: AppBarColor::Transparent, "c" }
        }
    });
    assert!(html.contains("skeuo-color-success"), "{html}");
    assert!(html.contains("skeuo-appbar--default"), "{html}");
    assert!(
        html.contains(r#"class="skeuo-appbar skeuo-appbar--static skeuo-appbar--transparent""#),
        "{html}"
    );
}

// ---------------------------------------------------------------- Drawer

#[test]
fn temporary_drawer_closed_renders_nothing() {
    let html = render(|| rsx! { Drawer { open: false, "Nav" } });
    assert!(!html.contains("Nav"), "{html}");
}

#[test]
fn temporary_drawer_open() {
    let html = render(|| {
        rsx! { Drawer { open: true, anchor: DrawerAnchor::Right, size: "320px", aria_label: "Nav", "Items" } }
    });
    assert!(
        html.contains("skeuo-drawer skeuo-drawer--temporary skeuo-drawer--right"),
        "{html}"
    );
    assert!(html.contains("--drawer-size:320px;"), "{html}");
    assert!(html.contains("skeuo-scrim skeuo-drawer__scrim"), "{html}");
    assert!(html.contains(r#"role="dialog""#), "{html}");
    assert!(html.contains(r#"aria-modal="true""#), "{html}");
    assert!(html.contains(r#"aria-label="Nav""#), "{html}");
    assert!(html.contains("skeuo-elev-4 skeuo-chassis--float"), "{html}");
    assert!(html.contains("skeuo-drawer__paper--right"), "{html}");
    assert!(html.contains("Items"), "{html}");
}

#[test]
fn drawer_inline_preview_and_hidden_backdrop() {
    let html = render(|| rsx! { Drawer { open: true, inline: true, hide_backdrop: true, "x" } });
    assert!(html.contains("skeuo-drawer--inline"), "{html}");
    assert!(!html.contains("skeuo-scrim"), "{html}");
}

#[test]
fn docked_drawers() {
    let html = render(|| {
        rsx! {
            Drawer { variant: DrawerVariant::Permanent, "perm" }
            Drawer { variant: DrawerVariant::Persistent, open: false, anchor: DrawerAnchor::Top, "pers" }
        }
    });
    assert!(
        html.contains(
            "skeuo-drawer--permanent skeuo-drawer--left skeuo-drawer--docked skeuo-drawer--open"
        ),
        "{html}"
    );
    assert!(
        html.contains(
            "skeuo-drawer--persistent skeuo-drawer--top skeuo-drawer--docked skeuo-drawer--closed"
        ),
        "{html}"
    );
    assert!(
        html.contains("skeuo-elev-0 skeuo-chassis--square"),
        "{html}"
    );
    assert!(!html.contains("role=\"dialog\""), "{html}");
}

// ---------------------------------------------------------------- Layout

#[test]
fn skeuo_box_component_tag() {
    let html = render(|| {
        rsx! {
            SkeuoBox { style: "color:red", "a" }
            SkeuoBox { component: "section", class: "s", "b" }
        }
    });
    assert!(
        html.contains(r#"<div class="skeuo-box" style="color:red">a</div>"#),
        "{html}"
    );
    assert!(
        html.contains(r#"<section class="skeuo-box s">b</section>"#),
        "{html}"
    );
}

#[test]
fn container_classes() {
    let html = render(|| {
        rsx! {
            Container { "a" }
            Container { max_width: ContainerMaxWidth::Sm, fixed: true, disable_gutters: true, "b" }
        }
    });
    assert!(
        html.contains(
            r#"class="skeuo-container skeuo-container--max-lg skeuo-container--gutters""#
        ),
        "{html}"
    );
    assert!(
        html.contains(r#"class="skeuo-container skeuo-container--max-sm skeuo-container--fixed""#),
        "{html}"
    );
}

#[test]
fn stack_spacing_and_alignment() {
    let html = render(|| {
        rsx! {
            Stack { direction: StackDirection::Row, spacing: 2.0, align_items: "center", use_flex_gap: true,
                span { "a" }
            }
        }
    });
    assert!(
        html.contains(r#"class="skeuo-stack skeuo-stack--row skeuo-stack--gap""#),
        "{html}"
    );
    assert!(html.contains("--stack-spacing:16px;"), "{html}");
    assert!(html.contains("align-items:center;"), "{html}");
}

#[component]
fn Item(n: u32) -> Element {
    rsx! { span { "item{n}" } }
}

#[test]
fn stack_divider_between_loop_items() {
    let html = render(|| {
        rsx! {
            Stack { divider: rsx! { hr { class: "div" } },
                for n in 0..3u32 {
                    span { "item{n}" }
                }
            }
        }
    });
    assert_eq!(html.matches(r#"<hr class="div""#).count(), 2, "{html}");
    let positions: Vec<usize> = ["item0", "div", "item1", "div", "item2"]
        .iter()
        .scan(0usize, |from, needle| {
            let at = *from + html[*from..].find(needle)?;
            *from = at + 1;
            Some(at)
        })
        .collect();
    assert_eq!(positions.len(), 5, "order: {html}");
}

#[test]
fn stack_divider_between_components_and_static_elements() {
    let html = render(|| {
        rsx! {
            Stack { divider: rsx! { hr { class: "div" } },
                Item { n: 1 }
                p { "static" }
                Item { n: 2 }
            }
        }
    });
    assert_eq!(html.matches(r#"<hr class="div""#).count(), 2, "{html}");
    assert!(html.contains("item1") && html.contains("static") && html.contains("item2"));
}

#[test]
fn grid_container_and_items() {
    let html = render(|| {
        rsx! {
            Grid { container: true, spacing: 2.0, columns: 16,
                Grid { size: 8, "a" }
                Grid { size: Responsive::xs(12).md(6), offset: 2, "b" }
                Grid { size: GridSize::Grow, "c" }
            }
        }
    });
    assert!(html.contains("skeuo-grid skeuo-grid--container"), "{html}");
    assert!(
        html.contains("--grid-row-gap:16px;--grid-column-gap:16px;"),
        "{html}"
    );
    assert!(
        html.contains("--grid-parent-columns:16;--grid-parent-gap:16px;"),
        "items read the parent's columns: {html}"
    );
    assert!(html.contains("skeuo-grid--xs-n"), "{html}");
    assert!(html.contains("--grid-size-xs:8;"), "{html}");
    assert!(html.contains("skeuo-grid--md-n"), "{html}");
    assert!(html.contains("--grid-size-md:6;"), "{html}");
    assert!(html.contains("skeuo-grid--xs-offset"), "{html}");
    assert!(html.contains("--grid-offset-xs:2;"), "{html}");
    assert!(html.contains("skeuo-grid--xs-grow"), "{html}");
}

#[test]
fn nested_grid_container_uses_parent_columns_for_itself() {
    let html = render(|| {
        rsx! {
            Grid { container: true, spacing: 1.0,
                Grid { container: true, size: 6, spacing: 3.0, columns: 4,
                    Grid { size: 2, "inner" }
                }
            }
        }
    });
    // The nested container is sized against the outer grid (12 cols, 8px)…
    assert!(
        html.contains("--grid-row-gap:24px;--grid-column-gap:24px;--grid-parent-columns:12;--grid-parent-gap:8px;"),
        "{html}"
    );
    // …and its own items against it (4 cols, 24px).
    assert!(
        html.contains("--grid-parent-columns:4;--grid-parent-gap:24px;"),
        "{html}"
    );
}
