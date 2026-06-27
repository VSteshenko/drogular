#include <drogular/auth_support.hpp>
#include <drogular/session_store.hpp>

#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

class CoreAuthSupportGuestPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override {
        context.set(
            "authenticated",
            drogular::AuthSupport::isAuthenticated(context)
        );
    }

    std::string templateHtml() const override {
        return R"(
@if(authenticated)
yes
@else
no
@endif
)";
    }
};

TEST(CoreAuthSupportTests, ReturnsFalseForGuest) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<
            CoreAuthSupportGuestPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "no"
        )
    );
}