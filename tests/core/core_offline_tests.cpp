#include <drogular/app.hpp>
#include <drogular/offline.hpp>

#include <gtest/gtest.h>

TEST(CoreOfflineTests, DefaultsToIndependentLiveStates) {
    const drogular::OfflineState state;

    EXPECT_EQ(state.connection, drogular::ConnectionState::Live);
    EXPECT_EQ(state.data, drogular::DataState::Live);
    EXPECT_EQ(state.capability, drogular::InteractionCapability::ReadWrite);
}

TEST(CoreOfflineTests, RepresentationIdentityIncludesContextAndScope) {
    drogular::RepresentationIdentity english{
        .kind = drogular::RepresentationKind::Fragment,
        .requestKey = "/fragments/projects?page=1&sort=name",
        .context = {.locale = "en"},
        .scope = {.kind = drogular::RepresentationScopeKind::Session, .key = "session-a"}
    };
    auto german = english;
    german.context.locale = "de";

    EXPECT_NE(english, german);
}

TEST(CoreOfflineTests, ApplicationOptionIsOptIn) {
    drogular::ApplicationOptions options;

    EXPECT_FALSE(options.offlineReadModelsEnabled());
    options.setOfflineReadModelsEnabled(true);
    EXPECT_TRUE(options.offlineReadModelsEnabled());
}

TEST(CoreOfflineTests, AppEntryPointEnablesOfflineReadModels) {
    drogular::App app;

    app.offlineReadModels();

    EXPECT_TRUE(app.options().offlineReadModelsEnabled());
}