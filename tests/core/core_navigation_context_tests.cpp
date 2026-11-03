#include <drogular/navigation_context.hpp>

#include <gtest/gtest.h>

TEST(CoreNavigationContextTests, AcceptsLocalTargetsAndPreservesQuery) {
    EXPECT_EQ(
        drogular::NavigationContext::resolve(
            "/projects?search=a%20b&page=2",
            "/projects",
            "/projects"
        ),
        "/projects?search=a%20b&page=2"
    );
    EXPECT_EQ(
        drogular::NavigationContext::resolve(
            "/projects",
            "/projects",
            "/projects"
        ),
        "/projects"
    );
}

TEST(CoreNavigationContextTests, RejectsExternalAndAmbiguousTargets) {
    for (const auto* candidate : {
        "",
        "https://evil.test",
        "//evil.test",
        "/\\evil.test",
        "/projects#fragment",
        "/projects\r\nInjected: yes",
        "/%2f%2fevil.test",
        "/projects%2fedit"
    }) {
        EXPECT_EQ(drogular::NavigationContext::resolve(candidate, "/projects"), "/projects") << candidate;
        EXPECT_FALSE(drogular::NavigationContext::isLocalTarget(candidate)) << candidate;
    }
}

TEST(CoreNavigationContextTests, RestrictsToCollectionWithoutPrefixConfusion) {
    for (const auto* candidate : {
        "/projects-other",
        "/projects/edit",
        "/departments",
        "/projects?x=1#fragment"
    }) {
        EXPECT_EQ(
            drogular::NavigationContext::resolve(
                candidate,
                "/projects",
                "/projects"
            ),
            "/projects"
        );
    }

    EXPECT_EQ(
        drogular::NavigationContext::resolve(
            "/staff/orders/42?returnUrl=x",
            "/staff/orders",
            "/staff/orders",
            true
        ),
        "/staff/orders/42?returnUrl=x"
    );
    EXPECT_EQ(
        drogular::NavigationContext::resolve(
            "/staff/orders-evil",
            "/staff/orders",
            "/staff/orders",
            true
        ),
        "/staff/orders"
    );
}

TEST(CoreNavigationContextTests, PropagatesEncodedReturnTarget) {
    EXPECT_EQ(
        drogular::NavigationContext::propagate(
            "/projects/1/edit",
            "/projects?search=foo&page=2"
        ),
        "/projects/1/edit?returnUrl=%2Fprojects%3Fsearch%3Dfoo%26page%3D2"
    );
    EXPECT_EQ(
        drogular::NavigationContext::propagate(
            "/projects/1?mode=edit",
            "/projects"
        ),
        "/projects/1?mode=edit&returnUrl=%2Fprojects"
    );
    EXPECT_EQ(
        drogular::NavigationContext::propagate(
            "https://evil.test",
            "/projects"
        ),
        "/"
    );
}