#include <drogular/page.hpp>
#include <drogular/pwa_page_support.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

class CorePwaPageSupportTestPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        drogular::PwaPageSupport::apply(context);
    }

    std::string templateHtml() const override {
        return R"(
{{{ manifestLink }}}
{{{ faviconLink }}}
{{{ themeColorMeta }}}
{{{ serviceWorkerRegistration }}}
)";
    }
};

TEST(CorePwaPageSupportTests, AppliesDefaultPwaValues) {
    const auto result =
        drogular::test::renderPage<
            CorePwaPageSupportTestPage
        >();

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
            "#4f46e5"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "/service-worker.js"
        )
    );
}