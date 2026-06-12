#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/inject.hpp>

#include <memory>
#include <string>

#include <gtest/gtest.h>

TEST(ServicesTests, StoresGraphQLClient) {
    drogular::ApplicationServices services;
    drogular::GraphQLResult result;

    auto client = std::make_shared<drogular::StaticGraphQLClient>(result);

    services.setGraphQLClient(client);

    EXPECT_NE(services.graphQLClient(), nullptr);
}

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

class DefaultService {
public:
    int value() const {
        return 42;
    }
};

TEST(ServicesTests, AddsServiceWithDefaultConstructor) {
    drogular::ApplicationServices services;

    const auto added =
        services.add<DefaultService>();

    const auto resolved =
        services.service<DefaultService>();

    ASSERT_NE(added, nullptr);
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(ServicesTests, AddsLazyService) {
    drogular::ApplicationServices services;

    bool created = false;

    services.addLazy<DefaultService>(
        [&created]() {
            created = true;
            return std::make_shared<DefaultService>();
        }
    );

    EXPECT_FALSE(created);

    const auto resolved = services.service<DefaultService>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_TRUE(created);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(ServicesTests, LazyServiceIsCreatedOnlyOnce) {
    drogular::ApplicationServices services;

    int createdCount = 0;

    services.addLazy<DefaultService>(
        [&createdCount]() {
            ++createdCount;
            return std::make_shared<DefaultService>();
        }
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first, second);
    EXPECT_EQ(createdCount, 1);
}

TEST(ServicesTests, LazyFactoryThrowsWhenReturningNullptr) {
    drogular::ApplicationServices services;

    services.addLazy<DefaultService>(
        []() {
            return nullptr;
        }
    );

    EXPECT_THROW(
        services.service<DefaultService>(),
        std::runtime_error
    );
}

TEST(ServicesTests, AddsFactorySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<DefaultService>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<DefaultService>();
        }
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(ServicesTests, AddsFactoryLazySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<DefaultService>(
        drogular::ServiceLifetime::LazySingleton,
        []() {
            return std::make_shared<DefaultService>();
        }
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(ServicesTests, AddsFactoryTransientUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<DefaultService>(
        drogular::ServiceLifetime::Transient,
        []() {
            return std::make_shared<DefaultService>();
        }
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}
TEST(ServicesTests, RequireServiceReturnsRegisteredService) {
    drogular::ApplicationServices services;

    services.add<DefaultService>();

    const auto resolved =
        services.requireService<
            DefaultService
        >();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(ServicesTests, RequireServiceThrowsWhenMissing) {
    drogular::ApplicationServices services;

    EXPECT_THROW(
        services.requireService<
            DefaultService
        >(),
        std::runtime_error
    );
}

TEST(ServicesTests, TransientCreatesNewInstances) {
    drogular::ApplicationServices services;

    services.addTransient<DefaultService>(
        []() {
            return std::make_shared<
                DefaultService
            >();
        }
    );

    const auto first =
        services.service<
            DefaultService
        >();

    const auto second =
        services.service<
            DefaultService
        >();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

TEST(ServicesTests, LazySingletonStillReturnsSameInstance) {
    drogular::ApplicationServices services;

    services.addLazy<
        DefaultService
    >(
        []() {
            return std::make_shared<
                DefaultService
            >();
        }
    );

    const auto first =
        services.service<
            DefaultService
        >();

    const auto second =
        services.service<
            DefaultService
        >();

    EXPECT_EQ(first.get(), second.get());
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

TEST(ServicesTests, AddsSingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<DefaultService>(
        drogular::ServiceLifetime::Singleton
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(ServicesTests, AddsLazySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<DefaultService>(
        drogular::ServiceLifetime::LazySingleton
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(ServicesTests, AddsTransientUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<DefaultService>(
        drogular::ServiceLifetime::Transient
    );

    const auto first = services.service<DefaultService>();
    const auto second = services.service<DefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

class ServiceRegistryComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>Service Registry Component</p>";
    }
};

TEST(ServicesTests, ProvidesComponentRegistry) {
    drogular::ApplicationServices services;

    services.components().registerComponent<ServiceRegistryComponent>(
        "ServiceRegistryComponent"
    );

    const auto component =
        services.components().create("ServiceRegistryComponent");

    ASSERT_NE(component, nullptr);
}

class InjectRepository {
public:
    std::string name() const {
        return "repository";
    }
};

class InjectService {
public:
    explicit InjectService(
        std::shared_ptr<InjectRepository> repository
    )
        : repository_(std::move(repository)) {
    }

    std::string repositoryName() const {
        return repository_->name();
    }

private:
    std::shared_ptr<InjectRepository> repository_;
};

TEST(ServicesTests, ValidatesRegisteredDependencies) {
    drogular::ApplicationServices services;

    services.add<InjectRepository>();

    services.addFactory<InjectService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<InjectService, InjectRepository>(services)
    );

    const auto result = services.validateDependencies();

    EXPECT_TRUE(result.valid());
}

class ValidationRepository {};

class ValidationService {
public:
    explicit ValidationService(
        std::shared_ptr<ValidationRepository>
    ) {
    }
};

TEST(ServicesTests, DetectsMissingDependency) {
    drogular::ApplicationServices services;

    services.addFactory<ValidationService>(
        drogular::ServiceLifetime::LazySingleton,
        drogular::inject<ValidationService, ValidationRepository>(services)
    );

    const auto result = services.validateDependencies();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
}

class CircularA {};
class CircularB {};
class CircularC {};

TEST(ServicesTests, ValidatesCircularDependencies) {
    drogular::ApplicationServices services;

    services.add<CircularA>();
    services.add<CircularB>();
    services.add<CircularC>();

    services.dependencyGraph().addDependency<CircularA, CircularB>();
    services.dependencyGraph().addDependency<CircularB, CircularC>();
    services.dependencyGraph().addDependency<CircularC, CircularA>();

    const auto result = services.validateDependencies();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0], "Circular dependency detected");
}