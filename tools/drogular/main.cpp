#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace {

void printUsage() {
    std::cout << "Usage:\n"
              << "  drogular new <project-name>\n";
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
        fs::create_directories(root / "src");
        fs::create_directories(root / "templates");

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
            root / "src" / "main.cpp",
            "#include <drogular/app.hpp>\n"
            "#include <drogular/page.hpp>\n\n"
            "#include <string>\n\n"
            "class HomePage final : public drogular::TemplatePage\n"
            "{\n"
            "public:\n"
            "    std::string templatePath() const override\n"
            "    {\n"
            "        return \"home.html\";\n"
            "    }\n"
            "};\n\n"
            "int main()\n"
            "{\n"
            "    drogular::App app;\n\n"
            "    app.templateRoot(\"templates\");\n"
            "    app.page<HomePage>(\"/\");\n"
            "    app.run(8080);\n\n"
            "    return 0;\n"
            "}\n");

        writeFile(
            root / "templates" / "home.html",
            "<!doctype html>\n"
            "<html lang=\"en\">\n"
            "<head>\n"
            "    <meta charset=\"utf-8\" />\n"
            "    <title>" + name + "</title>\n"
            "</head>\n"
            "<body>\n"
            "    <h1>Hello Drogular</h1>\n"
            "</body>\n"
            "</html>\n");
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
    if (argc == 3 && std::string_view(argv[1]) == "new") {
        return createProject(argv[2]);
    }

    printUsage();
    return argc == 1 ? 0 : 1;
}