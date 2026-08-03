#pragma once

#include "data/portal_dataset.hpp"

#include <drogular/developer_tools/application_inspection.hpp>

#include <json/json.h>

#include <memory>
#include <stdexcept>
#include <utility>

class PortalInspectionContributor final
    : public drogular::DeveloperToolsContributor
{
public:
    explicit PortalInspectionContributor(
        std::shared_ptr<const PortalDataset> dataset
    )
        : dataset_(std::move(dataset))
    {
        if (!dataset_) {
            throw std::invalid_argument(
                "PortalInspectionContributor requires a non-null dataset"
            );
        }
    }

    void contribute(
        drogular::ApplicationInspection& inspection
    ) const override {
        inspection.addSection({
            "portal-dataset",
            "Portal Dataset",
            "portal.summary",
            datasetSection()
        });

        inspection.addSection({
            "portal-graphql",
            "Portal GraphQL",
            "portal.summary",
            graphQLSection()
        });
    }

private:
    Json::Value datasetSection() const {
        Json::Value entities(Json::objectValue);
        entities["projects"] =
            static_cast<Json::UInt64>(dataset_->projects().size());
        entities["users"] =
            static_cast<Json::UInt64>(dataset_->users().size());
        entities["roles"] =
            static_cast<Json::UInt64>(dataset_->roles().size());
        entities["projectTypes"] =
            static_cast<Json::UInt64>(dataset_->projectTypes().size());
        entities["departments"] =
            static_cast<Json::UInt64>(dataset_->departments().size());
        entities["departmentMembers"] =
            static_cast<Json::UInt64>(dataset_->departmentMembers().size());

        Json::Value data(Json::objectValue);
        data["storage"] = "in-memory";
        data["entities"] = std::move(entities);
        return data;
    }

    static Json::Value graphQLSection() {
        Json::Value modules(Json::arrayValue);
        modules.append("projects");
        modules.append("users");
        modules.append("roles");
        modules.append("project-types");
        modules.append("departments");
        modules.append("department-members");

        Json::Value data(Json::objectValue);
        data["client"] = "InProcessGraphQLClient";
        data["server"] = "GraphQLServer";
        data["operationModules"] = std::move(modules);
        return data;
    }

    std::shared_ptr<const PortalDataset> dataset_;
};