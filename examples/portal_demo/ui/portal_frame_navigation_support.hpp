#pragma once

#include <string>

class PortalFrameNavigationSupport final {
public:
    static std::string fragmentUrl(const std::string& url) {
        if (url.empty() || url.front() != '/') {
            return url;
        }

        return "/fragments/pages" + url;
    }
};