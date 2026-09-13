#include <drogular/app.hpp>
#include <drogular/interactions_resources.hpp>

#include <gtest/gtest.h>

#include <string_view>

TEST(InteractionsResourcesTests, ShipsGenericBrowserRuntime) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_FALSE(script.empty());
    EXPECT_NE(script.find("[dg-get]"), std::string_view::npos);
    EXPECT_NE(script.find("dg-trigger"), std::string_view::npos);
    EXPECT_NE(script.find("dg-pause-on-failure"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-connection-state"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-connection-label"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-connection-detail"), std::string_view::npos);

    EXPECT_EQ(script.find("data-monitor-"), std::string_view::npos);
    EXPECT_EQ(script.find("dg-status-success"), std::string_view::npos);
}

TEST(InteractionsResourcesTests, HasStableAssetPath) {
    EXPECT_EQ(
        drogular::interactions_resources::ScriptPath,
        "/__drogular/assets/interactions.js"
    );
}

TEST(InteractionsResourcesTests, AppRegistrationIsIdempotent) {
    drogular::App app;

    EXPECT_NO_THROW(app.interactions());
    EXPECT_NO_THROW(app.interactions());
}