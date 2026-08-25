#include "embedded_template_source.hpp"
#include "project_generator.hpp"
#include "template_registry.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

using drogular::generation::EmbeddedTemplateSource;
using drogular::generation::ProjectGenerator;
using drogular::generation::TemplateRegistry;

namespace {

class TemporaryDirectory {
public:
    TemporaryDirectory()
        : path_(fs::temp_directory_path() /
                ("drogular-embedded-template-" +
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

} // namespace

TEST(ProjectGeneratorEmbeddedTemplateTests, RegistersMinimalTemplateFromEmbeddedSource) {
    EmbeddedTemplateSource source;
    TemplateRegistry registry;

    registry.load(source);

    const auto* projectTemplate = registry.find("minimal");
    ASSERT_NE(projectTemplate, nullptr);
    EXPECT_EQ(projectTemplate->name, "Minimal Application");
    EXPECT_EQ(projectTemplate->description, "Minimal Drogular application");
    EXPECT_EQ(projectTemplate->files.size(), 9u);
}

TEST(ProjectGeneratorEmbeddedTemplateTests, GeneratesMinimalDrogularProject) {
    TemporaryDirectory temporary;
    EmbeddedTemplateSource source;
    TemplateRegistry registry;
    registry.load(source);
    ProjectGenerator generator(registry, source);
    const fs::path destination = temporary.path() / "hello_drogular";

    generator.generate({
        .templateId = "minimal",
        .destination = destination,
        .variables = {
            {"PROJECT_NAME", "hello_drogular"},
            {"DROGULAR_VERSION", "0.22-test"},
            {"DROGULAR_GIT_REF", "main"},
        },
    });

    EXPECT_TRUE(fs::exists(destination / "CMakeLists.txt"));
    EXPECT_TRUE(fs::exists(destination / "README.md"));
    EXPECT_TRUE(fs::exists(destination / "src/main.cpp"));
    EXPECT_TRUE(fs::exists(destination / "src/home_page.hpp"));
    EXPECT_TRUE(fs::exists(destination / "src/components/home_component.hpp"));
    EXPECT_TRUE(fs::exists(destination / "templates/home.html"));
    EXPECT_TRUE(fs::exists(destination / "templates/components/home.html"));
    EXPECT_TRUE(fs::exists(destination / "public/.gitkeep"));

    EXPECT_NE(
        readFile(destination / "CMakeLists.txt").find("project(hello_drogular LANGUAGES CXX)"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "CMakeLists.txt").find("GIT_TAG main"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "README.md").find("Drogular 0.22-test"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "templates/home.html").find("<title>hello_drogular</title>"),
        std::string::npos);
}

TEST(ProjectGeneratorEmbeddedTemplateTests, RegistersPwaTemplateFromEmbeddedSource) {
    EmbeddedTemplateSource source;
    TemplateRegistry registry;

    registry.load(source);

    const auto* projectTemplate = registry.find("pwa");
    ASSERT_NE(projectTemplate, nullptr);
    EXPECT_EQ(projectTemplate->name, "PWA Starter");
    EXPECT_EQ(projectTemplate->description, "Installable Drogular PWA with offline support");
    EXPECT_EQ(projectTemplate->files.size(), 16u);
}

TEST(ProjectGeneratorEmbeddedTemplateTests, GeneratesPwaDrogularProject) {
    TemporaryDirectory temporary;
    EmbeddedTemplateSource source;
    TemplateRegistry registry;
    registry.load(source);
    ProjectGenerator generator(registry, source);
    const fs::path destination = temporary.path() / "my_pwa";

    generator.generate({
        .templateId = "pwa",
        .destination = destination,
        .variables = {
            {"PROJECT_NAME", "my_pwa"},
            {"DROGULAR_VERSION", "0.22-test"},
            {"DROGULAR_GIT_REF", "main"},
        },
    });

    EXPECT_TRUE(fs::exists(destination / "CMakeLists.txt"));
    EXPECT_TRUE(fs::exists(destination / "src/main.cpp"));
    EXPECT_TRUE(fs::exists(destination / "src/home_page.hpp"));
    EXPECT_TRUE(fs::exists(destination / "src/offline_page.hpp"));
    EXPECT_TRUE(fs::exists(destination / "src/components/app_shell.hpp"));
    EXPECT_TRUE(fs::exists(destination / "templates/layouts/app.html"));
    EXPECT_TRUE(fs::exists(destination / "templates/components/app_shell.html"));
    EXPECT_TRUE(fs::exists(destination / "public/manifest.webmanifest"));
    EXPECT_TRUE(fs::exists(destination / "public/service-worker.js"));
    EXPECT_TRUE(fs::exists(destination / "public/app-icon-192.svg"));
    EXPECT_TRUE(fs::exists(destination / "public/app-icon-512.svg"));

    EXPECT_NE(
        readFile(destination / "src/main.cpp").find("app.serviceWorker(\"public/service-worker.js\")"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "src/main.cpp").find("app.offlinePage<OfflinePage>()"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "public/manifest.webmanifest").find("\"name\": \"my_pwa\""),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "public/service-worker.js").find("my_pwa-pwa-v1"),
        std::string::npos);
    EXPECT_NE(
        readFile(destination / "README.md").find("Drogular 0.22-test"),
        std::string::npos);
}