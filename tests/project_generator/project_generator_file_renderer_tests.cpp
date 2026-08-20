#include "file_renderer.hpp"

#include <gtest/gtest.h>

using drogular::generation::FileRenderer;

TEST(ProjectGeneratorFileRendererTests, ReplacesKnownVariables) {
    FileRenderer renderer;

    EXPECT_EQ(
        renderer.render(
            "project={{PROJECT_NAME}} namespace={{PROJECT_NAMESPACE}}",
            {
                {"PROJECT_NAME", "hello-drogular"},
                {"PROJECT_NAMESPACE", "hello_drogular"},
            }),
        "project=hello-drogular namespace=hello_drogular");
}

TEST(ProjectGeneratorFileRendererTests, ReplacesRepeatedVariables) {
    FileRenderer renderer;

    EXPECT_EQ(
        renderer.render(
            "{{PROJECT_NAME}}/{{PROJECT_NAME}}",
            {{"PROJECT_NAME", "portal"}}),
        "portal/portal");
}

TEST(ProjectGeneratorFileRendererTests, LeavesUnknownVariablesUntouched) {
    FileRenderer renderer;

    EXPECT_EQ(
        renderer.render(
            "{{KNOWN}} {{UNKNOWN}}",
            {{"KNOWN", "value"}}),
        "value {{UNKNOWN}}");
}