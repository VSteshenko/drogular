#pragma once

#include "domain/store_models.hpp"
#include "features/catalog/ui/catalog_support.hpp"

#include <string>

class MiniStoreOrderSupport {
public:
    static std::string statusName(MiniStoreOrderStatus status) {
        switch (status) {
            case MiniStoreOrderStatus::New:
                return "New";
            case MiniStoreOrderStatus::Processing:
                return "Processing";
            case MiniStoreOrderStatus::Completed:
                return "Completed";
        }
        return "New";
    }

    static std::string statusKey(MiniStoreOrderStatus status) {
        switch (status) {
            case MiniStoreOrderStatus::New:
                return "new";
            case MiniStoreOrderStatus::Processing:
                return "processing";
            case MiniStoreOrderStatus::Completed:
                return "completed";
        }
        return "new";
    }

    static bool matches(MiniStoreOrderStatus status, const std::string& filter) {
        return filter.empty() || filter == "all" || statusKey(status) == filter;
    }

    static std::string money(std::int64_t cents) {
        return MiniStoreCatalogSupport::money(cents);
    }
};