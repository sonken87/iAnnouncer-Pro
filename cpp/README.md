# skeuo-ui · C++

skeuo-ui 的 C++20 版本：同一套 **Apple Magic Keyboard 拟物化**组件、同一套 MUI 风格 API，和 Rust 版（`crates/`）外观逐像素一致。

- **界面渲染在 WebView 中**（Windows：WebView2）。样式表**直接复用** Rust crate 的 `crates/skeuo-ui/assets/*.css`，构建时嵌入二进制；改一处，两个版本同时生效。
- **逻辑全部在 C++**：组件是普通函数，返回一棵 `Node` 树；点击、输入等事件回到 C++ 回调，C++ 更新状态后重新渲染，页面只就地更新变化的部分（保留焦点、光标和正在播放的动画）。
- 浅色 / 深色（Space Black）/ 跟随系统三种外观。

## 示例

```cpp
#include "skeuo/skeuo.hpp"
using namespace skeuo;

int main(int argc, char** argv) {
    std::string name;
    bool on_air = false;

    Runtime app([&] {
        return ThemeProvider({.theme = Theme::make_system()}, {
            TextField({.label = "Name", .value = name,
                       .oninput = [&](const std::string& v) { name = v; }}),
            FormControlLabel({.control = Switch({.checked = on_air,
                                                 .onchange = [&](bool v) { on_air = v; }}),
                              .label = "On air"}),
            Button({.variant = ButtonVariant::Contained, .onclick = [&] { /* save */ }}, "Save"),
        });
    });
    return run_app(app, {.title = "My app"}, argc, argv);
}
```

- Props 是聚合体，用 C++20 指定初始化器按 MUI 属性名传参（字段需按声明顺序）。`class` 是关键字，所以类名属性叫 `cls`；`inline` 同理叫 `inline_`。
- 回调直接拿到值：文本 `void(const std::string&)`、勾选 `void(bool)`、数值 `void(double)`。
- 组件内部状态用 `use_state<T>(...)`（类似 React hooks，按位置保存）。渲染列表或可切换的区域时，用 `KeyScope` / `keyed()` 给每一项独立的状态命名空间。

## 组件

与 Rust 版相同（约 90 个）：Button、IconButton、Fab、ButtonGroup、ToggleButton、TextField、TextareaAutosize、Select、NativeSelect、Autocomplete、TransferList；Checkbox、Radio、Switch、Slider、Rating、Form 系列；Typography、Avatar、Badge、Chip、Divider、List、Table、Tooltip、ImageList；Alert、Backdrop、Modal、Dialog、Progress、Skeleton、Snackbar；Paper、Card、Accordion、AppBar、Drawer、SkeuoBox、Container、Stack、Grid；Tabs、Breadcrumbs、Link、Menu、Popover、Pagination、Stepper、BottomNavigation、SpeedDial、Timeline；Icon、ThemeProvider。

头文件按分类：`include/skeuo/{inputs,selection,data_display,feedback,surfaces,navigation}.hpp`，总入口 `skeuo/skeuo.hpp`。

## 构建

需要 CMake 3.20+ 和支持 C++20 的编译器（GCC 11+ / Clang 14+ / MSVC 2022）。

```bash
# Windows（Visual Studio 或 MinGW）
cmake -S cpp -B build && cmake --build build --config Release
#   → build/gallery.exe + WebView2Loader.dll

# 在 Linux 上交叉编译 Windows 版并打包
../scripts/build-windows-cpp.sh        # → dist/skeuo-ui-cpp-gallery-windows-x64.zip

# Linux/macOS：编译库、测试和无界面运行模式（原生窗口目前只有 Windows）
cmake -S cpp -B build && cmake --build build && ./build/skeuo_tests
./build/gallery --snapshot out/        # 把每个页面渲染成静态 HTML（浅色 + 深色）
```

## 测试

- `skeuo_tests`：单元测试（渲染结果、ARIA、事件到 C++ 回调的连线）。
- `tools/live-*.js`：用 `tools/harness.js` 以 `--stdio` 模式启动真实的 C++ 程序，在 Chromium 里点击、输入、按键，验证交互：

```bash
PLAYWRIGHT_CORE=/path/to/node_modules/playwright-core node tools/live-inputs.js build/gallery
```

## 目录

```
include/skeuo/      公共头文件
src/                组件实现（按分类）、运行时、主题、图标
runtime/skeuo.js    页面内运行时：事件转发 + DOM 就地更新
host/               窗口宿主：webview2_host.cpp（Windows）、stdio_host.cpp（测试用）
gallery/            示例程序（每个分类一个页面）
tests/              单元测试
tools/              资源嵌入脚本、交互测试
third_party/webview2/  WebView2 SDK（NuGet，BSD 许可）
```

## 已知限制

- 原生窗口目前只实现了 Windows（WebView2）。macOS（WKWebView）/ Linux（WebKitGTK）宿主可以按 `host/webview2_host.cpp` 的协议补上；页面运行时已支持 WebKit 的消息通道。
- 每次事件都会整棵重新渲染并比对，适合常规桌面界面；超大列表请自行分页。
