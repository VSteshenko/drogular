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

TEST(CoreInteractionsResourcesTests, MarksGetAndPostRequestsAsInteractions) {
    const auto script = drogular::interactions_resources::script();

    const auto requestOptions = script.find("const requestOptions");
    ASSERT_NE(requestOptions, std::string_view::npos);

    const auto postBranch = script.find("method: 'POST'", requestOptions);
    ASSERT_NE(postBranch, std::string_view::npos);

    const auto getInteractionHeader = script.find(
        "'X-Drogular-Interaction': 'true'",
        requestOptions
    );
    ASSERT_NE(getInteractionHeader, std::string_view::npos);
    EXPECT_LT(getInteractionHeader, postBranch);

    const auto postInteractionHeader = script.find(
        "'X-Drogular-Interaction': 'true'",
        postBranch
    );
    EXPECT_NE(postInteractionHeader, std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, RedirectedResponsesNavigateTheWholePage) {
    const auto script = drogular::interactions_resources::script();

    const auto redirected = script.find("if (response.redirected)");
    ASSERT_NE(redirected, std::string_view::npos);

    const auto navigate = script.find(
        "window.location.assign(response.url)",
        redirected
    );
    ASSERT_NE(navigate, std::string_view::npos);

    const auto readBody = script.find(
        "const html = await response.text()",
        redirected
    );
    ASSERT_NE(readBody, std::string_view::npos);

    EXPECT_LT(navigate, readBody);
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

TEST(CoreInteractionsResourcesTests, EmbedsOfflineRepresentationStoreContract) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("const representationKey = (identity)"), std::string_view::npos);
    EXPECT_NE(script.find("identity.context?.locale"), std::string_view::npos);
    EXPECT_NE(script.find("identity.context?.dimensions"), std::string_view::npos);
    EXPECT_NE(script.find("identity.scope?.kind"), std::string_view::npos);
    EXPECT_NE(script.find("identity.scope?.key"), std::string_view::npos);
    EXPECT_NE(script.find("createIndexedDbRepresentationStore"), std::string_view::npos);
    EXPECT_NE(script.find("window.indexedDB.open"), std::string_view::npos);
    EXPECT_NE(script.find("Object.freeze({ get, put, removeScope, clear })"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineStoreIsIntegratedIntoGetPipeline) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("__drogularOfflineReadModelsEnabled"), std::string_view::npos);
    EXPECT_NE(script.find("element.hasAttribute('dg-offline-read')"), std::string_view::npos);
    EXPECT_NE(script.find("const normalizedRequestKey = (url)"), std::string_view::npos);
    EXPECT_NE(script.find("document.documentElement.lang"), std::string_view::npos);
    EXPECT_NE(script.find("window.sessionStorage"), std::string_view::npos);
    EXPECT_NE(script.find("representationStore.get(identity)"), std::string_view::npos);
    EXPECT_NE(script.find("representationStore.put({"), std::string_view::npos);
    EXPECT_NE(script.find("X-Drogular-Offline-Representation"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineReadFallbackIsExactAndGetOnly) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("element.hasAttribute('dg-get')"), std::string_view::npos);
    EXPECT_NE(script.find("kind = 'fragment'"), std::string_view::npos);
    EXPECT_NE(script.find("requestKey: kind === 'shell'"), std::string_view::npos);
    EXPECT_NE(script.find(": normalizedRequestKey(url)"), std::string_view::npos);
    EXPECT_NE(script.find("kind: 'session'"), std::string_view::npos);
    EXPECT_NE(script.find("if (!representation) throw error"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineRuntimeOwnsIndependentLifecycleState) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("const offlineState = {"), std::string_view::npos);
    EXPECT_NE(script.find("connection: 'live'"), std::string_view::npos);
    EXPECT_NE(script.find("data: 'live'"), std::string_view::npos);
    EXPECT_NE(script.find("capability: 'read-write'"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-connection-state"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-data-state"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-mode"), std::string_view::npos);
    EXPECT_NE(script.find("dg:offline-state"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineLifecycleSeparatesConnectivityFromData) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("window.addEventListener('offline'"), std::string_view::npos);
    EXPECT_NE(script.find("window.addEventListener('online'"), std::string_view::npos);
    EXPECT_NE(script.find("connection: 'reconnecting'"), std::string_view::npos);
    EXPECT_NE(script.find("markCachedRepresentation()"), std::string_view::npos);
    EXPECT_NE(script.find("markNetworkRepresentation()"), std::string_view::npos);
    EXPECT_NE(script.find("offlineState.data === 'cached'"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, ReadOnlyCapabilityBlocksMutations) {
    const auto script = drogular::interactions_resources::script();

    const auto readOnly = script.find("offlineState.capability === 'read-only'");
    ASSERT_NE(readOnly, std::string_view::npos);
    EXPECT_NE(script.find("form.method.toUpperCase() !== 'GET'", readOnly), std::string_view::npos);
    EXPECT_NE(script.find("event.stopImmediatePropagation()", readOnly), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineRepresentationContextIncludesServerRenderedDimensions) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("const representationContext = (locale"), std::string_view::npos);
    EXPECT_NE(script.find("data-dg-context-"), std::string_view::npos);
    EXPECT_NE(script.find("document.documentElement.lang"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineShellUsesSameRepresentationStore) {
    const auto script = drogular::interactions_resources::script();

    EXPECT_NE(script.find("[dg-offline-shell]"), std::string_view::npos);
    EXPECT_NE(script.find("'shell'"), std::string_view::npos);
    EXPECT_NE(script.find("html: shell.outerHTML"), std::string_view::npos);
    EXPECT_NE(script.find("void storeCurrentShell()"), std::string_view::npos);
}

TEST(CoreInteractionsResourcesTests, OfflineLocaleRestoreIsCacheOnly) {
    const auto script = drogular::interactions_resources::script();

    const auto restore = script.find("const restoreOfflineLocale = async (locale)");
    ASSERT_NE(restore, std::string_view::npos);

    const auto install = script.find("    const install = (element)", restore);
    ASSERT_NE(install, std::string_view::npos);

    const auto body = script.substr(restore, install - restore);
    EXPECT_NE(body.find("representationStore.get(identity)"), std::string_view::npos);
    EXPECT_NE(body.find("restoreOfflineReadElement"), std::string_view::npos);
    EXPECT_EQ(body.find("fetch("), std::string_view::npos);
    EXPECT_NE(script.find("form.hasAttribute('dg-offline-locale')"), std::string_view::npos);
}