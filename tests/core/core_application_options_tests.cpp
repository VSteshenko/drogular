#include <drogular/application_options.hpp>

#include <gtest/gtest.h>

TEST(CoreApplicationOptionsTests, StoresStaticFileMappings) {
    drogular::ApplicationOptions options;

    options.addStaticFiles(
        "/assets",
        "public"
    );

    ASSERT_EQ(
        options.staticFiles().size(),
        1
    );

    EXPECT_EQ(
        options.staticFiles()[0].routePrefix,
        "/assets"
    );

    EXPECT_EQ(
        options.staticFiles()[0].directory,
        std::filesystem::path("public")
    );
}

TEST(CoreApplicationOptionsTests, StoresStaticFileCacheOptions) {
    drogular::ApplicationOptions options;

    options.setStaticFileCacheEnabled(false);
    options.setStaticFileCacheMaxAge(
        std::chrono::seconds(60)
    );

    EXPECT_FALSE(
        options.staticFileCacheEnabled()
    );

    EXPECT_EQ(
        options.staticFileCacheMaxAge(),
        std::chrono::seconds(60)
    );
}