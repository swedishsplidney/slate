#include "core/engine.hpp"
#include <cstdio>
#include <cstdlib>
#include <exception>

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::setvbuf(stderr, nullptr, _IONBF, 0);

    try {
        slate::Engine engine{};
        engine.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal engine error: %s\n", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        std::fprintf(stderr, "fatal engine error: unknown exception\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}