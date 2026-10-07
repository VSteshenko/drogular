#include "localization_check.hpp"

#include <iostream>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    std::vector<std::string_view> arguments;
    arguments.reserve(argc > 1 ? static_cast<std::size_t>(argc - 1) : 0);
    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }

    const auto options = drogular::localization_check::parseArguments(arguments);
    return drogular::localization_check::run(options, std::cout, std::cerr);
}