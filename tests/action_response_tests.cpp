#include <drogular/action_response.hpp>

#include <gtest/gtest.h>

TEST(ActionResponseTests, ConvertsEmptyResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::empty()
        );

    ASSERT_NE(response, nullptr);
}

TEST(ActionResponseTests, ConvertsRedirectResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::redirect("/")
        );

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(
        response->statusCode(),
        drogon::k302Found
    );
}

TEST(ActionResponseTests, ConvertsHtmlResult) {
    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::html("<h1>Hello</h1>")
        );

    ASSERT_NE(response, nullptr);
    EXPECT_EQ(response->body(), "<h1>Hello</h1>");
}

TEST(ActionResponseTests, ConvertsJsonResult) {
    Json::Value json;
    json["ok"] = true;

    const auto response =
        drogular::toHttpResponse(
            drogular::ActionResult::json(json)
        );

    ASSERT_NE(response, nullptr);

    const auto responseJson =
        response->getJsonObject();

    ASSERT_NE(responseJson, nullptr);
    EXPECT_TRUE((*responseJson)["ok"].asBool());
}
