#pragma once

#include "portal_dataset.hpp"
#include "portal_dataset_schema_validator.hpp"

#include <stdexcept>

class DemoDataset {
public:
    static PortalDataset create() {
        PortalDataset dataset;

        dataset
            .addRole({
                .id = 1,
                .code = "admin",
                .title = "Administrator"
            })
            .addRole({
                .id = 2,
                .code = "user",
                .title = "User"
            })
            .addUser({
                .id = 1,
                .username = "admin",
                .password = "admin",
                .role = "admin"
            })
            .addUser({
                .id = 2,
                .username = "user",
                .password = "user",
                .role = "user"
            })
            .addUser({
                .id = 3,
                .username = "alice",
                .password = "alice",
                .role = "user"
            })
            .addUser({
                .id = 4,
                .username = "bob",
                .password = "bob",
                .role = "user"
            })
            .addUser({
                .id = 5,
                .username = "charlie",
                .password = "charlie",
                .role = "user"
            })
            .addUser({
                .id = 6,
                .username = "diana",
                .password = "diana",
                .role = "user"
            })
            .addUser({
                .id = 7,
                .username = "eric",
                .password = "eric",
                .role = "user"
            })
            .addUser({
                .id = 8,
                .username = "frank",
                .password = "frank",
                .role = "user"
            })
            .addProjectType({
                .id = 1,
                .code = "customer",
                .title = "Customer Project"
            })
            .addProjectType({
                .id = 2,
                .code = "internal",
                .title = "Internal Project"
            })
            .addProjectType({
                .id = 3,
                .code = "mobile",
                .title = "Mobile Application"
            })
            .addProjectType({
                .id = 4,
                .code = "service",
                .title = "Backend Service"
            })
            .addProjectType({
                .id = 5,
                .code = "library",
                .title = "Shared Library"
            })
            .addProject({
                .id = 1,
                .title = "Customer Portal",
                .status = "active",
                .ownerId = 1,
                .projectTypeId = 1
            })
            .addProject({
                .id = 2,
                .title = "Internal Dashboard",
                .status = "paused",
                .ownerId = 2,
                .projectTypeId = 2
            })
            .addProject({
                .id = 3,
                .title = "Analytics Platform",
                .status = "done",
                .ownerId = 3,
                .projectTypeId = 4
            })
            .addProject({
                .id = 4,
                .title = "Inventory Manager",
                .status = "active",
                .ownerId = 4,
                .projectTypeId = 2
            })
            .addProject({
                .id = 5,
                .title = "Billing Service",
                .status = "paused",
                .ownerId = 5,
                .projectTypeId = 4
            })
            .addProject({
                .id = 6,
                .title = "Mobile Store",
                .status = "active",
                .ownerId = 6,
                .projectTypeId = 3
            })
            .addProject({
                .id = 7,
                .title = "Monitoring Console",
                .status = "done",
                .ownerId = 7,
                .projectTypeId = 2
            })
            .addProject({
                .id = 8,
                .title = "Team Messenger",
                .status = "paused",
                .ownerId = 8,
                .projectTypeId = 3
            })
            .addProject({
                .id = 9,
                .title = "Task Scheduler",
                .status = "active",
                .ownerId = 3,
                .projectTypeId = 4
            })
            .addProject({
                .id = 10,
                .title = "Reporting Suite",
                .status = "done",
                .ownerId = 4,
                .projectTypeId = 1
            })
            .addProject({
                .id = 11,
                .title = "Customer Portal API",
                .status = "active",
                .ownerId = 5,
                .projectTypeId = 4
            })
            .addProject({
                .id = 12,
                .title = "Customer Portal Mobile",
                .status = "paused",
                .ownerId = 6,
                .projectTypeId = 3
            })
            .addProject({
                .id = 13,
                .title = "Authentication Library",
                .status = "done",
                .ownerId = 7,
                .projectTypeId = 5
            })
            .addProject({
                .id = 14,
                .title = "Document Archive",
                .status = "active",
                .ownerId = 8,
                .projectTypeId = 1
            })
            .addProject({
                .id = 15,
                .title = "Notification Service",
                .status = "paused",
                .ownerId = 1,
                .projectTypeId = 4
            })
            .addProject({
                .id = 16,
                .title = "UI Component Library",
                .status = "active",
                .ownerId = 2,
                .projectTypeId = 5
            })
            .addProject({
                .id = 17,
                .title = "Customer Portal",
                .status = "done",
                .ownerId = 3,
                .projectTypeId = 1
            })
            .addProject({
                .id = 18,
                .title = "Release Automation",
                .status = "paused",
                .ownerId = 4,
                .projectTypeId = 2
            })
            .addProject({
                .id = 19,
                .title = "Search Service",
                .status = "active",
                .ownerId = 5,
                .projectTypeId = 4
            })
            .addProject({
                .id = 20,
                .title = "Developer Portal",
                .status = "done",
                .ownerId = 6,
                .projectTypeId = 1
            })
            .addDepartment({1, "Engineering", "Product and platform engineering", 1, true})
            .addDepartment({2, "Operations", "Internal operations and delivery", 2, true})
            .addDepartment({3, "Archive", "Inactive historical department", 1, false})
            .addDepartmentMember({1, 1, 1})
            .addDepartmentMember({2, 1, 2})
            .addDepartmentMember({3, 2, 2})
            .addDepartmentMember({4, 2, 3});

        const auto validation =
            PortalDatasetSchemaValidator::validate(dataset);

        if (!validation.valid()) {
            throw std::runtime_error(
                validation.errors().front()
            );
        }

        return dataset;
    }
};