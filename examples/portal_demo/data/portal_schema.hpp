#pragma once

#include "portal_table_schema.hpp"

#include "../portal_project.hpp"
#include "../portal_project_type.hpp"
#include "../portal_role.hpp"
#include "../portal_user.hpp"

class PortalSchema {
public:
    static PortalTableSchema<PortalUser> users() {
        auto schema =
            PortalTableSchema<PortalUser>::forModel(
                "users"
            );

        schema
            .field("id", &PortalUser::id)
                .key()
                .displayName("ID");

        schema
            .field("username", &PortalUser::username)
                .required()
                .unique()
                .displayName("Username");

        schema
            .field("password", &PortalUser::password)
                .required()
                .displayName("Password");

        schema
            .field("role", &PortalUser::role)
                .required()
                .reference("roles", "code", "title")
                .displayName("Role");

        return schema;
    }

    static PortalTableSchema<PortalRole> roles() {
        auto schema =
            PortalTableSchema<PortalRole>::forModel(
                "roles"
            );

        schema
            .field("id", &PortalRole::id)
                .key()
                .displayName("ID");

        schema
            .field("code", &PortalRole::code)
                .required()
                .unique()
                .displayName("Code");

        schema
            .field("title", &PortalRole::title)
                .required()
                .displayName("Title");

        return schema;
    }

    static PortalTableSchema<PortalProject> projects() {
        auto schema =
            PortalTableSchema<PortalProject>::forModel(
                "projects"
            );

        schema
            .field("id", &PortalProject::id)
                .key()
                .displayName("ID");

        schema
            .field("title", &PortalProject::title)
                .required()
                .displayName("Title");

        schema
            .field("status", &PortalProject::status)
                .required()
                .displayName("Status");

        schema
            .field("ownerId", &PortalProject::ownerId)
                .required()
                .reference("users", "id", "username")
                .displayName("Owner");

        schema
            .field("projectTypeId", &PortalProject::projectTypeId)
                .required()
                .reference("projectTypes", "id", "title")
                .displayName("Type");

        return schema;
    }

    static PortalTableSchema<PortalProjectType> projectTypes() {
        auto schema =
            PortalTableSchema<PortalProjectType>::forModel(
                "projectTypes"
            );

        schema
            .field("id", &PortalProjectType::id)
                .key()
                .displayName("ID");

        schema
            .field("code", &PortalProjectType::code)
                .required()
                .unique()
                .displayName("Code");

        schema
            .field("title", &PortalProjectType::title)
                .required()
                .displayName("Title");

        return schema;
    }
};