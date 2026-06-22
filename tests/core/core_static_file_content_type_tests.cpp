#include <drogular/static_file_content_type.hpp>

#include <gtest/gtest.h>

TEST(CoreStaticFileContentTypeTests, DetectsHtml) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("index.html"),
        "text/html"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsCss) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("site.css"),
        "text/css"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsJavaScript) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("app.js"),
        "application/javascript"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsWebManifest) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("manifest.webmanifest"),
        "application/manifest+json"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsIco) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("favicon.ico"),
        "image/x-icon"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsSvg) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("logo.svg"),
        "image/svg+xml"
    );
}

TEST(CoreStaticFileContentTypeTests, DetectsUnknownAsOctetStream) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("file.unknown"),
        "application/octet-stream"
    );
}

TEST(CoreStaticFileContentTypeTests, IsCaseInsensitive) {
    EXPECT_EQ(
        drogular::StaticFileContentType::fromPath("LOGO.SVG"),
        "image/svg+xml"
    );
}