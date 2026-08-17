#include "project_generator.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#ifndef DROGULAR_CLI_VERSION
#define DROGULAR_CLI_VERSION "development"
#endif

#ifndef DROGULAR_CLI_TEMPLATE_DIR
#define DROGULAR_CLI_TEMPLATE_DIR ""
#endif

#ifndef DROGULAR_CLI_INSTALL_TEMPLATE_DIR
#define DROGULAR_CLI_INSTALL_TEMPLATE_DIR ""
#endif

namespace fs = std::filesystem;

namespace {

void printHelp() {
    std::cout << "Drogular CLI\n\n"
              << "Usage:\n"
              << "  drogular new <project>\n\n"
              << "Options:\n"
              << "  --help       Show this help\n"
              << "  --version    Show version\n";
}

bool validProjectName(std::string_view name) {
    if (name.empty()) {
        return false;
    }

    for (const char ch : name) {
        const bool valid =
            (ch >= 'a' && ch <= 'z') ||
            (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') ||
            ch == '_' || ch == '-';

        if (!valid) {
            return false;
        }
    }

    return true;
}

fs::path executablePath(const fs::path& fallback) {
#if defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);

    std::vector<char> buffer(size);
    if (_NSGetExecutablePath(buffer.data(), &size) == 0) {
        return fs::weakly_canonical(fs::path(buffer.data()));
    }
#elif defined(__linux__)
    {
        std::error_code error;
        const fs::path path = fs::read_symlink("/proc/self/exe", error);
        if (!error) {
            return path;
        }
    }
#endif

    std::error_code error;
    const fs::path path = fs::absolute(fallback, error);
    return error ? fallback : path;
}

fs::path findTemplatesRoot(const fs::path& executable) {
    std::vector<fs::path> candidates;

    const fs::path resolvedExecutable = executablePath(executable);
    candidates.push_back(
        resolvedExecutable.parent_path() / ".." / "share" / "drogular" / "templates");

    if (std::string_view(DROGULAR_CLI_TEMPLATE_DIR).size() > 0) {
        candidates.emplace_back(DROGULAR_CLI_TEMPLATE_DIR);
    }

    if (std::string_view(DROGULAR_CLI_INSTALL_TEMPLATE_DIR).size() > 0) {
        candidates.emplace_back(DROGULAR_CLI_INSTALL_TEMPLATE_DIR);
    }

    candidates.push_back(fs::current_path() / "tools" / "drogular" / "templates");
    candidates.push_back(fs::current_path() / "templates");

    for (const auto& candidate : candidates) {
        if (fs::is_directory(candidate / "minimal")) {
            return fs::weakly_canonical(candidate);
        }
    }

    throw std::runtime_error("Drogular CLI templates could not be found.");
}

int createProject(const std::string& name, const fs::path& executable) {
    if (!validProjectName(name)) {
        std::cerr << "Invalid project name: " << name << '\n';
        std::cerr << "Use only letters, digits, '-' and '_'.\n";
        return 1;
    }

    try {
        drogular::cli::ProjectGenerator generator(
            findTemplatesRoot(executable),
            DROGULAR_CLI_VERSION);
        generator.generate(name);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    std::cout << "Created Drogular project: " << name << "\n\n"
              << "Next steps:\n"
              << "  cd " << name << '\n'
              << "  cmake -S . -B build\n"
              << "  cmake --build build\n"
              << "  ./build/" << name << '\n';

    return 0;
}

}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        const std::string_view argument{argv[1]};

        if (argument == "--help" || argument == "-h") {
            printHelp();
            return 0;
        }

        if (argument == "--version" || argument == "-v") {
            std::cout << "Drogular CLI " << DROGULAR_CLI_VERSION << '\n';
            return 0;
        }
    }

    if (argc == 3 && std::string_view(argv[1]) == "new") {
        return createProject(argv[2], argv[0]);
    }

    printHelp();
    return argc == 1 ? 0 : 1;
}