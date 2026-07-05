#pragma once

#include "../../data/portal_dataset.hpp"
#include "../role_provider.hpp"

#include <memory>

class PortalGraphQLRoleProvider final
    : public PortalRoleProvider
{
public:
    explicit PortalGraphQLRoleProvider(
        std::shared_ptr<PortalDataset> dataset
    )
        : dataset_(std::move(dataset))
    {
    }

    std::vector<PortalRole> all() const override {
        return dataset_->roles();
    }

    std::optional<PortalRole> findByCode(
        const std::string& code
    ) const override {
        for (const auto& role : dataset_->roles()) {
            if (role.code == code) {
                return role;
            }
        }

        return std::nullopt;
    }

private:
    std::shared_ptr<PortalDataset> dataset_;
};