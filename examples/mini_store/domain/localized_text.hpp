#pragma once

#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

class MiniStoreLocalizedText {
public:
    using Values = std::unordered_map<std::string, std::string>;

    MiniStoreLocalizedText() = default;

    MiniStoreLocalizedText(std::initializer_list<Values::value_type> values)
        : values_(values) {}

    [[nodiscard]] std::string_view resolve(
        std::string_view locale,
        std::string_view fallbackLocale = "en") const noexcept {
        if (const auto* value = find(locale)) {
            return *value;
        }
        if (locale != fallbackLocale) {
            if (const auto* value = find(fallbackLocale)) {
                return *value;
            }
        }
        return {};
    }

    [[nodiscard]] bool contains(std::string_view locale) const noexcept {
        return find(locale) != nullptr;
    }

    [[nodiscard]] const Values& values() const noexcept {
        return values_;
    }

private:
    [[nodiscard]] const std::string* find(std::string_view locale) const noexcept {
        const auto it = values_.find(std::string(locale));
        return it == values_.end() ? nullptr : &it->second;
    }

    Values values_;
};