#include "features/diagnostics/portal_inspection_contributor.hpp"
#include "data/demo_dataset.hpp"

#include <drogular/application_inspection.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <stdexcept>

TEST(PortalInspectionContributorTests, AddsPortalSpecificSections) {
    auto dataset = std::make_shared<PortalDataset>(
        DemoDataset::create()
    );

    PortalInspectionContributor contributor(dataset);
    drogular::ApplicationInspection inspection;

    contributor.contribute(inspection);

    ASSERT_EQ(inspection.sections.size(), 2u);

    const auto& datasetSection = inspection.sections[0];
    EXPECT_EQ(datasetSection.id, "portal-dataset");
    EXPECT_EQ(datasetSection.title, "Portal Dataset");
    EXPECT_EQ(datasetSection.component, "portal.summary");
    EXPECT_EQ(datasetSection.data["storage"].asString(), "in-memory");
    EXPECT_EQ(
        datasetSection.data["entities"]["projects"].asUInt64(),
        dataset->projects().size()
    );
    EXPECT_EQ(
        datasetSection.data["entities"]["users"].asUInt64(),
        dataset->users().size()
    );

    const auto& graphQLSection = inspection.sections[1];
    EXPECT_EQ(graphQLSection.id, "portal-graphql");
    EXPECT_EQ(graphQLSection.component, "portal.summary");
    EXPECT_EQ(
        graphQLSection.data["client"].asString(),
        "InProcessGraphQLClient"
    );
    ASSERT_TRUE(graphQLSection.data["operationModules"].isArray());
    EXPECT_EQ(
        graphQLSection.data["operationModules"].size(),
        6u
    );
}

TEST(PortalInspectionContributorTests, ReadsCurrentDatasetState) {
    auto dataset = std::make_shared<PortalDataset>();
    PortalInspectionContributor contributor(dataset);

    dataset->addRole({
        1,
        "admin",
        "Administrator"
    });

    drogular::ApplicationInspection inspection;
    contributor.contribute(inspection);

    ASSERT_EQ(inspection.sections.size(), 2u);
    EXPECT_EQ(
        inspection.sections[0].data["entities"]["roles"].asUInt64(),
        1u
    );
}

TEST(PortalInspectionContributorTests, RejectsNullDataset) {
    EXPECT_THROW(
        PortalInspectionContributor(nullptr),
        std::invalid_argument
    );
}