#include <drogular/static_file_etag.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeEtagTestFile(
    const std::string& name,
    const std::string& content
) {
    const auto path =
        std::filesystem::temp_directory_path() /
        name;

    std::ofstream file(path);
    file << content;

    return path;
}

} // namespace

TEST(CoreStaticFileEtagTests, CreatesEtagForFile) {
    const auto path =
        writeEtagTestFile(
            "drogular_static_etag_test.txt",
            "hello"
        );

    const auto etag =
        drogular::StaticFileEtag::create(path);

    EXPECT_FALSE(etag.empty());

    EXPECT_TRUE(
        etag.starts_with("W/\"")
    );

    EXPECT_EQ(
        etag.back(),
        '"'
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileEtagTests, ChangesWhenFileSizeChanges) {
    const auto path =
        writeEtagTestFile(
            "drogular_static_etag_size_test.txt",
            "hello"
        );

    const auto first =
        drogular::StaticFileEtag::create(path);

    {
        std::ofstream file(path);
        file << "hello world";
    }

    const auto second =
        drogular::StaticFileEtag::create(path);

    EXPECT_NE(first, second);

    std::filesystem::remove(path);
}
