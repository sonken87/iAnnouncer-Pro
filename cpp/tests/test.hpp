// A minimal test framework: TEST(name) { CHECK(cond) << "context"; }
#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace skeuo_test {

struct Case {
    const char* name;
    std::function<void()> fn;
};

inline std::vector<Case>& cases() {
    static std::vector<Case> all;
    return all;
}

inline int& failures() {
    static int n = 0;
    return n;
}

struct Register {
    Register(const char* name, std::function<void()> fn) { cases().push_back({name, std::move(fn)}); }
};

/// Collects `<<` context and reports when the check failed.
class Check {
public:
    Check(bool ok, const char* expr, const char* file, int line) : ok_(ok), expr_(expr), file_(file), line_(line) {}
    template <class T>
    Check& operator<<(const T& v) {
        if (!ok_) msg_ << v;
        return *this;
    }
    ~Check() {
        if (ok_) return;
        ++failures();
        std::string m = msg_.str();
        if (m.size() > 600) m = m.substr(0, 600) + "…";
        std::cerr << "  FAILED " << file_ << ":" << line_ << ": " << expr_ << (m.empty() ? "" : "\n    ") << m << "\n";
    }

private:
    bool ok_;
    const char* expr_;
    const char* file_;
    int line_;
    std::ostringstream msg_;
};

}  // namespace skeuo_test

#define SKEUO_CAT2(a, b) a##b
#define SKEUO_CAT(a, b) SKEUO_CAT2(a, b)
#define TEST(name)                                                                              \
    static void SKEUO_CAT(test_, name)();                                                       \
    static ::skeuo_test::Register SKEUO_CAT(reg_, name)(#name, SKEUO_CAT(test_, name));         \
    static void SKEUO_CAT(test_, name)()
#define CHECK(cond) ::skeuo_test::Check(static_cast<bool>(cond), #cond, __FILE__, __LINE__)
/// `CONTAINS(html, needle)` with the html printed on failure.
#define CONTAINS(hay, needle) CHECK(std::string(hay).find(needle) != std::string::npos) << "missing " << (needle) << " in: " << (hay)
#define LACKS(hay, needle) CHECK(std::string(hay).find(needle) == std::string::npos) << "unexpected " << (needle) << " in: " << (hay)
