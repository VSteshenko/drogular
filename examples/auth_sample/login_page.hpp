#pragma once

#include <drogular/page.hpp>

#include <string>

class LoginPage final
    : public drogular::TemplatePage
{
public:
    std::string templatePath() const override {
        return "login.html";
    }

    std::string layoutPath() const override {
        return "layouts/main.html";
    }

    void onInit(drogular::RenderContext& context) override {
        context.set("pageTitle", std::string("Login"));
    }
};