#include <drogular/action_context.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

class ActionContextTestService {
public:
    int value() const {
        return 42;
    }
};

TEST(ActionContextTests, ReturnsRequest) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    EXPECT_EQ(context.request(), request);
}

TEST(ActionContextTests, ResolvesService) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ApplicationServices services;
    services.add<ActionContextTestService>();

    drogular::ActionContext context(request, &services);

    const auto service =
        context.service<ActionContextTestService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value(), 42);
}

TEST(ActionContextTests, RequiresService) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ApplicationServices services;
    services.add<ActionContextTestService>();

    drogular::ActionContext context(request, &services);

    const auto service =
        context.requireService<ActionContextTestService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value(), 42);
}

TEST(ActionContextTests, ThrowsWhenRequiredFormValueMissing) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    EXPECT_THROW(
        context.requireFormValue("title"),
        std::runtime_error
    );
}