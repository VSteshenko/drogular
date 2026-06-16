#include <drogular/action_handler.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

class TestRedirectAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext&
    ) override {
        return drogular::ActionResult::redirect("/");
    }
};

TEST(ActionHandlerTests, HandlesAction) {
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::ActionContext context(request, nullptr);

    TestRedirectAction action;

    const auto result = action.handle(context);

    EXPECT_EQ(
        result.type(),
        drogular::ActionResultType::Redirect
    );

    EXPECT_EQ(result.location(), "/");
}
