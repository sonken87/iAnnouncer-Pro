# skeuo-ui

用 Rust（[Dioxus](https://dioxuslabs.com) 0.7）编写的**拟物化** UI 组件库，目标是替代 MUI。

- **API 对齐 MUI**：`variant`、`color`、`size`、`disabled`、`label`、`helper_text`、`error`……从 MUI 迁移只需改语法。
- **拟物化外观，设计语言参照 Apple Magic Keyboard**：哑光喷砂铝、低键程的平顶键帽、无高光无浮雕；统一顶部光源，凸起物体向下投出柔和阴影，凹陷物体顶部内缘有阴影。配色采用 Apple 系统色（高对比度版本）。
- **跨平台**：同一套组件可运行在 Web（WASM）、桌面和移动端，也支持 SSR。样式是纯 CSS，不需要额外构建步骤。

![gallery](docs/gallery.png)

## 组件

### Button（对应 MUI `<Button>`）

| variant | 外观 |
|---|---|
| `Contained`（默认） | 彩色键帽，白色字符 |
| `Outlined` | 经典白色键帽，彩色字符和描边（`Neutral` 色即标准 Magic Keyboard 按键） |
| `Text` | 直接印在铝面板上的字符，没有键帽 |

所有按键都有一条很薄的键帽侧边（即键程），按下时键帽下移、侧边收起，底边位置不变，和真实按键一样。键盘聚焦时显示 macOS 风格的焦点环。

属性：`color`（Primary / Secondary / Success / Error / Warning / Info / Neutral）、`size`、`disabled`、`full_width`、`r#type`、`start_icon`、`end_icon`、`onclick`、`class`。任意 HTML 全局属性（`id`、`aria_label`、`style`……）会透传到 `<button>`。

### TextField（对应 MUI `<TextField>`）

| variant | 外观 |
|---|---|
| `Outlined`（默认） | 刻进面板的凹槽输入框 |
| `Filled` | 黑色边框里的背光荧光显示屏（VFD） |
| `Standard` | 面板上的一条刻线 |

属性：`label`、`value` / `default_value`（受控 / 非受控）、`placeholder`、`helper_text`、`error`、`disabled`、`required`、`read_only`、`full_width`、`multiline` + `rows`、`r#type`、`name`、`id`（不传则自动生成）、`autocomplete`、`autofocus`、`start_adornment`、`end_adornment`、`color`（焦点光晕颜色）、`size`，以及事件 `oninput` / `onchange` / `onkeydown` / `onfocus` / `onblur`。

`label` 和 `helper_text` 会自动通过 `for`、`aria-describedby` 关联；设置 `error` 时会同时设置 `aria-invalid`。

## 使用

```rust
use dioxus::prelude::*;
use skeuo_ui::prelude::*;

fn App() -> Element {
    let mut name = use_signal(String::new);
    rsx! {
        // 相当于 MUI 的 ThemeProvider + CssBaseline，在应用最外层包一次即可
        ThemeProvider {
            TextField {
                label: "Name",
                value: name(),
                oninput: move |e: FormEvent| name.set(e.value()),
                helper_text: "Required",
                required: true,
            }
            Button { onclick: move |_| save(), "Save" }
            Button { variant: ButtonVariant::Outlined, color: Color::Error, "Delete" }
        }
    }
}
```

自定义主题（相当于 `createTheme`）：

```rust
let mut theme = Theme::default();
// light / main / dark（键帽侧边）/ 文字色 / 焦点光晕
theme.primary = PaletteColor::new("#ff9f0a", "#c93400", "#8f2500", "#fff", "rgba(201,52,0,.35)");
theme.radius = 4.0;
rsx! { ThemeProvider { theme, /* … */ } }
```

## 开发

```bash
cargo test --workspace                       # SSR 渲染测试
dx serve -p gallery                          # 在浏览器中运行组件展示页（需要 dioxus-cli）
dx serve -p gallery --features desktop --no-default-features   # 桌面版
cargo run -p gallery --example snapshot -- gallery.html         # 不装 dx，直接渲染出静态 HTML
```

## 目录

```
crates/skeuo-ui/    组件库（src/ 组件，assets/skeuo.css 样式）
crates/gallery/     组件展示页
design/             设计稿（toggle-switch.cdr：三段拨杆开关）
```
