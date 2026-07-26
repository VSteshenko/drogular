#pragma once

#include "portal_table_schema.hpp"

#include "features/projects/data/portal_project.hpp"
#include "models/portal_project_type.hpp"
#include "features/roles/data/portal_role.hpp"
#include "features/users/data/portal_user.hpp"
#include "features/departments/data/portal_department.hpp"
#include "features/department_members/data/portal_department_member.hpp"

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

    static PortalTableSchema<PortalDepartment> departments() {
        auto schema =
            PortalTableSchema<PortalDepartment>::forModel(
                "departments"
            );

        schema
            .field("id", &PortalDepartment::id)
                .key()
                .labelKey("common.id");

        schema
            .field("name", &PortalDepartment::name)
                .required()
                .unique();

        schema
            .field("description", &PortalDepartment::description);

        schema
            .field("managerId", &PortalDepartment::managerId)
                .required()
                .reference("users", "id", "username");

        schema
            .field("isActive", &PortalDepartment::isActive)
                .required();

        return schema;
    }

    static PortalTableSchema<PortalDepartmentMember> departmentMembers() {
        auto schema =
            PortalTableSchema<PortalDepartmentMember>::forModel(
                "department_members"
            );

        schema
            .field("id", &PortalDepartmentMember::id)
                .key()
                .labelKey("common.id");

        schema
            .field("departmentId", &PortalDepartmentMember::departmentId)
                .required()
                .reference("departments", "id", "name");

        schema
            .field("userId", &PortalDepartmentMember::userId)
                .required()
                .reference("users", "id", "username");

        return schema;
    }

};