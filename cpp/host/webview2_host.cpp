// Windows host: a Win32 window with an embedded WebView2.
//
// The page posts each event as a web message; the message handler runs the
// C++ handlers through Runtime::handle and posts the reply back. Everything
// happens on the UI thread, in order.
#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <atomic>
#include <functional>
#include <string>
#include <string_view>

#include "WebView2.h"
#include "skeuo/host.hpp"

namespace skeuo {
namespace {

std::wstring widen(std::string_view s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), nullptr, 0);
    std::wstring out(size_t(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), int(s.size()), out.data(), n);
    return out;
}

std::string narrow(const wchar_t* s) {
    if (!s || !*s) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, s, -1, nullptr, 0, nullptr, nullptr);
    std::string out(size_t(n > 0 ? n - 1 : 0), '\0');
    WideCharToMultiByte(CP_UTF8, 0, s, -1, out.data(), n, nullptr, nullptr);
    return out;
}

/// A COM object implementing one WebView2 callback interface with a lambda.
template <class Interface, class... Args>
class Callback final : public Interface {
public:
    using Fn = std::function<HRESULT(Args...)>;
    Callback(const IID& iid, Fn fn) : iid_(iid), fn_(std::move(fn)) {}

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** out) override {
        if (!out) return E_POINTER;
        if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, iid_)) {
            *out = static_cast<Interface*>(this);
            AddRef();
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return ++refs_; }
    ULONG STDMETHODCALLTYPE Release() override {
        ULONG n = --refs_;
        if (n == 0) delete this;
        return n;
    }
    HRESULT STDMETHODCALLTYPE Invoke(Args... args) override { return fn_(args...); }

private:
    IID iid_;
    Fn fn_;
    std::atomic<ULONG> refs_{1};
};

struct Host {
    Runtime* runtime = nullptr;
    WindowOptions options;
    HWND hwnd = nullptr;
    ICoreWebView2Controller* controller = nullptr;
    ICoreWebView2* webview = nullptr;
    UINT dpi = 96;
};

Host* g_host = nullptr;

int scale(int logical, UINT dpi) { return MulDiv(logical, int(dpi), 96); }

void resize_webview() {
    if (!g_host || !g_host->controller) return;
    RECT bounds;
    GetClientRect(g_host->hwnd, &bounds);
    g_host->controller->put_Bounds(bounds);
}

void fail(const wchar_t* what, HRESULT hr) {
    wchar_t buf[512];
    wsprintfW(buf,
              L"%s (0x%08lX).\n\nThis program needs the Microsoft Edge WebView2 Runtime, which "
              L"ships with Windows 10 (April 2018 update or later) and Windows 11. Install the "
              L"Evergreen runtime from https://developer.microsoft.com/microsoft-edge/webview2/",
              what, static_cast<unsigned long>(hr));
    MessageBoxW(g_host ? g_host->hwnd : nullptr, buf, L"skeuo-ui", MB_ICONERROR | MB_OK);
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_SIZE:
            resize_webview();
            return 0;
        case WM_DPICHANGED: {
            if (g_host) g_host->dpi = HIWORD(wp);
            const RECT* r = reinterpret_cast<const RECT*>(lp);
            SetWindowPos(hwnd, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
                         SWP_NOZORDER | SWP_NOACTIVATE);
            return 0;
        }
        case WM_GETMINMAXINFO:
            if (g_host) {
                auto* info = reinterpret_cast<MINMAXINFO*>(lp);
                info->ptMinTrackSize.x = scale(g_host->options.min_width, g_host->dpi);
                info->ptMinTrackSize.y = scale(g_host->options.min_height, g_host->dpi);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

using CreateEnvironmentFn = HRESULT(STDAPICALLTYPE*)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*,
                                                     ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*);

void enable_dpi_awareness() {
    // Per-monitor v2 when available (Windows 10 1703+).
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        using SetCtx = BOOL(WINAPI*)(HANDLE);
        if (auto set = reinterpret_cast<SetCtx>(
                reinterpret_cast<void*>(GetProcAddress(user32, "SetProcessDpiAwarenessContext")))) {
            set(reinterpret_cast<HANDLE>(-4) /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 */);
            return;
        }
        using SetAware = BOOL(WINAPI*)();
        if (auto set = reinterpret_cast<SetAware>(
                reinterpret_cast<void*>(GetProcAddress(user32, "SetProcessDPIAware"))))
            set();
    }
}

UINT window_dpi(HWND hwnd) {
    if (HMODULE user32 = GetModuleHandleW(L"user32.dll")) {
        using GetDpi = UINT(WINAPI*)(HWND);
        if (auto get = reinterpret_cast<GetDpi>(
                reinterpret_cast<void*>(GetProcAddress(user32, hwnd ? "GetDpiForWindow" : "GetDpiForSystem")))) {
            return hwnd ? get(hwnd) : reinterpret_cast<UINT(WINAPI*)()>(reinterpret_cast<void*>(get))();
        }
    }
    return 96;
}

std::wstring user_data_folder() {
    // WebView2 keeps its profile here; next to the exe may not be writable.
    wchar_t base[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", base, MAX_PATH);
    std::wstring dir = (n > 0 && n < MAX_PATH) ? std::wstring(base, n) : L".";
    wchar_t exe[MAX_PATH];
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring name = exe;
    size_t slash = name.find_last_of(L"\\/");
    if (slash != std::wstring::npos) name = name.substr(slash + 1);
    size_t dot = name.rfind(L'.');
    if (dot != std::wstring::npos) name = name.substr(0, dot);
    return dir + L"\\" + name + L"\\WebView2";
}

void on_webview_ready(ICoreWebView2Controller* controller) {
    g_host->controller = controller;
    controller->AddRef();
    controller->get_CoreWebView2(&g_host->webview);
    resize_webview();

    ICoreWebView2Settings* settings = nullptr;
    if (SUCCEEDED(g_host->webview->get_Settings(&settings)) && settings) {
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDefaultContextMenusEnabled(FALSE);
        settings->put_IsZoomControlEnabled(FALSE);
        settings->Release();
    }

    EventRegistrationToken token;
    g_host->webview->add_WebMessageReceived(
        new Callback<ICoreWebView2WebMessageReceivedEventHandler, ICoreWebView2*,
                     ICoreWebView2WebMessageReceivedEventArgs*>(
            IID_ICoreWebView2WebMessageReceivedEventHandler,
            [](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                LPWSTR message = nullptr;
                if (FAILED(args->TryGetWebMessageAsString(&message)) || !message) return S_OK;
                std::string reply = g_host->runtime->handle(narrow(message));
                CoTaskMemFree(message);
                sender->PostWebMessageAsString(widen(reply).c_str());
                return S_OK;
            }),
        &token);

    std::wstring page = widen(g_host->runtime->page());
    g_host->webview->NavigateToString(page.c_str());
    g_host->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
}

}  // namespace

int run_window(Runtime& runtime, const WindowOptions& options) {
    Host host;
    host.runtime = &runtime;
    host.options = options;
    g_host = &host;

    enable_dpi_awareness();
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        fail(L"Could not initialise COM", E_FAIL);
        return 1;
    }

    HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, reinterpret_cast<LPCWSTR>(IDC_ARROW));
    wc.hbrBackground = CreateSolidBrush(RGB(0xd6, 0xd7, 0xdb));
    wc.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(1));
    wc.lpszClassName = L"SkeuoUiWindow";
    RegisterClassExW(&wc);

    host.dpi = window_dpi(nullptr);
    std::wstring title = widen(options.title);
    host.hwnd = CreateWindowExW(0, wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                CW_USEDEFAULT, scale(options.width, host.dpi), scale(options.height, host.dpi),
                                nullptr, nullptr, instance, nullptr);
    if (!host.hwnd) {
        fail(L"Could not create the window", HRESULT_FROM_WIN32(GetLastError()));
        return 1;
    }
    host.dpi = window_dpi(host.hwnd);
    ShowWindow(host.hwnd, SW_SHOWDEFAULT);
    UpdateWindow(host.hwnd);

    // Load the WebView2 loader at run time so no import library is needed;
    // WebView2Loader.dll ships next to the exe.
    HMODULE loader = LoadLibraryW(L"WebView2Loader.dll");
    auto create_environment =
        loader ? reinterpret_cast<CreateEnvironmentFn>(
                     reinterpret_cast<void*>(GetProcAddress(loader, "CreateCoreWebView2EnvironmentWithOptions")))
               : nullptr;
    if (!create_environment) {
        fail(L"WebView2Loader.dll was not found next to the program", HRESULT_FROM_WIN32(GetLastError()));
        return 1;
    }

    std::wstring data_dir = user_data_folder();
    HRESULT hr = create_environment(
        nullptr, data_dir.c_str(), nullptr,
        new Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler, HRESULT,
                     ICoreWebView2Environment*>(
            IID_ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler,
            [](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (FAILED(result) || !env) {
                    fail(L"Could not start WebView2", result);
                    PostQuitMessage(1);
                    return S_OK;
                }
                return env->CreateCoreWebView2Controller(
                    g_host->hwnd,
                    new Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler, HRESULT,
                                 ICoreWebView2Controller*>(
                        IID_ICoreWebView2CreateCoreWebView2ControllerCompletedHandler,
                        [](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                            if (FAILED(result) || !controller) {
                                fail(L"Could not create the WebView2 control", result);
                                PostQuitMessage(1);
                                return S_OK;
                            }
                            on_webview_ready(controller);
                            return S_OK;
                        }));
            }));
    if (FAILED(hr)) {
        fail(L"Could not start WebView2", hr);
        return 1;
    }

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (host.webview) host.webview->Release();
    if (host.controller) {
        host.controller->Close();
        host.controller->Release();
    }
    g_host = nullptr;
    CoUninitialize();
    return int(msg.wParam);
}

}  // namespace skeuo

#endif  // _WIN32
