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

TEST(CompiledTemplateTests, RendersIfTrueBranch) {
    drogular::RenderContext context;

    context.set("show", true);

    const auto compiled =
        drogular::template_compiler::compile(
            "@if(show)<p>Yes</p>@else<p>No</p>@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>Yes</p>"
    );
}

TEST(CompiledTemplateTests, RendersIfFalseBranch) {
    drogular::RenderContext context;

    context.set("show", false);

    const auto compiled =
        drogular::template_compiler::compile(
            "@if(show)<p>Yes</p>@else<p>No</p>@endif"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>No</p>"
    );
}

TEST(CompiledTemplateTests, RendersForeach) {
    drogular::RenderContext context;

    context.set(
        "items",
        std::vector<std::string>{
            "A",
            "B"
        }
    );

    const auto compiled =
        drogular::template_compiler::compile(
            "@foreach(item in items)<p>{{ item }}</p>@endforeach"
        );

    EXPECT_EQ(
        compiled.render(context),
        "<p>A</p><p>B</p>"
    );
}