#include <drogular/static_file_resolver.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path createDirectory(
    const std::string& name
) {
    const auto path =
        std::filesystem::temp_directory_path() /
        name;

    std::filesystem::create_directories(path);

    return path;
}

void writeFile(
    const std::filesystem::path& path,
    const std::string& content
) {
    std::ofstream file(path);
    file << content;
}

} // namespace

TEST(CoreStaticFileResolverTests, ResolvesFileInsideRoot) {
    const auto root =
        createDirectory("drogular_static_root");

    const auto file =
        root / "favicon.ico";

    writeFile(file, "icon");

    drogular::StaticFileResolver resolver(root);

    const auto resolved =
        resolver.resolve("favicon.ico");

    ASSERT_TRUE(resolved.has_value());
    EXPECT_EQ(
        std::filesystem::weakly_canonical(*resolved),
        std::filesystem::weakly_canonical(file)
    );

    std::filesystem::remove_all(root);
}

TEST(CoreStaticFileResolverTests, RejectsMissingFile) {
    const auto root =
        createDirectory("drogular_static_missing_root");

    drogular::StaticFileResolver resolver(root);

    EXPECT_FALSE(
        resolver.resolve("missing.ico").has_value()
    );

    std::filesystem::remove_all(root);
}

TEST(CoreStaticFileResolverTests, RejectsPathTraversal) {
    const auto root =
        createDirectory("drogular_static_secure_root");

    const auto outside =
        std::filesystem::temp_directory_path() /
        "drogular_static_secret.txt";

    writeFile(outside, "secret");

    drogular::StaticFileResolver resolver(root);

    EXPECT_FALSE(
        resolver.resolve("../drogular_static_secret.txt")
            .has_value()
    );

    std::filesystem::remove_all(root);
    std::filesystem::remove(outside);
}