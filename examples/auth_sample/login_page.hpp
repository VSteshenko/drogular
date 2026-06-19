#pragma once

#include "auth_template_path.hpp"

#include <drogular/page.hpp>

#include <string>

class LoginPage final
    : public drogular::TemplatePage
{
public:
    std::string templatePath() const override {
        return authTemplatePath("login.html");
    }
};