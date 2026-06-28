#include <drogular/translation_provider.hpp>
#include <drogular/translation_support.hpp>

#include <drogular/render_context.hpp>
#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

class CoreTestTranslationProvider final
    : public drogular::TranslationProvider
{
public:
    std::string translate(
        const std::string& locale,
        const std::string& key
    ) const override {
        if (locale == "de" &&
            key == "app.title") {
            return "Test Anwendung";
        }

        if (key == "app.title") {
            return "Test App";
        }

        return key;
    }
};

class CoreTranslationSupportPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        context.set(
            "title",
            drogular::TranslationSupport::translate(
                context,
                "app.title"
            )
        );
    }

    std::string templateHtml() const override {
        return "<h1>{{ title }}</h1>";
    }
};

TEST(CoreTranslationSupportTests, TranslatesUsingProvider) {
    drogular::ApplicationServices services;

    services.addFactory<drogular::TranslationProvider>(
        drogular::ServiceLifetime::Singleton,
        [] {
            return std::make_shared<
                CoreTestTranslationProvider
            >();
        }
    );

    const auto result =
        drogular::test::renderPage<
            CoreTranslationSupportPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>Test App</h1>"
        )
    );
}

TEST(CoreTranslationSupportTests, ReturnsKeyWithoutProvider) {
    drogular::RenderContext context;

    EXPECT_EQ(
        drogular::TranslationSupport::translate(
            context,
            "missing.key"
        ),
        "missing.key"
    );
}