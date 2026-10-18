#pragma once

#include <drogular/action_handler.hpp>
#include <drogular/action_renderer.hpp>
#include <drogular/component.hpp>
#include <drogular/page.hpp>

#include <concepts>
#include <string>

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class MiniStoreStorefrontPageFragmentComponent final : public drogular::TemplateComponent {
public:
    std::string templatePath() const override {
        return PageType{}.templatePath();
    }
    std::string layoutPath() const override {
        return "layouts/storefront_content.html";
    }
};

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class MiniStoreStaffPageFragmentComponent final : public drogular::TemplateComponent {
public:
    std::string templatePath() const override {
        return PageType{}.templatePath();
    }
    std::string layoutPath() const override {
        return "layouts/staff_content.html";
    }
};

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class MiniStoreStorefrontPageFragmentAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        return drogular::ActionRenderer::render<MiniStoreStorefrontPageFragmentComponent<PageType>>(
            context,
            [](drogular::RenderContext& renderContext) {
                PageType page;
                page.onInit(renderContext);
            }
        );
    }
};

template <typename PageType>
    requires std::derived_from<PageType, drogular::TemplatePage>
class MiniStoreStaffPageFragmentAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(drogular::ActionContext& context) override {
        return drogular::ActionRenderer::render<MiniStoreStaffPageFragmentComponent<PageType>>(
            context,
            [](drogular::RenderContext& renderContext) {
                PageType page;
                page.onInit(renderContext);
            }
        );
    }
};