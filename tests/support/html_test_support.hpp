#pragma once

#include <initializer_list>
#include <optional>
#include <utility>
#include <string>
#include <string_view>

class HtmlTestSupport {
public:
    using Attribute =
        std::pair<std::string_view, std::string_view>;

    static bool elementHasAttributes(
        const std::string& html,
        std::string_view tagName,
        std::initializer_list<Attribute> attributes
    ) {
        const std::string tagPrefix =
            "<" + std::string(tagName);

        std::size_t position = 0;

        while (true) {
            const auto tagStart =
                html.find(tagPrefix, position);

            if (tagStart == std::string::npos) {
                return false;
            }

            const auto nameEnd =
                tagStart + tagPrefix.size();

            if (
                nameEnd < html.size() &&
                html[nameEnd] != ' ' &&
                html[nameEnd] != '\t' &&
                html[nameEnd] != '\n' &&
                html[nameEnd] != '\r' &&
                html[nameEnd] != '>' &&
                html[nameEnd] != '/'
            ) {
                position = nameEnd;
                continue;
            }

            const auto tagEnd =
                html.find('>', nameEnd);

            if (tagEnd == std::string::npos) {
                return false;
            }

            const std::string_view tag(
                html.data() + tagStart,
                tagEnd - tagStart + 1
            );

            bool matches = true;

            for (const auto& [name, expectedValue] : attributes) {
                if (!tagHasAttribute(tag, name, expectedValue)) {
                    matches = false;
                    break;
                }
            }

            if (matches) {
                return true;
            }

            position = tagEnd + 1;
        }
    }

    static bool elementContainsElementWithAttributes(
        const std::string& html,
        std::string_view parentTagName,
        std::initializer_list<Attribute> parentAttributes,
        std::string_view childTagName,
        std::initializer_list<Attribute> childAttributes
    ) {
        const std::string parentPrefix =
            "<" + std::string(parentTagName);

        std::size_t position = 0;

        while (true) {
            const auto parentStart =
                html.find(parentPrefix, position);

            if (parentStart == std::string::npos) {
                return false;
            }

            const auto parentOpenEnd =
                html.find('>', parentStart);

            if (parentOpenEnd == std::string::npos) {
                return false;
            }

            const std::string parentOpeningTag =
                html.substr(
                    parentStart,
                    parentOpenEnd - parentStart + 1
                );

            if (
                !elementHasAttributes(
                    parentOpeningTag,
                    parentTagName,
                    parentAttributes
                )
            ) {
                position = parentOpenEnd + 1;
                continue;
            }

            const std::string closingTag =
                "</" + std::string(parentTagName) + ">";

            const auto parentEnd =
                html.find(closingTag, parentOpenEnd + 1);

            if (parentEnd == std::string::npos) {
                return false;
            }

            const auto contentStart =
                parentOpenEnd + 1;

            const auto content =
                html.substr(
                    contentStart,
                    parentEnd - contentStart
                );

            if (
                elementHasAttributes(
                    content,
                    childTagName,
                    childAttributes
                )
            ) {
                return true;
            }

            position =
                parentEnd + closingTag.size();
        }
    }

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

private:
    static bool tagHasAttribute(
        std::string_view tag,
        std::string_view name,
        std::string_view expectedValue
    ) {
        std::size_t position = 0;

        while (true) {
            position = tag.find(name, position);

            if (position == std::string_view::npos) {
                return false;
            }

            const bool validLeftBoundary =
                position > 0 &&
                (
                    tag[position - 1] == ' ' ||
                    tag[position - 1] == '\t' ||
                    tag[position - 1] == '\n' ||
                    tag[position - 1] == '\r'
                );

            const auto afterName =
                position + name.size();

            if (!validLeftBoundary || afterName >= tag.size()) {
                position = afterName;
                continue;
            }

            if (expectedValue.empty()) {
                const auto next = tag[afterName];

                if (
                    next == ' ' ||
                    next == '\t' ||
                    next == '\n' ||
                    next == '\r' ||
                    next == '>' ||
                    next == '/' ||
                    tag.substr(afterName, 3) == "=\"\""
                ) {
                    return true;
                }
            } else if (
                tag.substr(afterName, 2) == "=\"" &&
                tag.substr(
                    afterName + 2,
                    expectedValue.size()
                ) == expectedValue
            ) {
                const auto quotePosition =
                    afterName + 2 + expectedValue.size();

                if (
                    quotePosition < tag.size() &&
                    tag[quotePosition] == '\"'
                ) {
                    return true;
                }
            }

            position = afterName;
        }
    }
};