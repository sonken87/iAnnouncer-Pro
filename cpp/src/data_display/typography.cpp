#include "detail.hpp"

namespace skeuo {

namespace {
using data_display_detail::flag;

std::string_view default_tag(TypographyVariant v) {
    switch (v) {
        case TypographyVariant::H1: return "h1";
        case TypographyVariant::H2: return "h2";
        case TypographyVariant::H3: return "h3";
        case TypographyVariant::H4: return "h4";
        case TypographyVariant::H5: return "h5";
        case TypographyVariant::H6: return "h6";
        case TypographyVariant::Subtitle1:
        case TypographyVariant::Subtitle2: return "h6";
        case TypographyVariant::Body1:
        case TypographyVariant::Body2: return "p";
        default: return "span";
    }
}

std::string_view align_name(TypographyAlign a) {
    switch (a) {
        case TypographyAlign::Inherit: return "inherit";
        case TypographyAlign::Left: return "left";
        case TypographyAlign::Center: return "center";
        case TypographyAlign::Right: return "right";
        case TypographyAlign::Justify: return "justify";
    }
    return "inherit";
}

bool allowed_tag(std::string_view t) {
    for (std::string_view ok : {"h1", "h2", "h3", "h4", "h5", "h6", "p", "div", "label", "legend", "strong", "em",
                                "small", "li", "figcaption", "blockquote", "code"})
        if (t == ok) return true;
    return false;
}
}  // namespace

Node Typography(const TypographyProps& props, Node children) {
    std::string variant = "skeuo-typography--" + std::string(as_str(props.variant));
    std::string_view ink;
    std::string_view palette;
    switch (props.color.kind) {
        case TypographyColor::Kind::TextPrimary: ink = "skeuo-typography--text-primary"; break;
        case TypographyColor::Kind::TextSecondary: ink = "skeuo-typography--text-secondary"; break;
        case TypographyColor::Kind::TextDisabled: ink = "skeuo-typography--text-disabled"; break;
        case TypographyColor::Kind::Inherit: ink = "skeuo-typography--color-inherit"; break;
        case TypographyColor::Kind::Palette:
            ink = "skeuo-typography--ink";
            palette = color_class(props.color.palette);
            break;
    }
    std::string align = props.align == TypographyAlign::Inherit
                            ? std::string()
                            : "skeuo-typography--align-" + std::string(align_name(props.align));
    std::string tag = props.component ? *props.component
                      : props.paragraph ? std::string("p")
                                        : std::string(default_tag(props.variant));
    if (!allowed_tag(tag)) tag = "span";

    Node n = el(tag).cls(classes({
        "skeuo-typography",
        variant,
        ink,
        palette,
        align,
        flag(props.gutter_bottom, "skeuo-typography--gutter-bottom"),
        flag(props.no_wrap, "skeuo-typography--no-wrap"),
        flag(props.paragraph, "skeuo-typography--paragraph"),
        props.cls,
    }));
    if (!props.style.empty()) n.attr("style", props.style);
    if (props.id) n.attr("id", *props.id);
    return std::move(n).child(std::move(children));
}

}  // namespace skeuo
