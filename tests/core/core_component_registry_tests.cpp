#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <string>

class CoreRegistryTestComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>Registry Test</p>";
    }
};

TEST(CoreComponentRegistryTests, RegistersComponent) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");

    EXPECT_TRUE(registry.contains("RegistryTest"));
}

TEST(CoreComponentRegistryTests, CreatesRegisteredComponent) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");

    const auto component = registry.create("RegistryTest");

    ASSERT_NE(component, nullptr);
}

TEST(CoreComponentRegistryTests, ReturnsNullptrForMissingComponent) {
    drogular::ComponentRegistry registry;

    const auto component = registry.create("Missing");

    EXPECT_EQ(component, nullptr);
}

class CoreMetadataComponent final : public drogular::Component {
public:
    static constexpr auto tag = "CoreMetadataComponent";

    std::string render(drogular::RenderContext&) override {
        return "<p>Metadata Component</p>";
    }
};

TEST(CoreComponentRegistryTests, RegistersComponentUsingMetadataTag) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreMetadataComponent>();

    EXPECT_TRUE(registry.contains("CoreMetadataComponent"));

    const auto component =
        registry.create("CoreMetadataComponent");

    ASSERT_NE(component, nullptr);
}

TEST(CoreComponentRegistryTests, ReportsDuplicateComponentTag) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");
    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");

    ASSERT_EQ(registry.diagnostics().entries().size(), 1U);

    const auto& diagnostic = registry.diagnostics().entries().front();

    EXPECT_EQ(diagnostic.code, "DGL-CMP-001");
    EXPECT_EQ(
        diagnostic.severity,
        drogular::DiagnosticSeverity::Warning
    );
    EXPECT_EQ(
        diagnostic.message,
        "Component tag is already registered: RegistryTest"
    );
}

TEST(CoreComponentRegistryTests, ClearsRegistrationDiagnostics) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");
    registry.registerComponent<CoreRegistryTestComponent>("RegistryTest");

    registry.clearDiagnostics();

    EXPECT_TRUE(registry.diagnostics().empty());
}