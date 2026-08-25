#include <drogular/app.hpp>
#include <drogular/page.hpp>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>

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

TEST(CoreAppTests, RegistersExpressionFunctionAndMethod) {
    drogular::App app;

    EXPECT_NO_THROW(app.expressionFunction(
        "identity",
        [](std::span<const drogular::template_expression::ExpressionValue> arguments,
           const drogular::template_expression::BindingContext&) {
            return arguments.empty()
                ? drogular::template_expression::ExpressionValue()
                : arguments.front();
        }
    ));

    EXPECT_NO_THROW(app.expressionMethod(
        "identityMethod",
        [](const drogular::template_expression::ExpressionValue& self,
           std::span<const drogular::template_expression::ExpressionValue>,
           const drogular::template_expression::BindingContext&) {
            return self;
        }
    ));
}

TEST(CoreAppTests, RejectsDuplicateAndBuiltinExpressionNames) {
    drogular::App app;
    auto callback = [](
        std::span<const drogular::template_expression::ExpressionValue>,
        const drogular::template_expression::BindingContext&) {
        return drogular::template_expression::ExpressionValue();
    };

    EXPECT_NO_THROW(app.expressionFunction("custom", callback));
    EXPECT_THROW(app.expressionFunction("custom", callback), std::invalid_argument);
    EXPECT_THROW(app.expressionFunction("count", callback), std::invalid_argument);
}

TEST(CoreAppTests, TemplateRootUpdatesTemplateSourceCacheLoader) {
    namespace fs = std::filesystem;

    const auto root = fs::temp_directory_path() /
        "drogular-core-app-template-root-test";
    fs::remove_all(root);
    fs::create_directories(root);

    {
        std::ofstream file(root / "sample.html");
        file << "template-root-ok";
    }

    drogular::App app;
    app.templateRoot(root);

    EXPECT_EQ(
        app.services().templateSourceCache().load("sample.html"),
        "template-root-ok"
    );

    fs::remove_all(root);
}