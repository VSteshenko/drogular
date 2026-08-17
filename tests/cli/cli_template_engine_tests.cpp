#include "template_engine.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory()
        : path_(fs::temp_directory_path() /
                ("drogular-cli-template-test-" +
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

} // namespace

TEST(CliTemplateEngineTests, ReplacesAllKnownVariables) {
    TemporaryDirectory temp;
    const fs::path input = temp.path() / "input.in";
    const fs::path output = temp.path() / "nested" / "output.txt";

    {
        std::ofstream file(input);
        file << "name=@PROJECT_NAME@ version=@DROGULAR_VERSION@ ref=@DROGULAR_GIT_REF@";
    }

    drogular::cli::TemplateEngine engine;
    engine.renderFile(
        input,
        output,
        {
            {"PROJECT_NAME", "hello_drogular"},
            {"DROGULAR_VERSION", "0.21.0"},
            {"DROGULAR_GIT_REF", "v0.21.0"},
        });

    EXPECT_EQ(
        readFile(output),
        "name=hello_drogular version=0.21.0 ref=v0.21.0");
}