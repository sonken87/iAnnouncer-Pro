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
        theme.primary.main = "#123456".into();
        rsx! {
            ThemeProvider { theme, Button { "x" } }
        }
    });
    assert!(html.contains(".skeuo-key--filled"), "stylesheet missing");
    assert!(html.contains("--skeuo-primary-main:#123456"), "{html}");
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
