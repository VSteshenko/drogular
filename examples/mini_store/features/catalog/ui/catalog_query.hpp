#pragma once

#include <string>

struct MiniStoreCatalogQuery {
    std::string search;
    std::string category;
    std::string sort = "name";
};