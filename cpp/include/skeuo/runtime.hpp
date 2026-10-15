// skeuo-ui for C++ — the runtime that connects a render function to a webview.
//
//   Runtime rt([&] { return App(state); });
//   rt.page()            -> the HTML document to load into the webview
//   rt.handle(message)   -> runs the C++ handlers for a DOM event and returns
//                           the re-rendered body for the page to patch in
//
// Hosts (host/*.cpp) only move strings between the webview and `handle`.
#pragma once

#include <any>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "skeuo/node.hpp"

namespace skeuo {

/// Persistent state for components between renders, keyed by position: the
/// n-th `use_state` call of a given kind in a render gets the same slot every
/// render (like React hooks). Slots not used during a render are dropped, so a
/// component that disappears starts fresh when it comes back.
class HookScope {
public:
    void begin();
    void end();

    template <class T, class Init>
    T& slot(std::string_view kind, Init&& init) {
        std::string key = next_key(kind);
        auto it = slots_.find(key);
        if (it == slots_.end() || !std::any_cast<T>(&it->second.value)) {
            it = slots_.insert_or_assign(key, Slot{std::any(T(init())), true}).first;
        }
        it->second.touched = true;
        return *std::any_cast<T>(&it->second.value);
    }

    std::string unique_id(std::string_view prefix);

private:
    struct Slot {
        std::any value;
        bool touched = false;
    };
    std::string next_key(std::string_view kind);

    std::unordered_map<std::string, Slot> slots_;
    std::unordered_map<std::string, int> counters_;
    unsigned long long next_id_ = 0;
};

/// The scope hooks use during the current render (a process-wide fallback
/// scope outside of `Runtime` renders, e.g. in unit tests).
HookScope& current_hooks();

/// Component state that survives re-renders. `init` runs once.
/// Example: `bool& open = use_state<bool>("Select.open", [] { return false; });`
template <class T, class Init>
T& use_state(std::string_view kind, Init&& init) {
    return current_hooks().template slot<T>(kind, std::forward<Init>(init));
}

/// A DOM id unique in the process and stable while the component is rendered.
std::string use_unique_id(std::string_view prefix);

class Runtime {
public:
    using RenderFn = std::function<Node()>;

    explicit Runtime(RenderFn render, std::string title = "skeuo-ui");

    /// Extra markup for <head> (e.g. an app stylesheet).
    void set_head(std::string html) { head_ = std::move(html); }

    /// The complete HTML document: stylesheet, the first render, and the
    /// script that wires events back to `handle`.
    std::string page();

    /// Handles one JSON message from the page and returns the JSON reply.
    ///   {"t":"ready"}                                   -> {"t":"render","html":...}
    ///   {"t":"event","h":[ids],"e":{type,value,...}}    -> runs handlers, re-renders
    std::string handle(std::string_view message);

    /// Renders the body (registering handlers) and returns its HTML.
    std::string render();

    /// Runs the handlers with these ids from the last render.
    void dispatch(const std::vector<int>& handler_ids, const Event& event);

    /// Number of handlers registered by the last render.
    size_t handler_count() const { return handlers_.size(); }

private:
    RenderFn render_;
    std::string title_;
    std::string head_;
    HookScope hooks_;
    std::vector<Handler> handlers_;
};

/// Serializes a tree to HTML. With `handlers`, listeners are registered and
/// referenced from `data-sk-on`; without, they are dropped (static output).
std::string to_html(const Node& node, std::vector<Handler>* handlers = nullptr);

/// Renders `fn` once to static HTML (hooks get a throwaway scope). For
/// snapshots and tests.
std::string render_static(const std::function<Node()>& fn);

/// The JavaScript that runs in the page (event bridge + DOM patching).
std::string_view runtime_script();

}  // namespace skeuo
