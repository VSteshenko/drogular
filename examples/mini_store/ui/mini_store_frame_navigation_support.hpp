#pragma once

#include <string>

class MiniStoreFrameNavigationSupport final {
public:
    static std::string storefrontFragmentUrl(const std::string& url) {
        if (url.empty() || url.front() != '/') {
            return url;
        }
        return "/fragments/storefront" + url;
    }

    static std::string staffFragmentUrl(const std::string& url) {
        if (url.empty() || url.front() != '/') {
            return url;
        }
        if (url == "/staff") {
            return "/fragments/staff";
        }
        return "/fragments" + url;
    }

    static std::string staffOrderFragmentUrl(const std::string& url) {
        if (url.empty() || url.front() != '/') {
            return url;
        }
        return "/fragments" + url;
    }
};