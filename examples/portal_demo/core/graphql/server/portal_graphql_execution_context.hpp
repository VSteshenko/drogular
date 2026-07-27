#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

class PortalGraphQLExecutionContext {
public:
    PortalGraphQLExecutionContext& set(
        std::string name,
        std::string value
    ) {
        values_.insert_or_assign(
            std::move(name),
            std::move(value)
        );
        return *this;
    }

    std::optional<std::string> value(
        const std::string& name
    ) const {
        const auto iterator = values_.find(name);
        if (iterator == values_.end()) {
            return std::nullopt;
        }
        return iterator->second;
    }

private:
    std::unordered_map<std::string, std::string> values_;
};