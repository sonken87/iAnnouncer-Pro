#include "../gallery.hpp"

namespace gallery {

Node IconsPage() {
    Node grid = el("div").cls("icon-grid");
    for (IconName n : kAllIcons)
        grid.child(el("div").cls("icon-cell skeuo-key")
                       .child(Icon({.name = n, .size = "22px"}))
                       .child(el("span").child(std::string(icon_name(n)))));
    return {
        el("style").child(raw_html(
            ".icon-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(104px, 1fr)); gap: 10px; }"
            ".icon-cell { display: flex; flex-direction: column; align-items: center; gap: 8px; padding: 14px 6px 10px; }"
            ".icon-cell span { font-size: 11px; color: var(--sk-ink-2); }")),
        Demo("Icon", "Icon({.name = IconName::…}) — 1em, currentColor; pass .title for an accessible name.",
             std::move(grid)),
    };
}

}  // namespace gallery
