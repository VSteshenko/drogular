#pragma once

#include "../portal_project.hpp"
#include "../portal_user.hpp"
#include "../portal_role.hpp"
#include "portal_dataset_validation_result.hpp"

#include <set>
#include <string>
#include <utility>
#include <vector>

class PortalDataset {
public:
    PortalDataset& addRole(
        PortalRole role
    ) {
        roles_.push_back(std::move(role));
        return *this;
    }

    std::vector<PortalRole>& roles() {
        return roles_;
    }

    const std::vector<PortalRole>& roles() const {
        return roles_;
    }

    PortalDataset& addUser(
        PortalUser user
    ) {
        users_.push_back(
            std::move(user)
        );

        return *this;
    }

    PortalDataset& addProject(
        PortalProject project
    ) {
        projects_.push_back(
            std::move(project)
        );

        return *this;
    }

    const std::vector<PortalUser>& users() const {
        return users_;
    }

    std::vector<PortalUser>& users()
    {
        return users_;
    }

    const std::vector<PortalProject>& projects() const {
        return projects_;
    }

    std::vector<PortalProject>& projects() {
        return projects_;
    }

    PortalDatasetValidationResult validate() const
    {
        PortalDatasetValidationResult result;

        validateUserIds(result);
        validateUsernames(result);
        validateRoleIds(result);
        validateRoleCodes(result);
        validateProjectIds(result);
        validateProjectOwners(result);

        return result;
    }

private:
    std::vector<PortalRole> roles_;
    std::vector<PortalUser> users_;
    std::vector<PortalProject> projects_;

    void validateUserIds(
        PortalDatasetValidationResult& result
    ) const {
        std::set<int> ids;

        for (const auto& user : users_) {
            if (user.id <= 0) {
                result.addError("User id must be positive");
                continue;
            }

            if (!ids.insert(user.id).second) {
                result.addError("Duplicate user id: " + std::to_string(user.id));
            }
        }
    }

    void validateUsernames(
        PortalDatasetValidationResult& result
    ) const {
        std::set<std::string> usernames;

        for (const auto& user : users_) {
            if (user.username.empty()) {
                result.addError("User username must not be empty");
                continue;
            }

            if (!usernames.insert(user.username).second) {
                result.addError("Duplicate username: " + user.username);
            }
        }
    }

    void validateRoleIds(
        PortalDatasetValidationResult& result
    ) const {
        std::set<int> ids;

        for (const auto& role : roles_) {
            if (role.id <= 0) {
                result.addError("Role id must be positive");
                continue;
            }

            if (!ids.insert(role.id).second) {
                result.addError("Duplicate role id: " + std::to_string(role.id));
            }
        }
    }

    void validateRoleCodes(
        PortalDatasetValidationResult& result
    ) const {
        std::set<std::string> codes;

        for (const auto& role : roles_) {
            if (role.code.empty()) {
                result.addError("Role code must not be empty");
                continue;
            }

            if (!codes.insert(role.code).second) {
                result.addError("Duplicate role code: " + role.code);
            }
        }
    }

    void validateProjectIds(
        PortalDatasetValidationResult& result
    ) const {
        std::set<int> ids;

        for (const auto& project : projects_) {
            if (project.id <= 0) {
                result.addError("Project id must be positive");
                continue;
            }

            if (!ids.insert(project.id).second) {
                result.addError("Duplicate project id: " + std::to_string(project.id));
            }
        }
    }

    void validateProjectOwners(
        PortalDatasetValidationResult& result
    ) const {
        std::set<int> userIds;

        for (const auto& user : users_) {
            userIds.insert(user.id);
        }

        for (const auto& project : projects_) {
            if (project.ownerId <= 0) {
                result.addError("Project ownerId must be positive");
                continue;
            }

            if (!userIds.contains(project.ownerId)) {
                result.addError(
                    "Project owner does not exist: " +
                    std::to_string(project.ownerId)
                );
            }
        }
    }
};