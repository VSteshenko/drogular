#include <drogular/detail/request_context_state.hpp>

#include <gtest/gtest.h>

namespace {

class ScopedProbe final {
};

} // namespace

TEST(RequestContextStateTests, StoresRequestServicesAndRouteParameters) {
    drogular::ApplicationServices services;
    auto request = drogon::HttpRequest::newHttpRequest();

    drogular::detail::RequestContextState state(
        request,
        &services
    );

    state.setRouteParam("id", "42");

    EXPECT_EQ(state.request(), request);
    EXPECT_EQ(state.services(), &services);
    ASSERT_TRUE(state.routeParam("id").has_value());
    EXPECT_EQ(*state.routeParam("id"), "42");
    EXPECT_FALSE(state.routeParam("missing").has_value());
}

TEST(RequestContextStateTests, OwnsOneServiceScopeForItsLifetime) {
    drogular::ApplicationServices services;
    services.addScoped<ScopedProbe>(
        [] {
            return std::make_shared<ScopedProbe>();
        }
    );

    drogular::detail::RequestContextState state(
        nullptr,
        &services
    );

    auto first =
        services.service<ScopedProbe>(state.serviceScope());
    auto second =
        services.service<ScopedProbe>(state.serviceScope());

    ASSERT_NE(first, nullptr);
    EXPECT_EQ(first, second);
}

TEST(RequestContextStateTests, DifferentStatesHaveDifferentServiceScopes) {
    drogular::ApplicationServices services;
    services.addScoped<ScopedProbe>(
        [] {
            return std::make_shared<ScopedProbe>();
        }
    );

    drogular::detail::RequestContextState firstState(
        nullptr,
        &services
    );
    drogular::detail::RequestContextState secondState(
        nullptr,
        &services
    );

    auto first =
        services.service<ScopedProbe>(firstState.serviceScope());
    auto second =
        services.service<ScopedProbe>(secondState.serviceScope());

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_NE(first, second);
}