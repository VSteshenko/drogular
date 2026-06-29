#include <drogular/render_context.hpp>
#include <drogular/services.hpp>

#include <gtest/gtest.h>

#include <string>

TEST(CoreRenderContextTests, ReturnsApplicationServices) {
    drogular::ApplicationServices services;
    drogular::RenderContext context;

    context.setServices(&services);

    EXPECT_EQ(context.services(), &services);
}

class CoreDefaultService {
public:
    int value() const {
        return 42;
    }
};

TEST(CoreRenderContextTests, RequireServiceResolvesRegisteredService) {
    drogular::ApplicationServices services;

    services.add<CoreDefaultService>();

    drogular::RenderContext context;

    context.setServices(&services);

    const auto resolved =
        context.requireService<CoreDefaultService>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->value(), 42);
}

TEST(CoreRenderContextTests, RequireServiceThrowsWhenServicesMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.requireService<CoreDefaultService>(),
        std::runtime_error
    );
}

class CoreRenderContextTestLogger {
public:
    explicit CoreRenderContextTestLogger(std::string name)
        : name_(std::move(name)) {
    }

    const std::string& name() const {
        return name_;
    }

private:
    std::string name_;
};

TEST(CoreRenderContextTests, ResolvesTypedService) {
    drogular::ApplicationServices services;
    drogular::RenderContext context;

    auto logger =
        std::make_shared<CoreRenderContextTestLogger>("main");

    services.registerService<CoreRenderContextTestLogger>(logger);

    context.setServices(&services);

    const auto resolved =
        context.service<CoreRenderContextTestLogger>();

    ASSERT_NE(resolved, nullptr);
    EXPECT_EQ(resolved->name(), "main");
}

TEST(CoreRenderContextTests, StoresAndReadsStringValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.get<std::string>("title");

    ASSERT_TRUE(title.has_value());
    EXPECT_EQ(*title, "Hello");
}

TEST(CoreRenderContextTests, StoresAndReadsIntegerValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.get<int>("count");

    ASSERT_TRUE(count.has_value());
    EXPECT_EQ(*count, 42);
}

TEST(CoreRenderContextTests, ReturnsNulloptForMissingKey) {
    drogular::RenderContext context;

    const auto value = context.get<std::string>("missing");

    EXPECT_FALSE(value.has_value());
}

TEST(CoreRenderContextTests, ReturnsNulloptForWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto value = context.get<std::string>("count");

    EXPECT_FALSE(value.has_value());
}

TEST(CoreRenderContextTests, ChecksContains) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    EXPECT_TRUE(context.contains("title"));
    EXPECT_FALSE(context.contains("missing"));
}

TEST(CoreRenderContextTests, RemovesValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.remove("title");

    EXPECT_FALSE(context.contains("title"));
}

TEST(CoreRenderContextTests, ClearsValues) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));
    context.set("count", 42);

    context.clear();

    EXPECT_FALSE(context.contains("title"));
    EXPECT_FALSE(context.contains("count"));
}

TEST(CoreRenderContextTests, RequiresExistingValue) {
    drogular::RenderContext context;

    context.set("title", std::string("Hello"));

    const auto title = context.require<std::string>("title");

    EXPECT_EQ(title, "Hello");
}

TEST(CoreRenderContextTests, ThrowsWhenRequiredValueIsMissing) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.require<std::string>("title"),
        drogular::RenderContextError
    );
}

TEST(CoreRenderContextTests, ThrowsWhenRequiredValueHasWrongType) {
    drogular::RenderContext context;

    context.set("count", 42);

    EXPECT_THROW(
        context.require<std::string>("count"),
        drogular::RenderContextError
    );
}

TEST(CoreRenderContextTests, ReturnsDefaultValueWhenKeyIsMissing) {
    drogular::RenderContext context;

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 0);
}

TEST(CoreRenderContextTests, ReturnsStoredValueInsteadOfDefaultValue) {
    drogular::RenderContext context;

    context.set("count", 42);

    const auto count = context.getOr<int>("count", 0);

    EXPECT_EQ(count, 42);
}

TEST(CoreRenderContextTests, ChildContextReadsParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();

    EXPECT_EQ(child.require<std::string>("title"), "Parent");
}

TEST(CoreRenderContextTests, ChildContextOverridesParentValue) {
    drogular::RenderContext parent;

    parent.set("title", std::string("Parent"));

    auto child = parent.createChild();
    child.set("title", std::string("Child"));

    EXPECT_EQ(child.require<std::string>("title"), "Child");
    EXPECT_EQ(parent.require<std::string>("title"), "Parent");
}

TEST(CoreRenderContextTests, ChildContextContainsParentValue) {
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

TEST(CoreRenderContextTests, ScopedServiceReturnsSameInstanceInsideScope) {
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

TEST(CoreRenderContextTests, ScopedServiceCreatesNewInstanceForChildScope) {
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

TEST(CoreRenderContextTests, RequireServiceSupportsScopedService) {
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

TEST(CoreRenderContextTests, AddsScopedUsingLifetime) {
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

TEST(CoreRenderContextTests, AddsFactoryScopedUsingLifetime) {
    drogular::ApplicationServices services;

    services.addFactory<ScopedCounterService>(
        drogular::ServiceLifetime::Scoped,
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
}

TEST(CoreRenderContextTests, TranslatesKeyWithoutProvider) {
    drogular::RenderContext context;

    EXPECT_EQ(
        context.translate("app.title"),
        "app.title"
    );
}

TEST(CoreRenderContextTests, SetsTranslatedValues) {
    drogular::RenderContext context;

    context.setTranslations({
        {"title", "app.title"},
        {"logout", "nav.logout"}
    });

    EXPECT_EQ(context.get<std::string>("title"), "app.title");
    EXPECT_EQ(context.get<std::string>("logout"), "nav.logout");
}

TEST(CoreRenderContextTests, StoresRouteParams) {
    drogular::RenderContext context;

    context.setRouteParam("id", "42");

    const auto id =
        context.routeParam("id");

    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(*id, "42");
}

TEST(CoreRenderContextTests, ThrowsForMissingRequiredRouteParam) {
    drogular::RenderContext context;

    EXPECT_THROW(
        context.requireRouteParam("id"),
        std::runtime_error
    );
}