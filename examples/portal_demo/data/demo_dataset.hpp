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
            });

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