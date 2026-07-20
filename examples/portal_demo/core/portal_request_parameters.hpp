#pragma once

#include <drogon/HttpRequest.h>

#include <algorithm>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

class PortalRequestParameters {
public:
    explicit PortalRequestParameters(
        drogon::HttpRequestPtr request
    )
        : request_(std::move(request))
    {
    }

    std::string value(std::string_view name) const {
        if (!request_) {
            return {};
        }

        return request_->getParameter(std::string(name));
    }

    std::optional<std::string> optionalString(
        std::string_view name
    ) const {
        auto result = value(name);
        if (result.empty()) {
            return std::nullopt;
        }

        return result;
    }

    std::optional<int> positiveInteger(
        std::string_view name
    ) const {
        const auto text = value(name);
        if (text.empty()) {
            return std::nullopt;
        }

        int result = 0;
        const auto parsed = std::from_chars(
            text.data(),
            text.data() + text.size(),
            result
        );

        if (parsed.ec != std::errc{} ||
            parsed.ptr != text.data() + text.size() ||
            result <= 0) {
            return std::nullopt;
        }

        return result;
    }

    int positiveIntegerOr(
        std::string_view name,
        int fallback
    ) const {
        return positiveInteger(name).value_or(fallback);
    }

    int boundedPositiveIntegerOr(
        std::string_view name,
        int fallback,
        int maximum
    ) const {
        return std::min(
            positiveIntegerOr(name, fallback),
            maximum
        );
    }

private:
    drogon::HttpRequestPtr request_;
};