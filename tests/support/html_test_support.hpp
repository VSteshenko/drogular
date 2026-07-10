#pragma once

#include <optional>
#include <string>
#include <string_view>

class HtmlTestSupport {
public:
    static std::string openingTag(
        const std::string& html,
        const std::string& marker
    ) {
        const auto markerPosition =
            html.find(marker);

        if (markerPosition == std::string::npos) {
            return {};
        }

        const auto tagStart =
            html.rfind('<', markerPosition);

        if (tagStart == std::string::npos) {
            return {};
        }

        const auto tagEnd =
            html.find('>', markerPosition);

        if (tagEnd == std::string::npos) {
            return {};
        }

        return html.substr(
            tagStart,
            tagEnd - tagStart + 1
        );
    }

    static bool hasAttribute(
        const std::string& html,
        const std::string& marker,
        const std::string& attribute
    ) {
        const auto tag =
            openingTag(html, marker);

        if (tag.empty()) {
            return false;
        }

        return tag.find(attribute) !=
               std::string::npos;
    }

    static std::optional<std::string> attributeValue(
        const std::string& html,
        const std::string& marker,
        const std::string& attribute
    ) {
        const auto tag =
            openingTag(html, marker);

        if (tag.empty()) {
            return std::nullopt;
        }

        const auto prefix =
            attribute + "=\"";

        const auto valueStart =
            tag.find(prefix);

        if (valueStart == std::string::npos) {
            return std::nullopt;
        }

        const auto contentStart =
            valueStart + prefix.size();

        const auto valueEnd =
            tag.find('"', contentStart);

        if (valueEnd == std::string::npos) {
            return std::nullopt;
        }

        return tag.substr(
            contentStart,
            valueEnd - contentStart
        );
    }

    static bool containsOption(
        const std::string& html,
        const std::string& value
    ) {
        return html.find(
            "value=\"" + value + "\""
        ) != std::string::npos;
    }

    static bool containsText(
        const std::string& html,
        std::string_view text
    ) {
        return html.find(text) !=
               std::string::npos;
    }

    static bool optionSelected(
        const std::string& html,
        const std::string& value
    ) {
        const auto option =
            openingTag(
                html,
                "value=\"" + value + "\""
            );

        if (option.empty()) {
            return false;
        }

        return option.find("selected") !=
               std::string::npos;
    }
};