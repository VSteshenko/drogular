#pragma once

#include <drogular/page.hpp>

class TodoPage final : public drogular::Page {
public:
    void onInit(drogular::RenderContext& context) override;

    std::optional<drogular::gql::Query> query() const override;

    std::string render(drogular::RenderContext& context) override;
};
