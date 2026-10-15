use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

#[test]
fn button_default_classes_and_type() {
    let html = render(|| rsx! { Button { "Save" } });
    assert!(
        html.contains(
            r#"class="skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-btn skeuo-btn--contained skeuo-color-primary skeuo-btn--medium""#
        ),
        "{html}"
    );
    assert!(html.contains(r#"type="button""#), "{html}");
    assert!(html.contains("Save"), "{html}");
    assert!(!html.contains("disabled"), "{html}");
}

#[test]
fn button_props_and_passthrough_attributes() {
    let html = render(|| {
        rsx! {
            Button {
                variant: ButtonVariant::Outlined,
                color: Color::Error,
                size: Size::Small,
                disabled: true,
                full_width: true,
                r#type: "submit",
                class: "mine",
                id: "del",
                aria_label: "Delete item",
                start_icon: rsx! { "✕" },
                "Delete"
            }
        }
    });
    for needle in [
        "skeuo-btn--outlined",
        "skeuo-color-error",
        "skeuo-btn--small",
        "skeuo-btn--full-width",
        "mine",
        r#"type="submit""#,
        r#"id="del""#,
        r#"aria-label="Delete item""#,
        "disabled",
        "skeuo-btn__icon--start",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
    assert!(!html.contains("skeuo-btn__icon--end"), "{html}");
    assert_eq!(html.matches("class=").count(), 3, "{html}");
}

#[test]
fn text_field_links_label_and_helper() {
    let html = render(|| {
        rsx! {
            TextField {
                id: "email",
                label: "Email",
                helper_text: "We never share it",
                required: true,
                r#type: "email",
                value: "a@b.c",
            }
        }
    });
    for needle in [
        r#"for="email""#,
        r#"id="email""#,
        r#"type="email""#,
        r#"value="a@b.c""#,
        r#"aria-describedby="email-helper""#,
        r#"id="email-helper""#,
        "We never share it",
        "skeuo-field__asterisk",
        "required",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
}

#[test]
fn text_field_generates_unique_ids() {
    let html = render(|| {
        rsx! {
            TextField { label: "A" }
            TextField { label: "B" }
        }
    });
    let ids: Vec<&str> = html
        .match_indices(r#"for=""#)
        .map(|(i, m)| {
            let rest = &html[i + m.len()..];
            &rest[..rest.find('"').unwrap()]
        })
        .collect();
    assert_eq!(ids.len(), 2, "{html}");
    assert_ne!(ids[0], ids[1]);
    for id in ids {
        assert!(html.contains(&format!(r#"id="{id}""#)), "{html}");
    }
}

#[test]
fn text_field_error_multiline_and_adornments() {
    let html = render(|| {
        rsx! {
            TextField {
                variant: TextFieldVariant::Filled,
                error: true,
                multiline: true,
                rows: 4,
                start_adornment: rsx! { "¥" },
            }
        }
    });
    for needle in [
        "skeuo-field--filled",
        "skeuo-field--error",
        "skeuo-field--multiline",
        "<textarea",
        r#"rows=4"#,
        r#"aria-invalid=true"#,
        "skeuo-field__adornment--start",
    ] {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
    assert!(!html.contains("<label"), "{html}");
    assert!(!html.contains("aria-describedby"), "{html}");
}

#[test]
fn theme_provider_injects_stylesheet_and_vars() {
    let html = render(|| {
        let mut theme = Theme::default();
        theme.light.primary.main = "#123456".into();
        rsx! {
            ThemeProvider { theme, Button { "x" } }
        }
    });
    assert!(html.contains(".skeuo-key--filled"), "stylesheet missing");
    assert!(
        html.contains("--skeuo-light-primary-main:#123456"),
        "{html}"
    );
    assert!(html.contains("skeuo-root"), "{html}");
}

#[test]
fn text_field_default_value_renders_and_value_wins() {
    let html = render(|| {
        rsx! {
            TextField { default_value: "initial" }
            TextField { multiline: true, default_value: "notes" }
            TextField { value: "controlled", default_value: "ignored" }
        }
    });
    assert!(html.contains(r#"value="initial""#), "{html}");
    assert!(html.contains("notes"), "{html}");
    assert!(html.contains(r#"value="controlled""#), "{html}");
    assert!(!html.contains("ignored"), "{html}");
}

fn assert_all(html: &str, needles: &[&str]) {
    for needle in needles {
        assert!(html.contains(needle), "missing {needle}: {html}");
    }
}

// ---------------------------------------------------------------- Button

#[test]
fn button_loading_center_disables_and_shows_spinner() {
    let html = render(|| rsx! { Button { loading: true, "Submit" } });
    assert_all(
        &html,
        &[
            "skeuo-btn--loading",
            "skeuo-btn--loading-center",
            "skeuo-btn__loader--center",
            "skeuo-spinner",
            r#"role="progressbar""#,
            r#"aria-busy="true""#,
            "disabled",
            "Submit",
        ],
    );
}

#[test]
fn button_loading_start_replaces_start_icon() {
    let html = render(|| {
        rsx! {
            Button {
                loading: true,
                loading_position: LoadingPosition::Start,
                start_icon: rsx! { span { class: "my-icon" } },
                "Save"
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-btn--loading-start",
            "skeuo-btn__icon--start skeuo-btn__loader",
        ],
    );
    assert!(!html.contains("my-icon"), "{html}");
    let idle = render(|| rsx! { Button { loading: false, "Save" } });
    assert!(
        !idle.contains("loading") && !idle.contains("aria-busy"),
        "{idle}"
    );
}

#[test]
fn button_group_hands_props_to_buttons() {
    let html = render(|| {
        rsx! {
            ButtonGroup {
                variant: ButtonVariant::Outlined,
                color: Color::Error,
                size: Size::Small,
                orientation: Orientation::Vertical,
                disabled: true,
                aria_label: "Edit",
                Button { "One" }
                Button { "Two" }
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"role="group""#,
            r#"aria-orientation="vertical""#,
            r#"aria-label="Edit""#,
            "skeuo-btn-group--outlined",
            "skeuo-btn-group--vertical",
        ],
    );
    assert_eq!(html.matches("skeuo-btn--outlined").count(), 2, "{html}");
    assert_eq!(html.matches("skeuo-btn--small").count(), 2, "{html}");
    assert_eq!(html.matches("skeuo-color-error").count(), 3, "{html}");
    assert_eq!(html.matches("disabled=true").count(), 2, "{html}");
    assert!(!html.contains("skeuo-key--filled"), "{html}");
}

// ------------------------------------------------------- IconButton / Fab

#[test]
fn icon_button_variants_edge_and_label() {
    let html = render(|| {
        rsx! {
            IconButton { aria_label: "Delete", edge: IconButtonEdge::End, Icon { name: IconName::Delete } }
            IconButton {
                variant: IconButtonVariant::Contained,
                color: Color::Success,
                size: Size::Large,
                round: true,
                disabled: true,
                aria_label: "Add",
                Icon { name: IconName::Add }
            }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-icon-btn skeuo-icon-btn--text skeuo-color-neutral skeuo-icon-btn--medium skeuo-icon-btn--edge-end",
            r#"aria-label="Delete""#,
            "skeuo-key skeuo-key--pressable skeuo-key--filled skeuo-icon-btn skeuo-icon-btn--contained skeuo-color-success skeuo-icon-btn--large skeuo-key--round",
            r#"aria-label="Add""#,
            "disabled=true",
        ],
    );
}

#[test]
fn fab_is_a_round_key() {
    let html = render(|| {
        rsx! {
            Fab { color: Color::Primary, aria_label: "Add", Icon { name: IconName::Add } }
            Fab { variant: FabVariant::Extended, size: Size::Small, "Go" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-key skeuo-key--pressable skeuo-key--round skeuo-key--filled skeuo-fab skeuo-fab--circular skeuo-color-primary skeuo-fab--large",
            "skeuo-fab--extended skeuo-color-neutral skeuo-fab--small",
            r#"type="button""#,
        ],
    );
    // A neutral (default) FAB is a white key.
    assert_eq!(html.matches("skeuo-key--filled").count(), 1, "{html}");
}

// ---------------------------------------------------------- ToggleButton

#[test]
fn toggle_button_standalone_selected() {
    let html = render(|| {
        rsx! {
            ToggleButton { value: "bold", selected: true, color: Color::Secondary, "B" }
            ToggleButton { value: "italic", "I" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-key--down skeuo-toggle--selected",
            r#"aria-pressed=true"#,
            r#"aria-pressed=false"#,
            r#"value="bold""#,
            "skeuo-color-secondary",
        ],
    );
    assert_eq!(html.matches("skeuo-key--down").count(), 1, "{html}");
}

#[test]
fn toggle_group_controlled_and_uncontrolled_selection() {
    let html = render(|| {
        rsx! {
            ToggleButtonGroup { value: vec!["b".to_string(), "c".to_string()], size: Size::Large,
                ToggleButton { value: "a", "A" }
                ToggleButton { value: "b", "B" }
                ToggleButton { value: "c", "C" }
            }
        }
    });
    assert_eq!(html.matches("skeuo-toggle--selected").count(), 2, "{html}");
    assert_eq!(html.matches("skeuo-toggle--large").count(), 3, "{html}");
    assert_all(&html, &[r#"role="group""#, "skeuo-toggle-group"]);

    let html = render(|| {
        rsx! {
            ToggleButtonGroup { exclusive: true, default_value: vec!["y".to_string()], disabled: true,
                ToggleButton { value: "x", "X" }
                ToggleButton { value: "y", "Y" }
            }
        }
    });
    assert_eq!(html.matches("skeuo-toggle--selected").count(), 1, "{html}");
    assert_eq!(html.matches("disabled=true").count(), 2, "{html}");
    let y = html.find(">Y<").unwrap();
    let sel = html.find("skeuo-toggle--selected").unwrap();
    assert!(sel < y && sel > html.find(">X<").unwrap(), "{html}");
}

// ---------------------------------------------------------------- Select

fn station_options() -> Vec<SelectOption> {
    vec![
        SelectOption::new("a", "Alpha"),
        SelectOption::new("b", "Bravo"),
        SelectOption::new("c", "Charlie").disabled(),
    ]
}

#[test]
fn select_closed_shows_value_and_combobox_aria() {
    let html = render(|| {
        rsx! {
            Select {
                id: "st",
                label: "Station",
                options: station_options(),
                default_value: "b",
                helper_text: "Pick one",
                name: "station",
            }
        }
    });
    assert_all(
        &html,
        &[
            r#"role="combobox""#,
            r#"aria-haspopup="listbox""#,
            r#"aria-expanded=false"#,
            r#"aria-controls="st-listbox""#,
            r#"aria-labelledby="st-label st""#,
            r#"aria-describedby="st-helper""#,
            r#"id="st-label""#,
            "skeuo-well skeuo-well--focusable skeuo-select__trigger",
            ">Bravo<",
            r#"type="hidden""#,
            r#"name="station""#,
            r#"value="b""#,
            "skeuo-select__chevron",
        ],
    );
    assert!(!html.contains(r#"role="listbox""#), "{html}");
}

#[test]
fn select_open_lists_options_with_selection_and_disabled() {
    let html = render(|| {
        rsx! {
            Select { id: "s", options: station_options(), value: "a", open: true, variant: TextFieldVariant::Filled }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-select--open",
            "skeuo-well--shallow",
            r#"aria-expanded=true"#,
            r#"role="listbox""#,
            "skeuo-chassis skeuo-chassis--float skeuo-elev-3",
            r#"id="s-opt-0""#,
            r#"aria-selected=true"#,
            r#"aria-selected=false"#,
            r#"aria-disabled="true""#,
            "skeuo-select__option--selected",
            "skeuo-select__option--disabled",
        ],
    );
    assert_eq!(html.matches(r#"role="option""#).count(), 3, "{html}");
    assert_eq!(
        html.matches("skeuo-select__option--selected").count(),
        1,
        "{html}"
    );
}

#[test]
fn select_multiple_placeholder_error_disabled() {
    let html = render(|| {
        rsx! {
            Select {
                options: station_options(),
                multiple: true,
                default_values: vec!["a".to_string(), "b".to_string()],
                open: true,
            }
            Select { options: station_options(), placeholder: "Choose", error: true, disabled: true, open: true }
        }
    });
    assert_all(
        &html,
        &[
            "Alpha, Bravo",
            r#"aria-multiselectable="true""#,
            "skeuo-select__placeholder",
            "Choose",
            "skeuo-field--error",
            "skeuo-field--disabled",
            r#"tabindex="-1""#,
        ],
    );
    // The disabled select stays closed even with `open`.
    assert_eq!(html.matches(r#"role="listbox""#).count(), 1, "{html}");
    assert_eq!(
        html.matches("skeuo-select__option--selected").count(),
        2,
        "{html}"
    );
}

#[test]
fn native_select_marks_selected_option() {
    let html = render(|| {
        rsx! {
            NativeSelect { id: "n", label: "Minutes", options: station_options(), default_value: "b", variant: TextFieldVariant::Standard }
        }
    });
    assert_all(
        &html,
        &[
            "<select",
            r#"for="n""#,
            r#"id="n""#,
            "skeuo-native-select",
            "skeuo-field--standard",
            "skeuo-select__chevron",
        ],
    );
    let bravo = html.find(">Bravo<").unwrap();
    let selected = html.find("selected=true").unwrap();
    assert!(
        selected < bravo && selected > html.find(">Alpha<").unwrap(),
        "{html}"
    );
    assert!(
        html[html.find(">Bravo<").unwrap()..].contains("disabled=true"),
        "{html}"
    );
}

// ---------------------------------------------------------- Autocomplete

fn shows() -> Vec<String> {
    ["Breakfast", "Drive", "Jazz", "News"]
        .into_iter()
        .map(String::from)
        .collect()
}

#[test]
fn autocomplete_single_value_and_aria() {
    let html = render(|| {
        rsx! {
            Autocomplete { id: "ac", label: "Show", options: shows(), default_value: "Drive" }
        }
    });
    assert_all(
        &html,
        &[
            r#"for="ac""#,
            r#"role="combobox""#,
            r#"aria-autocomplete="list""#,
            r#"aria-expanded=false"#,
            r#"aria-controls="ac-listbox""#,
            r#"value="Drive""#,
            r#"autocomplete="off""#,
            "skeuo-autocomplete__clear",
            "skeuo-autocomplete__popup",
        ],
    );
    assert!(!html.contains(r#"role="listbox""#), "{html}");
}

#[test]
fn autocomplete_filters_by_input_and_shows_no_options() {
    let html = render(|| {
        rsx! {
            Autocomplete { options: shows(), input_value: "a", open: true }
        }
    });
    // "a" matches Breakfast, Drive? no; Jazz, Breakfast.
    assert_eq!(html.matches(r#"role="option""#).count(), 2, "{html}");
    assert_all(&html, &[">Breakfast<", ">Jazz<"]);
    let html = render(|| {
        rsx! {
            Autocomplete { options: shows(), input_value: "zzz", open: true, no_options_text: "Nothing" }
        }
    });
    assert_all(&html, &["skeuo-select__empty", "Nothing"]);
    assert!(!html.contains(r#"role="option""#), "{html}");
    // Free solo shows no empty list.
    let html = render(|| {
        rsx! {
            Autocomplete { options: shows(), input_value: "zzz", open: true, free_solo: true }
        }
    });
    assert!(!html.contains("skeuo-select__empty"), "{html}");
    assert!(!html.contains("skeuo-autocomplete__popup"), "{html}");
}

#[test]
fn autocomplete_multiple_renders_tag_pills() {
    let html = render(|| {
        rsx! {
            Autocomplete {
                options: shows(),
                multiple: true,
                values: vec!["Jazz".to_string(), "News".to_string()],
                open: true,
                placeholder: "Add",
            }
        }
    });
    assert_eq!(
        html.matches("skeuo-autocomplete__tag skeuo-key").count(),
        2,
        "{html}"
    );
    assert_all(
        &html,
        &[
            "skeuo-autocomplete--has-tags",
            r#"aria-label="Remove Jazz""#,
            r#"aria-multiselectable="true""#,
        ],
    );
    assert_eq!(
        html.matches("skeuo-select__option--selected").count(),
        2,
        "{html}"
    );
    // The placeholder hides once there are tags.
    assert!(!html.contains(r#"placeholder="Add""#), "{html}");
    let disabled = render(|| {
        rsx! {
            Autocomplete { options: shows(), multiple: true, default_values: vec!["Jazz".to_string()], disabled: true }
        }
    });
    assert!(
        !disabled.contains("skeuo-autocomplete__tag-remove"),
        "{disabled}"
    );
    assert!(
        !disabled.contains("skeuo-autocomplete__clear"),
        "{disabled}"
    );
}

// ---------------------------------------------------------- TransferList

#[test]
fn transfer_list_renders_both_wells_and_checked_rows() {
    let html = render(|| {
        rsx! {
            TransferList {
                default_left: vec!["a".to_string(), "b".to_string()],
                default_right: vec!["c".to_string()],
                default_checked: vec!["b".to_string()],
                left_title: "From",
                right_title: "To",
            }
        }
    });
    assert_eq!(
        html.matches("skeuo-transfer__list skeuo-well").count(),
        2,
        "{html}"
    );
    assert_eq!(html.matches(r#"type="checkbox""#).count(), 5, "{html}");
    assert_eq!(
        html.matches("skeuo-transfer__row--checked").count(),
        1,
        "{html}"
    );
    assert_all(
        &html,
        &[
            "From",
            "To",
            "1/2 selected",
            "0/1 selected",
            r#"aria-checked="mixed""#,
            r#"aria-label="Move selected right""#,
            r#"aria-label="Move all left""#,
        ],
    );
    // Nothing is checked on the right, so "move selected left" is disabled.
    let left_btn = html.find(r#"aria-label="Move selected left""#).unwrap();
    let tag_start = html[..left_btn].rfind("<button").unwrap();
    assert!(
        html[tag_start..left_btn].contains("disabled=true"),
        "{html}"
    );
}

#[test]
fn transfer_list_controlled_lists_win() {
    let html = render(|| {
        rsx! {
            TransferList {
                left: vec!["x".to_string()],
                right: Vec::<String>::new(),
                default_left: vec!["ignored".to_string()],
            }
        }
    });
    assert!(html.contains(">x<"), "{html}");
    assert!(!html.contains("ignored"), "{html}");
    assert!(html.contains("Empty"), "{html}");
}

// ------------------------------------------------------ TextareaAutosize

#[test]
fn textarea_autosize_rows_follow_content() {
    let html = render(|| {
        rsx! {
            TextareaAutosize { min_rows: 2, aria_label: "Empty" }
            TextareaAutosize { max_rows: 3, default_value: "1\n2\n3\n4\n5" }
            TextareaAutosize { value: "a\nb\nc", default_value: "ignored" }
        }
    });
    assert_all(
        &html,
        &[
            "skeuo-textarea skeuo-well skeuo-well--focusable",
            "rows=2",
            "rows=3",
            "--ta-min-rows: 2;",
            "--ta-max-rows: 3;",
            "skeuo-textarea--bounded",
            r#"aria-label="Empty""#,
            "1\n2\n3\n4\n5",
        ],
    );
    assert_eq!(html.matches("rows=3").count(), 2, "{html}");
    assert!(!html.contains("ignored"), "{html}");
}
