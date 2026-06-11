#include <drogular/component_registry.hpp>
#include <drogular/component.hpp>

#include <gtest/gtest.h>

#include <string>

class RegistryTestComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>Registry Test</p>";
    }
};

TEST(ComponentRegistryTests, RegistersComponent) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<RegistryTestComponent>("RegistryTest");

    EXPECT_TRUE(registry.contains("RegistryTest"));
}

TEST(ComponentRegistryTests, CreatesRegisteredComponent) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<RegistryTestComponent>("RegistryTest");

    const auto component = registry.create("RegistryTest");

    ASSERT_NE(component, nullptr);
}

TEST(ComponentRegistryTests, ReturnsNullptrForMissingComponent) {
    drogular::ComponentRegistry registry;

    const auto component = registry.create("Missing");

    EXPECT_EQ(component, nullptr);
}

class MetadataComponent final : public drogular::Component {
public:
    static constexpr auto tag = "MetadataComponent";

    std::string render(drogular::RenderContext&) override {
        return "<p>Metadata Component</p>";
    }
};

TEST(ComponentRegistryTests, RegistersComponentUsingMetadataTag) {
    drogular::ComponentRegistry registry;

    registry.registerComponent<MetadataComponent>();

    EXPECT_TRUE(registry.contains("MetadataComponent"));

    const auto component =
        registry.create("MetadataComponent");

    ASSERT_NE(component, nullptr);
}