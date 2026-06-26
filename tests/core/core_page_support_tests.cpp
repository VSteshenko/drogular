#include <drogular/page_support.hpp>
#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

class CorePageSupportTestPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        drogular::PageSupport::apply(
            context,
            "Test Page"
        );
    }

    std::string templateHtml() const override {
        return R"(
<h1>{{ pageTitle }}</h1>
<p>{{ currentPath }}</p>
{{{ manifestLink }}}
{{{ faviconLink }}}
{{{ themeColorMeta }}}
{{{ serviceWorkerRegistration }}}
)";
    }
};

TEST(CorePageSupportTests, AppliesCommonPageValues) {
    const auto result =
        drogular::test::renderPage<
            CorePageSupportTestPage
        >();

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "<h1>Test Page</h1>"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/assets/manifest.webmanifest"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/assets/favicon.ico"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/service-worker.js"
        )
    );
}