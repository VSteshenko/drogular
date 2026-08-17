#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#ifndef DROGULAR_CLI_VERSION
#define DROGULAR_CLI_VERSION "development"
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

void writeFile(const fs::path& path, std::string_view content) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot create file: " + path.string());
    }

    output << content;
}

int createProject(const std::string& name) {
    if (!validProjectName(name)) {
        std::cerr << "Invalid project name: " << name << '\n';
        std::cerr << "Use only letters, digits, '-' and '_'.\n";
        return 1;
    }

    const fs::path root{name};
    if (fs::exists(root)) {
        std::cerr << "Path already exists: " << root << '\n';
        return 1;
    }

    try {
        fs::create_directories(root / "src" / "components");
        fs::create_directories(root / "templates" / "components");
        fs::create_directories(root / "public");

        writeFile(
            root / "CMakeLists.txt",
            "cmake_minimum_required(VERSION 3.24)\n\n"
            "project(" + name + " LANGUAGES CXX)\n\n"
            "set(CMAKE_CXX_STANDARD 20)\n"
            "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
            "set(CMAKE_CXX_EXTENSIONS OFF)\n\n"
            "include(FetchContent)\n\n"
            "set(DROGULAR_BUILD_EXAMPLES OFF CACHE BOOL \"\" FORCE)\n"
            "set(DROGULAR_BUILD_TESTS OFF CACHE BOOL \"\" FORCE)\n"
            "set(DROGULAR_BUILD_TOOLS OFF CACHE BOOL \"\" FORCE)\n\n"
            "FetchContent_Declare(\n"
            "    drogular\n"
            "    GIT_REPOSITORY https://github.com/VSteshenko/drogular.git\n"
            "    GIT_TAG main\n"
            ")\n\n"
            "FetchContent_MakeAvailable(drogular)\n\n"
            "add_executable(" + name + "\n"
            "    src/main.cpp\n"
            ")\n\n"
            "target_link_libraries(" + name + "\n"
            "    PRIVATE\n"
            "        Drogular::drogular\n"
            ")\n");

        writeFile(
            root / "src" / "home_page.hpp",
            "#pragma once\n\n"
            "#include <drogular/page.hpp>\n\n"
            "#include <string>\n\n"
            "class HomePage final : public drogular::TemplatePage\n"
            "{\n"
            "public:\n"
            "    std::string templatePath() const override\n"
            "    {\n"
            "        return \"home.html\";\n"
            "    }\n"
            "};\n");

        writeFile(
            root / "src" / "components" / "home_component.hpp",
            "#pragma once\n\n"
            "#include <drogular/component.hpp>\n\n"
            "#include <string>\n\n"
            "class HomeComponent final : public drogular::TemplateComponent\n"
            "{\n"
            "public:\n"
            "    static constexpr auto tag = \"Home\";\n\n"
            "    std::string templatePath() const override\n"
            "    {\n"
            "        return \"components/home.html\";\n"
            "    }\n"
            "};\n");

        writeFile(
            root / "src" / "main.cpp",
            "#include \"components/home_component.hpp\"\n"
            "#include \"home_page.hpp\"\n\n"
            "#include <drogular/app.hpp>\n\n"
            "int main()\n"
            "{\n"
            "    drogular::App app;\n\n"
            "    app.templateRoot(\"templates\");\n"
            "    app.staticFiles(\"/assets\", \"public\");\n\n"
            "    app.component<HomeComponent>();\n"
            "    app.page<HomePage>(\"/\");\n\n"
            "    app.run(8080);\n\n"
            "    return 0;\n"
            "}\n");

        writeFile(
            root / "templates" / "home.html",
            "<!doctype html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "    <meta charset=\"utf-8\" />\n"
            "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\n"
            "    <title>" + name + "</title>\n"
            "</head>\n"
            "<body>\n"
            "    <h1>Hello Drogular</h1>\n"
            "    <Home />\n"
            "</body>\n"
            "</html>\n");

        writeFile(
            root / "templates" / "components" / "home.html",
            "<p>Your Drogular application is running.</p>\n");

        writeFile(
            root / ".gitignore",
            "build/\n"
            ".DS_Store\n");

        writeFile(
            root / "README.md",
            "# " + name + "\n\n"
            "A Drogular application.\n\n"
            "## Build\n\n"
            "```bash\n"
            "cmake -S . -B build\n"
            "cmake --build build\n"
            "```\n\n"
            "## Run\n\n"
            "```bash\n"
            "./build/" + name + "\n"
            "```\n\n"
            "Then open http://localhost:8080/.\n");
    } catch (const std::exception& error) {
        std::error_code ignored;
        fs::remove_all(root, ignored);
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
        return createProject(argv[2]);
    }

    printHelp();
    return argc == 1 ? 0 : 1;
}