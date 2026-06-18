#include <drogular/action_result.hpp>

#include <gtest/gtest.h>

TEST(CoreActionResultTests, CreatesEmptyResult) {
    const auto result = drogular::ActionResult::empty();

    EXPECT_EQ(result.type(), drogular::ActionResultType::Empty);
}

TEST(CoreActionResultTests, CreatesRedirectResult) {
    const auto result =
        drogular::ActionResult::redirect("/todos");

    EXPECT_EQ(result.type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.location(), "/todos");
}

TEST(CoreActionResultTests, CreatesHtmlResult) {
    const auto result =
        drogular::ActionResult::html("<h1>Hello</h1>");

    EXPECT_EQ(result.type(), drogular::ActionResultType::Html);
    EXPECT_EQ(result.body(), "<h1>Hello</h1>");
}

TEST(CoreActionResultTests, CreatesJsonResult) {
    Json::Value json;
    json["ok"] = true;

    const auto result =
        drogular::ActionResult::json(json);

    EXPECT_EQ(result.type(), drogular::ActionResultType::Json);
    EXPECT_TRUE(result.json()["ok"].asBool());
}