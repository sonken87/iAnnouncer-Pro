use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn save() {}

#[allow(non_snake_case, dead_code)]
fn App() -> Element {
    let mut name = use_signal(String::new);
    let mut on_air = use_signal(|| false);
    rsx! {
        ThemeProvider {
            TextField {
                label: "Name",
                value: name(),
                oninput: move |e: FormEvent| name.set(e.value()),
            }
            FormControlLabel {
                control: rsx! { Switch { checked: on_air(), onchange: move |e: FormEvent| on_air.set(e.checked()) } },
                label: "On air",
            }
            Button { onclick: move |_| save(), "Save" }
        }
    }
}

#[test]
fn readme_example_renders() {
    let mut dom = VirtualDom::new(App);
    dom.rebuild_in_place();
    let html = dioxus_ssr::render(&dom);
    assert!(html.contains(r#"role="switch""#), "{html}");
}
