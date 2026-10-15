#include "skeuo/node.hpp"

#include <algorithm>

namespace skeuo {

Node::Node(const char* text) : kind_(Kind::Text), content_(text ? text : "") {}
Node::Node(std::string text) : kind_(Kind::Text), content_(std::move(text)) {}
Node::Node(std::string_view text) : kind_(Kind::Text), content_(text) {}

Node::Node(std::initializer_list<Node> children) {
    for (const auto& c : children) add_child(c);
}

Node::Node(std::vector<Node> children) {
    for (auto& c : children) add_child(std::move(c));
}

Node Node::element(std::string tag) {
    Node n;
    n.kind_ = Kind::Element;
    n.tag_ = std::move(tag);
    return n;
}

Node Node::text(std::string text) { return Node(std::move(text)); }

Node Node::raw(std::string html) {
    Node n;
    n.kind_ = Kind::Raw;
    n.content_ = std::move(html);
    return n;
}

Node Node::fragment(std::vector<Node> children) { return Node(std::move(children)); }

const std::string* Node::get_attr(std::string_view name) const {
    for (const auto& [k, v] : attrs_)
        if (k == name) return &v;
    return nullptr;
}

bool Node::has_class(std::string_view name) const {
    const std::string* c = get_attr("class");
    if (!c) return false;
    std::string_view s = *c;
    size_t pos = 0;
    while (pos <= s.size()) {
        size_t end = s.find(' ', pos);
        if (end == std::string_view::npos) end = s.size();
        if (s.substr(pos, end - pos) == name) return true;
        pos = end + 1;
    }
    return false;
}

void Node::set_attr(std::string name, std::string value) {
    for (auto& [k, v] : attrs_) {
        if (k == name) {
            v = std::move(value);
            return;
        }
    }
    attrs_.emplace_back(std::move(name), std::move(value));
}

void Node::remove_attr(std::string_view name) {
    attrs_.erase(std::remove_if(attrs_.begin(), attrs_.end(),
                                [&](const auto& a) { return a.first == name; }),
                 attrs_.end());
}

void Node::add_class(std::string_view names) {
    if (names.empty()) return;
    for (auto& [k, v] : attrs_) {
        if (k == "class") {
            if (!v.empty()) v += ' ';
            v += names;
            return;
        }
    }
    attrs_.emplace_back("class", std::string(names));
}

void Node::add_style(std::string_view css) {
    if (css.empty()) return;
    for (auto& [k, v] : attrs_) {
        if (k == "style") {
            if (!v.empty() && v.back() != ';') v += ';';
            v += css;
            return;
        }
    }
    attrs_.emplace_back("style", std::string(css));
}

void Node::add_listener(std::string type, Handler handler, unsigned flags) {
    if (!handler) return;
    listeners_.push_back({std::move(type), std::move(handler), flags});
}

void Node::add_child(Node node) {
    if (node.kind_ == Kind::Fragment) {
        for (auto& c : node.children_) add_child(std::move(c));
        return;
    }
    if (node.kind_ == Kind::Text && node.content_.empty()) return;
    children_.push_back(std::move(node));
}

void Node::walk(const std::function<void(Node&)>& visit) {
    visit(*this);
    for (auto& c : children_) c.walk(visit);
}

Handler Node::wrap(std::function<void()> f) {
    if (!f) return {};
    return [f = std::move(f)](const Event&) { f(); };
}

std::string classes(std::initializer_list<std::string_view> parts) {
    std::string out;
    for (auto p : parts) {
        if (p.empty()) continue;
        if (!out.empty()) out += ' ';
        out += p;
    }
    return out;
}

std::string escape_html(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c;
        }
    }
    return out;
}

}  // namespace skeuo
