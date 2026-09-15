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
    EXPECT_NE(css.find(".dg-nav-group"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-nav-submenu"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-nav-subitem"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-topbar"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-main"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-page"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-page-header"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-card-header"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-card-body"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-card-footer"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-field"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-input"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-select"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-form-actions"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-table-container"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-table"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-empty-state"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-details"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-details-item"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-details-label"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-details-value"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-stack"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-collapsible"), std::string_view::npos);
    EXPECT_NE(css.find(".dg-card-summary"), std::string_view::npos);
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

TEST(UiResourcesTests, CardFooterUsesCardSurface) {
    const auto css = drogular::ui_resources::stylesheet();

    const auto firstFooter = css.find(".dg-card-footer {");
    ASSERT_NE(firstFooter, std::string_view::npos);
    const auto footer = css.find(".dg-card-footer {", firstFooter + 1);
    ASSERT_NE(footer, std::string_view::npos);
    const auto footerEnd = css.find('}', footer);
    ASSERT_NE(footerEnd, std::string_view::npos);
    const auto block = css.substr(footer, footerEnd - footer);

    EXPECT_NE(block.find("background: var(--dg-surface)"), std::string_view::npos);
    EXPECT_EQ(block.find("background: var(--dg-surface-muted)"), std::string_view::npos);
}

TEST(UiResourcesTests, ShipsCollapsePersistenceRuntime) {
    const auto script = drogular::ui_resources::script();

    EXPECT_FALSE(script.empty());
    EXPECT_NE(script.find("data-dg-collapse-key"), std::string_view::npos);
    EXPECT_NE(script.find("localStorage"), std::string_view::npos);
    EXPECT_NE(script.find("drogular.ui.collapse."), std::string_view::npos);
    EXPECT_NE(script.find("HTMLDetailsElement"), std::string_view::npos);
}

TEST(UiResourcesTests, HasStableAssetPath) {
    EXPECT_EQ(
        drogular::ui_resources::StylesheetPath,
        "/__drogular/assets/ui.css"
    );
    EXPECT_EQ(
        drogular::ui_resources::ScriptPath,
        "/__drogular/assets/ui.js"
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