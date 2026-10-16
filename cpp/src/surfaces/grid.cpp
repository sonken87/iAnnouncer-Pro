#include <charconv>

#include "skeuo/surfaces.hpp"

namespace skeuo {

namespace {

constexpr const char* kBreakpoints[] = {"xs", "sm", "md", "lg", "xl"};

/// Formats like Rust's `Display` for f32 (shortest round-trip: 16, 12.5).
std::string fmt_f32(float v) {
    char buf[32];
    auto res = std::to_chars(buf, buf + sizeof buf, v);
    return std::string(buf, res.ptr);
}

bool starts_with_at(const std::string& s, size_t pos, std::string_view prefix) {
    return s.compare(pos, prefix.size(), prefix) == 0;
}

/// Where a Grid's parent variables go in its style: after the container
/// declarations Grid() writes first (gaps, then the optional flex props).
size_t parent_vars_offset(const Node& grid) {
    if (!grid.has_class("skeuo-grid--container")) return 0;
    const std::string* style = grid.get_attr("style");
    if (!style) return 0;
    size_t pos = 0;
    auto skip_decl = [&] {
        size_t end = style->find(';', pos);
        pos = end == std::string::npos ? style->size() : end + 1;
    };
    skip_decl();  // --grid-row-gap
    skip_decl();  // --grid-column-gap
    for (std::string_view prop : {"flex-direction:", "align-items:", "justify-content:", "flex-wrap:"})
        if (starts_with_at(*style, pos, prop)) skip_decl();
    return pos;
}

/// Tells the Grid items in a container's subtree (down to and including the
/// next nested containers) the container's columns and column gap — what the
/// Rust Grid reads from context.
void configure_items(Node& node, const std::string& vars) {
    if (node.kind() == Node::Kind::Element && node.has_class("skeuo-grid")) {
        std::string style = node.get_attr("style") ? *node.get_attr("style") : std::string();
        style.insert(parent_vars_offset(node), vars);
        node.set_attr("style", style);
        if (node.has_class("skeuo-grid--container")) return;  // its items are its own
    }
    for (auto& kid : node.kids()) configure_items(kid, vars);
}

}  // namespace

Node Grid(const GridProps& props, Node children) {
    float column_gap = static_cast<float>(props.column_spacing.value_or(props.spacing)) * 8.0f;
    float row_gap = static_cast<float>(props.row_spacing.value_or(props.spacing)) * 8.0f;

    std::vector<std::string> class_list = {"skeuo-grid"};
    std::string style;
    if (props.container) {
        class_list.push_back("skeuo-grid--container");
        style += "--grid-row-gap:" + fmt_f32(row_gap) + "px;--grid-column-gap:" + fmt_f32(column_gap) + "px;";
        for (const auto& [prop, value] : {std::pair{"flex-direction", &props.direction},
                                          std::pair{"align-items", &props.align_items},
                                          std::pair{"justify-content", &props.justify_content},
                                          std::pair{"flex-wrap", &props.wrap}}) {
            if (*value) style += std::string(prop) + ":" + **value + ";";
        }
        std::string vars = "--grid-parent-columns:" + std::to_string(props.columns < 1 ? 1 : props.columns) +
                           ";--grid-parent-gap:" + fmt_f32(column_gap) + "px;";
        configure_items(children, vars);
    }
    for (size_t bp = 0; bp < 5; ++bp) {
        std::string name = kBreakpoints[bp];
        if (const auto& size = props.size.values[bp]) {
            switch (size->kind) {
                case GridSize::Kind::Columns:
                    class_list.push_back("skeuo-grid--" + name + "-n");
                    style += "--grid-size-" + name + ":" + std::to_string(size->columns) + ";";
                    break;
                case GridSize::Kind::Auto: class_list.push_back("skeuo-grid--" + name + "-auto"); break;
                case GridSize::Kind::Grow: class_list.push_back("skeuo-grid--" + name + "-grow"); break;
            }
        }
        if (const auto& offset = props.offset.values[bp]) {
            if (offset->kind == GridSize::Kind::Columns) {
                class_list.push_back("skeuo-grid--" + name + "-offset");
                style += "--grid-offset-" + name + ":" + std::to_string(offset->columns) + ";";
            } else {
                class_list.push_back("skeuo-grid--" + name + "-offset-auto");
            }
        }
    }
    style += props.style;
    class_list.push_back(props.cls);

    std::string cls;
    for (const auto& c : class_list) {
        if (c.empty()) continue;
        if (!cls.empty()) cls += ' ';
        cls += c;
    }
    return el("div").cls(cls).attr("style", style).child(std::move(children));
}

}  // namespace skeuo
