use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn render(app: fn() -> Element) -> String {
    let mut dom = VirtualDom::new(app);
    dom.rebuild_in_place();
    dioxus_ssr::render(&dom)
}

#[test]
fn default_theme_is_light_and_carries_both_palettes() {
    let html = render(|| rsx! { ThemeProvider { "x" } });
    assert!(html.contains(r#"data-skeuo-mode="light""#), "{html}");
    assert!(
        html.contains("--skeuo-light-primary-main:#0071e3"),
        "{html}"
    );
    assert!(html.contains("--skeuo-dark-primary-main:#0a84ff"), "{html}");
}

#[test]
fn dark_and_system_modes_set_the_root_attribute() {
    let dark = render(|| rsx! { ThemeProvider { theme: Theme::dark(), "x" } });
    assert!(dark.contains(r#"data-skeuo-mode="dark""#), "{dark}");
    let system = render(|| rsx! { ThemeProvider { theme: Theme::system(), "x" } });
    assert!(system.contains(r#"data-skeuo-mode="system""#), "{system}");
}

#[test]
fn stylesheet_has_dark_rules_for_dark_and_system() {
    let css = skeuo_ui::STYLESHEET.as_str();
    assert!(
        !css.contains("SKEUO_DARK"),
        "placeholder left in stylesheet"
    );
    assert!(css.contains(r#".skeuo-root[data-skeuo-mode="dark"] {"#));
    let media = css
        .find("@media (prefers-color-scheme: dark)")
        .expect("system media query");
    assert!(css[media..].contains(r#".skeuo-root[data-skeuo-mode="system"] {"#));
    assert!(css.contains("--skeuo-primary-main: var(--skeuo-dark-primary-main)"));
}

#[test]
fn nested_providers_inject_the_stylesheet_once() {
    let html = render(|| {
        rsx! {
            ThemeProvider {
                ThemeProvider { theme: Theme { radius: 4.0, ..Theme::default() }, "inner" }
            }
        }
    });
    assert_eq!(
        html.matches("<style>").count(),
        1,
        "stylesheet injected more than once"
    );
    assert_eq!(html.matches(r#"class="skeuo-root""#).count(), 2);
    assert!(html.contains("--skeuo-radius:4px"));
}

#[test]
fn palette_colors_can_be_overridden_per_mode() {
    let html = render(|| {
        let mut theme = Theme::dark();
        theme.dark.get_mut(Color::Primary).main = "#ff00aa".into();
        rsx! { ThemeProvider { theme, "x" } }
    });
    assert!(html.contains("--skeuo-dark-primary-main:#ff00aa"), "{html}");
}
