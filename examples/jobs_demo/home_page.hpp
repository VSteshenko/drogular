#pragma once

#include <drogular/page.hpp>

class JobsDemoHomePage final : public drogular::TemplatePage {
public:
    std::string templatePath() const override {
        return "index.html";
    }
};