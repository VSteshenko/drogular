#include <drogular/static_file_response.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path writeStaticFile(
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

TEST(CoreStaticFileResponseTests, SetsContentTypeFromFileExtension) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_test.svg",
            "<svg></svg>"
        );

    const auto response =
        drogular::StaticFileResponse::create(path);

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->contentTypeString(),
        "image/svg+xml"
    );

    std::filesystem::remove(path);
}