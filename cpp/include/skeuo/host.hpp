// skeuo-ui for C++ — hosts that show a Runtime.
#pragma once

#include <string>

#include "skeuo/runtime.hpp"

namespace skeuo {

struct WindowOptions {
    std::string title = "skeuo-ui";
    int width = 1280;  // logical pixels (scaled for the monitor's DPI)
    int height = 860;
    int min_width = 640;
    int min_height = 480;
};

/// A native window with an embedded webview (Windows: WebView2). Blocks until
/// the window is closed; returns the process exit code. On platforms without
/// a native host it prints a message and returns 1.
int run_window(Runtime& runtime, const WindowOptions& options);

/// Headless host for tests: writes {"t":"page","html":...} as the first line
/// of stdout, then answers each JSON message read from stdin with one line.
int run_stdio(Runtime& runtime);

/// `--stdio` runs the stdio host, anything else opens the window.
int run_app(Runtime& runtime, const WindowOptions& options, int argc, char** argv);

}  // namespace skeuo
