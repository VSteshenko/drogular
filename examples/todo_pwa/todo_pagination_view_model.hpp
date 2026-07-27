#pragma once

#include <json/json.h>

#include <functional>
#include <string>

inline Json::Value makeTodoPaginationViewModel(
    int currentPage,
    int totalPages,
    const std::function<std::string(int)>& pageUrl
) {
    Json::Value model(Json::objectValue);
    Json::Value pages(Json::arrayValue);

    model["visible"] = totalPages > 1;
    model["hasPrevious"] = currentPage > 1;
    model["hasNext"] = currentPage < totalPages;
    model["previousUrl"] = currentPage > 1 ? pageUrl(currentPage - 1) : "";
    model["nextUrl"] = currentPage < totalPages ? pageUrl(currentPage + 1) : "";

    for (int page = 1; page <= totalPages; ++page) {
        Json::Value item(Json::objectValue);
        item["number"] = page;
        item["url"] = pageUrl(page);
        item["current"] = page == currentPage;
        pages.append(std::move(item));
    }

    model["pages"] = std::move(pages);
    return model;
}