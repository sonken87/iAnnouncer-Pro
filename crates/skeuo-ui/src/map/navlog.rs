use dioxus::prelude::*;

use super::geo::{format_course, format_hours};
use super::map_view::{Waypoint, route_legs};
use crate::data_display::{
    Table, TableBody, TableCell, TableCellAlign, TableContainer, TableFooter, TableHead, TableRow,
};
use crate::{Color, Size};

/// A navigation log: one row per leg with true course, distance, cumulative
/// distance and, given a ground speed, the estimated time en route. Built
/// from the data-display Table, so it sits on an aluminium plate with
/// engraved rules. Not part of MUI.
#[derive(Props, Clone, PartialEq)]
pub struct NavlogProps {
    pub waypoints: Vec<Waypoint>,
    /// Ground speed in knots; adds the ETE columns.
    pub ground_speed_kt: Option<f64>,
    /// Highlights the leg ending at this waypoint (e.g. the map's selection).
    pub selected: Option<usize>,
    #[props(default)]
    pub color: Color,
    #[props(default = Size::Small)]
    pub size: Size,
    #[props(default, into)]
    pub class: String,
    /// Called with the index of the waypoint a clicked leg ends at.
    pub onselect: Option<EventHandler<usize>>,
}

#[component]
pub fn Navlog(props: NavlogProps) -> Element {
    let legs = route_legs(&props.waypoints);
    let gs = props.ground_speed_kt.filter(|g| *g > 0.0);
    let total = legs.last().map_or(0.0, |l| l.cumulative_nm);
    let onselect = props.onselect;
    rsx! {
        TableContainer { class: props.class,
            Table { size: props.size, color: props.color, aria_label: "Navigation log",
                TableHead {
                    TableRow {
                        TableCell { "Leg" }
                        TableCell { "From" }
                        TableCell { "To" }
                        TableCell { align: TableCellAlign::Right, "TC °T" }
                        TableCell { align: TableCellAlign::Right, "Dist NM" }
                        TableCell { align: TableCellAlign::Right, "Cum NM" }
                        if gs.is_some() {
                            TableCell { align: TableCellAlign::Right, "ETE" }
                            TableCell { align: TableCellAlign::Right, "Cum ETE" }
                        }
                    }
                }
                TableBody {
                    if legs.is_empty() {
                        TableRow {
                            TableCell { col_span: if gs.is_some() { 8 } else { 6 },
                                "Add at least two waypoints to see the legs."
                            }
                        }
                    }
                    for leg in legs.iter().copied() {
                        TableRow {
                            hover: true,
                            selected: props.selected == Some(leg.from + 1),
                            onclick: move |_| {
                                if let Some(h) = &onselect {
                                    h.call(leg.from + 1);
                                }
                            },
                            TableCell { "{leg.from + 1}" }
                            TableCell { "{props.waypoints[leg.from].ident}" }
                            TableCell { "{props.waypoints[leg.from + 1].ident}" }
                            TableCell { align: TableCellAlign::Right, "{format_course(leg.course_deg)}" }
                            TableCell { align: TableCellAlign::Right, "{leg.distance_nm:.1}" }
                            TableCell { align: TableCellAlign::Right, "{leg.cumulative_nm:.1}" }
                            if let Some(gs) = gs {
                                TableCell { align: TableCellAlign::Right, "{format_hours(leg.distance_nm / gs)}" }
                                TableCell { align: TableCellAlign::Right, "{format_hours(leg.cumulative_nm / gs)}" }
                            }
                        }
                    }
                }
                if !legs.is_empty() {
                    TableFooter {
                        TableRow {
                            TableCell { col_span: 5, "Total" }
                            TableCell { align: TableCellAlign::Right, "{total:.1}" }
                            if let Some(gs) = gs {
                                TableCell {}
                                TableCell { align: TableCellAlign::Right, "{format_hours(total / gs)}" }
                            }
                        }
                    }
                }
            }
        }
    }
}
