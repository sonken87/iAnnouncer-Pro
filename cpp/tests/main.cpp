#include "test.hpp"

int main() {
    int run = 0;
    for (const auto& c : skeuo_test::cases()) {
        int before = skeuo_test::failures();
        c.fn();
        ++run;
        std::cout << (skeuo_test::failures() == before ? "ok   " : "FAIL ") << c.name << "\n";
    }
    std::cout << run << " tests, " << skeuo_test::failures() << " failed checks\n";
    return skeuo_test::failures() == 0 ? 0 : 1;
}
