#include <charconv>
#include <cmath>

#include "detail.hpp"

namespace skeuo::nav {

std::vector<std::string> class_tokens(const Node& n) {
    std::vector<std::string> out;
    const std::string* c = n.get_attr("class");
    if (!c) return out;
    std::string_view s = *c;
    size_t pos = 0;
    while (pos < s.size()) {
        size_t end = s.find(' ', pos);
        if (end == std::string_view::npos) end = s.size();
        if (end > pos) out.emplace_back(s.substr(pos, end - pos));
        pos = end + 1;
    }
    return out;
}

void insert_class_after(Node& n, std::string_view after, std::string_view extra) {
    if (extra.empty() || n.has_class(extra)) return;
    std::vector<std::string> tokens = class_tokens(n);
    size_t at = 0;
    for (size_t i = 0; i < tokens.size(); ++i)
        if (tokens[i] == after) at = i + 1;
    tokens.insert(tokens.begin() + static_cast<std::ptrdiff_t>(at), std::string(extra));
    std::string joined;
    for (const auto& t : tokens) {
        if (!joined.empty()) joined += ' ';
        joined += t;
    }
    n.set_attr("class", joined);
}

void prepend_listener(Node& n, const std::string& type, Handler handler, unsigned flags) {
    for (auto& l : n.listeners()) {
        if (l.type != type) continue;
        l.handler = [first = std::move(handler), second = l.handler](const Event& e) {
            first(e);
            if (second) second(e);
        };
        l.flags |= flags;
        return;
    }
    n.add_listener(type, std::move(handler), flags);
}

std::string id_part(std::string_view value) {
    std::string out;
    for (unsigned char c : value) {
        bool alnum = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        // Rust maps each *char* (not byte) to '-'; skip UTF-8 continuation bytes.
        if (alnum) out += static_cast<char>(c);
        else if ((c & 0xC0) != 0x80) out += '-';
    }
    return out;
}

std::string fmt_num(double v) {
    if (v == std::floor(v) && std::fabs(v) < 1e15) return std::to_string(static_cast<long long>(v));
    char buf[64];
    auto res = std::to_chars(buf, buf + sizeof buf, v);
    return std::string(buf, res.ptr);
}

std::optional<std::size_t> roving_neighbor(const std::vector<bool>& disabled, std::optional<std::size_t> from,
                                           int step) {
    const long n = static_cast<long>(disabled.size());
    if (n == 0) return std::nullopt;
    long i = from && static_cast<long>(*from) < n ? static_cast<long>(*from) : (step > 0 ? -1 : n);
    for (long k = 0; k < n; ++k) {
        i = ((i + step) % n + n) % n;
        if (!disabled[static_cast<size_t>(i)]) return static_cast<size_t>(i);
    }
    return std::nullopt;
}

void freshen(Node& container, const std::vector<Node*>& items, const RovingFocus& r) {
    if (!r.engaged) return;
    auto& kids = container.kids();
    std::vector<bool> flip(kids.size(), false);
    for (size_t i = 0; i < items.size(); ++i) {
        if (!r.flipped(i)) continue;
        for (size_t j = 0; j < kids.size(); ++j) {
            bool inside = false;
            visit(kids[j], [&](Node& n) {
                if (&n == items[i]) inside = true;
                return !inside;
            });
            if (inside) {
                flip[j] = true;
                break;
            }
        }
    }
    std::vector<Node> slots;
    slots.reserve(kids.size() * 2);
    for (size_t j = 0; j < kids.size(); ++j) {
        if (flip[j]) slots.push_back(Node::raw("<!---->"));
        slots.push_back(std::move(kids[j]));
        if (!flip[j]) slots.push_back(Node::raw("<!---->"));
    }
    kids = std::move(slots);
}

}  // namespace skeuo::nav
