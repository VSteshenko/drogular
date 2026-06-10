#include <drogular/component.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(RenderContextTests, ReturnsApplicationServices) {
    drogular::ApplicationServices services;
    drogular::RenderContext context;

    context.setServices(&services);

    EXPECT_EQ(context.services(), &services);
}

class DefaultService {
public:
    int value() const {
        return 42;
    }
};

TEST(RenderContextTests, RequireServiceResolvesRegisteredService) {
    drogular::ApplicationServices services;

    services.add<DefaultService>();

    drogular::RenderContext context;

    context.setServices(&services);

    const auto resolved =
        context.requireService<
            DefaultService
        >();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(RenderContextTests, RequireServiceThrowsWhenServicesMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.requireService<
            DefaultService
        >(),
        std::runtime_error
    );
}

class RenderContextTestLogger {
public:
    explicit RenderContextTestLogger(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

TEST(RenderContextTests, ResolvesTypedService) {
    drogular::ApplicationServices services;
    drogular::RenderContext context;

    auto logger =
        std::make_shared<RenderContextTestLogger>("main");

    services.registerService<RenderContextTestLogger>(logger);

    context.setServices(&services);

    const auto resolved =
        context.service<RenderContextTestLogger>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

TEST(RenderContextTests, StoresAndReadsStringValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.get<std::string>("title");

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, "Hello");
}

TEST(RenderContextTests, StoresAndReadsIntegerValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.get<int>("count");

    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 42);
}

TEST(RenderContextTests, ReturnsNulloptForMissingKey) {
    drogular::RenderContext context;

    const auto value = context.get<std::string>("missing");

    EXPECT_FALSE(value.has_value());
}

TEST(RenderContextTests, ReturnsNulloptForWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto value = context.get<std::string>("count");

    EXPECT_FALSE(value.has_value());
}

TEST(RenderContextTests, ChecksContains) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    EXPECT_TRUE(context.contains("title"));
    EXPECT_FALSE(context.contains("missing"));
}

TEST(RenderContextTests, RemovesValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.remove("title");

    EXPECT_FALSE(context.contains("title"));
}

TEST(RenderContextTests, ClearsValues) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.set("count", 42);

    context.clear();

    EXPECT_FALSE(context.contains("title"));
    EXPECT_FALSE(context.contains("count"));
}

TEST(RenderContextTests, RequiresExistingValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.require<std::string>("title");

    EXPECT_EQ(title, "Hello");
}

TEST(RenderContextTests, ThrowsWhenRequiredValueIsMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.require<std::string>("title"),
        drogular::RenderContextError
    );
}

TEST(RenderContextTests, ThrowsWhenRequiredValueHasWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    EXPECT_THROW(
        context.require<std::string>("count"),
        drogular::RenderContextError
    );
}

TEST(RenderContextTests, ReturnsDefaultValueWhenKeyIsMissing) {
    drogular::RenderContext context;

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 0);
}

TEST(RenderContextTests, ReturnsStoredValueInsteadOfDefaultValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 42);
}

TEST(RenderContextTests, ChildContextReadsParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();

    EXPECT_EQ(child.require<std::string>("title"), "Parent");
}

TEST(RenderContextTests, ChildContextOverridesParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();
    child.set("title", std::string("Child"));

    EXPECT_EQ(child.require<std::string>("title"), "Child");
    EXPECT_EQ(parent.require<std::string>("title"), "Parent");
}

TEST(RenderContextTests, ChildContextContainsParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();

    EXPECT_TRUE(child.contains("title"));
}

class ScopedCounterService {
public:
    ScopedCounterService() {
        ++createdCount;
    }

    static inline int createdCount = 0;
};

TEST(RenderContextTests, ScopedServiceReturnsSameInstanceInsideScope) {
    ScopedCounterService::createdCount = 0;

    drogular::ApplicationServices services;

    services.addScoped<ScopedCounterService>(
        []() {
            return std::make_shared<ScopedCounterService>();
        }
    );

    drogular::RenderContext context;
    context.setServices(&services);

    const auto first = context.service<ScopedCounterService>();
    const auto second = context.service<ScopedCounterService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
    EXPECT_EQ(ScopedCounterService::createdCount, 1);
}

TEST(RenderContextTests, ScopedServiceCreatesNewInstanceForChildScope) {
    ScopedCounterService::createdCount = 0;

    drogular::ApplicationServices services;

    services.addScoped<ScopedCounterService>(
        []() {
            return std::make_shared<ScopedCounterService>();
        }
    );

    drogular::RenderContext parent;
    parent.setServices(&services);

    auto child = parent.createChild();

    const auto parentService = parent.service<ScopedCounterService>();
    const auto childService = child.service<ScopedCounterService>();

    ASSERT_NE(parentService, nullptr);
    ASSERT_NE(childService, nullptr);

    EXPECT_NE(parentService.get(), childService.get());
    EXPECT_EQ(ScopedCounterService::createdCount, 2);
}

TEST(RenderContextTests, RequireServiceSupportsScopedService) {
    drogular::ApplicationServices services;

    services.addScoped<ScopedCounterService>(
        []() {
            return std::make_shared<ScopedCounterService>();
        }
    );

    drogular::RenderContext context;
    context.setServices(&services);

    const auto service = context.requireService<ScopedCounterService>();

    ASSERT_NE(service, nullptr);
}

TEST(RenderContextTests, AddsScopedUsingLifetime) {
    drogular::ApplicationServices services;

    services.add<ScopedCounterService>(
        drogular::ServiceLifetime::Scoped
    );

    drogular::RenderContext context;
    context.setServices(&services);

    const auto first = context.service<ScopedCounterService>();
    const auto second = context.service<ScopedCounterService>();

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    EXPECT_EQ(first.get(), second.get());
}