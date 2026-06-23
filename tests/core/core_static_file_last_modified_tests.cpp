#include <drogular/static_file_last_modified.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeLastModifiedTestFile(
    const std::string& name
) {
    const auto path =
        std::filesystem::temp_directory_path() /
        name;

    std::ofstream file(path);
    file << "hello";

    return path;
}

} // namespace

TEST(CoreStaticFileLastModifiedTests, CreatesHttpDate) {
    const auto path =
        writeLastModifiedTestFile(
            "drogular_last_modified_test.txt"
        );

    const auto value =
        drogular::StaticFileLastModified::create(path);

    EXPECT_FALSE(value.empty());
    EXPECT_NE(
        value.find("GMT"),
        std::string::npos
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileLastModifiedTests, MatchesRequestValue) {
    const auto path =
        writeLastModifiedTestFile(
            "drogular_last_modified_match_test.txt"
        );

    const auto value =
        drogular::StaticFileLastModified::create(path);

    EXPECT_TRUE(
        drogular::StaticFileLastModified::matches(
            path,
            value
        )
    );

    EXPECT_FALSE(
        drogular::StaticFileLastModified::matches(
            path,
            "invalid"
        )
    );

    std::filesystem::remove(path);
}