#include "skeuo/json.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>

namespace skeuo::json {

const Value* Value::get(std::string_view key) const {
    if (type != Type::Object) return nullptr;
    auto it = object.find(std::string(key));
    return it == object.end() ? nullptr : &it->second;
}

std::string Value::str(std::string_view key, std::string fallback) const {
    const Value* v = get(key);
    return v && v->type == Type::String ? v->string : fallback;
}

double Value::num(std::string_view key, double fallback) const {
    const Value* v = get(key);
    return v && v->type == Type::Number ? v->number : fallback;
}

bool Value::flag(std::string_view key, bool fallback) const {
    const Value* v = get(key);
    return v && v->type == Type::Bool ? v->boolean : fallback;
}

namespace {

struct Parser {
    std::string_view s;
    size_t i = 0;
    bool ok = true;

    void ws() {
        while (i < s.size() && (s[i] == ' ' || s[i] == '\n' || s[i] == '\r' || s[i] == '\t')) ++i;
    }

    bool eat(char c) {
        ws();
        if (i < s.size() && s[i] == c) {
            ++i;
            return true;
        }
        return false;
    }

    static void utf8(std::string& out, unsigned cp) {
        if (cp < 0x80) {
            out += char(cp);
        } else if (cp < 0x800) {
            out += char(0xC0 | (cp >> 6));
            out += char(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            out += char(0xE0 | (cp >> 12));
            out += char(0x80 | ((cp >> 6) & 0x3F));
            out += char(0x80 | (cp & 0x3F));
        } else {
            out += char(0xF0 | (cp >> 18));
            out += char(0x80 | ((cp >> 12) & 0x3F));
            out += char(0x80 | ((cp >> 6) & 0x3F));
            out += char(0x80 | (cp & 0x3F));
        }
    }

    unsigned hex4() {
        if (i + 4 > s.size()) {
            ok = false;
            return 0;
        }
        unsigned v = 0;
        for (int k = 0; k < 4; ++k) {
            char c = s[i++];
            v <<= 4;
            if (c >= '0' && c <= '9') v |= unsigned(c - '0');
            else if (c >= 'a' && c <= 'f') v |= unsigned(c - 'a' + 10);
            else if (c >= 'A' && c <= 'F') v |= unsigned(c - 'A' + 10);
            else ok = false;
        }
        return v;
    }

    std::string string() {
        std::string out;
        ++i;  // opening quote
        while (i < s.size() && s[i] != '"') {
            char c = s[i++];
            if (c != '\\') {
                out += c;
                continue;
            }
            if (i >= s.size()) break;
            char e = s[i++];
            switch (e) {
                case 'n': out += '\n'; break;
                case 't': out += '\t'; break;
                case 'r': out += '\r'; break;
                case 'b': out += '\b'; break;
                case 'f': out += '\f'; break;
                case 'u': {
                    unsigned cp = hex4();
                    if (cp >= 0xD800 && cp < 0xDC00 && i + 6 <= s.size() && s[i] == '\\' && s[i + 1] == 'u') {
                        i += 2;
                        unsigned lo = hex4();
                        cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                    }
                    utf8(out, cp);
                    break;
                }
                default: out += e;
            }
        }
        if (i < s.size()) ++i;  // closing quote
        else ok = false;
        return out;
    }

    Value value(int depth = 0) {
        Value v;
        ws();
        if (i >= s.size() || depth > 64) {
            ok = false;
            return v;
        }
        char c = s[i];
        if (c == '{') {
            ++i;
            v.type = Value::Type::Object;
            if (eat('}')) return v;
            do {
                ws();
                if (i >= s.size() || s[i] != '"') {
                    ok = false;
                    return v;
                }
                std::string key = string();
                if (!eat(':')) {
                    ok = false;
                    return v;
                }
                v.object[key] = value(depth + 1);
            } while (ok && eat(','));
            if (!eat('}')) ok = false;
        } else if (c == '[') {
            ++i;
            v.type = Value::Type::Array;
            if (eat(']')) return v;
            do {
                v.array.push_back(value(depth + 1));
            } while (ok && eat(','));
            if (!eat(']')) ok = false;
        } else if (c == '"') {
            v.type = Value::Type::String;
            v.string = string();
        } else if (s.substr(i, 4) == "true") {
            i += 4;
            v.type = Value::Type::Bool;
            v.boolean = true;
        } else if (s.substr(i, 5) == "false") {
            i += 5;
            v.type = Value::Type::Bool;
        } else if (s.substr(i, 4) == "null") {
            i += 4;
        } else {
            size_t start = i;
            while (i < s.size() && (std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '-' ||
                                    s[i] == '+' || s[i] == '.' || s[i] == 'e' || s[i] == 'E'))
                ++i;
            if (start == i) {
                ok = false;
                return v;
            }
            v.type = Value::Type::Number;
            v.number = std::strtod(std::string(s.substr(start, i - start)).c_str(), nullptr);
        }
        return v;
    }
};

}  // namespace

Value parse(std::string_view text) {
    Parser p{text};
    Value v = p.value();
    return p.ok ? v : Value{};
}

std::string quote(std::string_view s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x", c);
                    out += buf;
                } else {
                    out += char(c);
                }
        }
    }
    out += '"';
    return out;
}

}  // namespace skeuo::json
