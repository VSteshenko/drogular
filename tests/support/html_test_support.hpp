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
        const auto valueMarker =
            "value=\"" + value + "\"";

        std::size_t position = 0;

        while (true) {
            const auto optionStart =
                html.find("<option", position);

            if (optionStart == std::string::npos) {
                return false;
            }

            const auto optionEnd =
                html.find('>', optionStart);

            if (optionEnd == std::string::npos) {
                return false;
            }

            const auto optionTag =
                html.substr(
                    optionStart,
                    optionEnd - optionStart + 1
                );

            if (optionTag.find(valueMarker) !=
                std::string::npos) {
                return true;
                }

            position =
                optionEnd + 1;
        }
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
        const auto valueMarker =
            "value=\"" + value + "\"";

        std::size_t position = 0;

        while (true) {
            const auto optionStart =
                html.find("<option", position);

            if (optionStart == std::string::npos) {
                return false;
            }

            const auto optionEnd =
                html.find('>', optionStart);

            if (optionEnd == std::string::npos) {
                return false;
            }

            const auto optionTag =
                html.substr(
                    optionStart,
                    optionEnd - optionStart + 1
                );

            if (optionTag.find(valueMarker) !=
                std::string::npos) {
                return optionTag.find("selected") !=
                       std::string::npos;
                }

            position =
                optionEnd + 1;
        }
    }

    static std::string elementContent(
        const std::string& html,
        const std::string& marker,
        const std::string& closingTag
    ) {
        const auto markerPosition =
            html.find(marker);

        if (markerPosition == std::string::npos) {
            return {};
        }

        const auto elementStart =
            html.rfind('<', markerPosition);

        if (elementStart == std::string::npos) {
            return {};
        }

        const auto elementEnd =
            html.find(
                closingTag,
                markerPosition
            );

        if (elementEnd == std::string::npos) {
            return {};
        }

        return html.substr(
            elementStart,
            elementEnd + closingTag.size() -
                elementStart
        );
    }

    static bool optionSelectedInSelect(
        const std::string& html,
        const std::string& selectMarker,
        const std::string& value
    ) {
        const auto select =
            elementContent(
                html,
                selectMarker,
                "</select>"
            );

        if (select.empty()) {
            return false;
        }

        return optionSelected(
            select,
            value
        );
    }

    static std::string decodeEntities(
        std::string value
    ) {
        const auto replaceAll =
            [&value](
                const std::string& from,
                const std::string& to
            ) {
                std::size_t position = 0;

                while (
                    (position = value.find(
                        from,
                        position
                    )) != std::string::npos
                ) {
                    value.replace(
                        position,
                        from.size(),
                        to
                    );

                    position +=
                        to.size();
                }
        };

        replaceAll("&amp;", "&");
        replaceAll("&quot;", "\"");
        replaceAll("&#39;", "'");
        replaceAll("&lt;", "<");
        replaceAll("&gt;", ">");

        return value;
    }
};