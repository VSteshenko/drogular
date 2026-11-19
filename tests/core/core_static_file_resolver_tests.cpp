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

TEST(CoreStaticFileResolverTests, NestedPathsRequireExplicitOptIn) {
    const auto root = createDirectory("drogular_static_nested_root");
    std::filesystem::create_directories(root / "js" / "components");
    writeFile(root / "js" / "components" / "menu.js", "menu");
    drogular::StaticFileResolver resolver(root);

    EXPECT_FALSE(resolver.resolve("js/components/menu.js").has_value());
    EXPECT_TRUE(resolver.resolve("js/components/menu.js", true).has_value());
    std::filesystem::remove_all(root);
}

TEST(CoreStaticFileResolverTests, BlocksUnsafeNestedPaths) {
    const auto root = createDirectory("drogular_static_nested_secure_root");
    std::filesystem::create_directories(root / "css");
    writeFile(root / "css" / "style.css", "css");
    drogular::StaticFileResolver resolver(root);

    for (const char* path : {
        "../secret", "css/../style.css", "css/./style.css",
        "/etc/passwd", "css\\style.css", "css/%2e%2e/secret",
        "css/%2fsecret", "css/%5csecret", "css/%00secret"
    }) {
        EXPECT_FALSE(resolver.resolve(path, true).has_value()) << path;
    }
    std::filesystem::remove_all(root);
}

TEST(CoreStaticFileResolverTests, RejectsSymlinkLeavingRoot) {
    const auto root = createDirectory("drogular_static_symlink_root");
    const auto outside = createDirectory("drogular_static_symlink_outside");
    writeFile(outside / "secret.txt", "secret");
    std::error_code ec;
    std::filesystem::create_directory_symlink(outside, root / "external", ec);

    if (!ec) {
        drogular::StaticFileResolver resolver(root);
        EXPECT_FALSE(resolver.resolve("external/secret.txt", true).has_value());
    }
    std::filesystem::remove_all(root);
    std::filesystem::remove_all(outside);
}