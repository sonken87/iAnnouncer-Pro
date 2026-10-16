#include <charconv>

#include "skeuo/surfaces.hpp"

namespace skeuo {

namespace {

/// Formats like Rust's `Display` for f32 (shortest round-trip: 16, 12.5).
std::string fmt_f32(double v) {
    char buf[32];
    auto res = std::to_chars(buf, buf + sizeof buf, static_cast<float>(v));
    return std::string(buf, res.ptr);
}

bool is_one_of(const std::string& tag, std::initializer_list<const char*> tags) {
    for (const char* t : tags)
        if (tag == t) return true;
    return false;
}

}  // namespace

Node SkeuoBox(const SkeuoBoxProps& props, Node children) {
    std::string tag = props.component.value_or("div");
    if (!is_one_of(tag, {"span", "section", "article", "main", "header", "footer", "nav", "aside", "ul", "ol", "li",
                         "form", "p"}))
        tag = "div";
    Node box = el(tag).cls(classes({"skeuo-box", props.cls}));
    for (const auto& [k, v] : props.attrs) box.set_attr(k, v);
    return std::move(box).child(std::move(children));
}

Node Container(const ContainerProps& props, Node children) {
    Node div = el("div").cls(classes({
        "skeuo-container",
        "skeuo-container--max-" + std::string(as_str(props.max_width)),
        props.fixed ? "skeuo-container--fixed" : "",
        props.disable_gutters ? "" : "skeuo-container--gutters",
        props.cls,
    }));
    for (const auto& [k, v] : props.attrs) div.set_attr(k, v);
    return std::move(div).child(std::move(children));
}

Node Stack(const StackProps& props, Node children) {
    std::string cls = classes({
        "skeuo-stack",
        "skeuo-stack--" + std::string(as_str(props.direction)),
        props.use_flex_gap ? "skeuo-stack--gap" : "skeuo-stack--margin",
        props.cls,
    });
    std::string style = "--stack-spacing:" + fmt_f32(static_cast<float>(props.spacing) * 8.0f) + "px;";
    for (const auto& [prop, value] : {std::pair{"align-items", &props.align_items},
                                      std::pair{"justify-content", &props.justify_content},
                                      std::pair{"flex-wrap", &props.flex_wrap}}) {
        if (*value) style += std::string(prop) + ":" + **value + ";";
    }
    style += props.style;

    std::string tag = props.component.value_or("div");
    if (!is_one_of(tag, {"ul", "ol", "section", "nav"})) tag = "div";
    Node root = el(tag).cls(cls).attr("style", style);

    if (props.divider && children.kind() == Node::Kind::Fragment) {
        bool first = true;
        for (auto& item : children.kids()) {
            if (!first) root.child(*props.divider);
            first = false;
            root.child(std::move(item));
        }
        return root;
    }
    return std::move(root).child(std::move(children));
}

}  // namespace skeuo
