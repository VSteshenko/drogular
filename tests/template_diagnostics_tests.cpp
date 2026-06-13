#include <drogular/compiled_template.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(TemplateDiagnosticsTests, DetectsUnclosedIf) {
    const auto result =
        compileWithDiagnostics("@if(show)<p>Hello</p>");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].message, "Unclosed @if");
}

TEST(TemplateDiagnosticsTests, DetectsUnclosedForeach) {
    const auto result =
        compileWithDiagnostics(
            "@foreach(item in items)<p>{{ item }}</p>"
        );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].message, "Unclosed @foreach");
}

TEST(TemplateDiagnosticsTests, ValidTemplateHasNoErrors) {
    const auto result =
        compileWithDiagnostics(
            "@if(show)<p>Hello</p>@endif"
        );

    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result.diagnostics.errors().empty());
}
