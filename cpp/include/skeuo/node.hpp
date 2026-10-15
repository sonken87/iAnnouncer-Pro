// skeuo-ui for C++ — the UI tree.
//
// A render function builds a tree of `Node`s (elements, text, raw HTML,
// fragments). The runtime serializes it to HTML for the webview and keeps the
// event handlers attached to it; when the user interacts, the matching
// handler runs in C++ and the tree is rendered again.
#pragma once

#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace skeuo {

/// What the webview reports about a DOM event.
struct Event {
    std::string type;    // "click", "input", "change", "keydown", ...
    std::string value;   // input / select / textarea value
    bool checked = false;  // checkbox / radio state after the event
    std::string key;     // keydown/keyup: KeyboardEvent.key ("Enter", "ArrowUp", "a")
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
    bool meta = false;
};

using Handler = std::function<void(const Event&)>;

/// Flags for `Node::on`.
enum EventFlag : unsigned {
    NoFlags = 0,
    StopPropagation = 1,  // ancestors' handlers for the same event don't run
    PreventDefault = 2,   // the browser's default action is cancelled
};

class Node {
public:
    enum class Kind { Fragment, Element, Text, Raw };

    struct Listener {
        std::string type;
        Handler handler;
        unsigned flags = NoFlags;
    };

    /// An empty fragment (renders nothing).
    Node() = default;
    /// A text node (escaped when serialized).
    Node(const char* text);
    Node(std::string text);
    Node(std::string_view text);
    /// A fragment of several nodes: `{Icon(...), "Save"}`.
    Node(std::initializer_list<Node> children);
    explicit Node(std::vector<Node> children);

    static Node element(std::string tag);
    static Node text(std::string text);
    /// Trusted HTML inserted verbatim (e.g. SVG markup, stylesheets).
    static Node raw(std::string html);
    static Node fragment(std::vector<Node> children = {});

    // Builders. Each has an lvalue (`Node&`) and an rvalue (`Node&&`) form so
    // both `node.cls("x")` and `el("div").cls("x").child(...)` chain cheaply.

    /// Appends class names (empty strings are ignored).
    Node& cls(std::string_view names) & { add_class(names); return *this; }
    Node&& cls(std::string_view names) && { add_class(names); return std::move(*this); }

    /// Sets an attribute (replacing an existing one of the same name).
    Node& attr(std::string name, std::string value) & { set_attr(std::move(name), std::move(value)); return *this; }
    Node&& attr(std::string name, std::string value) && { set_attr(std::move(name), std::move(value)); return std::move(*this); }

    /// Sets an attribute only when `when` is true.
    Node& attr_if(bool when, std::string name, std::string value) & { if (when) set_attr(std::move(name), std::move(value)); return *this; }
    Node&& attr_if(bool when, std::string name, std::string value) && { if (when) set_attr(std::move(name), std::move(value)); return std::move(*this); }

    /// Boolean attribute (`disabled`, `checked`, ...): present when `on`.
    Node& flag(std::string name, bool on) & { if (on) set_attr(std::move(name), ""); else remove_attr(name); return *this; }
    Node&& flag(std::string name, bool on) && { if (on) set_attr(std::move(name), ""); else remove_attr(name); return std::move(*this); }

    /// Appends inline CSS declarations.
    Node& style(std::string_view css) & { add_style(css); return *this; }
    Node&& style(std::string_view css) && { add_style(css); return std::move(*this); }

    /// Listens to a DOM event (`"click"`, `"input"`, `"keydown"`, `"mouseenter"`, ...).
    Node& on(std::string type, Handler handler, unsigned flags = NoFlags) & { add_listener(std::move(type), std::move(handler), flags); return *this; }
    Node&& on(std::string type, Handler handler, unsigned flags = NoFlags) && { add_listener(std::move(type), std::move(handler), flags); return std::move(*this); }

    /// Shorthand for a handler that ignores the event payload.
    Node& on(std::string type, std::function<void()> handler, unsigned flags = NoFlags) & { add_listener(std::move(type), wrap(std::move(handler)), flags); return *this; }
    Node&& on(std::string type, std::function<void()> handler, unsigned flags = NoFlags) && { add_listener(std::move(type), wrap(std::move(handler)), flags); return std::move(*this); }

    /// Appends a child (fragments are flattened, empty fragments skipped).
    Node& child(Node node) & { add_child(std::move(node)); return *this; }
    Node&& child(Node node) && { add_child(std::move(node)); return std::move(*this); }

    /// Appends several children.
    Node& children(std::vector<Node> nodes) & { for (auto& n : nodes) add_child(std::move(n)); return *this; }
    Node&& children(std::vector<Node> nodes) && { for (auto& n : nodes) add_child(std::move(n)); return std::move(*this); }

    Kind kind() const { return kind_; }
    const std::string& tag() const { return tag_; }
    /// Text of a text node, or the HTML of a raw node.
    const std::string& content() const { return content_; }
    const std::vector<std::pair<std::string, std::string>>& attrs() const { return attrs_; }
    const std::vector<Listener>& listeners() const { return listeners_; }
    std::vector<Listener>& listeners() { return listeners_; }
    const std::vector<Node>& kids() const { return children_; }
    std::vector<Node>& kids() { return children_; }

    /// The value of an attribute, or nullptr.
    const std::string* get_attr(std::string_view name) const;
    bool has_class(std::string_view name) const;
    bool empty() const { return kind_ == Kind::Fragment && children_.empty(); }

    void set_attr(std::string name, std::string value);
    void remove_attr(std::string_view name);
    void add_class(std::string_view names);
    void add_style(std::string_view css);
    void add_listener(std::string type, Handler handler, unsigned flags);
    void add_child(Node node);

    /// Calls `visit` on this node and every descendant (pre-order). Used by
    /// container components to configure the children they were given.
    void walk(const std::function<void(Node&)>& visit);

private:
    static Handler wrap(std::function<void()> f);

    Kind kind_ = Kind::Fragment;
    std::string tag_;
    std::string content_;
    std::vector<std::pair<std::string, std::string>> attrs_;
    std::vector<Listener> listeners_;
    std::vector<Node> children_;
};

/// An element: `el("div").cls("x").child("hello")`.
inline Node el(std::string tag) { return Node::element(std::move(tag)); }
inline Node text(std::string s) { return Node::text(std::move(s)); }
inline Node raw_html(std::string html) { return Node::raw(std::move(html)); }

/// Joins the non-empty class names with spaces.
std::string classes(std::initializer_list<std::string_view> parts);

/// Escapes text for HTML content and attribute values.
std::string escape_html(std::string_view s);

}  // namespace skeuo
