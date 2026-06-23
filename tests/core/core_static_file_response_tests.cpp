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

TEST(CoreStaticFileResponseTests, AddsCacheControlHeader) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_cache.txt",
            "hello"
        );

    const auto response =
        drogular::StaticFileResponse::create(path);

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->getHeader("Cache-Control"),
        "public, max-age=86400"
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, CanDisableCacheControl) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_no_cache.txt",
            "hello"
        );

    drogular::StaticFileResponseOptions options;
    options.cacheEnabled = false;

    const auto response =
        drogular::StaticFileResponse::create(
            path,
            options
        );

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->getHeader("Cache-Control"),
        "no-store"
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, UsesCustomMaxAge) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_custom_cache.txt",
            "hello"
        );

    drogular::StaticFileResponseOptions options;
    options.maxAge = std::chrono::seconds(60);

    const auto response =
        drogular::StaticFileResponse::create(
            path,
            options
        );

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->getHeader("Cache-Control"),
        "public, max-age=60"
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, AddsEtagHeader) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_etag.txt",
            "hello"
        );

    const auto response =
        drogular::StaticFileResponse::create(path);

    ASSERT_NE(response, nullptr);

    EXPECT_FALSE(
        response->getHeader("ETag").empty()
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, CanDisableEtagHeader) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_no_etag.txt",
            "hello"
        );

    drogular::StaticFileResponseOptions options;
    options.etagEnabled = false;

    const auto response =
        drogular::StaticFileResponse::create(
            path,
            options
        );

    ASSERT_NE(response, nullptr);

    EXPECT_TRUE(
        response->getHeader("ETag").empty()
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, CreatesNotModifiedResponse) {
    const auto response =
        drogular::StaticFileResponse::notModified(
            "W/\"123-456\""
        );

    ASSERT_NE(response, nullptr);

    EXPECT_EQ(
        response->statusCode(),
        drogon::k304NotModified
    );

    EXPECT_EQ(
        response->getHeader("ETag"),
        "W/\"123-456\""
    );
}

TEST(CoreStaticFileResponseTests, AddsLastModifiedHeader) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_last_modified.txt",
            "hello"
        );

    const auto response =
        drogular::StaticFileResponse::create(path);

    ASSERT_NE(response, nullptr);

    EXPECT_FALSE(
        response->getHeader("Last-Modified").empty()
    );

    std::filesystem::remove(path);
}

TEST(CoreStaticFileResponseTests, CanDisableLastModifiedHeader) {
    const auto path =
        writeStaticFile(
            "drogular_static_response_no_last_modified.txt",
            "hello"
        );

    drogular::StaticFileResponseOptions options;
    options.lastModifiedEnabled = false;

    const auto response =
        drogular::StaticFileResponse::create(
            path,
            options
        );

    ASSERT_NE(response, nullptr);

    EXPECT_TRUE(
        response->getHeader("Last-Modified").empty()
    );

    std::filesystem::remove(path);
}