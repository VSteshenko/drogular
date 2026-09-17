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

TEST(CoreInteractionsResourcesTests, FormSubmitterContributesItsNameAndValue) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("submitter && submitter.name"), std::string_view::npos);
    EXPECT_NE(script.find("event.submitter || null"), std::string_view::npos);
    EXPECT_NE(
        script.find("url.searchParams.set(submitter.name, submitter.value)"),
        std::string_view::npos
    );
}

TEST(CoreInteractionsResourcesTests, DeclarativeResetRestoresValuesAndRefreshesForm) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("[dg-reset-value]"), std::string_view::npos);
    EXPECT_NE(script.find("element.reset()"), std::string_view::npos);
    EXPECT_NE(
        script.find("control.getAttribute('dg-reset-value')"),
        std::string_view::npos
    );
    EXPECT_NE(script.find("[dg-reset]"), std::string_view::npos);
    EXPECT_NE(script.find("resetForm(element)"), std::string_view::npos);
    EXPECT_NE(script.find("refresh(element)"), std::string_view::npos);
}

TEST(CoreInteractionsResources, HistoryContractIsEmbedded) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("dg-history"), std::string_view::npos);
    EXPECT_NE(script.find("dg-history-url"), std::string_view::npos);
    EXPECT_NE(script.find("window.history.replaceState"), std::string_view::npos);
    EXPECT_NE(script.find("window.history.pushState"), std::string_view::npos);
    EXPECT_NE(script.find("syncHistory(element, url)"), std::string_view::npos);
}
TEST(CoreInteractionsResourcesTests, CurrentUrlControlsAreUpdatedBeforeSubmit) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("[dg-current-url]"), std::string_view::npos);
    EXPECT_NE(script.find("window.location.pathname"), std::string_view::npos);
    EXPECT_NE(script.find("window.location.search"), std::string_view::npos);
    EXPECT_NE(script.find("window.location.hash"), std::string_view::npos);
    EXPECT_NE(script.find("control.value = currentUrl"), std::string_view::npos);
}
