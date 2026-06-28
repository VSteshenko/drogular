#include <drogular/template_runtime.hpp>
#include <drogular/render_context.hpp>

#include <gtest/gtest.h>
#include <json/json.h>

using namespace drogular::template_compiler;

TEST(CoreTemplateRuntimeTests, ResolvesEscapedVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("<Hello>"));

    const auto value = resolveVariable("title", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "&lt;Hello&gt;");
}

TEST(CoreTemplateRuntimeTests, ResolvesRawVariable) {
    drogular::RenderContext context;

    context.set("html", std::string("<strong>Hello</strong>"));

    const auto value = resolveRawVariable("html", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "<strong>Hello</strong>");
}

TEST(CoreTemplateRuntimeTests, ResolvesJsonPath) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["name"] = "Vadim";

    context.set("user", user);

    const auto value = resolveVariable("user.profile.name", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "Vadim");
}

TEST(CoreTemplateRuntimeTests, EvaluatesBoolCondition) {
    drogular::RenderContext context;

    context.set("show", true);

    EXPECT_TRUE(evaluateCondition("show", context));
}

TEST(CoreTemplateRuntimeTests, EvaluatesJsonCondition) {
    drogular::RenderContext context;

    Json::Value user;
    user["active"] = true;

    context.set("user", user);

    EXPECT_TRUE(evaluateCondition("user.active", context));
}

TEST(CoreTemplateRuntimeTests, ResolvesJsonValue) {
    drogular::RenderContext context;

    Json::Value user;
    user["profile"]["name"] = "Vadim";

    context.set("user", user);

    const auto value =
        resolveJsonValue("user.profile.name", context);

    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE(value->isString());
    EXPECT_EQ(value->asString(), "Vadim");
}