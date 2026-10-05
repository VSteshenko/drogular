#include <drogular/runtime_diagnostics.hpp>

#include <gtest/gtest.h>

#include <thread>
#include <vector>

TEST(RuntimeDiagnosticsTests, StartsWithEmptySnapshot) {
    drogular::RuntimeDiagnostics diagnostics;

    const auto snapshot = diagnostics.snapshot();

    EXPECT_EQ(snapshot.requests.requests, 0u);
    EXPECT_EQ(snapshot.interactions.requests, 0u);
    EXPECT_EQ(snapshot.rendering.componentsRendered, 0u);
    EXPECT_EQ(snapshot.serviceScopes.resolutions, 0u);
}

TEST(RuntimeDiagnosticsTests, RecordsIndependentRuntimeAreas) {
    drogular::RuntimeDiagnostics diagnostics;

    diagnostics.recordRequest();
    diagnostics.recordAction();
    diagnostics.recordRenderedAction();
    diagnostics.recordRedirect();

    diagnostics.recordInteractionRequest();
    diagnostics.recordInteractionGetRequest();
    diagnostics.recordInteractionPostRequest();
    diagnostics.recordInteractionSuccessfulResponse();
    diagnostics.recordInteractionClientErrorResponse();
    diagnostics.recordInteractionRedirect();

    diagnostics.recordComponentRendered(2);
    diagnostics.recordComponentRendered(7);
    diagnostics.recordComponentRendered(4);
    diagnostics.recordRenderFailure();

    diagnostics.recordServiceScopeResolution();
    diagnostics.recordServiceScopeResolution();
    diagnostics.recordServiceScopeCacheHit();
    diagnostics.recordServiceScopeServiceCreated();

    const auto snapshot = diagnostics.snapshot();

    EXPECT_EQ(snapshot.requests.requests, 1u);
    EXPECT_EQ(snapshot.requests.actions, 1u);
    EXPECT_EQ(snapshot.requests.renderedActions, 1u);
    EXPECT_EQ(snapshot.requests.redirects, 1u);

    EXPECT_EQ(snapshot.interactions.requests, 1u);
    EXPECT_EQ(snapshot.interactions.getRequests, 1u);
    EXPECT_EQ(snapshot.interactions.postRequests, 1u);
    EXPECT_EQ(snapshot.interactions.successfulResponses, 1u);
    EXPECT_EQ(snapshot.interactions.clientErrorResponses, 1u);
    EXPECT_EQ(snapshot.interactions.redirects, 1u);

    EXPECT_EQ(snapshot.rendering.componentsRendered, 3u);
    EXPECT_EQ(snapshot.rendering.renderFailures, 1u);
    EXPECT_EQ(snapshot.rendering.maximumDepth, 7u);

    EXPECT_EQ(snapshot.serviceScopes.resolutions, 2u);
    EXPECT_EQ(snapshot.serviceScopes.cacheHits, 1u);
    EXPECT_EQ(snapshot.serviceScopes.servicesCreated, 1u);
}

TEST(RuntimeDiagnosticsTests, SupportsConcurrentRecording) {
    drogular::RuntimeDiagnostics diagnostics;
    constexpr std::size_t threadCount = 8;
    constexpr std::size_t iterations = 1000;

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (std::size_t thread = 0; thread < threadCount; ++thread) {
        threads.emplace_back([&diagnostics, thread] {
            for (std::size_t i = 0; i < iterations; ++i) {
                diagnostics.recordRequest();
                diagnostics.recordComponentRendered(thread + 1);
                diagnostics.recordServiceScopeResolution();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    const auto snapshot = diagnostics.snapshot();
    const auto expected = threadCount * iterations;

    EXPECT_EQ(snapshot.requests.requests, expected);
    EXPECT_EQ(snapshot.rendering.componentsRendered, expected);
    EXPECT_EQ(snapshot.rendering.maximumDepth, threadCount);
    EXPECT_EQ(snapshot.serviceScopes.resolutions, expected);
}