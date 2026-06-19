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
};