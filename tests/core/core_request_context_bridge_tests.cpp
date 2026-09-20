#include <drogular/action_context.hpp>
#include <drogular/detail/request_context_bridge.hpp>
#include <drogular/render_context.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

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