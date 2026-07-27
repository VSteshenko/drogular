#pragma once

#include <json/json.h>

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace drogular {

struct PaginationLink {
    int number{1};
    std::string url;
    bool current{false};
};

struct PaginationModel {
    bool visible{false};
    bool hasPrevious{false};
    bool hasNext{false};
    std::string previousUrl;
    std::string nextUrl;
    std::vector<PaginationLink> pages;
};

template<typename PageUrlBuilder>
PaginationModel makePaginationModel(
    int currentPage,
    int totalPages,
    PageUrlBuilder&& pageUrl
) {
    PaginationModel model;

    totalPages = std::max(1, totalPages);
    currentPage = std::clamp(currentPage, 1, totalPages);

    model.visible = totalPages > 1;
    model.hasPrevious = currentPage > 1;
    model.hasNext = currentPage < totalPages;

    if (model.hasPrevious) {
        model.previousUrl = pageUrl(currentPage - 1);
    }

    if (model.hasNext) {
        model.nextUrl = pageUrl(currentPage + 1);
    }

    model.pages.reserve(static_cast<std::size_t>(totalPages));

    for (int page = 1; page <= totalPages; ++page) {
        model.pages.push_back({
            page,
            pageUrl(page),
            page == currentPage
        });
    }

    return model;
}

inline Json::Value toJson(const PaginationLink& link) {
    Json::Value json(Json::objectValue);

    json["number"] = link.number;
    json["url"] = link.url;
    json["current"] = link.current;

    return json;
}

inline Json::Value toJson(const PaginationModel& model) {
    Json::Value json(Json::objectValue);
    Json::Value pages(Json::arrayValue);

    for (const auto& page : model.pages) {
        pages.append(toJson(page));
    }

    json["visible"] = model.visible;
    json["hasPrevious"] = model.hasPrevious;
    json["hasNext"] = model.hasNext;
    json["previousUrl"] = model.previousUrl;
    json["nextUrl"] = model.nextUrl;
    json["pages"] = std::move(pages);

    return json;
}

} // namespace drogular