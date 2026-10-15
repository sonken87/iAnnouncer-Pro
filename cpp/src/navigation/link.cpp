#include "skeuo/navigation.hpp"

namespace skeuo {

namespace {
std::string_view underline_name(LinkUnderline u) {
    switch (u) {
        case LinkUnderline::None: return "none";
        case LinkUnderline::Hover: return "hover";
        case LinkUnderline::Always: return "always";
    }
    return "always";
}
}  // namespace

Node Link(const LinkProps& props, Node children) {
    std::string underline = "skeuo-link--underline-" + std::string(underline_name(props.underline));
    std::string cls = classes({"skeuo-link", underline, color_class(props.color), props.cls});
    Node link;
    if (props.href) {
        link = el("a").cls(cls).attr("href", *props.href);
        if (props.target) link.attr("target", *props.target);
        if (props.rel) link.attr("rel", *props.rel);
    } else {
        link = el("button").cls(cls + " skeuo-link--button").attr("type", "button");
    }
    link.attr_if(props.current, "aria-current", "page");
    if (props.onclick) link.on("click", props.onclick);
    return std::move(link).child(std::move(children));
}

}  // namespace skeuo
