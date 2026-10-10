#pragma once

#include "features/catalog/ui/catalog_query.hpp"

#include <drogular/request_parameters.hpp>

class MiniStoreCatalogQueryParser {
public:
    static MiniStoreCatalogQuery fromRequest(const drogon::HttpRequestPtr& request) {
        const drogular::RequestParameters parameters(request);

        MiniStoreCatalogQuery query;
        query.search = parameters.optionalString("q").value_or("");
        query.category = parameters.optionalString("category").value_or("");
        const auto sort = parameters.value("sort");
        query.sort = sort == "price-asc" || sort == "price-desc" ? sort : "name";

        return query;
    }
};