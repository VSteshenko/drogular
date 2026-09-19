#pragma once

#include <drogular/url.hpp>

#include <string>

class PortalDepartmentNavigationSupport final {
public:
    static std::string departmentsReturnUrl(
        const std::string& candidate
    ) {
        if (candidate == "/departments" ||
            candidate.starts_with("/departments?")) {
            return candidate;
        }

        return "/departments";
    }

    static std::string detailsUrl(int id, const std::string& returnUrl) {
        return "/departments/" + std::to_string(id) + "?returnUrl=" +
            drogular::Url::encode(departmentsReturnUrl(returnUrl));
    }

    static std::string editUrl(int id, const std::string& returnUrl) {
        return "/departments/" + std::to_string(id) + "/edit?returnUrl=" +
            drogular::Url::encode(departmentsReturnUrl(returnUrl));
    }
};