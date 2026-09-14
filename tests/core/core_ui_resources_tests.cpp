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
    EXPECT_NE(css.find(".dg-shell"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-sidebar"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-nav-item"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-topbar"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-main"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-page"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-page-header"), std::string_view::npos);
    EXPECT_NE(
        css.find("[data-dg-connection-state=\"offline\"]"),
        std::string_view::npos
    );
}

TEST(UiResourcesTests, ShipsThemeTokensAndModes) {
    const auto css = drogular::ui_resources::stylesheet();

    EXPECT_NE(css.find("--dg-bg"), std::string_view::npos);
    EXPECT_NE(css.find("--dg-surface"), std::string_view::npos);
    EXPECT_NE(css.find("--dg-text"), std::string_view::npos);
    EXPECT_NE(
        css.find("[data-dg-theme=\"light\"]"),
        std::string_view::npos
    );
    EXPECT_NE(
        css.find("[data-dg-theme=\"dark\"]"),
        std::string_view::npos
    );
    EXPECT_NE(
        css.find("[data-dg-theme=\"system\"]"),
        std::string_view::npos
    );
    EXPECT_NE(
        css.find("prefers-color-scheme: dark"),
        std::string_view::npos
    );
}

TEST(UiResourcesTests, PrimitivesUseSemanticThemeTokens) {
    const auto css = drogular::ui_resources::stylesheet();

    EXPECT_NE(
        css.find("background: var(--dg-surface)"),
        std::string_view::npos
    );
    EXPECT_NE(
        css.find("color: var(--dg-text)"),
        std::string_view::npos
    );
    EXPECT_NE(
        css.find("border: 1px solid var(--dg-border)"),
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