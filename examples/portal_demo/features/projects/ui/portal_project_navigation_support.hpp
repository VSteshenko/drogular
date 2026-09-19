#pragma once

#include <drogular/url.hpp>

#include <string>

class PortalProjectNavigationSupport final {
public:
    static std::string projectsReturnUrl(
        const std::string& candidate
    ) {
        if (candidate == "/projects" ||
            candidate.starts_with("/projects?")) {
            return candidate;
        }

        return "/projects";
    }

    static std::string detailsUrl(
        int projectId,
        const std::string& returnUrl
    ) {
        return "/projects/" + std::to_string(projectId) +
            "?returnUrl=" +
            drogular::Url::encode(projectsReturnUrl(returnUrl));
    }

    static std::string editUrl(
        int projectId,
        const std::string& returnUrl
    ) {
        return "/projects/" + std::to_string(projectId) +
            "/edit?returnUrl=" +
            drogular::Url::encode(projectsReturnUrl(returnUrl));
    }
};