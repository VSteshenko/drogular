#pragma once

#include "portal_dataset.hpp"

class DemoDataset {
public:
    static PortalDataset create() {
        PortalDataset dataset;

        dataset
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
                .status = "active"
            })
            .addProject({
                .id = 2,
                .title = "Internal Dashboard",
                .status = "paused"
            });

        return dataset;
    }
};