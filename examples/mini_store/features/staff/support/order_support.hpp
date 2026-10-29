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

    static std::string displayTimestamp(const std::string& value) {
        // Keep the persisted ISO-8601 value untouched; only make the staff UI easier to scan.
        if (value.size() >= 20 && value[4] == '-' && value[7] == '-' && value[10] == 'T') {
            return value.substr(0, 10) + " · " + value.substr(11, 5) + " UTC";
        }
        return value;
    }

    static std::string orderUrl(int id, const std::string& returnUrl) {
        return "/staff/orders/" + std::to_string(id) + "?returnUrl=" + encode(returnUrl);
    }

    static std::string safeOrderReturnUrl(const std::string& value) {
        return value.rfind("/staff/orders", 0) == 0 && value.rfind("//", 0) != 0
            ? value
            : std::string("/staff/orders");
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
};