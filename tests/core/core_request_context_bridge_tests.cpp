#include <drogular/action_context.hpp>
#include <drogular/job_runtime.hpp>
#include <drogular/detail/request_context_bridge.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>
#include <drogular/security/principal_builder.hpp>
#include <drogular/security/security_context.hpp>

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <string>
#include <type_traits>

namespace {

class RequestScopedService {
};

} // namespace

TEST(
    RequestContextBridgeTests,
    PreservesRequestServicesRouteParametersAndScopedService
) {
    drogular::ApplicationServices services;

    services.addScoped<RequestScopedService>(
        [] {
            return std::make_shared<RequestScopedService>();
        }
    );

    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext actionContext(
        request,
        &services
    );

    actionContext.setRouteParam("projectId", "42");

    auto actionService =
        actionContext.requireService<RequestScopedService>();

    auto renderContext =
        drogular::detail::RequestContextBridge::renderContext(
            actionContext
        );

    auto renderService =
        renderContext.requireService<RequestScopedService>();

    EXPECT_EQ(renderContext.request(), request);
    EXPECT_EQ(renderContext.services(), &services);
    EXPECT_EQ(
        renderContext.requireRouteParam("projectId"),
        "42"
    );
    EXPECT_EQ(renderService, actionService);
}

TEST(
    RequestContextBridgeTests,
    ChildRenderContextKeepsBridgedRequestScope
) {
    drogular::ApplicationServices services;

    services.addScoped<RequestScopedService>(
        [] {
            return std::make_shared<RequestScopedService>();
        }
    );

    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext actionContext(
        request,
        &services
    );

    auto actionService =
        actionContext.requireService<RequestScopedService>();

    auto renderContext =
        drogular::detail::RequestContextBridge::renderContext(
            actionContext
        );

    auto child = renderContext.createChild();

    EXPECT_EQ(
        child.requireService<RequestScopedService>(),
        actionService
    );
}

TEST(
    RequestContextBridgeTests,
    DifferentActionRequestsKeepIndependentScopes
) {
    drogular::ApplicationServices services;

    services.addScoped<RequestScopedService>(
        [] {
            return std::make_shared<RequestScopedService>();
        }
    );

    drogular::ActionContext firstAction(
        drogon::HttpRequest::newHttpRequest(),
        &services
    );

    drogular::ActionContext secondAction(
        drogon::HttpRequest::newHttpRequest(),
        &services
    );

    auto firstRender =
        drogular::detail::RequestContextBridge::renderContext(
            firstAction
        );

    auto secondRender =
        drogular::detail::RequestContextBridge::renderContext(
            secondAction
        );

    EXPECT_EQ(
        firstRender.requireService<RequestScopedService>(),
        firstAction.requireService<RequestScopedService>()
    );
    EXPECT_EQ(
        secondRender.requireService<RequestScopedService>(),
        secondAction.requireService<RequestScopedService>()
    );
    EXPECT_NE(
        firstRender.requireService<RequestScopedService>(),
        secondRender.requireService<RequestScopedService>()
    );
}

TEST(RequestContextBridgeTests, AnonymousSecurityContextIsDefault) {
    drogular::ActionContext action(drogon::HttpRequest::newHttpRequest(), nullptr);
    EXPECT_FALSE(action.securityContext().authenticated());
    EXPECT_EQ(action.principal(), nullptr);

    const auto render = drogular::detail::RequestContextBridge::renderContext(action);
    EXPECT_FALSE(render.securityContext().authenticated());
    EXPECT_EQ(render.principal(), nullptr);
}

TEST(RequestContextBridgeTests, SecuritySnapshotSharedWithRenderChildrenAndDetachedCopy) {
    using namespace drogular::security;
    auto principal = PrincipalBuilder{}
        .issuer("test-issuer")
        .subject("alice")
        .role("editor")
        .build();
    ASSERT_TRUE(principal);

    SecurityContext detached;
    {
        drogular::ActionContext action(
            drogon::HttpRequest::newHttpRequest(), nullptr);
        drogular::detail::RequestContextBridge::bindSecurityContext(
            action, SecurityContext::forPrincipal(
                std::move(principal.value()), std::string{"test-adapter"}));
        auto render = drogular::detail::RequestContextBridge::renderContext(action);
        auto child = render.createChild();

        ASSERT_TRUE(action.securityContext().authenticated());
        ASSERT_NE(action.principal(), nullptr);
        EXPECT_EQ(action.principal(), render.principal());
        EXPECT_EQ(render.principal(), child.principal());
        EXPECT_EQ(child.principal()->id().subject, "alice");
        detached = child.securityContext();
    }
    ASSERT_TRUE(detached.authenticated());
    ASSERT_NE(detached.principal(), nullptr);
    EXPECT_EQ(detached.principal()->id().subject, "alice");
    ASSERT_TRUE(detached.authenticationMethod());
    EXPECT_EQ(*detached.authenticationMethod(), "test-adapter");
}

TEST(RequestContextBridgeTests, SecurityContextsAreIsolatedAcrossRequests) {
    using namespace drogular::security;
    auto principal = PrincipalBuilder{}
        .issuer("test-issuer")
        .subject("alice")
        .build();
    ASSERT_TRUE(principal);

    drogular::ActionContext first(drogon::HttpRequest::newHttpRequest(), nullptr);
    drogular::ActionContext second(drogon::HttpRequest::newHttpRequest(), nullptr);
    drogular::detail::RequestContextBridge::bindSecurityContext(
        first,
        SecurityContext::forPrincipal(std::move(principal.value()))
    );
    EXPECT_TRUE(first.securityContext().authenticated());
    EXPECT_FALSE(second.securityContext().authenticated());
    EXPECT_EQ(second.principal(), nullptr);
}

TEST(RequestContextBridgeTests, IdentityFlowsExplicitlyFromRequestToBackgroundJob) {
    using namespace drogular::security;
    using namespace std::chrono_literals;

    auto built = PrincipalBuilder{}
        .issuer("test-issuer")
        .subject("background-user")
        .claim("department", std::string{"engineering"})
        .build();
    ASSERT_TRUE(built);

    drogular::JobRuntime jobs(1, 4);
    std::promise<std::string> completed;
    auto future = completed.get_future();
    std::weak_ptr<const Principal> weak;
    {
        drogular::ActionContext action(drogon::HttpRequest::newHttpRequest(), nullptr);
        drogular::detail::RequestContextBridge::bindSecurityContext(
            action, SecurityContext::forPrincipal(std::move(built.value())));
        const auto render = drogular::detail::RequestContextBridge::renderContext(action);
        const auto child = render.createChild();
        const auto identity = child.securityContext();
        weak = identity.principal();
        ASSERT_EQ(action.principal(), child.principal());

        jobs.submit("attribution-only", [&completed](drogular::JobContext& job) {
            const auto initiator = job.initiator();
            completed.set_value(initiator ? initiator->id().subject : "missing");
        }, identity);
    }

    // No HTTP context survives, but the explicitly captured identity does.
    EXPECT_FALSE(weak.expired());
    ASSERT_EQ(future.wait_for(5s), std::future_status::ready);
    EXPECT_EQ(future.get(), "background-user");
    jobs.shutdown();
}

TEST(RequestContextBridgeTests, AnonymousRequestDoesNotImplyJobIdentity) {
    using namespace std::chrono_literals;
    drogular::ActionContext action(drogon::HttpRequest::newHttpRequest(), nullptr);
    drogular::JobRuntime jobs(1, 4);
    std::promise<bool> result;
    auto future = result.get_future();
    jobs.submit("anonymous", [&result](drogular::JobContext& job) {
        result.set_value(job.initiator() == nullptr);
    }, action.securityContext());
    ASSERT_EQ(future.wait_for(5s), std::future_status::ready);
    EXPECT_TRUE(future.get());
    jobs.shutdown();
}