#pragma once

#include <drogular/page.hpp>
#include <drogular/render_context.hpp>

class DeveloperToolsExamplePage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return R"HTML(<!doctype html>
<html lang="en">
<head><meta charset="utf-8"><title>Drogular Developer Tools Example</title></head>
<body>
  <h1>Drogular Developer Tools Example</h1>
  <p><a href="/__drogular">Open Developer Tools</a></p>
</body>
</html>)HTML";
    }
};