#include "detail.hpp"
#include "skeuo/icons.hpp"

namespace skeuo {

namespace {
using data_display_detail::flag;

// ---------------------------------------------------------------- cell resolution
//
// Rust resolves a cell from the TableContext (size, padding, sticky header)
// and the section context (head / body / footer). In C++ the cell is built
// first, so it records its own explicit props in `data-sk-cell` and the
// enclosing TableHead / TableFooter / Table rebuild it once they know more:
//
//   data-sk-cell="<variant><padding><size><sort><scope?><section><align>|<class>"
//
// variant h/b/f/-   padding n/c/0/-   size s/m/l/-   sort a/d/-
// scope 1 = explicit   section h/b/f   align i/l/c/r/j

constexpr const char* kCellData = "data-sk-cell";

struct TableCtx {
    Size size = Size::Medium;
    bool sticky_header = false;
    TableCellPadding padding = TableCellPadding::Normal;
};

char padding_code(TableCellPadding p) {
    switch (p) {
        case TableCellPadding::Normal: return 'n';
        case TableCellPadding::Checkbox: return 'c';
        case TableCellPadding::None: return '0';
    }
    return 'n';
}

TableCellPadding padding_from(char c) {
    return c == 'c' ? TableCellPadding::Checkbox : c == '0' ? TableCellPadding::None : TableCellPadding::Normal;
}

char size_code(Size s) { return s == Size::Small ? 's' : s == Size::Large ? 'l' : 'm'; }

char align_code(TableCellAlign a) {
    switch (a) {
        case TableCellAlign::Inherit: return 'i';
        case TableCellAlign::Left: return 'l';
        case TableCellAlign::Center: return 'c';
        case TableCellAlign::Right: return 'r';
        case TableCellAlign::Justify: return 'j';
    }
    return 'i';
}

std::string_view align_class(char a) {
    switch (a) {
        case 'l': return "skeuo-table-cell--align-left";
        case 'c': return "skeuo-table-cell--align-center";
        case 'r': return "skeuo-table-cell--align-right";
        case 'j': return "skeuo-table-cell--align-justify";
        default: return "";
    }
}

/// Rebuilds a cell for `section` (0 = keep its current one) in a table with
/// `ctx`. `final` drops the helper attribute.
void resolve_cell(Node& n, char section, const TableCtx& ctx, bool final) {
    const std::string* data = n.get_attr(kCellData);
    if (!data || data->size() < 8) return;
    std::string d = *data;
    if (section) d[5] = section;
    char v = d[0], p = d[1], s = d[2], o = d[3], scope_explicit = d[4], sec = d[5], a = d[6];
    std::string user_cls = d.substr(8);

    char variant = v != '-' ? v : sec;
    bool small = s != '-' ? s == 's' : ctx.size == Size::Small;
    TableCellPadding padding = p != '-' ? padding_from(p) : ctx.padding;
    bool head = variant == 'h';

    Node out = el(head ? "th" : "td");
    out.set_attr("class", classes({
                              "skeuo-table-cell",
                              variant == 'h'   ? "skeuo-table-cell--head"
                              : variant == 'f' ? "skeuo-table-cell--footer"
                                               : "skeuo-table-cell--body",
                              align_class(a),
                              padding == TableCellPadding::Checkbox ? "skeuo-table-cell--padding-checkbox"
                              : padding == TableCellPadding::None   ? "skeuo-table-cell--padding-none"
                                                                    : "",
                              flag(small, "skeuo-table-cell--small"),
                              flag(ctx.sticky_header && sec == 'h', "skeuo-table-cell--sticky"),
                              user_cls,
                          }));
    if (const std::string* style = n.get_attr("style")) out.set_attr("style", *style);
    const std::string* scope = n.get_attr("scope");
    if (scope_explicit == '1' && scope) out.set_attr("scope", *scope);
    else if (head) out.set_attr("scope", sec == 'h' ? "col" : "row");
    if (const std::string* c = n.get_attr("colspan")) out.set_attr("colspan", *c);
    if (const std::string* r = n.get_attr("rowspan")) out.set_attr("rowspan", *r);
    if (head && o != '-') out.set_attr("aria-sort", o == 'a' ? "ascending" : "descending");
    if (!final) out.set_attr(kCellData, d);
    out.listeners() = std::move(n.listeners());
    out.kids() = std::move(n.kids());
    n = std::move(out);
}

bool is_table(const Node& n) { return n.has_class("skeuo-table"); }

/// Turns the rows and cells under a TableHead / TableFooter into that section's.
void apply_section(Node& children, char section, std::string_view row_class) {
    auto visit = [&](Node& n) {
        if (n.get_attr(kCellData)) resolve_cell(n, section, TableCtx{}, false);
        else if (n.tag() == "tr" && n.has_class("skeuo-table-row"))
            data_display_detail::insert_class_after_first(n, row_class);
    };
    if (children.kind() == Node::Kind::Element && !is_table(children)) visit(children);
    data_display_detail::walk_children(children, is_table, visit);
}

Node section_node(const char* tag, std::string_view base, const std::string& cls, Node children) {
    return el(tag).cls(classes({base, cls})).child(std::move(children));
}
}  // namespace

Node TableContainer(const TableContainerProps& props, Node children) {
    int elevation = props.elevation < 0 ? 0 : props.elevation > 4 ? 4 : props.elevation;
    std::string elev = "skeuo-elev-" + std::to_string(elevation);
    std::string style = props.max_height ? "max-height: " + *props.max_height + ";" + props.style : props.style;
    Node div = el("div").cls(classes({"skeuo-chassis", elev, "skeuo-table-container", props.cls}));
    if (!style.empty()) div.attr("style", style);
    return std::move(div).child(std::move(children));
}

Node Table(const TableProps& props, Node children) {
    TableCtx ctx{props.size, props.sticky_header, props.padding};
    auto visit = [&](Node& n) {
        if (n.get_attr(kCellData)) resolve_cell(n, 0, ctx, true);
    };
    if (children.kind() == Node::Kind::Element) visit(children);
    data_display_detail::walk_children(children, is_table, visit);

    Node table = el("table").cls(classes({
        "skeuo-table",
        color_class(props.color),
        props.size == Size::Small ? "skeuo-table--small" : "skeuo-table--medium",
        flag(props.sticky_header, "skeuo-table--sticky-header"),
        props.cls,
    }));
    if (props.aria_label) table.attr("aria-label", *props.aria_label);
    return std::move(table).child(std::move(children));
}

Node TableHead(const TableHeadProps& props, Node children) {
    apply_section(children, 'h', "skeuo-table-row--head");
    return section_node("thead", "skeuo-table-head", props.cls, std::move(children));
}

Node TableBody(const TableBodyProps& props, Node children) {
    return section_node("tbody", "skeuo-table-body", props.cls, std::move(children));
}

Node TableFooter(const TableFooterProps& props, Node children) {
    apply_section(children, 'f', "skeuo-table-row--footer");
    return section_node("tfoot", "skeuo-table-footer", props.cls, std::move(children));
}

Node TableRow(const TableRowProps& props, Node children) {
    Node tr = el("tr").cls(classes({
        "skeuo-table-row",
        flag(props.hover, "skeuo-table-row--hover"),
        flag(props.selected, "skeuo-table-row--selected"),
        flag(static_cast<bool>(props.onclick), "skeuo-table-row--clickable"),
        props.cls,
    }));
    if (props.selected) tr.attr("aria-selected", "true");
    if (props.onclick) tr.on("click", props.onclick);
    return std::move(tr).child(std::move(children));
}

Node TableCell(const TableCellProps& props, Node children) {
    std::string data;
    data += props.variant ? (*props.variant == TableCellVariant::Head   ? 'h'
                             : *props.variant == TableCellVariant::Footer ? 'f'
                                                                          : 'b')
                          : '-';
    data += props.padding ? padding_code(*props.padding) : '-';
    data += props.size ? size_code(*props.size) : '-';
    data += props.sort_direction ? (*props.sort_direction == SortDirection::Asc ? 'a' : 'd') : '-';
    data += props.scope ? '1' : '0';
    data += 'b';
    data += align_code(props.align);
    data += '|';
    data += props.cls;

    Node cell = el("td");
    if (!props.style.empty()) cell.set_attr("style", props.style);
    if (props.scope) cell.set_attr("scope", *props.scope);
    if (props.col_span) cell.set_attr("colspan", std::to_string(*props.col_span));
    if (props.row_span) cell.set_attr("rowspan", std::to_string(*props.row_span));
    cell.set_attr(kCellData, data);
    cell.child(std::move(children));
    resolve_cell(cell, 0, TableCtx{}, false);
    return cell;
}

Node TableSortLabel(const TableSortLabelProps& props, Node children) {
    Node b = el("button")
                 .cls(classes({
                     "skeuo-table-sort-label",
                     flag(props.active, "skeuo-table-sort-label--active"),
                     props.direction == SortDirection::Asc ? "skeuo-table-sort-label--asc"
                                                           : "skeuo-table-sort-label--desc",
                     props.cls,
                 }))
                 .attr("type", "button");
    if (props.onclick) b.on("click", props.onclick);
    b.child(std::move(children));
    if (!props.hide_sort_icon || props.active)
        b.child(Icon({.name = IconName::ArrowForward, .cls = "skeuo-table-sort-label__icon"}));
    return b;
}

}  // namespace skeuo
