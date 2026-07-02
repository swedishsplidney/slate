#include "core/engine.hpp"
#include <iostream>
#include <exception>

int main() {
    slate::Engine engine{};

    try {
        engine.run();
    } catch (const std::exception& e) {
        std::cerr << "fatal engine error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}