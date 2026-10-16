#include <cstdio>

#include "skeuo/data_display.hpp"
#include "skeuo/map.hpp"

namespace skeuo {

namespace {
std::string fixed1(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.1f", v);
    return buf;
}
}  // namespace

Node Navlog(const NavlogProps& props) {
    auto legs = route_legs(props.waypoints);
    std::optional<double> gs;
    if (props.ground_speed_kt && *props.ground_speed_kt > 0) gs = props.ground_speed_kt;
    double total = legs.empty() ? 0.0 : legs.back().cumulative_nm;
    auto right = [](std::string text) { return TableCell({.align = TableCellAlign::Right}, std::move(text)); };

    Node head_row = TableRow({}, {
        TableCell({}, "Leg"), TableCell({}, "From"), TableCell({}, "To"),
        right("TC °T"), right("Dist NM"), right("Cum NM"),
    });
    if (gs) {
        head_row.kids().push_back(right("ETE"));
        head_row.kids().push_back(right("Cum ETE"));
    }

    Node body;
    if (legs.empty()) {
        body.add_child(TableRow({}, TableCell({.col_span = gs ? 8u : 6u}, "Add at least two waypoints to see the legs.")));
    }
    for (const Leg& leg : legs) {
        Node row = TableRow(
            {.hover = true,
             .selected = props.selected == leg.from + 1,
             .onclick = props.onselect ? std::function<void()>([cb = props.onselect, i = leg.from + 1] { cb(i); })
                                       : std::function<void()>()},
            {
                TableCell({}, std::to_string(leg.from + 1)),
                TableCell({}, props.waypoints[leg.from].ident),
                TableCell({}, props.waypoints[leg.from + 1].ident),
                right(format_course(leg.course_deg)),
                right(fixed1(leg.distance_nm)),
                right(fixed1(leg.cumulative_nm)),
            });
        if (gs) {
            // TableRow wraps its cells; append to the <tr> itself.
            row.kids().push_back(right(format_hours(leg.distance_nm / *gs)));
            row.kids().push_back(right(format_hours(leg.cumulative_nm / *gs)));
        }
        body.add_child(std::move(row));
    }

    Node table_children{TableHead({}, std::move(head_row)), TableBody({}, std::move(body))};
    if (!legs.empty()) {
        Node foot = TableRow({}, {TableCell({.col_span = 5u}, "Total"), right(fixed1(total))});
        if (gs) {
            foot.kids().push_back(TableCell());
            foot.kids().push_back(right(format_hours(total / *gs)));
        }
        table_children.add_child(TableFooter({}, std::move(foot)));
    }
    return TableContainer({.cls = props.cls},
                          Table({.size = props.size, .color = props.color, .aria_label = "Navigation log"},
                                std::move(table_children)));
}

}  // namespace skeuo
