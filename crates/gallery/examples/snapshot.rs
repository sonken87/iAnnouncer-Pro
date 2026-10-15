//! Renders the showcase to a static HTML file (no wasm toolchain needed),
//! e.g. for screenshots: `cargo run -p gallery --example snapshot -- out.html`

use dioxus::prelude::*;

fn main() {
    let out = std::env::args()
        .nth(1)
        .unwrap_or_else(|| "gallery.html".into());
    let mut dom = VirtualDom::new(gallery::Showcase);
    dom.rebuild_in_place();
    let body = dioxus_ssr::render(&dom);
    let html = format!(
        "<!doctype html><html><head><meta charset=\"utf-8\">\
         <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
         <title>skeuo-ui gallery</title></head><body>{body}</body></html>"
    );
    std::fs::write(&out, html).expect("write snapshot");
    println!("wrote {out}");
}
