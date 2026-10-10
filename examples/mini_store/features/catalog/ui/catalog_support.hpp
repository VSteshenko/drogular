#pragma once

#include "data/mini_store_store.hpp"
#include "features/catalog/ui/catalog_query.hpp"

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

class MiniStoreCatalogSupport {
public:
    static std::vector<MiniStoreProduct> filter(std::vector<MiniStoreProduct> products,
                                                const std::vector<MiniStoreCategory>& categories,
                                                const MiniStoreCatalogQuery& query,
                                                const std::string& locale) {
        auto categoryId = 0;
        for (const auto& category : categories) {
            if (category.slug == query.category) {
                categoryId = category.id;
            }
        }

        const auto needle = lower(query.search);
        std::erase_if(products, [&](const auto& product) {
            if (!product.active) {
                return true;
            }
            if (categoryId != 0 && product.categoryId != categoryId) {
                return true;
            }
            if (!needle.empty()) {
                const auto haystack =
                    lower(std::string(product.name.resolve(locale)) + " " +
                        std::string(product.description.resolve(locale)) + " " + product.sku);

                if (haystack.find(needle) == std::string::npos) {
                    return true;
                }
            }

            return false;
        });

        std::sort(products.begin(), products.end(), [&](const auto& a, const auto& b) {
            if (query.sort == "price-asc") {
                return a.priceCents < b.priceCents;
            }
            if (query.sort == "price-desc") {
                return a.priceCents > b.priceCents;
            }

            return lower(std::string(a.name.resolve(locale))) < lower(std::string(b.name.resolve(locale)));
        });

        return products;
    }

    static std::string catalogUrl(const MiniStoreCatalogQuery& query) {
        std::vector<std::string> parts;
        if (!query.search.empty()) {
            parts.push_back("q=" + encode(query.search));
        }
        if (!query.category.empty()) {
            parts.push_back("category=" + encode(query.category));
        }
        if (query.sort != "name") {
            parts.push_back("sort=" + encode(query.sort));
        }

        std::string url = "/catalog";
        for (std::size_t i = 0; i < parts.size(); ++i) {
            url += (i == 0 ? "?" : "&") + parts[i];
        }

        return url;
    }

    static std::string productUrl(int id, const MiniStoreCatalogQuery& query) {
        return "/products/" + std::to_string(id) + "?returnUrl=" + encode(catalogUrl(query));
    }

    static std::string safeCatalogReturnUrl(const std::string& value) {
        return value.rfind("/catalog", 0) == 0 && value.rfind("//", 0) != 0
            ? value
            : std::string("/catalog");
    }

    static std::string money(std::int64_t cents) {
        std::ostringstream out;
        out << "€" << (cents / 100) << '.' << std::setw(2) << std::setfill('0') << (cents % 100);
        return out.str();
    }

private:
    static std::string encode(const std::string& value) {
        std::ostringstream out;
        out << std::uppercase << std::hex;
        for (const unsigned char c : value) {
            if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                out << static_cast<char>(c);
            }
            else {
                out << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            }
        }

        return out.str();
    }

    static std::string lower(std::string value) {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
        return value;
    }
};