#include <algorithm>
#include <cctype>
#include <charconv>

#include "detail.hpp"
#include "skeuo/icons.hpp"
#include "skeuo/runtime.hpp"

namespace skeuo {

namespace data_display_detail {
std::string format_number(double v) {
    float f = float(v);
    char buf[64];
    auto res = std::to_chars(buf, buf + sizeof buf, f);
    return std::string(buf, res.ptr);
}
}  // namespace data_display_detail

namespace {

std::string_view variant_name(AvatarVariant v) {
    switch (v) {
        case AvatarVariant::Circular: return "circular";
        case AvatarVariant::Rounded: return "rounded";
        case AvatarVariant::Square: return "square";
    }
    return "circular";
}

bool is_blank(const Node& n) { return n.empty() || (n.kind() == Node::Kind::Text && n.content().empty()); }

/// The first character of `s`, upper-cased (ASCII letters only; other
/// characters are kept as they are).
std::string first_letter(const std::string& s) {
    if (s.empty()) return {};
    unsigned char c = static_cast<unsigned char>(s[0]);
    size_t len = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
    if (len == 1) return std::string(1, char(std::toupper(c)));
    return s.substr(0, len);
}

// The runtime forwards no `error` event, so a failed image reports itself by
// re-dispatching its (non-bubbling) `error` as a bubbling event type the
// runtime does forward. Only the <img>'s own listener receives it
// (StopPropagation), and nothing else on an <img> fires `animationend`.
constexpr const char* kImageErrorBridge = "this.dispatchEvent(new Event('animationend',{bubbles:true}))";

}  // namespace

Node Avatar(const AvatarProps& props, Node children) {
    // Keyed by the URL so that avatars appearing or disappearing elsewhere on
    // the page don't hand their failure state to each other.
    bool* failed = nullptr;
    if (props.src) failed = &use_state<bool>("Avatar.failed:" + *props.src, [] { return false; });
    bool has_image = props.src && !*failed;

    std::string variant = "skeuo-avatar--" + std::string(variant_name(props.variant));
    std::string size = "skeuo-avatar--" + std::string(as_str(props.size));
    std::string_view material = has_image ? "skeuo-well skeuo-avatar--img"
                                : (props.color && *props.color != Color::Neutral)
                                    ? "skeuo-key skeuo-key--filled skeuo-avatar--letter"
                                    : "skeuo-key skeuo-avatar--letter";
    Node root = el("span").cls(classes({material, "skeuo-avatar", variant, size,
                                        props.color ? color_class(*props.color) : std::string_view(), props.cls}));
    if (!props.style.empty()) root.attr("style", props.style);

    if (has_image) {
        Node img = el("img").cls("skeuo-avatar__img").attr("src", *props.src);
        if (props.src_set) img.attr("srcset", *props.src_set);
        img.attr("alt", props.alt.value_or(""));
        img.attr("onerror", kImageErrorBridge);
        img.on("animationend", [failed] { *failed = true; }, StopPropagation);
        return std::move(root).child(std::move(img));
    }

    if (props.alt) root.attr("role", "img").attr("aria-label", *props.alt);
    if (!is_blank(children)) {
        root.child(std::move(children));
    } else if (props.alt && !props.alt->empty()) {
        root.child(first_letter(*props.alt));
    } else {
        root.child(Icon({.name = IconName::Person, .cls = "skeuo-avatar__fallback"}));
    }
    return root;
}

Node AvatarGroup(const AvatarGroupProps& props) {
    // Same arithmetic as MUI.
    size_t count = props.avatars.size();
    size_t total = props.total.value_or(count);
    size_t clamped_max = std::max<size_t>(props.max, 2);
    if (total == clamped_max) ++clamped_max;
    clamped_max = std::min(clamped_max, total + 1);
    size_t shown = std::min(count, clamped_max - 1);
    auto sat_sub = [](size_t a, size_t b) { return a > b ? a - b : 0; };
    size_t extra = std::max(sat_sub(total, clamped_max), sat_sub(total, shown));

    Node group = el("div")
                     .cls(classes({"skeuo-avatar-group", props.cls}))
                     .attr("style", "--avatar-overlap: " + data_display_detail::format_number(props.spacing.overlap) + "px")
                     .attr("role", "group");
    for (size_t i = 0; i < shown; ++i)
        group.child(el("span").cls("skeuo-avatar-group__item").child(props.avatars[i]));
    if (extra > 0) {
        std::string n = std::to_string(extra);
        group.child(el("span").cls("skeuo-avatar-group__item").child(
            Avatar({.alt = n + " more", .variant = props.variant, .size = props.size, .cls = "skeuo-avatar--surplus"},
                   "+" + n)));
    }
    return group;
}

}  // namespace skeuo
