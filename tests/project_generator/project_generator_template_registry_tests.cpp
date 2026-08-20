#include "template_registry.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

using drogular::generation::ProjectTemplate;
using drogular::generation::TemplateRegistry;

TEST(ProjectGeneratorTemplateRegistryTests, RegistersAndFindsTemplateById) {
    TemplateRegistry registry;
    registry.add({
        .id = "minimal",
        .name = "Minimal Application",
        .description = "Minimal Drogular application",
    });

    const ProjectTemplate* projectTemplate = registry.find("minimal");

    ASSERT_NE(projectTemplate, nullptr);
    EXPECT_EQ(projectTemplate->name, "Minimal Application");
}

TEST(ProjectGeneratorTemplateRegistryTests, ReturnsNullForUnknownTemplate) {
    TemplateRegistry registry;

    EXPECT_EQ(registry.find("missing"), nullptr);
}

TEST(ProjectGeneratorTemplateRegistryTests, RejectsEmptyTemplateId) {
    TemplateRegistry registry;

    EXPECT_THROW(
        registry.add({.name = "Invalid"}),
        std::invalid_argument);
}

TEST(ProjectGeneratorTemplateRegistryTests, RejectsDuplicateTemplateId) {
    TemplateRegistry registry;
    registry.add({.id = "minimal", .name = "First"});

    EXPECT_THROW(
        registry.add({.id = "minimal", .name = "Second"}),
        std::invalid_argument);
}