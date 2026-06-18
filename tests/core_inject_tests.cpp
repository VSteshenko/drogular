#include <drogular/inject.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

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

TEST(CoreInjectTests, CreatesFactoryWithSingleDependency) {
    drogular::ApplicationServices services;

    services.add<CoreInjectRepository>();

    const auto factory =
        drogular::inject<CoreInjectService, CoreInjectRepository>(services);

    const auto service = factory();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->repositoryName(), "repository");
}

TEST(CoreInjectTests, RegistersInjectedFactory) {
    drogular::ApplicationServices services;

    services.add<CoreInjectRepository>();

    services.addFactory<CoreInjectService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<CoreInjectService, CoreInjectRepository>(services)
    );

    const auto service =
        services.requireService<CoreInjectService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->repositoryName(), "repository");
}

TEST(CoreInjectTests, ThrowsWhenDependencyIsMissing) {
    drogular::ApplicationServices services;

    const auto factory =
        drogular::inject<CoreInjectService, CoreInjectRepository>(services);

    EXPECT_THROW(
        factory(),
        std::runtime_error
    );
}

class CoreInjectLogger {
public:
    std::string name() const {
        return "logger";
    }
};

class CoreInjectComplexService {
public:
    CoreInjectComplexService(
        std::shared_ptr<CoreInjectRepository> repository,
        std::shared_ptr<CoreInjectLogger> logger
    )
        : repository_(std::move(repository)),
          logger_(std::move(logger)) {
    }

    std::string description() const {
        return repository_->name() + " + " + logger_->name();
    }

private:
    std::shared_ptr<CoreInjectRepository> repository_;
    std::shared_ptr<CoreInjectLogger> logger_;
};

TEST(CoreInjectTests, CreatesFactoryWithMultipleDependencies) {
    drogular::ApplicationServices services;

    services.add<CoreInjectRepository>();
    services.add<CoreInjectLogger>();

    const auto factory =
        drogular::inject<
            CoreInjectComplexService,
            CoreInjectRepository,
            CoreInjectLogger
        >(services);

    const auto service = factory();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->description(), "repository + logger");
}

TEST(CoreInjectTests, RegistersInjectedFactoryWithMultipleDependencies) {
    drogular::ApplicationServices services;

    services.add<CoreInjectRepository>();
    services.add<CoreInjectLogger>();

    services.addFactory<CoreInjectComplexService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<
            CoreInjectComplexService,
            CoreInjectRepository,
            CoreInjectLogger
        >(services)
    );

    const auto service =
        services.requireService<CoreInjectComplexService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->description(), "repository + logger");
}

TEST(CoreInjectTests, RegistersDependencyMetadata) {
    drogular::ApplicationServices services;

    const auto factory =
        drogular::inject<
            CoreInjectComplexService,
            CoreInjectRepository,
            CoreInjectLogger
        >(services);

    (void)factory;

    EXPECT_TRUE(
        services.dependencyGraph().dependsOn(
            std::type_index(typeid(CoreInjectComplexService)),
            std::type_index(typeid(CoreInjectRepository))
        )
    );

    EXPECT_TRUE(
        services.dependencyGraph().dependsOn(
            std::type_index(typeid(CoreInjectComplexService)),
            std::type_index(typeid(CoreInjectLogger))
        )
    );
}