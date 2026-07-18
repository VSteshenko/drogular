#include "data/demo_dataset.hpp"
#include "providers/graphql/portal_dataset_graphql_client.hpp"
#include "providers/graphql/portal_graphql_project_type_provider.hpp"
#include "providers/graphql/portal_graphql_role_provider.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>

struct ProjectTypeProviderContract {
    using Provider = PortalGraphQLProjectTypeProvider;
    using Model = PortalProjectType;
    using CreateInput = PortalProjectTypeCreate;
    using UpdateInput = PortalProjectTypeUpdate;

    static std::unique_ptr<Provider> makeProvider(
        const std::shared_ptr<PortalDataset>& dataset
    ) {
        auto client =
            std::make_shared<
                PortalDatasetGraphQLClient
            >(dataset);

        return std::make_unique<Provider>(
            std::move(client)
        );
    }

    static const std::vector<Model>& items(
        const PortalDataset& dataset
    ) {
        return dataset.projectTypes();
    }

    static Model existing(
        const PortalDataset& dataset
    ) {
        return dataset.projectTypes().front();
    }

    static Model used(
        const PortalDataset& dataset
    ) {
        const auto usedId =
            dataset.projects()
                .front()
                .projectTypeId;

        for (const auto& item :
             dataset.projectTypes()) {
            if (item.id == usedId) {
                return item;
            }
             }

        return {};
    }

    static CreateInput createInput() {
        return {
            .code = "contract-type",
            .title = "Contract Type"
        };
    }

    static UpdateInput updateTitle(
        int id
    ) {
        UpdateInput input;
        input.id = id;
        input.title = "Updated Contract Type";

        return input;
    }

    static const std::string& code(
        const Model& model
    ) {
        return model.code;
    }

    static const std::string& title(
        const Model& model
    ) {
        return model.title;
    }
};

struct RoleProviderContract {
    using Provider = PortalGraphQLRoleProvider;
    using Model = PortalRole;
    using CreateInput = PortalRoleCreate;
    using UpdateInput = PortalRoleUpdate;

    static std::unique_ptr<Provider> makeProvider(
        const std::shared_ptr<PortalDataset>& dataset
    ) {
        auto client =
            std::make_shared<
                PortalDatasetGraphQLClient
            >(dataset);

        return std::make_unique<Provider>(
            std::move(client)
        );
    }

    static const std::vector<Model>& items(
        const PortalDataset& dataset
    ) {
        return dataset.roles();
    }

    static Model existing(
        const PortalDataset& dataset
    ) {
        return dataset.roles().front();
    }

    static Model used(
        const PortalDataset& dataset
    ) {
        const auto usedCode =
            dataset.users()
                .front()
                .role;

        for (const auto& item :
             dataset.roles()) {
            if (item.code == usedCode) {
                return item;
            }
             }

        return {};
    }

    static CreateInput createInput() {
        return {
            .code = "contract-role",
            .title = "Contract Role"
        };
    }

    static UpdateInput updateTitle(
        int id
    ) {
        UpdateInput input;
        input.id = id;
        input.title = "Updated Contract Role";

        return input;
    }

    static const std::string& code(
        const Model& model
    ) {
        return model.code;
    }

    static const std::string& title(
        const Model& model
    ) {
        return model.title;
    }
};

template <typename TContract>
class ReferenceProviderContractTests
    : public testing::Test
{
protected:
    using Contract = TContract;
    using Provider = typename Contract::Provider;

    void SetUp() override {
        dataset =
            std::make_shared<PortalDataset>(
                DemoDataset::create()
            );

        provider =
            Contract::makeProvider(
                dataset
            );
    }

    std::shared_ptr<PortalDataset> dataset;
    std::unique_ptr<Provider> provider;
};

using ReferenceProviderContracts =
    testing::Types<
        ProjectTypeProviderContract,
        RoleProviderContract
    >;

TYPED_TEST_SUITE(ReferenceProviderContractTests, ReferenceProviderContracts);

TYPED_TEST(ReferenceProviderContractTests, ReturnsAllEntities) {
    using Contract =
        typename TestFixture::Contract;

    const auto result =
        this->provider->all();

    EXPECT_EQ(
        result.size(),
        Contract::items(
            *this->dataset
        ).size()
    );
}

TYPED_TEST(ReferenceProviderContractTests, FindsExistingEntityById) {
    using Contract = typename TestFixture::Contract;

    const auto expected =
        Contract::existing(
            *this->dataset
        );

    const auto result =
        this->provider->findById(
            expected.id
        );

    ASSERT_TRUE(
        result.has_value()
    );

    EXPECT_EQ(
        result->id,
        expected.id
    );

    EXPECT_EQ(
        Contract::code(*result),
        Contract::code(expected)
    );

    EXPECT_EQ(
        Contract::title(*result),
        Contract::title(expected)
    );
}

TYPED_TEST(ReferenceProviderContractTests, ReturnsEmptyForMissingId) {
    const auto result =
        this->provider->findById(
            999999
        );

    EXPECT_FALSE(result.has_value());
}

TYPED_TEST(ReferenceProviderContractTests, CreatesEntity) {
    using Contract =
        typename TestFixture::Contract;

    const auto input =
        Contract::createInput();

    const auto beforeCount =
        Contract::items(
            *this->dataset
        ).size();

    const auto created =
        this->provider->create(
            input
        );

    EXPECT_GT(
        created.id,
        0
    );

    EXPECT_EQ(
        Contract::code(created),
        input.code
    );

    EXPECT_EQ(
        Contract::title(created),
        input.title
    );

    EXPECT_EQ(
        Contract::items(
            *this->dataset
        ).size(),
        beforeCount + 1
    );
}

TYPED_TEST(ReferenceProviderContractTests, UpdatesOnlyProvidedFields) {
    using Contract = typename TestFixture::Contract;

    const auto before =
        Contract::existing(
            *this->dataset
        );

    const auto input =
        Contract::updateTitle(
            before.id
        );

    const auto updated =
        this->provider->update(
            input
        );

    EXPECT_EQ(
        updated.id,
        before.id
    );

    EXPECT_EQ(
        Contract::code(updated),
        Contract::code(before)
    );

    ASSERT_TRUE(
        input.title.has_value()
    );

    EXPECT_EQ(
        Contract::title(updated),
        *input.title
    );
}

TYPED_TEST(ReferenceProviderContractTests, RemovesUnusedEntity) {
    using Contract = typename TestFixture::Contract;

    const auto input =
        Contract::createInput();

    const auto created =
        this->provider->create(
            input
        );

    ASSERT_GT(
        created.id,
        0
    );

    EXPECT_TRUE(
        this->provider->remove(
            created.id
        )
    );

    EXPECT_FALSE(
        this->provider
            ->findById(created.id)
            .has_value()
    );
}

TYPED_TEST(ReferenceProviderContractTests, RejectsRemovingUsedEntity) {
    using Contract = typename TestFixture::Contract;

    const auto used =
        Contract::used(
            *this->dataset
        );

    ASSERT_GT(
        used.id,
        0
    );

    EXPECT_FALSE(
        this->provider->remove(
            used.id
        )
    );

    EXPECT_TRUE(
        this->provider
            ->findById(used.id)
            .has_value()
    );
}