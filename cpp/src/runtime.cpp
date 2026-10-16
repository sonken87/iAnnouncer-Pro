#include "skeuo/runtime.hpp"

#include <algorithm>

#include "skeuo/json.hpp"
#include "skeuo/theme.hpp"
#include "assets.hpp"

namespace skeuo {

// ---------------------------------------------------------------- hooks

void HookScope::begin() {
    counters_.clear();
    prefixes_.clear();
    for (auto& [_, slot] : slots_) slot.touched = false;
}

void HookScope::end() {
    for (auto it = slots_.begin(); it != slots_.end();) {
        if (it->second.touched) ++it;
        else it = slots_.erase(it);
    }
}

void HookScope::push_key(std::string_view key) {
    std::string prefix = prefixes_.empty() ? std::string() : prefixes_.back();
    prefixes_.push_back(prefix + std::string(key) + "/");
}

void HookScope::pop_key() {
    if (!prefixes_.empty()) prefixes_.pop_back();
}

std::string HookScope::next_key(std::string_view kind) {
    std::string scoped = (prefixes_.empty() ? std::string() : prefixes_.back()) + std::string(kind);
    int n = counters_[scoped]++;
    return scoped + "#" + std::to_string(n);
}

std::string HookScope::unique_id(std::string_view prefix) {
    return slot<std::string>(std::string("id:") + std::string(prefix), [&] {
        return std::string(prefix) + "-" + std::to_string(next_id_++);
    });
}

namespace {
thread_local HookScope* active_scope = nullptr;

HookScope& fallback_scope() {
    static thread_local HookScope scope;
    return scope;
}

struct ScopeGuard {
    HookScope* previous;
    HookScope& scope;
    explicit ScopeGuard(HookScope& s) : previous(active_scope), scope(s) {
        active_scope = &s;
        s.begin();
    }
    ~ScopeGuard() {
        scope.end();
        active_scope = previous;
    }
};
}  // namespace

HookScope& current_hooks() { return active_scope ? *active_scope : fallback_scope(); }

std::string use_unique_id(std::string_view prefix) { return current_hooks().unique_id(prefix); }

KeyScope::KeyScope(std::string_view key) { current_hooks().push_key(key); }
KeyScope::~KeyScope() { current_hooks().pop_key(); }

Node keyed(std::string_view key, const std::function<Node()>& fn) {
    KeyScope scope(key);
    return fn();
}

// ---------------------------------------------------------------- HTML

namespace {

bool is_void(const std::string& tag) {
    static const char* const tags[] = {"area", "base", "br", "col", "embed", "hr", "img",
                                       "input", "link", "meta", "source", "track", "wbr"};
    return std::any_of(std::begin(tags), std::end(tags), [&](const char* t) { return tag == t; });
}

void write(const Node& n, std::string& out, std::vector<Handler>* handlers) {
    switch (n.kind()) {
        case Node::Kind::Text: out += escape_html(n.content()); return;
        case Node::Kind::Raw: out += n.content(); return;
        case Node::Kind::Fragment:
            for (const auto& c : n.kids()) write(c, out, handlers);
            return;
        case Node::Kind::Element: break;
    }
    out += '<';
    out += n.tag();
    for (const auto& [k, v] : n.attrs()) {
        out += ' ';
        out += k;
        if (!v.empty()) {
            out += "=\"";
            out += escape_html(v);
            out += '"';
        }
    }
    if (handlers && !n.listeners().empty()) {
        // data-sk-on="click:12,keydown:13!~"  (! = stop propagation, ~ = prevent default)
        out += " data-sk-on=\"";
        bool first = true;
        for (const auto& l : n.listeners()) {
            if (!first) out += ',';
            first = false;
            out += escape_html(l.type);
            out += ':';
            out += std::to_string(handlers->size());
            if (l.flags & StopPropagation) out += '!';
            if (l.flags & PreventDefault) out += '~';
            handlers->push_back(l.handler);
        }
        out += '"';
    }
    out += '>';
    if (is_void(n.tag())) return;
    for (const auto& c : n.kids()) write(c, out, handlers);
    out += "</";
    out += n.tag();
    out += '>';
}

}  // namespace

std::string to_html(const Node& node, std::vector<Handler>* handlers) {
    std::string out;
    write(node, out, handlers);
    return out;
}

std::string render_static(const std::function<Node()>& fn) {
    HookScope scope;
    Node tree;
    {
        ScopeGuard guard(scope);
        tree = fn();
    }
    return to_html(tree);
}

std::string_view runtime_script() { return assets::runtime_js; }

// ---------------------------------------------------------------- Runtime

Runtime::Runtime(RenderFn render, std::string title)
    : render_(std::move(render)), title_(std::move(title)) {}

std::string Runtime::render() {
    Node tree;
    {
        ScopeGuard guard(hooks_);
        tree = render_();
    }
    handlers_.clear();
    return to_html(tree, &handlers_);
}

std::string Runtime::page() {
    std::string body = render();
    std::string out;
    out.reserve(body.size() + stylesheet().size() + runtime_script().size() + assets::leaflet_js.size() +
                assets::map_js.size() + 1024);
    out += "<!doctype html><html><head><meta charset=\"utf-8\">"
           "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"><title>";
    out += escape_html(title_);
    out += "</title><style>";
    out += stylesheet();
    out += "</style>";
    out += head_;
    out += "</head><body><div id=\"sk-root\">";
    out += body;
    // Leaflet + the map widget (MapView), then the event bridge.
    out += "</div><script>";
    out += assets::leaflet_js;
    out += "</script><script>";
    out += assets::map_js;
    out += "</script><script>";
    out += runtime_script();
    out += "</script></body></html>";
    return out;
}

void Runtime::dispatch(const std::vector<int>& handler_ids, const Event& event) {
    // Copy first: a handler may trigger nothing, but the vector is only
    // replaced on the next render, so ids stay valid for the whole dispatch.
    for (int id : handler_ids) {
        if (id >= 0 && size_t(id) < handlers_.size() && handlers_[size_t(id)]) handlers_[size_t(id)](event);
    }
}

std::string Runtime::handle(std::string_view message) {
    json::Value msg = json::parse(message);
    std::string type = msg.str("t");
    if (type == "event") {
        std::vector<int> ids;
        if (const json::Value* h = msg.get("h"); h && h->type == json::Value::Type::Array)
            for (const auto& v : h->array)
                if (v.type == json::Value::Type::Number) ids.push_back(int(v.number));
        Event e;
        if (const json::Value* d = msg.get("e")) {
            e.type = d->str("type");
            e.value = d->str("value");
            e.checked = d->flag("checked");
            e.key = d->str("key");
            e.shift = d->flag("shift");
            e.ctrl = d->flag("ctrl");
            e.alt = d->flag("alt");
            e.meta = d->flag("meta");
            e.related_inside = d->flag("relatedInside");
        }
        dispatch(ids, e);
    }
    std::string reply = "{\"t\":\"render\",\"seq\":";
    reply += std::to_string(long(msg.num("seq", 0)));
    reply += ",\"html\":";
    reply += json::quote(render());
    reply += '}';
    return reply;
}

}  // namespace skeuo
