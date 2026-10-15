#include "skeuo/surfaces.hpp"

namespace skeuo {

Node Card(const CardProps& props, Node children) {
    int elevation = props.raised ? 4 : props.elevation;
    Node div = el("div").cls(
        classes({chassis_classes(props.variant, elevation, props.square), "skeuo-card", props.cls}));
    for (const auto& [k, v] : props.attrs) div.set_attr(k, v);
    return std::move(div).child(std::move(children));
}

Node CardHeader(const CardHeaderProps& props) {
    Node root = el("div").cls(classes({"skeuo-card-header", props.cls}));
    if (props.avatar) root.child(el("div").cls("skeuo-card-header__avatar").child(*props.avatar));
    Node content = el("div").cls("skeuo-card-header__content");
    if (props.title) content.child(el("span").cls("skeuo-card-header__title").child(*props.title));
    if (props.subheader)
        content.child(el("span").cls("skeuo-card-header__subheader skeuo-caption").child(*props.subheader));
    root.child(std::move(content));
    if (props.action) root.child(el("div").cls("skeuo-card-header__action").child(*props.action));
    return root;
}

Node CardMedia(const CardMediaProps& props, Node children) {
    std::string height = props.height ? "height:" + *props.height + ";" : "";
    if (props.component && *props.component == "img") {
        return el("img")
            .cls(classes({"skeuo-card-media", "skeuo-card-media--img", props.cls}))
            .attr("src", props.image.value_or(""))
            .attr("alt", props.alt.value_or(""))
            .attr("style", height + props.style);
    }
    std::string bg = props.image ? "background-image:url(\"" + *props.image + "\");" : "";
    Node div = el("div").cls(classes({"skeuo-card-media", props.cls})).attr("style", bg + height + props.style);
    if (props.title) div.attr("role", "img").attr("aria-label", *props.title);
    return std::move(div).child(std::move(children));
}

Node CardContent(const CardContentProps& props, Node children) {
    return el("div").cls(classes({"skeuo-card-content", props.cls})).child(std::move(children));
}

Node CardActions(const CardActionsProps& props, Node children) {
    return el("div")
        .cls(classes({"skeuo-card-actions", props.disable_spacing ? "" : "skeuo-card-actions--spacing", props.cls}))
        .child(std::move(children));
}

Node CardActionArea(const CardActionAreaProps& props, Node children) {
    Node button = el("button")
                      .cls(classes({"skeuo-card-action-area", props.cls}))
                      .attr("type", "button")
                      .flag("disabled", props.disabled);
    if (props.onclick) button.on("click", props.onclick);
    return std::move(button).child(std::move(children));
}

}  // namespace skeuo
