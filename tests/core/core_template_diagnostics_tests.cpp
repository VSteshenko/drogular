#include <drogular/compiled_template.hpp>

#include <gtest/gtest.h>

using namespace drogular::template_compiler;

TEST(CoreTemplateDiagnosticsTests, DetectsUnclosedIf) {
    const auto result =
        compileWithDiagnostics("@if(show)<p>Hello</p>");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unclosed @if at position 0"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnclosedForeach) {
    const auto result =
        compileWithDiagnostics(
            "@foreach(item in items)<p>{{ item }}</p>"
        );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unclosed @foreach at position 0"
    );
}

TEST(CoreTemplateDiagnosticsTests, ValidTemplateHasNoErrors) {
    const auto result =
        compileWithDiagnostics(
            "@if(show)<p>Hello</p>@endif"
        );

    EXPECT_TRUE(result.valid());
    EXPECT_TRUE(result.diagnostics.errors().empty());
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedElse) {
    const auto result =
        compileWithDiagnostics("@else");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @else at position 0"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedEndIf) {
    const auto result =
        compileWithDiagnostics("@endif");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @endif at position 0"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedEndForeach) {
    const auto result =
        compileWithDiagnostics("@endforeach");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @endforeach at position 0"
    );
}