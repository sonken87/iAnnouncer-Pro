// Minimal JSON for the webview <-> C++ messages.
#pragma once

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace skeuo::json {

struct Value {
    enum class Type { Null, Bool, Number, String, Array, Object } type = Type::Null;
    bool boolean = false;
    double number = 0;
    std::string string;
    std::vector<Value> array;
    std::map<std::string, Value> object;

    const Value* get(std::string_view key) const;
    std::string str(std::string_view key, std::string fallback = {}) const;
    double num(std::string_view key, double fallback = 0) const;
    bool flag(std::string_view key, bool fallback = false) const;
};

/// Parses a JSON document; returns a Null value on malformed input.
Value parse(std::string_view text);

/// `"..."` with JSON escaping.
std::string quote(std::string_view s);

}  // namespace skeuo::json
