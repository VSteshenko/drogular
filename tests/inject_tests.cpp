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
