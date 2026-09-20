#include <drogular/action_renderer.hpp>
#include <drogular/services.hpp>

#include <drogon/HttpRequest.h>

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

namespace {

class RequestScopedService {
};

class ProbeComponent final : public drogular::Component {
public:
    static std::shared_ptr<RequestScopedService> renderedService;
    static std::string renderedRouteParam;

    std::string render(drogular::RenderContext& context) override {
        renderedService =
            context.requireService<RequestScopedService>();
        renderedRouteParam =
            context.requireRouteParam("projectId");

        return "<div>probe</div>";
    }
};

std::shared_ptr<RequestScopedService>
    ProbeComponent::renderedService;
std::string ProbeComponent::renderedRouteParam;

class LifecycleComponent final : public drogular::Component {
public:
    static int initCount;
    static int destroyCount;

    void onInit(drogular::RenderContext&) override {
        ++initCount;
    }

    void onDestroy(drogular::RenderContext&) override {
        ++destroyCount;
    }

    std::string render(drogular::RenderContext&) override {
        throw std::runtime_error("render failed");
    }
};

int LifecycleComponent::initCount = 0;
int LifecycleComponent::destroyCount = 0;

class PlainComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<span>plain</span>";
    }
};

} // namespace

TEST(
    CoreActionRendererTests,
    PreservesRequestStateAndAppliesSetup
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

    const auto result =
        drogular::ActionRenderer::render<ProbeComponent>(
            actionContext,
            [](drogular::RenderContext& context) {
                context.set("message", "prepared");
            },
            drogon::k422UnprocessableEntity
        );

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::Html
    );
    EXPECT_EQ(
        result.statusCode(),
        drogon::k422UnprocessableEntity
    );
    EXPECT_EQ(result.body(), "<div>probe</div>");
    EXPECT_EQ(
        ProbeComponent::renderedService,
        actionService
    );
    EXPECT_EQ(
        ProbeComponent::renderedRouteParam,
        "42"
    );
}

TEST(
    CoreActionRendererTests,
    SupportsRenderingWithoutSetup
) {
    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext actionContext(request, nullptr);

    const auto result =
        drogular::ActionRenderer::render<PlainComponent>(
            actionContext
        );

    EXPECT_EQ(
        result.statusCode(),
        drogon::k200OK
    );
    EXPECT_EQ(result.body(), "<span>plain</span>");
}

TEST(
    CoreActionRendererTests,
    GuaranteesDestroyWhenComponentRenderThrows
) {
    LifecycleComponent::initCount = 0;
    LifecycleComponent::destroyCount = 0;

    auto request = drogon::HttpRequest::newHttpRequest();
    drogular::ActionContext actionContext(request, nullptr);

    EXPECT_THROW(
        drogular::ActionRenderer::render<LifecycleComponent>(
            actionContext
        ),
        std::runtime_error
    );

    EXPECT_EQ(LifecycleComponent::initCount, 1);
    EXPECT_EQ(LifecycleComponent::destroyCount, 1);
}