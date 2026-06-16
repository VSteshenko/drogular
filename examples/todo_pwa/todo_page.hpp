#pragma once

#include <drogular/page.hpp>

class TodoPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override;

    std::string templateHtml() const override;
};