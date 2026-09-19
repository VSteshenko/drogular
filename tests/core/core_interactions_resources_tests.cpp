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
    EXPECT_NE(script.find("event.submitter"), std::string_view::npos);
    EXPECT_NE(script.find("submitter.name"), std::string_view::npos);
    EXPECT_NE(script.find("submitter.value"), std::string_view::npos);
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

TEST(CoreInteractionsResourcesTests, SupportsPostInteractions) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("[dg-get], [dg-post]"), std::string_view::npos);
    EXPECT_NE(script.find("element.getAttribute('dg-post')"), std::string_view::npos);
    EXPECT_NE(script.find("method: 'POST'"), std::string_view::npos);
    EXPECT_NE(script.find("X-Drogular-Interaction"), std::string_view::npos);
    EXPECT_NE(script.find("requestParameters(element, submitter)"), std::string_view::npos);
    EXPECT_NE(script.find("dg-on-success-refresh"), std::string_view::npos);
    EXPECT_NE(script.find("document.querySelectorAll(successRefresh)"), std::string_view::npos);
    EXPECT_NE(script.find("if (!response.ok && !postInteraction)"), std::string_view::npos);
    EXPECT_NE(script.find("if (!response.ok)"), std::string_view::npos);
}

TEST(CoreInteractionsResources, PostInteractionsDoNotLoadAndReinstallRenderedInteractions) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(
        script.find("(element.hasAttribute('dg-post') ? 'submit' : 'load')"),
        std::string_view::npos
    );
    EXPECT_NE(
        script.find("if (element.hasAttribute('data-dg-installed')) return"),
        std::string_view::npos
    );
    EXPECT_NE(
        script.find("target.querySelectorAll('[dg-get], [dg-post]').forEach(install)"),
        std::string_view::npos
    );
}

TEST(CoreInteractionsResourcesTests, PostInteractionMayOmitTarget) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(
        script.find("element.hasAttribute('dg-post') ? null : element"),
        std::string::npos
    );
    EXPECT_NE(script.find("if (!url) return;"), std::string::npos);
    EXPECT_NE(
        script.find("setState(element, target ? responseState(html) : 'ready')"),
        std::string::npos
    );
}

TEST(CoreInteractionsResourcesTests, SupportsSuccessNavigation) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(
        script.find("dg-on-success-navigate"),
        std::string_view::npos
    );
    EXPECT_NE(
        script.find("window.location.assign(successNavigate)"),
        std::string_view::npos
    );
}