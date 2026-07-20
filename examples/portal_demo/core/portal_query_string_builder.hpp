#pragma once

#include <drogular/url.hpp>

#include <string>
#include <string_view>
#include <optional>

class PortalQueryStringBuilder {
public:
    PortalQueryStringBuilder& add(
        std::string_view name,
        std::string_view value
    ) {
        result_ += result_.empty() ? '?' : '&';
        result_ += name;
        result_ += '=';
        result_ += drogular::Url::encode(std::string(value));

        return *this;
    }

    PortalQueryStringBuilder& add(
        std::string_view name,
        int value
    ) {
        return add(name, std::to_string(value));
    }

    PortalQueryStringBuilder& addIf(
        bool condition,
        std::string_view name,
        std::string_view value
    ) {
        if (condition) {
            add(name, value);
        }

        return *this;
    }

    PortalQueryStringBuilder& addIf(
        bool condition,
        std::string_view name,
        int value
    ) {
        if (condition) {
            add(name, value);
        }

        return *this;
    }

    const std::string& build() const noexcept {
        return result_;
    }

private:
    std::string result_;
};