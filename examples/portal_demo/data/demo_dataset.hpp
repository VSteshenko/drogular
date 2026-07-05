#pragma once

#include "portal_dataset.hpp"

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
            .addProject({
                .id = 1,
                .title = "Customer Portal",
                .status = "active",
                .ownerId = 1
            })
            .addProject({
                .id = 2,
                .title = "Internal Dashboard",
                .status = "paused",
                .ownerId = 2
            });

        const auto validation =
            dataset.validate();

        if (!validation.valid()) {
            throw std::runtime_error(
                validation.errors().front()
            );
        }

        return dataset;
    }
};