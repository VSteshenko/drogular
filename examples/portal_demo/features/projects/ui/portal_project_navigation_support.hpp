#pragma once

#include <drogular/navigation_context.hpp>

#include <string>

class PortalProjectNavigationSupport final {
public:
    static std::string projectsReturnUrl(
        const std::string& candidate
    ) {
        return drogular::NavigationContext::resolve(candidate, "/projects", "/projects");
    }

    static std::string detailsUrl(
        int projectId,
        const std::string& returnUrl
    ) {
        return drogular::NavigationContext::propagate(
            "/projects/" + std::to_string(projectId), projectsReturnUrl(returnUrl));
    }

    static std::string editUrl(
        int projectId,
        const std::string& returnUrl
    ) {
        return drogular::NavigationContext::propagate(
            "/projects/" + std::to_string(projectId) + "/edit", projectsReturnUrl(returnUrl));
    }
};