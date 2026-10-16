//! Data display: Avatar, Badge, Chip, Divider, List, Table, Tooltip, Typography, ImageList.

use dioxus::prelude::*;

mod avatar;
mod badge;
mod chip;
mod divider;
mod image_list;
mod list;
mod table;
mod tooltip;
mod typography;

pub use avatar::{
    Avatar, AvatarGroup, AvatarGroupProps, AvatarGroupSpacing, AvatarProps, AvatarVariant,
};
pub use badge::{
    Badge, BadgeAnchorOrigin, BadgeContent, BadgeHorizontal, BadgeOverlap, BadgeProps,
    BadgeVariant, BadgeVertical,
};
pub use chip::{Chip, ChipProps, ChipVariant};
pub use divider::{Divider, DividerProps, DividerTextAlign, DividerVariant};
pub use image_list::{
    ImageList, ImageListItem, ImageListItemBar, ImageListItemBarPosition, ImageListItemBarProps,
    ImageListItemProps, ImageListProps, ImageListVariant,
};
pub use list::{
    List, ListItem, ListItemAvatar, ListItemAvatarProps, ListItemButton, ListItemButtonProps,
    ListItemIcon, ListItemIconProps, ListItemProps, ListItemText, ListItemTextProps, ListProps,
    ListSubheader, ListSubheaderProps,
};
pub use table::{
    SortDirection, Table, TableBody, TableBodyProps, TableCell, TableCellAlign, TableCellPadding,
    TableCellProps, TableCellVariant, TableContainer, TableContainerProps, TableFooter,
    TableFooterProps, TableHead, TableHeadProps, TableProps, TableRow, TableRowProps,
    TableSortLabel, TableSortLabelProps,
};
pub use tooltip::{Tooltip, TooltipProps};
pub use typography::{
    Typography, TypographyAlign, TypographyColor, TypographyProps, TypographyVariant,
};

/// True when an `Element` prop was left at its default (no children given).
pub(crate) fn is_empty(element: &Element) -> bool {
    match element {
        Ok(vnode) => *vnode == VNode::placeholder(),
        Err(_) => true,
    }
}

/// `None` for an empty string, so no empty `style=""` is rendered.
pub(crate) fn non_empty(s: String) -> Option<String> {
    if s.is_empty() { None } else { Some(s) }
}
