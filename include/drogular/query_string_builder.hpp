#pragma once

#include <drogular/url.hpp>

#include <string>
#include <string_view>

namespace drogular {

class QueryStringBuilder {
public:
    QueryStringBuilder& add(
        std::string_view name,
        std::string_view value
    ) {
        result_ += result_.empty() ? '?' : '&';
        result_ += name;
        result_ += '=';
        result_ += Url::encode(std::string(value));
        return *this;
    }

    QueryStringBuilder& add(
        std::string_view name,
        int value
    ) {
        return add(name, std::to_string(value));
    }

    QueryStringBuilder& addIf(
        bool condition,
        std::string_view name,
        std::string_view value
    ) {
        if (condition) {
            add(name, value);
        }
        return *this;
    }

    QueryStringBuilder& addIf(
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

    bool empty() const noexcept {
        return result_.empty();
    }

private:
    std::string result_;
};

} // namespace drogular