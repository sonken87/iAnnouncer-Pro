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

// ------------------------------------------------------------------ Tabs

#[test]
fn tabs_roles_and_selected_keycap() {
    let html = render(|| {
        rsx! {
            Tabs { value: "b", aria_label: "Views", class: "mine",
                Tab { value: "a", label: "Alpha" }
                Tab { value: "b", label: "Beta" }
                Tab { value: "c", label: "Gamma", disabled: true }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-tabs skeuo-tabs--standard skeuo-tabs--horizontal skeuo-tabs--medium skeuo-color-neutral mine",
            r#"role="tablist""#,
            r#"aria-orientation="horizontal""#,
            r#"aria-label="Views""#,
            "skeuo-tabs__tray skeuo-well skeuo-well--shallow",
            r#"class="skeuo-tab skeuo-key skeuo-tab--selected""#,
            "Alpha",
            "Gamma",
        ],
    );
    assert_eq!(html.matches(r#"role="tab""#).count(), 3, "{html}");
    assert_eq!(html.matches(r#"aria-selected="true""#).count(), 1, "{html}");
    assert_eq!(
        html.matches(r#"aria-selected="false""#).count(),
        2,
        "{html}"
    );
    // Roving tab stop: only the selected tab is in the tab order.
    assert_eq!(html.matches(r#"tabindex="0""#).count(), 1, "{html}");
    assert_eq!(html.matches(r#"tabindex="-1""#).count(), 2, "{html}");
    assert_eq!(html.matches("disabled").count(), 1, "{html}");
}

#[test]
fn tabs_uncontrolled_default_value_and_variants() {
    let html = render(|| {
        rsx! {
            Tabs {
                default_value: "two",
                variant: TabsVariant::FullWidth,
                orientation: Orientation::Vertical,
                centered: true,
                size: Size::Small,
                color: Color::Primary,
                Tab { value: "one", label: "One" }
                Tab { value: "two", label: "Two", icon: rsx! { Icon { name: IconName::Home } }, icon_position: IconPosition::Start }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-tabs--full-width",
            "skeuo-tabs--vertical",
            "skeuo-tabs--centered",
            "skeuo-tabs--small",
            "skeuo-color-primary",
            r#"aria-orientation="vertical""#,
            "skeuo-tab skeuo-key skeuo-tab--selected skeuo-tab--icon-start",
            "skeuo-tab__icon",
        ],
    );
    let scrollable = render(|| {
        rsx! {
            Tabs { variant: TabsVariant::Scrollable, Tab { value: "x", label: "X" } }
        }
    });
    assert!(
        scrollable.contains("skeuo-tabs--scrollable"),
        "{scrollable}"
    );
    // Nothing selected: every tab is a tab stop.
    assert!(scrollable.contains(r#"tabindex="0""#), "{scrollable}");
    assert!(!scrollable.contains("skeuo-tab--selected"), "{scrollable}");
}

#[test]
fn tab_context_links_tabs_and_panels() {
    let html = render(|| {
        rsx! {
            TabContext { value: "script", id: "studio",
                Tabs {
                    Tab { value: "rundown", label: "Rundown" }
                    Tab { value: "script", label: "Script" }
                }
                TabPanel { value: "rundown", "Rundown body" }
                TabPanel { value: "script", "Script body" }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"id="studio-tab-script""#,
            r#"aria-controls="studio-panel-script""#,
            r#"role="tabpanel""#,
            r#"id="studio-panel-script""#,
            r#"aria-labelledby="studio-tab-script""#,
            "Script body",
        ],
    );
    assert!(!html.contains("Rundown body"), "{html}");
    assert_eq!(html.matches("hidden").count(), 1, "{html}");
    assert!(
        html.contains(r#"class="skeuo-tab skeuo-key skeuo-tab--selected" type="button" role="tab" id="studio-tab-script""#),
        "{html}"
    );
}

// ------------------------------------------------------------ Breadcrumbs

#[test]
fn breadcrumbs_marks_current_page() {
    let html = render(|| {
        rsx! {
            Breadcrumbs {
                items: vec![
                    rsx! { Link { href: "/", "Home" } },
                    rsx! { Link { href: "/shows", "Shows" } },
                    rsx! { span { "Today" } },
                ],
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<nav class="skeuo-breadcrumbs" aria-label="breadcrumb""#,
            "<ol",
            r#"aria-current="page""#,
            "Today",
        ],
    );
    assert_eq!(html.matches(r#"aria-current="page""#).count(), 1, "{html}");
    assert_eq!(
        html.matches("skeuo-breadcrumbs__separator").count(),
        2,
        "{html}"
    );
    assert!(!html.contains("skeuo-breadcrumbs__collapsed"), "{html}");
}

#[test]
fn breadcrumbs_collapse_into_keycap() {
    let html = render(|| {
        let items = (1..=6)
            .map(|i| rsx! { span { "crumb{i}" } })
            .collect::<Vec<_>>();
        rsx! {
            Breadcrumbs {
                items,
                max_items: 4,
                items_before_collapse: 1,
                items_after_collapse: 2,
                separator: rsx! { "/" },
            }
        }
    });
    assert_all(
        &html,
        &[
            "crumb1",
            "crumb5",
            "crumb6",
            "skeuo-key skeuo-key--pressable skeuo-breadcrumbs__collapsed",
            r#"aria-label="Show path""#,
        ],
    );
    for hidden in ["crumb2", "crumb3", "crumb4"] {
        assert!(
            !html.contains(hidden),
            "{hidden} should be collapsed: {html}"
        );
    }
    // 1 + "…" + 2 = 4 entries, 3 separators.
    assert_eq!(
        html.matches("skeuo-breadcrumbs__separator").count(),
        3,
        "{html}"
    );
}

// ------------------------------------------------------------------- Link

#[test]
fn link_anchor_and_button_forms() {
    let html = render(|| {
        rsx! {
            Link { href: "/docs", underline: LinkUnderline::Hover, color: Color::Secondary, target: "_blank", "Docs" }
            Link { onclick: |_| {}, "Act" }
        }
    });
    assert_all(
        &html,
        &[
            r#"<a class="skeuo-link skeuo-link--underline-hover skeuo-color-secondary" href="/docs" target="_blank""#,
            r#"<button class="skeuo-link skeuo-link--underline-always skeuo-color-primary skeuo-link--button" type="button""#,
            "Docs",
            "Act",
        ],
    );
}

// -------------------------------------------------------- Menu & Popover

#[test]
fn menu_closed_renders_only_anchor() {
    let html = render(|| {
        rsx! {
            Menu { open: false, anchor: rsx! { button { "Trigger" } },
                MenuItem { "Hidden" }
            }
        }
    });
    assert!(html.contains("Trigger"), "{html}");
    assert!(html.contains("skeuo-popover-anchor"), "{html}");
    assert!(!html.contains("Hidden"), "{html}");
    assert!(!html.contains(r#"role="menu""#), "{html}");
}

#[test]
fn menu_open_has_backdrop_floating_chassis_and_items() {
    let html = render(|| {
        rsx! {
            Menu { open: true, id: "m", aria_labelledby: "t", placement: Placement::Top, dense: true,
                anchor: rsx! { button { id: "t", "Edit" } },
                MenuItem { shortcut: "⌘C", icon: rsx! { Icon { name: IconName::Copy } }, "Copy" }
                MenuItem { selected: true, divider: true, "Paste" }
                MenuItem { disabled: true, "Delete" }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-popover-backdrop",
            "skeuo-popover skeuo-chassis skeuo-elev-3 skeuo-chassis--float skeuo-popover--top skeuo-menu",
            r#"role="menu""#,
            r#"id="m""#,
            r#"aria-labelledby="t""#,
            "skeuo-menu-list skeuo-menu-list--dense",
            r#"<kbd class="skeuo-menu-item__shortcut">⌘C</kbd>"#,
            "skeuo-menu-item skeuo-menu-item--selected skeuo-menu-item--dense",
            r#"aria-disabled="true""#,
            r#"<hr class="skeuo-rule skeuo-menu-item__divider" role="separator""#,
            "skeuo-menu-item__icon",
        ],
    );
    assert_eq!(html.matches(r#"role="menuitem""#).count(), 3, "{html}");
    assert_eq!(html.matches(r#"role="none""#).count(), 3, "{html}");
    // The check legend only on the selected item.
    assert_eq!(
        html.matches("<path d=\"M5 12.5l4.5 4.5L19 7.5\"").count(),
        1,
        "{html}"
    );
}

#[test]
fn menu_inline_preview_has_no_backdrop() {
    let html = render(|| {
        rsx! {
            Menu { open: true, inline: true, MenuItem { "One" } }
        }
    });
    assert!(html.contains("skeuo-popover--inline"), "{html}");
    assert!(html.contains("skeuo-popover-anchor--inline"), "{html}");
    assert!(!html.contains("skeuo-popover-backdrop"), "{html}");
}

#[test]
fn menu_list_standalone() {
    let html = render(|| {
        rsx! {
            MenuList { aria_label: "Speed", MenuItem { "1×" } }
        }
    });
    assert_all(
        &html,
        &[
            r#"<ul class="skeuo-menu-list" role="menu" tabindex="-1" aria-label="Speed""#,
            r#"role="menuitem""#,
        ],
    );
}

#[test]
fn popover_open_and_closed() {
    let open = render(|| {
        rsx! {
            Popover { open: true, placement: Placement::Right, aria_label: "Info",
                anchor: rsx! { button { "i" } },
                "Details"
            }
        }
    });
    assert_all(
        &open,
        &[
            "skeuo-popover-backdrop",
            "skeuo-popover--right",
            "skeuo-chassis--float",
            r#"aria-label="Info""#,
            r#"tabindex="-1""#,
            "Details",
        ],
    );
    assert!(!open.contains(r#"role="menu""#), "{open}");
    let closed = render(|| {
        rsx! {
            Popover { anchor: rsx! { button { "i" } }, "Details" }
        }
    });
    assert!(!closed.contains("Details"), "{closed}");
}

// ------------------------------------------------------------- Pagination

#[test]
fn pagination_items_match_mui() {
    use PaginationItemType::*;
    let items = pagination_items(10, 1, 1, 1, false, false, false, false);
    assert_eq!(
        items,
        vec![
            Previous,
            Page(1),
            Page(2),
            Page(3),
            Page(4),
            Page(5),
            EndEllipsis,
            Page(10),
            Next
        ]
    );
    let items = pagination_items(10, 5, 1, 1, true, true, false, false);
    assert_eq!(
        items,
        vec![
            First,
            Previous,
            Page(1),
            StartEllipsis,
            Page(4),
            Page(5),
            Page(6),
            EndEllipsis,
            Page(10),
            Next,
            Last
        ]
    );
    let items = pagination_items(3, 2, 1, 1, false, false, true, true);
    assert_eq!(items, vec![Page(1), Page(2), Page(3)]);
}

#[test]
fn pagination_controlled_page_is_latched_colored_key() {
    let html = render(|| {
        rsx! {
            Pagination { count: 10, page: 5, color: Color::Success, variant: PaginationVariant::Outlined, shape: PaginationShape::Rounded, size: Size::Small }
        }
    });
    assert_all(
        &html,
        &[
            r#"<nav class="skeuo-pagination" aria-label="pagination navigation""#,
            "skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--down skeuo-pagination-item skeuo-pagination-item--outlined skeuo-pagination-item--rounded skeuo-pagination-item--small skeuo-color-success skeuo-pagination-item--selected",
            r#"aria-label="page 5""#,
            r#"aria-current="page""#,
            r#"aria-label="Go to page 4""#,
            r#"aria-label="Go to previous page""#,
            "skeuo-pagination-item--ellipsis",
        ],
    );
    assert_eq!(html.matches(r#"aria-current="page""#).count(), 1, "{html}");
    assert_eq!(html.matches("…").count(), 2, "{html}");
}

#[test]
fn pagination_uncontrolled_default_and_disabled_ends() {
    let html = render(|| {
        rsx! {
            Pagination { count: 4, default_page: 1, show_first_button: true, show_last_button: true }
        }
    });
    assert!(html.contains(r#"aria-label="page 1""#), "{html}");
    // First and previous are disabled on page 1; next and last are not.
    assert_eq!(html.matches("disabled").count(), 2, "{html}");
    // Text variant: plain pages are printed (no keycap), arrows are keys.
    assert!(
        html.contains(r#"class="skeuo-pagination-item skeuo-pagination-item--text skeuo-pagination-item--circular skeuo-pagination-item--medium skeuo-color-primary" type="button" aria-label="Go to page 2""#),
        "{html}"
    );
    let disabled = render(|| rsx! { Pagination { count: 3, disabled: true } });
    assert_eq!(disabled.matches("disabled").count(), 5, "{disabled}");
}

// ---------------------------------------------------------------- Stepper

#[test]
fn stepper_linear_states() {
    let html = render(|| {
        rsx! {
            Stepper { active_step: 1,
                Step { StepLabel { "One" } }
                Step { StepLabel { optional: rsx! { "Optional" }, "Two" } }
                Step { StepLabel { "Three" } }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-stepper skeuo-stepper--horizontal skeuo-color-primary",
            r#"class="skeuo-step skeuo-step--completed""#,
            r#"class="skeuo-step skeuo-step--joined skeuo-step--active""#,
            r#"class="skeuo-step skeuo-step--joined skeuo-step--disabled""#,
            "skeuo-step-icon skeuo-key skeuo-key--round skeuo-key--filled skeuo-step-icon--active",
            "skeuo-step-label__optional skeuo-caption",
            ">3<",
        ],
    );
    // Two connectors; the one leading to the active step is filled.
    assert_eq!(
        html.matches("skeuo-step-connector skeuo-groove").count(),
        2,
        "{html}"
    );
    assert_eq!(
        html.matches("skeuo-step-connector__fill--on").count(),
        1,
        "{html}"
    );
    // The completed step shows a check instead of its number.
    assert!(!html.contains(">1<"), "{html}");
    assert!(html.contains(">2<"), "{html}");
}

#[test]
fn stepper_vertical_content_alternative_and_error() {
    let html = render(|| {
        rsx! {
            Stepper { active_step: 0, orientation: Orientation::Vertical,
                Step { StepLabel { "A" } StepContent { "Body A" } }
                Step { StepLabel { error: true, "B" } StepContent { "Body B" } }
                Step { expanded: true, StepLabel { "C" } StepContent { "Body C" } }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-stepper--vertical",
            "Body A",
            "Body C",
            "skeuo-step-content__line skeuo-groove",
            "skeuo-color-error skeuo-step-icon--error",
            "skeuo-step-label--error",
        ],
    );
    assert!(!html.contains("Body B"), "{html}");

    let alt = render(|| {
        rsx! {
            Stepper { alternative_label: true, Step { StepLabel { "A" } StepContent { "never" } } }
        }
    });
    assert!(alt.contains("skeuo-stepper--alternative-label"), "{alt}");
    assert!(!alt.contains("never"), "{alt}");
}

#[test]
fn stepper_non_linear_step_buttons() {
    let html = render(|| {
        rsx! {
            Stepper { active_step: 2, non_linear: true,
                Step { completed: true, StepButton { "A" } }
                Step { StepButton { "B" } }
                Step { StepButton { "C" } }
            }
        }
    });
    assert_eq!(html.matches("skeuo-step-button").count(), 3, "{html}");
    assert!(!html.contains("disabled"), "{html}");
    assert!(html.contains(r#"aria-current="step""#), "{html}");
    assert_eq!(html.matches("skeuo-step--completed").count(), 1, "{html}");
}

#[test]
fn mobile_stepper_variants() {
    let dots = render(|| {
        rsx! {
            MobileStepper { steps: 4, active_step: 1, position: MobileStepperPosition::Static,
                back_button: rsx! { button { "Back" } },
                next_button: rsx! { button { "Next" } },
            }
        }
    });
    assert_all(
        &dots,
        &[
            "skeuo-mobile-stepper skeuo-chassis skeuo-elev-1 skeuo-mobile-stepper--dots skeuo-mobile-stepper--static",
            r#"aria-label="Step 2 of 4""#,
            "Back",
            "Next",
        ],
    );
    assert_eq!(dots.matches("skeuo-led--off").count(), 3, "{dots}");

    let progress = render(|| {
        rsx! { MobileStepper { steps: 5, active_step: 2, variant: MobileStepperVariant::Progress } }
    });
    assert_all(
        &progress,
        &[
            "skeuo-mobile-stepper--bottom",
            r#"role="progressbar""#,
            r#"aria-valuenow="50""#,
            "width: 50%",
        ],
    );
    let text = render(|| {
        rsx! { MobileStepper { steps: 6, active_step: 1, variant: MobileStepperVariant::Text } }
    });
    assert!(text.contains("2 / 6"), "{text}");
}

// ------------------------------------------------------- BottomNavigation

#[test]
fn bottom_navigation_selected_and_labels() {
    let html = render(|| {
        rsx! {
            BottomNavigation { default_value: "b", color: Color::Error,
                BottomNavigationAction { value: "a", label: "Alpha", icon: rsx! { Icon { name: IconName::Home } } }
                BottomNavigationAction { value: "b", label: "Beta", icon: rsx! { Icon { name: IconName::Star } } }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<nav class="skeuo-bottom-nav skeuo-chassis skeuo-elev-1 skeuo-color-error" aria-label="Navigation""#,
            "skeuo-key--down skeuo-bottom-nav__action--selected",
            r#"aria-current="page""#,
            "Beta",
            // Without show_labels the unselected action is icon-only, named by aria-label.
            "skeuo-bottom-nav__action--icon-only",
            r#"aria-label="Alpha""#,
        ],
    );
    assert!(!html.contains(">Alpha<"), "{html}");

    let labelled = render(|| {
        rsx! {
            BottomNavigation { value: "a", show_labels: true,
                BottomNavigationAction { value: "a", label: "Alpha" }
                BottomNavigationAction { value: "b", label: "Beta" }
            }
        }
    });
    assert!(
        labelled.contains(">Alpha<") && labelled.contains(">Beta<"),
        "{labelled}"
    );
    assert!(!labelled.contains("icon-only"), "{labelled}");
}

// -------------------------------------------------------------- SpeedDial

#[test]
fn speed_dial_open_and_closed() {
    let open = render(|| {
        rsx! {
            SpeedDial { aria_label: "Actions", open: true, direction: SpeedDialDirection::Left, color: Color::Secondary,
                SpeedDialAction { icon: rsx! { Icon { name: IconName::Copy } }, tooltip_title: "Copy", tooltip_open: true }
                SpeedDialAction { icon: rsx! { Icon { name: IconName::Edit } }, tooltip_title: "Edit" }
            }
        }
    });
    assert_all(
        &open,
        &[
            "skeuo-speed-dial skeuo-speed-dial--left skeuo-speed-dial--open skeuo-color-secondary",
            "skeuo-speed-dial__fab skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-key--round",
            r#"aria-label="Actions""#,
            r#"aria-expanded="true""#,
            r#"aria-haspopup="true""#,
            r#"role="menu""#,
            r#"aria-orientation="horizontal""#,
            "skeuo-speed-dial-icon skeuo-speed-dial-icon--open",
            "skeuo-speed-dial-action skeuo-speed-dial-action--tooltip-open",
            r#"aria-label="Copy""#,
            "skeuo-speed-dial-action__key skeuo-key skeuo-key--pressable skeuo-key--round",
        ],
    );
    assert_eq!(open.matches(r#"role="menuitem""#).count(), 2, "{open}");

    let closed = render(|| {
        rsx! {
            SpeedDial { aria_label: "Actions", hidden: true,
                SpeedDialAction { icon: rsx! { "x" }, tooltip_title: "X" }
            }
        }
    });
    assert_all(
        &closed,
        &[
            r#"aria-expanded="false""#,
            "skeuo-speed-dial--hidden",
            "skeuo-speed-dial--up",
            r#"aria-orientation="vertical""#,
        ],
    );
    assert!(!closed.contains("skeuo-speed-dial--open"), "{closed}");
    assert!(!closed.contains("skeuo-speed-dial-icon--open"), "{closed}");
}

// --------------------------------------------------------------- Timeline

#[test]
fn timeline_structure_and_dots() {
    let html = render(|| {
        rsx! {
            Timeline { position: TimelinePosition::Alternate,
                TimelineItem {
                    TimelineOppositeContent { "06:00" }
                    TimelineSeparator {
                        TimelineDot { color: Color::Success }
                        TimelineConnector {}
                    }
                    TimelineContent { "News" }
                }
                TimelineItem { position: TimelinePosition::Left,
                    TimelineSeparator { TimelineDot { variant: TimelineDotVariant::Outlined } }
                    TimelineContent { "Close" }
                }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"<ul class="skeuo-timeline skeuo-timeline--alternate""#,
            r#"<li class="skeuo-timeline-item""#,
            r#"class="skeuo-timeline-item skeuo-timeline-item--left""#,
            "skeuo-timeline-opposite skeuo-caption",
            "skeuo-timeline-dot skeuo-led skeuo-timeline-dot--filled skeuo-color-success",
            "skeuo-timeline-dot skeuo-led skeuo-led--off skeuo-timeline-dot--outlined skeuo-timeline-dot--grey",
            "skeuo-timeline-connector skeuo-groove",
            "skeuo-timeline-content",
            "News",
        ],
    );
}
