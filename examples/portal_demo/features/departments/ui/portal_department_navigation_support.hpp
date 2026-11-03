#pragma once

#include <drogular/navigation_context.hpp>

#include <string>

class PortalDepartmentNavigationSupport final {
public:
    static std::string departmentsReturnUrl(
        const std::string& candidate
    ) {
        return drogular::NavigationContext::resolve(
            candidate,
            "/departments",
            "/departments");
    }

    static std::string detailsUrl(int id, const std::string& returnUrl) {
        return drogular::NavigationContext::propagate(
            "/departments/" + std::to_string(id), departmentsReturnUrl(returnUrl));
    }

    static std::string editUrl(int id, const std::string& returnUrl) {
        return drogular::NavigationContext::propagate(
            "/departments/" + std::to_string(id) + "/edit", departmentsReturnUrl(returnUrl));
    }
};