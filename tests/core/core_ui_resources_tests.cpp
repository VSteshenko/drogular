#include <drogular/app.hpp>
#include <drogular/ui_resources.hpp>

#include <gtest/gtest.h>

#include <string_view>

TEST(UiResourcesTests, ShipsPresentationPrimitives) {
    const auto css = drogular::ui_resources::stylesheet();

    EXPECT_FALSE(css.empty());
    EXPECT_NE(css.find(".dg-card"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-button"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-toolbar"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-status"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-badge"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-segmented"), std::string_view::npos);
    EXPECT_NE(
        css.find("[data-dg-connection-state=\"offline\"]"),
        std::string_view::npos
    );
}

TEST(UiResourcesTests, HasStableAssetPath) {
    EXPECT_EQ(
        drogular::ui_resources::StylesheetPath,
        "/__drogular/assets/ui.css"
    );
}

TEST(UiResourcesTests, AppRegistrationIsIdempotent) {
    drogular::App app;

    EXPECT_NO_THROW(app.ui());
    EXPECT_NO_THROW(app.ui());
}

TEST(UiResourcesTests, UiAndInteractionsCanBeEnabledIndependently) {
    drogular::App uiOnly;
    drogular::App interactionsOnly;

    EXPECT_NO_THROW(uiOnly.ui());
    EXPECT_NO_THROW(interactionsOnly.interactions());
}