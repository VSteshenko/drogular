#include <drogular/inject.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>

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

TEST(InjectTests, CreatesFactoryWithSingleDependency) {
    drogular::ApplicationServices services;

    services.add<InjectRepository>();

    const auto factory =
        drogular::inject<InjectService, InjectRepository>(services);

    const auto service = factory();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->repositoryName(), "repository");
}

TEST(InjectTests, RegistersInjectedFactory) {
    drogular::ApplicationServices services;

    services.add<InjectRepository>();

    services.addFactory<InjectService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<InjectService, InjectRepository>(services)
    );

    const auto service =
        services.requireService<InjectService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->repositoryName(), "repository");
}

TEST(InjectTests, ThrowsWhenDependencyIsMissing) {
    drogular::ApplicationServices services;

    const auto factory =
        drogular::inject<InjectService, InjectRepository>(services);

    EXPECT_THROW(
        factory(),
        std::runtime_error
    );
}

class InjectLogger {
public:
    std::string name() const {
        return "logger";
    }
};

class InjectComplexService {
public:
    InjectComplexService(
        std::shared_ptr<InjectRepository> repository,
        std::shared_ptr<InjectLogger> logger
    )
        : repository_(std::move(repository)),
          logger_(std::move(logger)) {
    }

    std::string description() const {
        return repository_->name() + " + " + logger_->name();
    }

private:
    std::shared_ptr<InjectRepository> repository_;
    std::shared_ptr<InjectLogger> logger_;
};

TEST(InjectTests, CreatesFactoryWithMultipleDependencies) {
    drogular::ApplicationServices services;

    services.add<InjectRepository>();
    services.add<InjectLogger>();

    const auto factory =
        drogular::inject<
            InjectComplexService,
            InjectRepository,
            InjectLogger
        >(services);

    const auto service = factory();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->description(), "repository + logger");
}

TEST(InjectTests, RegistersInjectedFactoryWithMultipleDependencies) {
    drogular::ApplicationServices services;

    services.add<InjectRepository>();
    services.add<InjectLogger>();

    services.addFactory<InjectComplexService>(
        drogular::ServiceLifetime::Singleton,
        drogular::inject<
            InjectComplexService,
            InjectRepository,
            InjectLogger
        >(services)
    );

    const auto service =
        services.requireService<InjectComplexService>();

    ASSERT_NE(service, nullptr);
    EXPECT_EQ(service->description(), "repository + logger");
}

TEST(InjectTests, RegistersDependencyMetadata) {
    drogular::ApplicationServices services;

    const auto factory =
        drogular::inject<
            InjectComplexService,
            InjectRepository,
            InjectLogger
        >(services);

    (void)factory;

    EXPECT_TRUE(
        services.dependencyGraph().dependsOn(
            std::type_index(typeid(InjectComplexService)),
            std::type_index(typeid(InjectRepository))
        )
    );

    EXPECT_TRUE(
        services.dependencyGraph().dependsOn(
            std::type_index(typeid(InjectComplexService)),
            std::type_index(typeid(InjectLogger))
        )
    );
}