#include "ui/components/portal_pagination_component.hpp"
#include "ui/components/portal_select_component.hpp"
#include "ui/models/portal_pagination_view_model.hpp"

#include <drogular/application_options.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <filesystem>

TEST(PortalPaginationViewModelTests, BuildsLinksAndNavigationState) {
    const auto model = makePortalPaginationViewModel(
        2,
        3,
        [](int page) {
            return "/projects?page=" + std::to_string(page);
        }
    );

    ASSERT_TRUE(model.visible);
    ASSERT_TRUE(model.hasPrevious);
    ASSERT_TRUE(model.hasNext);
    EXPECT_EQ(model.previousUrl, "/projects?page=1");
    EXPECT_EQ(model.nextUrl, "/projects?page=3");
    ASSERT_EQ(model.pages.size(), 3u);
    EXPECT_FALSE(model.pages[0].current);
    EXPECT_TRUE(model.pages[1].current);
    EXPECT_FALSE(model.pages[2].current);
}

TEST(PortalPaginationViewModelTests, HidesSinglePageNavigation) {
    const auto model = makePortalPaginationViewModel(
        1,
        1,
        [](int page) {
            return "/users?page=" + std::to_string(page);
        }
    );

    EXPECT_FALSE(model.visible);
    EXPECT_FALSE(model.hasPrevious);
    EXPECT_FALSE(model.hasNext);
    ASSERT_EQ(model.pages.size(), 1u);
}

TEST(PortalUiComponentTests, RendersSelectFromContextOptions) {
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR)
        / "examples/portal_demo/templates"
    );
    options.setTemplateCacheEnabled(false);
    drogular::ApplicationServices services;
    services.setOptions(&options);
    services.components().registerComponent<PortalSelectComponent>();

    drogular::RenderContext context;
    context.setServices(&services);

    Json::Value roleOoptions(Json::arrayValue);
    Json::Value first(Json::objectValue);
    first["value"] = "admin";
    first["label"] = "Administrator";
    first["selected"] = true;
    first["disabled"] = false;
    roleOoptions.append(first);
    context.set("roleOptions", roleOoptions);

    const auto html = drogular::component_renderer::render(
        R"(<PortalSelect id="role" name="role" options="roleOptions" required="true" />)",
        services.components(),
        context
    );

    EXPECT_NE(html.find("id=\"role\""), std::string::npos);
    EXPECT_NE(html.find("name=\"role\""), std::string::npos);
    EXPECT_NE(html.find("required"), std::string::npos);
    EXPECT_NE(html.find("value=\"admin\""), std::string::npos);
    EXPECT_NE(html.find("selected"), std::string::npos);
    EXPECT_NE(html.find("Administrator"), std::string::npos);
}

TEST(PortalUiComponentTests, RendersPaginationFromViewModel) {
    drogular::ApplicationOptions options;
    options.setTemplateRoot(
        std::filesystem::path(DROGULAR_SOURCE_DIR)
        / "examples/portal_demo/templates"
    );
    options.setTemplateCacheEnabled(false);
    drogular::ApplicationServices services;
    services.setOptions(&options);
    services.components().registerComponent<PortalPaginationComponent>();

    drogular::RenderContext context;
    context.setServices(&services);
    context.setJson(
        "pagination",
        makePortalPaginationViewModel(
            2,
            3,
            [](int page) {
                return "/departments?page=" + std::to_string(page);
            }
        )
    );

    const auto html = drogular::component_renderer::render(
        R"(<PortalPagination model="pagination" ariaLabel="Department pages" previousLabel="Previous" nextLabel="Next" />)",
        services.components(),
        context
    );

    EXPECT_NE(html.find("aria-label=\"Department pages\""), std::string::npos);
    EXPECT_NE(html.find("/departments?page=1"), std::string::npos);
    EXPECT_NE(html.find("aria-current=\"page\">2"), std::string::npos);
    EXPECT_NE(html.find("/departments?page=3"), std::string::npos);
}