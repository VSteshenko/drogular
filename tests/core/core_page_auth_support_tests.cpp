#include <drogular/page_auth_support.hpp>
#include <drogular/session_store.hpp>

#include <drogular/render_context.hpp>
#include <drogular/page.hpp>
#include <drogular/testing.hpp>

#include <gtest/gtest.h>

class CorePageAuthSupportGuestPage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        const auto allowed =
            drogular::PageAuthSupport::requireAuthentication(
                context
            );

        context.set("allowed", allowed);
    }

    std::string templateHtml() const override
    {
        return R"(
@if(loginRequired)
login required
@endif

@if(allowed)
allowed
@else
blocked
@endif
)";
    }
};

TEST(CorePageAuthSupportTests, BlocksGuestPage) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<
            CorePageAuthSupportGuestPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "login required"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "blocked"
        )
    );
}

class CorePageAuthSupportRolePage final
    : public drogular::TemplatePage
{
public:
    void onInit(
        drogular::RenderContext& context
    ) override
    {
        const auto allowed =
            drogular::PageAuthSupport::requireSessionValue(
                context,
                "role",
                "admin"
            );

        context.set("allowed", allowed);
    }

    std::string templateHtml() const override
    {
        return R"(
@if(accessDenied)
access denied
@endif

@if(allowed)
allowed
@else
blocked
@endif
)";
    }
};

TEST(CorePageAuthSupportTests, BlocksMissingSessionValue) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<
            CorePageAuthSupportRolePage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "access denied"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "blocked"
        )
    );
}

TEST(CorePageAuthSupportTests, RequiresAuthenticationForGuest) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<
            CorePageAuthSupportGuestPage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "login required"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "blocked"
        )
    );
}

TEST(CorePageAuthSupportTests, DeniesMissingSessionValue) {
    drogular::ApplicationServices services;

    services.add<drogular::SessionStore>(
        drogular::ServiceLifetime::Singleton
    );

    const auto result =
        drogular::test::renderPage<
            CorePageAuthSupportRolePage
        >(&services);

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "access denied"
        )
    );

    EXPECT_TRUE(
        drogular::test::contains(
            result.html,
            "blocked"
        )
    );
}