#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <string>

class AppTestPage final : public drogular::Page {
public:
    std::string render(drogular::RenderContext&) override {
        return "<h1>App Test</h1>";
    }
};

TEST(AppTests, AppCanRegisterPageType) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.page<AppTestPage>("/app-test");
    });
}

class AppRegisteredComponent final : public drogular::Component {
public:
    std::string render(drogular::RenderContext&) override {
        return "<p>App Registered Component</p>";
    }
};

TEST(AppTests, AppCanRegisterComponent) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.component<AppRegisteredComponent>("AppRegisteredComponent");
    });
}

class AppMetadataComponent final : public drogular::Component {
public:
    static constexpr auto tag = "AppMetadataComponent";

    std::string render(drogular::RenderContext&) override {
        return "<p>App Metadata Component</p>";
    }
};

TEST(AppTests, AppCanRegisterComponentUsingMetadataTag) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.component<AppMetadataComponent>();
    });
}

class AppTestAction final : public drogular::ActionHandler {
public:
    drogular::ActionResult handle(
        drogular::ActionContext&
    ) override {
        return drogular::ActionResult::redirect("/");
    }
};

TEST(AppTests, AppCanRegisterAction) {
    drogular::App app;

    EXPECT_NO_THROW({
        app.action<AppTestAction>("/test-action");
    });
}