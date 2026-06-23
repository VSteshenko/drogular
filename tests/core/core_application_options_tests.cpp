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

TEST(CoreApplicationOptionsTests, StoresStaticFileEtagOptions) {
    drogular::ApplicationOptions options;

    options.setStaticFileEtagEnabled(false);

    EXPECT_FALSE(
        options.staticFileEtagEnabled()
    );
}

TEST(CoreApplicationOptionsTests, AppliesDisabledStaticFileCacheProfile) {
    drogular::ApplicationOptions options;

    options.setStaticFileCacheProfile(
        drogular::StaticFileCacheProfile::Disabled
    );

    EXPECT_FALSE(options.staticFileCacheEnabled());
    EXPECT_FALSE(options.staticFileEtagEnabled());
    EXPECT_FALSE(options.staticFileLastModifiedEnabled());
}

TEST(CoreApplicationOptionsTests, AppliesDevelopmentStaticFileCacheProfile) {
    drogular::ApplicationOptions options;

    options.setStaticFileCacheProfile(
        drogular::StaticFileCacheProfile::Development
    );

    EXPECT_FALSE(options.staticFileCacheEnabled());
    EXPECT_TRUE(options.staticFileEtagEnabled());
    EXPECT_TRUE(options.staticFileLastModifiedEnabled());
}

TEST(CoreApplicationOptionsTests, AppliesProductionStaticFileCacheProfile) {
    drogular::ApplicationOptions options;

    options.setStaticFileCacheProfile(
        drogular::StaticFileCacheProfile::Production
    );

    EXPECT_TRUE(options.staticFileCacheEnabled());
    EXPECT_EQ(
        options.staticFileCacheMaxAge(),
        std::chrono::hours(24)
    );
    EXPECT_TRUE(options.staticFileEtagEnabled());
    EXPECT_TRUE(options.staticFileLastModifiedEnabled());
}