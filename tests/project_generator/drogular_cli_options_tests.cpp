#include "cli_options.hpp"

#include <gtest/gtest.h>

#include <string_view>
#include <vector>

using drogular::cli::Command;
using drogular::cli::Options;
using drogular::cli::parseArguments;
using drogular::cli::projectNamespace;
using drogular::cli::validProjectName;

TEST(DrogularCliOptionsTests, DefaultsToHelpWithoutArguments) {
    const Options options = parseArguments({});

    EXPECT_EQ(options.command, Command::Help);
}

TEST(DrogularCliOptionsTests, ParsesNewProjectWithDefaultTemplate) {
    const Options options = parseArguments({"new", "MyApp"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.projectPath, "MyApp");
    EXPECT_EQ(options.projectName, "MyApp");
    EXPECT_EQ(options.templateId, "minimal");
}

TEST(DrogularCliOptionsTests, ParsesProjectPathAndUsesLeafAsProjectName) {
    const Options options = parseArguments({"new", "examples/admin/My-App"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.projectPath, "examples/admin/My-App");
    EXPECT_EQ(options.projectName, "My-App");
}

TEST(DrogularCliOptionsTests, NormalizesTrailingSeparatorInProjectPath) {
    const Options options = parseArguments({"new", "examples/admin/MyApp/"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.projectPath, "examples/admin/MyApp");
    EXPECT_EQ(options.projectName, "MyApp");
}

TEST(DrogularCliOptionsTests, AllowsParentDirectoriesInProjectPath) {
    const Options options = parseArguments({"new", "../generated/MyApp"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.projectPath, "../generated/MyApp");
    EXPECT_EQ(options.projectName, "MyApp");
}

TEST(DrogularCliOptionsTests, RejectsInvalidProjectNameAtEndOfPath) {
    const Options options = parseArguments({"new", "examples/My App"});

    EXPECT_EQ(options.command, Command::Invalid);
    EXPECT_FALSE(options.error.empty());
}

TEST(DrogularCliOptionsTests, ParsesNewProjectWithTemplateOption) {
    const Options options = parseArguments({"new", "MyApp", "--template", "pwa"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.templateId, "pwa");
}

TEST(DrogularCliOptionsTests, ParsesNewProjectWithInlineTemplateOption) {
    const Options options = parseArguments({"new", "MyApp", "--template=pwa"});

    EXPECT_EQ(options.command, Command::NewProject);
    EXPECT_EQ(options.templateId, "pwa");
}

TEST(DrogularCliOptionsTests, ParsesTemplatesCommand) {
    const Options options = parseArguments({"templates"});

    EXPECT_EQ(options.command, Command::ListTemplates);
}

TEST(DrogularCliOptionsTests, RejectsMissingProjectName) {
    const Options options = parseArguments({"new"});

    EXPECT_EQ(options.command, Command::Invalid);
    EXPECT_FALSE(options.error.empty());
}

TEST(DrogularCliOptionsTests, RejectsMissingTemplateId) {
    const Options options = parseArguments({"new", "MyApp", "--template"});

    EXPECT_EQ(options.command, Command::Invalid);
    EXPECT_FALSE(options.error.empty());
}

TEST(DrogularCliOptionsTests, RejectsDuplicateTemplateOption) {
    const Options options = parseArguments({
        "new", "MyApp", "--template", "minimal", "--template=pwa"});

    EXPECT_EQ(options.command, Command::Invalid);
}

TEST(DrogularCliOptionsTests, RejectsUnknownOption) {
    const Options options = parseArguments({"new", "MyApp", "--unknown"});

    EXPECT_EQ(options.command, Command::Invalid);
}

TEST(DrogularCliOptionsTests, ValidatesProjectNames) {
    EXPECT_TRUE(validProjectName("My-App_2"));
    EXPECT_FALSE(validProjectName(""));
    EXPECT_FALSE(validProjectName("My App"));
    EXPECT_FALSE(validProjectName("../MyApp"));
}

TEST(DrogularCliOptionsTests, BuildsCppSafeProjectNamespace) {
    EXPECT_EQ(projectNamespace("my-app"), "my_app");
    EXPECT_EQ(projectNamespace("42-app"), "_42_app");
}