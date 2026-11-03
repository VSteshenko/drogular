#pragma once

#include <drogular/url.hpp>

#include <string>
#include <string_view>

namespace drogular {

/**
 * Validates and propagates an application-local navigation destination.
 * A return URL is untrusted input, even if it came from a hidden form field.
 */
class NavigationContext final {
public:
    /** Rejects external, protocol-relative, malformed and ambiguous local URLs. */
    static bool isLocalTarget(std::string_view target) {
        if (target.empty() || target.front() != '/' ||
            (target.size() > 1 && target[1] == '/')) {
            return false;
        }
        const auto query = target.find('?');
        const auto path = target.substr(0, query);
        // A backslash can be treated as a slash by browsers. Percent-encoded
        // path separators and controls can also change routing after decoding.
        for (const unsigned char c : target) {
            if (c <= 0x20 || c == 0x7f || c == '\\' || c == '#') {
                return false;
            }
        }
        return path.find('%') == std::string_view::npos;
    }

    /**
     * Accepts only a base path and its query string; optionally accepts
     * descendants separated by '/'. Prefix lookalikes are never accepted.
     */
    static std::string resolve(
        std::string_view candidate,
        std::string_view fallback,
        std::string_view basePath = {},
        bool allowDescendants = false
    ) {
        const auto safeFallback = isLocalTarget(fallback)
            ? std::string(fallback)
            : std::string("/");
        if (!isLocalTarget(candidate)) {
            return safeFallback;
        }
        if (basePath.empty()) {
            return std::string(candidate);
        }
        if (!isLocalTarget(basePath) || basePath.find('?') != std::string_view::npos) {
            return safeFallback;
        }
        if (candidate == basePath ||
            (candidate.size() > basePath.size() &&
             candidate.substr(0, basePath.size()) == basePath &&
             (candidate[basePath.size()] == '?' ||
              (allowDescendants && candidate[basePath.size()] == '/')))) {
            return std::string(candidate);
        }
        return safeFallback;
    }

    /** Appends a URL-encoded returnUrl to a local destination. */
    static std::string propagate(
        std::string_view destination,
        std::string_view returnUrl
    ) {
        if (!isLocalTarget(destination)) {
            return "/";
        }
        std::string result(destination);
        result += destination.find('?') == std::string_view::npos ? '?' : '&';
        result += "returnUrl=";
        result += Url::encode(std::string(returnUrl));
        return result;
    }
};

} // namespace drogular