#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

TEST(ServicesTests, StoresGraphQLClient) {
    drogular::ApplicationServices services;
    drogular::GraphQLResult result;

    auto client = std::make_shared<drogular::StaticGraphQLClient>(result);

    services.setGraphQLClient(client);

    EXPECT_NE(services.graphQLClient(), nullptr);
}

#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

class TestLogger {
public:
    explicit TestLogger(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

TEST(ServicesTests, RegistersAndReturnsServiceByType) {
    drogular::ApplicationServices services;

    auto logger = std::make_shared<TestLogger>("main");

    services.registerService<TestLogger>(logger);

    const auto resolved = services.service<TestLogger>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

TEST(ServicesTests, ReturnsNullptrForMissingService) {
    drogular::ApplicationServices services;

    const auto resolved = services.service<TestLogger>();

    EXPECT_EQ(resolved, nullptr);
}

class ConfigurableService {
public:
    explicit ConfigurableService(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

TEST(ServicesTests, AddsServiceWithDefaultConstructor) {
    class DefaultService {
    public:
        int value() const {
            return 42;
        }
    };

    drogular::ApplicationServices services;

    const auto added =
        services.add<DefaultService>();

    const auto resolved =
        services.service<DefaultService>();

    ASSERT_NE(added, nullptr);
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(ServicesTests, AddsServiceWithConstructorArguments) {
    drogular::ApplicationServices services;

    const auto added =
        services.add<ConfigurableService>("main");

    const auto resolved =
        services.service<ConfigurableService>();

    ASSERT_NE(added, nullptr);
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

class RepositoryService {
public:
    std::string name() const {
        return "repository";
    }
};

class BusinessService {
public:
    explicit BusinessService(
        std::shared_ptr<RepositoryService> repository
    )
        : repository_(std::move(repository)) {
    }

    std::string repositoryName() const {
        return repository_->name();
    }

private:
    std::shared_ptr<RepositoryService> repository_;
};

TEST(ServicesTests, AddsServiceUsingFactory) {
    drogular::ApplicationServices services;

    services.add<RepositoryService>();

    const auto business =
        services.addFactory<BusinessService>(
            [&services]() {
                return std::make_shared<BusinessService>(
                    services.service<RepositoryService>()
                );
            }
        );

    ASSERT_NE(business, nullptr);
    EXPECT_EQ(business->repositoryName(), "repository");

    const auto resolved =
        services.service<BusinessService>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->repositoryName(), "repository");
}

TEST(ServicesTests, ThrowsWhenFactoryReturnsNullptr) {
    drogular::ApplicationServices services;

    EXPECT_THROW(
        services.addFactory<BusinessService>(
            []() {
                return nullptr;
            }
        ),
        std::runtime_error
    );
}