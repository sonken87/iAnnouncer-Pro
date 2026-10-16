#include <cstring>
#include <iostream>
#include <string>

#include "skeuo/host.hpp"
#include "skeuo/json.hpp"

namespace skeuo {

int run_stdio(Runtime& runtime) {
    std::ios::sync_with_stdio(false);
    std::cout << "{\"t\":\"page\",\"html\":" << json::quote(runtime.page()) << "}\n" << std::flush;
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::cout << runtime.handle(line) << "\n" << std::flush;
    }
    return 0;
}

int run_app(Runtime& runtime, const WindowOptions& options, int argc, char** argv) {
    for (int i = 1; i < argc; ++i)
        if (std::strcmp(argv[i], "--stdio") == 0) return run_stdio(runtime);
    return run_window(runtime, options);
}

#ifndef _WIN32
int run_window(Runtime&, const WindowOptions&) {
    std::cerr << "skeuo-ui: no native window host on this platform yet; run with --stdio\n";
    return 1;
}
#endif

}  // namespace skeuo
