#include <drogular/graphql_client.hpp>
#include <drogular/services.hpp>
#include <drogular/inject.hpp>

#include <memory>
#include <string>

#include <gtest/gtest.h>

TEST(CoreServicesTests, StoresGraphQLClient) {
    drogular::ApplicationServices services;
    drogular::GraphQLResult result;

    auto client = std::make_shared<drogular::StaticGraphQLClient>(result);

    services.setGraphQLClient(client);

    EXPECT_NE(services.graphQLClient(), nullptr);
}

class CoreTestLogger {
public:
    explicit CoreTestLogger(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

TEST(CoreServicesTests, RegistersAndReturnsServiceByType) {
    drogular::ApplicationServices services;

    auto logger = std::make_shared<CoreTestLogger>("main");

    services.registerService<CoreTestLogger>(logger);

    const auto resolved = services.service<CoreTestLogger>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

TEST(CoreServicesTests, ReturnsNullptrForMissingService) {
    drogular::ApplicationServices services;

    const auto resolved = services.service<CoreTestLogger>();

    EXPECT_EQ(resolved, nullptr);
}

class CoreConfigurableService {
public:
    explicit CoreConfigurableService(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

class CoreDefaultService {
public:
    int value() const {
        return 42;
    }
};

TEST(CoreServicesTests, AddsServiceWithDefaultConstructor) {
    drogular::ApplicationServices services;

    const auto added =
        services.add<CoreDefaultService>();

    const auto resolved =
        services.service<CoreDefaultService>();

    ASSERT_NE(added, nullptr);
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(CoreServicesTests, AddsLazyService) {
    drogular::ApplicationServices services;

    bool created = false;

    services.addLazy<CoreDefaultService>(
        [&created]() {
            created = true;
            return std::make_shared<CoreDefaultService>();
        }
    );

    EXPECT_FALSE(created);

    const auto resolved = services.service<CoreDefaultService>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_TRUE(created);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(CoreServicesTests, LazyServiceIsCreatedOnlyOnce) {
    drogular::ApplicationServices services;

    int createdCount = 0;

    services.addLazy<CoreDefaultService>(
        [&createdCount]() {
            ++createdCount;
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first, second);
    EXPECT_EQ(createdCount, 1);
}

TEST(ServicesTests, LazyFactoryThrowsWhenReturningNullptr) {
    drogular::ApplicationServices services;

    services.addLazy<CoreDefaultService>(
        []() {
            return nullptr;
        }
    );

    EXPECT_THROW(
        services.service<CoreDefaultService>(),
        std::runtime_error
    );
}

TEST(CoreServicesTests, AddsFactorySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<CoreDefaultService>(
        drogular::ServiceLifetime::Singleton,
        []() {
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreServicesTests, AddsFactoryLazySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<CoreDefaultService>(
        drogular::ServiceLifetime::LazySingleton,
        []() {
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreServicesTests, AddsFactoryTransientUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<CoreDefaultService>(
        drogular::ServiceLifetime::Transient,
        []() {
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}
TEST(CoreServicesTests, RequireServiceReturnsRegisteredService) {
    drogular::ApplicationServices services;

    services.add<CoreDefaultService>();

    const auto resolved =
        services.requireService<
            CoreDefaultService
        >();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(CoreServicesTests, RequireServiceThrowsWhenMissing) {
    drogular::ApplicationServices services;

    EXPECT_THROW(
        services.requireService<
            CoreDefaultService
        >(),
        std::runtime_error
    );
}

TEST(CoreServicesTests, TransientCreatesNewInstances) {
    drogular::ApplicationServices services;

    services.addTransient<CoreDefaultService>(
        []() {
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first =
        services.service<CoreDefaultService>();

    const auto second =
        services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

TEST(CoreServicesTests, LazySingletonStillReturnsSameInstance) {
    drogular::ApplicationServices services;

    services.addLazy<CoreDefaultService>(
        []() {
            return std::make_shared<CoreDefaultService>();
        }
    );

    const auto first =
        services.service<CoreDefaultService>();

    const auto second =
        services.service<CoreDefaultService>();

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreServicesTests, AddsServiceWithConstructorArguments) {
    drogular::ApplicationServices services;

    const auto added =
        services.add<CoreConfigurableService>("main");

    const auto resolved =
        services.service<CoreConfigurableService>();

    ASSERT_NE(added, nullptr);
    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

class CoreRepositoryService {
public:
    std::string name() const {
        return "repository";
    }
};

class CoreBusinessService {
public:
    explicit CoreBusinessService(
        std::shared_ptr<CoreRepositoryService> repository
    )
        : repository_(std::move(repository)) {
    }

    std::string repositoryName() const {
        return repository_->name();
    }

private:
    std::shared_ptr<CoreRepositoryService> repository_;
};

TEST(CoreServicesTests, AddsServiceUsingFactory) {
    drogular::ApplicationServices services;

    services.add<CoreRepositoryService>();

    const auto business =
        services.addFactory<CoreBusinessService>(
            [&services]() {
                return std::make_shared<CoreBusinessService>(
                    services.service<CoreRepositoryService>()
                );
            }
        );

    ASSERT_NE(business, nullptr);
    EXPECT_EQ(business->repositoryName(), "repository");

    const auto resolved =
        services.service<CoreBusinessService>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->repositoryName(), "repository");
}

TEST(CoreServicesTests, ThrowsWhenFactoryReturnsNullptr) {
    drogular::ApplicationServices services;

    EXPECT_THROW(
        services.addFactory<CoreBusinessService>(
            []() {
                return nullptr;
            }
        ),
        std::runtime_error
    );
}

TEST(CoreServicesTests, AddsSingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<CoreDefaultService>(
        drogular::ServiceLifetime::Singleton
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreServicesTests, AddsLazySingletonUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<CoreDefaultService>(
        drogular::ServiceLifetime::LazySingleton
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}

TEST(CoreServicesTests, AddsTransientUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<CoreDefaultService>(
        drogular::ServiceLifetime::Transient
    );

    const auto first = services.service<CoreDefaultService>();
    const auto second = services.service<CoreDefaultService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_NE(first.get(), second.get());
}

class CoreServiceRegistryComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>Service Registry Component</p>";
    }
};

TEST(CoreServicesTests, ProvidesComponentRegistry) {
    drogular::ApplicationServices services;

    services.components().registerComponent<CoreServiceRegistryComponent>(
        "CoreServiceRegistryComponent"
    );

    const auto component =
        services.components().create("CoreServiceRegistryComponent");

    ASSERT_NE(component, nullptr);
}

class CoreInjectRepository {
public:
    std::string name() const {
        return "repository";
    }
};

class CoreInjectService {
public:
    explicit CoreInjectService(
        std::shared_ptr<CoreInjectRepository> repository
    )
        : repository_(std::move(repository)) {
    }

    std::string repositoryName() const {
        return repository_->name();
    }

private:
    std::shared_ptr<CoreInjectRepository> repository_;
};

TEST(CoreServicesTests, ValidatesRegisteredDependencies) {
    drogular::ApplicationServices services;

    services.add<CoreInjectRepository>();

    services.addFactory<CoreInjectService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<CoreInjectService, CoreInjectRepository>(services)
    );

    const auto result = services.validateDependencies();

    EXPECT_TRUE(result.valid());
}

class CoreValidationRepository {};

class CoreValidationService {
public:
    explicit CoreValidationService(
        std::shared_ptr<CoreValidationRepository>
    ) {
    }
};

TEST(CoreServicesTests, DetectsMissingDependency) {
    drogular::ApplicationServices services;

    services.addFactory<CoreValidationService>(
        drogular::ServiceLifetime::LazySingleton,
        drogular::inject<CoreValidationService, CoreValidationRepository>(services)
    );

    const auto result = services.validateDependencies();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
}

class CoreCircularA {};
class CoreCircularB {};
class CoreCircularC {};

TEST(CoreServicesTests, ValidatesCircularDependencies) {
    drogular::ApplicationServices services;

    services.add<CoreCircularA>();
    services.add<CoreCircularB>();
    services.add<CoreCircularC>();

    services.dependencyGraph().addDependency<CoreCircularA, CoreCircularB>();
    services.dependencyGraph().addDependency<CoreCircularB, CoreCircularC>();
    services.dependencyGraph().addDependency<CoreCircularC, CoreCircularA>();

    const auto result = services.validateDependencies();

    ASSERT_FALSE(result.valid());
    ASSERT_EQ(result.errors().size(), 1);
    EXPECT_EQ(result.errors()[0], "Circular dependency detected");
}