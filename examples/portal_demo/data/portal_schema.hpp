#pragma once

#include "portal_table_schema.hpp"

#include "models/portal_project.hpp"
#include "models/portal_project_type.hpp"
#include "models/portal_role.hpp"
#include "models/portal_user.hpp"

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
                .labelKey("common.id");

        schema
            .field("username", &PortalUser::username)
                .required()
                .unique();

        schema
            .field("password", &PortalUser::password)
                .required();

        schema
            .field("role", &PortalUser::role)
                .required()
                .reference("roles", "code", "title");

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
                .labelKey("common.id");

        schema
            .field("code", &PortalRole::code)
                .required()
                .unique();

        schema
            .field("title", &PortalRole::title)
                .required();

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
                .labelKey("common.id");

        schema
            .field("title", &PortalProject::title)
                .required()
                .labelKey("projects.title.label");

        schema
            .field("status", &PortalProject::status)
                .required()
                .labelKey("projects.status.label");

        schema
            .field("ownerId", &PortalProject::ownerId)
                .required()
                .reference("users", "id", "username")
                .labelKey("projects.owner");

        schema
            .field("projectTypeId", &PortalProject::projectTypeId)
                .required()
                .reference("projectTypes", "id", "title")
                .labelKey("projects.type");

        return schema;
    }

    static PortalTableSchema<PortalProjectType> projectTypes() {
        auto schema =
            PortalTableSchema<PortalProjectType>::forModel(
                "project_types"
            );

        schema
            .field("id", &PortalProjectType::id)
                .key()
                .labelKey("common.id");

        schema
            .field("code", &PortalProjectType::code)
                .required()
                .unique();

        schema
            .field("title", &PortalProjectType::title)
                .required();

        return schema;
    }
};