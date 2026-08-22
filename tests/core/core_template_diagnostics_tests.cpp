#include <drogular/compiled_template.hpp>

#include <gtest/gtest.h>

using namespace drogular;
using namespace drogular::template_compiler;

TEST(CoreTemplateDiagnosticsTests, DetectsMissingEndIf) {
    const auto result = compileWithDiagnostics(
        "@if(show)<p>Hello</p>",
        "pages/example.html"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    const auto& diagnostic = result.diagnostics.errors()[0];

    EXPECT_EQ(diagnostic.code, "DGL-TPL-004");
    EXPECT_EQ(diagnostic.severity, DiagnosticSeverity::Error);
    EXPECT_EQ(diagnostic.message, "Missing @endif");
    EXPECT_EQ(diagnostic.location.source, "pages/example.html");
    EXPECT_EQ(diagnostic.location.position, 0);
    EXPECT_EQ(diagnostic.location.line, 1);
    EXPECT_EQ(diagnostic.location.column, 1);
}

TEST(CoreTemplateDiagnosticsTests, DetectsMissingEndForeach) {
    const auto result = compileWithDiagnostics(
        "@foreach(item in items)<p>{{ item }}</p>"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-005");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Missing @endforeach"
    );
}

TEST(CoreTemplateDiagnosticsTests, ValidTemplateHasNoDiagnostics) {
    const auto result = compileWithDiagnostics(
        "@if(show)<p>Hello</p>@endif"
    );

    EXPECT_TRUE(result.valid());
    EXPECT_FALSE(result.diagnostics.hasErrors());
    EXPECT_TRUE(result.diagnostics.empty());
    EXPECT_TRUE(result.diagnostics.entries().empty());
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedElse) {
    const auto result = compileWithDiagnostics("@else");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-001");
    EXPECT_EQ(result.diagnostics.errors()[0].message, "Unexpected @else");
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedEndIf) {
    const auto result = compileWithDiagnostics("@endif");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-002");
    EXPECT_EQ(result.diagnostics.errors()[0].message, "Unexpected @endif");
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedEndForeach) {
    const auto result = compileWithDiagnostics("@endforeach");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-003");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @endforeach"
    );
}

TEST(CoreTemplateDiagnosticsTests, ReportsLineAndColumn) {
    const auto result = compileWithDiagnostics(
        "<main>\n  <p>Hello</p>\n  @endif\n</main>",
        "pages/dashboard.html"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    const auto& location = result.diagnostics.errors()[0].location;

    EXPECT_EQ(location.source, "pages/dashboard.html");
    EXPECT_EQ(location.line, 3);
    EXPECT_EQ(location.column, 3);
}

TEST(CoreTemplateDiagnosticsTests, KeepsAllSeverityLevels) {
    Diagnostics diagnostics("abc", "test.html");

    diagnostics.info("DGL-TEST-001", "Info", 0);
    diagnostics.warning("DGL-TEST-002", "Warning", 1);
    diagnostics.error("DGL-TEST-003", "Error", 2);

    ASSERT_EQ(diagnostics.entries().size(), 3);
    ASSERT_EQ(diagnostics.errors().size(), 1);
    EXPECT_TRUE(diagnostics.hasErrors());
    EXPECT_FALSE(diagnostics.empty());
    EXPECT_EQ(
        diagnostics.entries()[1].severity,
        DiagnosticSeverity::Warning
    );
    EXPECT_EQ(diagnostics.errors()[0].code, "DGL-TEST-003");
}

TEST(CoreTemplateDiagnosticsTests, DetectsInvalidIfExpression) {
    const auto result = compileWithDiagnostics(
        "<main>\n  @if(page >)Visible@endif\n</main>",
        "pages/example.html"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);

    const auto& diagnostic = result.diagnostics.errors()[0];

    EXPECT_EQ(diagnostic.code, "DGL-TPL-006");
    EXPECT_EQ(
        diagnostic.message,
        "Invalid @if expression: Expected value after '>'"
    );
    EXPECT_EQ(diagnostic.location.source, "pages/example.html");
    EXPECT_EQ(diagnostic.location.line, 2);
    EXPECT_EQ(diagnostic.location.column, 13);
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnterminatedIfDirective) {
    const auto result = compileWithDiagnostics(
        "@if(status == \"active)Visible@endif"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-006");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Invalid @if expression: Missing closing ')'"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsInvalidForeachExpression) {
    const auto result = compileWithDiagnostics(
        "@foreach(item items){{ item }}@endforeach",
        "pages/example.html"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-007");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Invalid @foreach expression: Expected 'in'"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsInvalidForeachWhereCondition) {
    const auto result = compileWithDiagnostics(
        "@foreach(item in items where item.score >){{ item }}@endforeach",
        "pages/example.html"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-008");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Invalid @foreach where condition: Expected value after '>'"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnterminatedForeachDirective) {
    const auto result = compileWithDiagnostics(
        "@foreach(item in items where (item.active){{ item }}@endforeach"
    );

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-007");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Invalid @foreach expression: Missing closing ')'"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsUnexpectedEmpty) {
    const auto result = compileWithDiagnostics("@empty");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-009");
    EXPECT_EQ(result.diagnostics.errors()[0].message, "Unexpected @empty");
}

TEST(CoreTemplateDiagnosticsTests, DetectsBreakOutsideForeach) {
    const auto result = compileWithDiagnostics("@break");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-010");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @break outside @foreach"
    );
}

TEST(CoreTemplateDiagnosticsTests, DetectsContinueOutsideForeach) {
    const auto result = compileWithDiagnostics("@continue");

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.diagnostics.errors().size(), 1);
    EXPECT_EQ(result.diagnostics.errors()[0].code, "DGL-TPL-011");
    EXPECT_EQ(
        result.diagnostics.errors()[0].message,
        "Unexpected @continue outside @foreach"
    );
}