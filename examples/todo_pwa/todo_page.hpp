#pragma once

#include <drogular/page.hpp>

class TodoPage final : public drogular::TemplatePage {
public:
    void onInit(drogular::RenderContext& context) override;

    std::string templatePath() const override {
        return "todo.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }
};