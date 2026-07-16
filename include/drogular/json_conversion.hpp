#pragma once

#include <json/json.h>

#include <concepts>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace drogular {

inline Json::Value toJsonValue(
    const Json::Value& value
) {
    return value;
}

inline Json::Value toJsonValue(
    Json::Value&& value
) {
    return std::move(value);
}

inline Json::Value toJsonValue(
    const std::string& value
) {
    return Json::Value(value);
}

inline Json::Value toJsonValue(
    std::string_view value
) {
    return Json::Value(
        std::string(value)
    );
}

inline Json::Value toJsonValue(
    const char* value
) {
    return Json::Value(
        value != nullptr
            ? value
            : ""
    );
}

inline Json::Value toJsonValue(
    bool value
) {
    return Json::Value(value);
}

template <std::integral T>
requires (!std::same_as<T, bool>)
Json::Value toJsonValue(
    T value
) {
    if constexpr (std::is_signed_v<T>) {
        return Json::Value(
            static_cast<Json::Int64>(value)
        );
    } else {
        return Json::Value(
            static_cast<Json::UInt64>(value)
        );
    }
}

template <std::floating_point T>
Json::Value toJsonValue(
    T value
) {
    return Json::Value(
        static_cast<double>(value)
    );
}

template <typename T>
Json::Value toJsonValue(
    const std::optional<T>& value
) {
    if (!value.has_value()) {
        return Json::Value();
    }

    return toJsonValue(*value);
}

template <typename T>
Json::Value toJsonValue(
    const std::vector<T>& values
) {
    Json::Value result(Json::arrayValue);

    for (const auto& value : values) {
        result.append(
            toJsonValue(value)
        );
    }

    return result;
}

namespace detail {

template <typename T>
concept HasAdlToJson =
    requires(const T& value) {
    {
        toJson(value)
    } -> std::same_as<Json::Value>;
    };

} // namespace detail

template <typename T>
requires detail::HasAdlToJson<T>
Json::Value toJsonValue(
    const T& value
) {
    return toJson(value);
}

} // namespace drogular