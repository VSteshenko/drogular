#include <drogular/compiled_template.hpp>

#include <gtest/gtest.h>

TEST(CompiledTemplateTests, RendersText) {
    drogular::RenderContext context;

    const auto compiled =
        drogular::template_compiler::compile("Hello");

    EXPECT_EQ(
        compiled.render(context),
        "Hello"
    );
}

TEST(CompiledTemplateTests, RendersVariable) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto compiled =
        drogular::template_compiler::compile("<h1>{{ title }}</h1>");

    EXPECT_EQ(
        compiled.render(context),
        "<h1>Hello</h1>"
    );
}

TEST(CompiledTemplateTests, RendersRawVariable) {
    drogular::RenderContext context;

    context.set("content", std::string("<strong>Hello</strong>"));

    const auto compiled =
        drogular::template_compiler::compile("{{{ content }}}");

    EXPECT_EQ(
        compiled.render(context),
        "<strong>Hello</strong>"
    );
}
