# skeuo-ui design language

**Apple Magic Keyboard, applied to a whole UI.** Matte bead-blasted aluminium,
flat low-profile keycaps with short travel, wells cut into the metal. No gloss,
no embossed or engraved text effects on controls, no saturated gradients. One
light source straight above: raised things cast a soft shadow downward,
recessed things are shaded on their top inner edge.

## Materials

Every component is assembled from these materials (`assets/materials.css`).
Component CSS composes them and sets their variables; it does **not** invent
new shadows, bevels or gradients.

| Material | Class | Used for |
|---|---|---|
| Keycap | `.skeuo-key` (+ `--pressable`, `--filled`, `--round`, `--down`) | anything you press or grab: buttons, toggle buttons, FAB, checkbox/radio caps, slider & switch thumbs, pagination items, the selected segment of tabs |
| Well | `.skeuo-well` (+ `--focusable`, `--shallow`) | anything that holds a value you type or pick: text fields, selects, autocomplete, segmented-control trays, list boxes |
| Groove | `.skeuo-groove` + `.skeuo-groove__fill` | narrow channels: slider rail, switch track, linear progress |
| Chassis | `.skeuo-chassis` + `.skeuo-elev-0…4` (+ `--float`, `--square`) | surfaces: paper, cards, accordions, app bars, drawers, menus, popovers, dialogs, snackbars, tooltips |
| Rule | `.skeuo-rule` (+ `--vertical`) | dividers, separators inside lists and tables |
| LED | `.skeuo-led` (+ `--off`) | badges, status dots, the "on" light of toggles |
| Lettering | `.skeuo-label`, `.skeuo-caption` | labels, helper text, secondary text |
| Scrim | `.skeuo-scrim` | behind modal surfaces (dialog, drawer, backdrop) |

### Elevation

| Level | Shadow | MUI equivalent |
|---|---|---|
| 0 | none (flush) | `elevation={0}`, outlined |
| 1 | resting | 1–2 (cards, paper) |
| 2 | raised panel | 3–6 (app bar, raised cards) |
| 3 | floating | 7–12 (menus, popovers, tooltips, snackbars, speed dial) |
| 4 | modal | 13–24 (dialogs, drawers) |

Floating surfaces (level ≥ 3) add `.skeuo-chassis--float` (slightly lighter,
closer to the light) and enter with the `skeuo-rise-in` animation.

## Color

- Palette: Apple system colors, increased-contrast variants (`Theme::default()`).
- A component's `color` prop adds `Color::class()` (`skeuo-color-*`), which sets
  `--c-light / --c-main / --c-dark / --c-contrast / --c-glow / --c-ink / --c-tint`.
  `.skeuo-root` defaults them to primary.
- Color appears in only four ways:
  1. a **colored keycap** (`.skeuo-key--filled`): top `--c-main`, side `--c-dark`, legend `--c-contrast`;
  2. **ink**: lettering/icons in `--c-ink` on a light surface;
  3. a **fill** of a groove or a checked cap (progress, slider, switch on, checked box);
  4. a **tint** (`--c-tint`) wash for selected rows, soft chips, alerts; and the
     **glow** (`--c-glow`) focus ring.
- Neutral text: `--sk-ink` (primary), `--sk-ink-2` (secondary), `--sk-ink-3` (placeholder/disabled).

## Dark theme — Space Black

`ThemeMode::Dark` (or `System` when the OS is dark) is the Space Black Magic
Keyboard: black keycaps on dark anodised aluminium with light-grey legends.
The light still comes from above, so the materials keep their geometry; only
tokens change (`assets/dark.css`):

| | Light | Dark |
|---|---|---|
| chassis | silver aluminium `#f1f2f4 → #e3e4e7` | dark aluminium `#3a3a3d → #2f2f32` |
| keycap | white `#fbfbfc`, grey side | black `#1e1e20`, near-black side, faint top sheen |
| well / groove | light floor, soft shade under the lip | black pit, heavy shade, faint catch-light on the lower lip |
| ink | `#1d1d1f` / `#6e6e73` / `#a1a1a6` | `#f5f5f7` / `#a1a1a6` / `#6e6e73` |
| colored lettering | palette `main` | palette `light` (bright variant) |
| shadows | soft | heavier (darker surfaces need more contrast) |

Rules for component CSS, so both modes keep working:
- Never hard-code a surface, ink, edge or shadow color; use the `--sk-*` tokens
  (`--sk-cut` / `--sk-catch` for engraved lines, `--sk-key-edge`,
  `--sk-chassis-edge`, `--sk-elev-N`, `--sk-error-ink`, …).
- Lettering in a palette color uses `--c-ink`, never `--c-dark`.
- Each mode has its own palette (`Theme.light`, `Theme.dark`); the root carries
  both, and the stylesheet selects one by `data-skeuo-mode`.

## Shape, type, motion

- Radii: keycap `--sk-radius-key` (6px), well `--sk-radius-well` (8px),
  chassis `--sk-radius-chassis` (14px), pills/round knobs `999px`.
- Type: system font (SF Pro on Apple), base 14px. Legends 13px/500. Labels 12px/600
  `--sk-ink-2`, sentence case. Headings tight letter-spacing (−0.01 to −0.02em).
- Sizes: `small` / `medium` / `large` → control heights 28 / 34 / 44px.
- Motion: key travel `--sk-fast` (60ms), state changes `--sk-medium` (160ms),
  panels `--sk-slow` (260ms), easing `--sk-ease`. Respect `prefers-reduced-motion`
  (handled globally in `tokens.css`).

## States

| State | Treatment |
|---|---|
| hover | keycap: `brightness(1.035)`; lettering on aluminium: `--sk-hover` wash |
| pressed | keycap travels down (`:active` or latched `.skeuo-key--down`); aluminium: `--sk-press` wash |
| selected / checked | keycap latched down and/or filled with color; rows get `--c-tint` |
| focus (keyboard) | 3px `--c-glow` ring (`:focus-visible` only) |
| disabled | `opacity: .45`, `grayscale(.6)`, `cursor: not-allowed`, no hover/press |
| error | `color: Error` palette: well rim, label and helper text turn error |

## Component API conventions (MUI parity)

- Component and prop names follow MUI, in Rust case: `helperText` → `helper_text`,
  `onChange` → `onchange`, `variant="outlined"` → `variant: XxxVariant::Outlined`.
- Every component takes `class: String` and passes it to its root element.
- Controlled + uncontrolled: `value` / `checked` / `open` (controlled) and
  `default_value` / `default_checked` (uncontrolled, read once at mount);
  callbacks are `Option<EventHandler<…>>`.
- Shared enums: `Color`, `Size`, `Orientation`, `Placement` (crate root).
- DOM ids come from `use_unique_id(prefix)`; labels are linked with `for` /
  `aria-labelledby`, helper text with `aria-describedby`.
- Icons come from `Icon { name: IconName::… }`.
- Use native elements and ARIA roles (`button`, `input type=checkbox`,
  `role="switch"`, `role="slider"`, `role="tablist"`, `role="dialog"` + `aria-modal`, …).
  Overlays close on `Escape` and on a click outside / on the scrim.
- z-index scale (as MUI): app bar 1100, drawer 1200, modal/dialog 1300,
  snackbar 1400, tooltip 1500.
- Names that would shadow Rust's prelude get a `Skeuo` prefix (MUI `Box` → `SkeuoBox`).
