#pragma once

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
};