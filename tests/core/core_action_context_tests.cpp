#include <drogular/action_context.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

class CoreActionContextTestService {
public:
    int value() const {
        return 42;
    }
};

TEST(CoreActionContextTests, ReturnsRequest) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    EXPECT_EQ(context.request(), request);
}

TEST(CoreActionContextTests, ResolvesService) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ApplicationServices services;
    services.add<CoreActionContextTestService>();

    drogular::ActionContext context(request, &services);

    const auto service =
        context.service<CoreActionContextTestService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value(), 42);
}

TEST(CoreActionContextTests, RequiresService) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ApplicationServices services;
    services.add<CoreActionContextTestService>();

    drogular::ActionContext context(request, &services);

    const auto service =
        context.requireService<CoreActionContextTestService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->value(), 42);
}

TEST(CoreActionContextTests, ThrowsWhenRequiredFormValueMissing) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    EXPECT_THROW(
        context.requireFormValue("title"),
        std::runtime_error
    );
}

TEST(CoreActionContextTests, RequireFormThrowsWhenMissing) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    EXPECT_THROW(
        context.requireForm<int>("id"),
        drogular::ActionValidationError
    );
}

TEST(CoreActionContextTests, ReadsCookie) {
    auto request =
        drogon::HttpRequest::newHttpRequest();

    request->addCookie("session_id", "abc123");

    drogular::ActionContext context(
        request,
        nullptr
    );

    const auto value =
        context.cookie("session_id");

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "abc123");
}