//! Renders every gallery page to static HTML (no wasm toolchain needed),
//! e.g. for screenshots:
//! `cargo run -p gallery --example snapshot -- out_dir`
//! writes `out_dir/<page>.html` and `out_dir/<page>-dark.html` for each page.

use dioxus::prelude::*;
use gallery::{GalleryShell, Page};
use skeuo_ui::ThemeMode;

fn main() {
    let out = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "snapshots".into());
    std::fs::create_dir_all(&out).expect("create output dir");
    for (mode, suffix) in [(ThemeMode::Light, ""), (ThemeMode::Dark, "-dark")] {
        for page in Page::ALL {
            let mut dom =
                VirtualDom::new_with_props(GalleryShell, gallery::GalleryShellProps { page, mode });
            dom.rebuild_in_place();
            let body = dioxus_ssr::render(&dom);
            let html = format!(
                "<!doctype html><html><head><meta charset=\"utf-8\">\
             <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
             <title>skeuo-ui — {}</title></head><body>{body}</body></html>",
                page.title()
            );
            let path = format!("{out}/{}{suffix}.html", page.slug());
            std::fs::write(&path, html).expect("write snapshot");
            println!("wrote {path}");
        }
    }
}
