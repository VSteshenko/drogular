#include "project_generator.hpp"
#include "template_registry.hpp"
#include "template_source.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace fs = std::filesystem;

using drogular::generation::GenerationRequest;
using drogular::generation::ProjectGenerator;
using drogular::generation::TemplateRegistry;
using drogular::generation::TemplateSource;

namespace {

class MemoryTemplateSource final : public TemplateSource {
public:
    MemoryTemplateSource() = default;

    MemoryTemplateSource(
        std::initializer_list<std::pair<const std::string, std::string>> values)
        : files(values)
    {
    }

    std::unordered_map<std::string, std::string> files;

    std::optional<std::string> load(std::string_view path) const override {
        const auto it = files.find(std::string(path));
        return it == files.end() ? std::nullopt : std::optional{it->second};
    }
};

class TemporaryDirectory {
public:
    TemporaryDirectory()
        : path_(fs::temp_directory_path() /
                ("drogular-project-generator-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count())))
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
    std::ifstream stream(path, std::ios::binary);
    return {
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()};
}

TemplateRegistry minimalRegistry() {
    TemplateRegistry registry;
    registry.add({
        .id = "minimal",
        .name = "Minimal",
        .files = {
            {.source = "main.cpp.in", .destination = "src/main.cpp"},
            {.source = "readme.in", .destination = "{{PROJECT_NAME}}.md"},
        },
    });
    return registry;
}

} // namespace

TEST(ProjectGeneratorGenerationTests, GeneratesFilesAndRendersVariables) {
    TemporaryDirectory temporary;
    TemplateRegistry registry = minimalRegistry();
    MemoryTemplateSource source{
        {"main.cpp.in", "namespace {{PROJECT_NAMESPACE}} {}\n"},
        {"readme.in", "# {{PROJECT_NAME}}\n"},
    };
    ProjectGenerator generator(registry, source);
    const fs::path destination = temporary.path() / "hello";

    generator.generate({
        .templateId = "minimal",
        .destination = destination,
        .variables = {
            {"PROJECT_NAME", "hello-drogular"},
            {"PROJECT_NAMESPACE", "hello_drogular"},
        },
    });

    EXPECT_EQ(
        readFile(destination / "src/main.cpp"),
        "namespace hello_drogular {}\n");
    EXPECT_EQ(
        readFile(destination / "hello-drogular.md"),
        "# hello-drogular\n");
}

TEST(ProjectGeneratorGenerationTests, RejectsUnknownTemplate) {
    TemporaryDirectory temporary;
    TemplateRegistry registry;
    MemoryTemplateSource source;
    ProjectGenerator generator(registry, source);

    EXPECT_THROW(
        generator.generate({
            .templateId = "missing",
            .destination = temporary.path() / "project",
        }),
        std::runtime_error);
}

TEST(ProjectGeneratorGenerationTests, RejectsExistingDestination) {
    TemporaryDirectory temporary;
    TemplateRegistry registry = minimalRegistry();
    MemoryTemplateSource source;
    ProjectGenerator generator(registry, source);

    EXPECT_THROW(
        generator.generate({
            .templateId = "minimal",
            .destination = temporary.path(),
        }),
        std::runtime_error);
}

TEST(ProjectGeneratorGenerationTests, RemovesPartialProjectWhenTemplateFileIsMissing) {
    TemporaryDirectory temporary;
    TemplateRegistry registry = minimalRegistry();
    MemoryTemplateSource source{{
        {"main.cpp.in", "int main() {}\n"},
    }};
    ProjectGenerator generator(registry, source);
    const fs::path destination = temporary.path() / "incomplete";

    EXPECT_THROW(
        generator.generate({
            .templateId = "minimal",
            .destination = destination,
        }),
        std::runtime_error);

    EXPECT_FALSE(fs::exists(destination));
}

TEST(ProjectGeneratorGenerationTests, RejectsDestinationOutsideProjectDirectory) {
    TemporaryDirectory temporary;
    TemplateRegistry registry;
    registry.add({
        .id = "invalid",
        .name = "Invalid",
        .files = {
            {.source = "file.in", .destination = "../outside.txt"},
        },
    });
    MemoryTemplateSource source{{{"file.in", "content"}}};
    ProjectGenerator generator(registry, source);
    const fs::path destination = temporary.path() / "project";

    EXPECT_THROW(
        generator.generate({
            .templateId = "invalid",
            .destination = destination,
        }),
        std::runtime_error);

    EXPECT_FALSE(fs::exists(destination));
    EXPECT_FALSE(fs::exists(temporary.path() / "outside.txt"));
}