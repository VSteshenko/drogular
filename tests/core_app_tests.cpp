#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class CoreAppTestPage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return "<h1>App Test</h1>";
    }
};

TEST(CoreAppTests, AppCanRegisterPageType) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.page<CoreAppTestPage>("/app-test");
    });
}

class CoreAppRegisteredComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>App Registered Component</p>";
    }
};

TEST(CoreAppTests, AppCanRegisterComponent) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.component<CoreAppRegisteredComponent>("AppRegisteredComponent");
    });
}

class CoreAppMetadataComponent final : public drogular::Component {
public:
    static constexpr auto tag = "AppMetadataComponent";

    std::string render(drogular::RenderContext&) override {
        return "<p>App Metadata Component</p>";
    }
};

TEST(CoreAppTests, AppCanRegisterComponentUsingMetadataTag) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.component<CoreAppMetadataComponent>();
    });
}

class CoreAppTestAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext&
    ) override {
        return drogular::ActionResult::redirect("/");
    }
};

TEST(CoreAppTests, AppCanRegisterAction) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.action<CoreAppTestAction>("/test-action");
    });
}