#pragma once

#include "portal_table_schema.hpp"

#include "../portal_project.hpp"
#include "../portal_project_type.hpp"
#include "../portal_role.hpp"
#include "../portal_user.hpp"

class PortalSchema {
public:
    static PortalTableSchema<PortalUser> users() {
        return PortalTableSchema<PortalUser>::forModel("users")
            .key("id", &PortalUser::id)
            .unique("username", &PortalUser::username)
            .required("password", &PortalUser::password)
            .reference("role", &PortalUser::role, "roles", "code");
    }

    static PortalTableSchema<PortalRole> roles() {
        return PortalTableSchema<PortalRole>::forModel("roles")
            .key("id", &PortalRole::id)
            .unique("code", &PortalRole::code)
            .required("title", &PortalRole::title);
    }

    static PortalTableSchema<PortalProject> projects() {
        return PortalTableSchema<PortalProject>::forModel("projects")
            .key("id", &PortalProject::id)
            .required("title", &PortalProject::title)
            .required("status", &PortalProject::status)
            .reference("ownerId", &PortalProject::ownerId, "users", "id")
            .reference("projectTypeId", &PortalProject::projectTypeId, "projectTypes", "id");
    }

    static PortalTableSchema<PortalProjectType> projectTypes() {
        return PortalTableSchema<PortalProjectType>::forModel("projectTypes")
            .key("id", &PortalProjectType::id)
            .unique("code", &PortalProjectType::code)
            .required("title", &PortalProjectType::title);
    }
};