#include "project_generator.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

class CurrentPathGuard {
public:
    explicit CurrentPathGuard(const fs::path& path)
        : previous_(fs::current_path())
    {
        fs::current_path(path);
    }

    ~CurrentPathGuard() { fs::current_path(previous_); }

private:
    fs::path previous_;
};

class TemporaryDirectory {
public:
    TemporaryDirectory()
        : path_(fs::temp_directory_path() /
                ("drogular-cli-generator-test-" +
                 std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())))
    {
        fs::create_directories(path_);
    }

    ~TemporaryDirectory() {
        std::error_code ignored;
        fs::remove_all(path_, ignored);
    }

    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

std::string readFile(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

fs::path templatesRoot() {
    return fs::path(DROGULAR_SOURCE_DIR) / "tools" / "drogular" / "templates";
}

} // namespace

TEST(CliProjectGeneratorTests, CreatesExpectedMinimalProject) {
    TemporaryDirectory temp;
    CurrentPathGuard currentPath(temp.path());

    drogular::cli::ProjectGenerator generator(
        templatesRoot(),
        "0.21.0",
        "v0.21.0");
    generator.generate("hello-drogular");

    const fs::path root = temp.path() / "hello-drogular";
    const std::vector<fs::path> expectedFiles{
        ".gitignore",
        "CMakeLists.txt",
        "README.md",
        "public/.gitkeep",
        "src/main.cpp",
        "src/home_page.hpp",
        "src/components/home_component.hpp",
        "templates/home.html",
        "templates/components/home.html",
    };

    for (const auto& relative : expectedFiles) {
        EXPECT_TRUE(fs::is_regular_file(root / relative)) << relative;
    }
    EXPECT_FALSE(fs::exists(root / "template.json"));
}

TEST(CliProjectGeneratorTests, UsesRequestedDrogularVersionAndGitRef) {
    TemporaryDirectory temp;
    CurrentPathGuard currentPath(temp.path());

    drogular::cli::ProjectGenerator generator(
        templatesRoot(),
        "0.21.0",
        "v0.21.0");
    generator.generate("release-app");

    const std::string cmake = readFile(temp.path() / "release-app" / "CMakeLists.txt");
    const std::string readme = readFile(temp.path() / "release-app" / "README.md");

    EXPECT_NE(cmake.find("GIT_TAG v0.21.0"), std::string::npos);
    EXPECT_EQ(cmake.find("GIT_TAG main"), std::string::npos);
    EXPECT_NE(readme.find("Drogular 0.21.0"), std::string::npos);
}

TEST(CliProjectGeneratorTests, ReplacesAllTemplatePlaceholders) {
    TemporaryDirectory temp;
    CurrentPathGuard currentPath(temp.path());

    drogular::cli::ProjectGenerator generator(
        templatesRoot(),
        "0.21.0",
        "v0.21.0");
    generator.generate("placeholder-check");

    const fs::path root = temp.path() / "placeholder-check";
    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const std::string content = readFile(entry.path());
        EXPECT_EQ(content.find("@PROJECT_NAME@"), std::string::npos) << entry.path();
        EXPECT_EQ(content.find("@PROJECT_NAMESPACE@"), std::string::npos) << entry.path();
        EXPECT_EQ(content.find("@DROGULAR_VERSION@"), std::string::npos) << entry.path();
        EXPECT_EQ(content.find("@DROGULAR_GIT_REF@"), std::string::npos) << entry.path();
        EXPECT_EQ(content.find("@YEAR@"), std::string::npos) << entry.path();
    }
}

TEST(CliProjectGeneratorTests, RejectsExistingDestination) {
    TemporaryDirectory temp;
    CurrentPathGuard currentPath(temp.path());
    fs::create_directories("existing-app");

    drogular::cli::ProjectGenerator generator(
        templatesRoot(),
        "0.21.0",
        "v0.21.0");

    EXPECT_THROW(generator.generate("existing-app"), std::runtime_error);
}