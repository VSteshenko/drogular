#include <drogular/security/security_context.hpp>
#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <future>
#include <thread>
#include <vector>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

using namespace drogular::security;

namespace {

Principal makePrincipal() {
    auto result = PrincipalBuilder{}
        .issuer("identity.example")
        .subject("user-123")
        .role("editor")
        .claim("department", std::string{"engineering"})
        .build();
    EXPECT_TRUE(result);
    return std::move(result.value());
}

} // namespace

TEST(CoreSecurityContext, DefaultAndExplicitAnonymousAreUnauthenticated) {
    const SecurityContext initial;
    const auto anonymous = SecurityContext::anonymous();
    EXPECT_FALSE(initial.authenticated());
    EXPECT_FALSE(anonymous.authenticated());
    EXPECT_EQ(initial.principal(), nullptr);
    EXPECT_EQ(anonymous.principal(), nullptr);
    EXPECT_FALSE(initial.authenticationMethod());
    EXPECT_FALSE(anonymous.authenticationMethod());
}

TEST(CoreSecurityContext, HoldsImmutablePrincipalSnapshot) {
    const auto context = SecurityContext::forPrincipal(makePrincipal(), std::string{"test-adapter"});
    ASSERT_TRUE(context.authenticated());
    ASSERT_TRUE(context.principal());
    EXPECT_EQ(context.principal()->id().issuer, "identity.example");
    EXPECT_EQ(context.principal()->id().subject, "user-123");
    EXPECT_TRUE(context.principal()->hasRole("editor"));
    EXPECT_EQ(context.principal()->claims().get<std::string>("department"), "engineering");
    ASSERT_TRUE(context.authenticationMethod());
    EXPECT_EQ(*context.authenticationMethod(), "test-adapter");

    static_assert(std::is_same_v<decltype(std::declval<const SecurityContext&>().principal()),
                                 std::shared_ptr<const Principal>>);
}

TEST(CoreSecurityContext, ContextCopiesShareIdentityAndSurviveSourceLifetime) {
    SecurityContext copy;
    std::weak_ptr<const Principal> weak;
    {
        const auto original = SecurityContext::forPrincipal(makePrincipal());
        copy = original;
        weak = original.principal();
        EXPECT_EQ(copy.principal(), original.principal());
        EXPECT_FALSE(copy.authenticationMethod());
    }
    EXPECT_FALSE(weak.expired());
    EXPECT_EQ(copy.principal()->id().subject, "user-123");

    copy = SecurityContext::anonymous();
    EXPECT_TRUE(weak.expired());
}

TEST(CoreSecurityContext, MoveDoesNotInvalidateDestination) {
    auto original = SecurityContext::forPrincipal(makePrincipal(), std::string{"session"});
    const auto moved = std::move(original);
    ASSERT_TRUE(moved.principal());
    EXPECT_EQ(moved.principal()->id().subject, "user-123");
    EXPECT_EQ(moved.authenticationMethod(), "session");
}

TEST(CoreSecurityContext, CarriesIdentityNotAuthorization) {
    const auto context = SecurityContext::forPrincipal(makePrincipal());
    // SecurityContext has no grant, permission, or credential APIs.
    EXPECT_TRUE(context.authenticated());
    EXPECT_FALSE(context.authenticationMethod());
}

TEST(CoreSecurityContext, DetachedCopiesSupportConcurrentReadOnlyUse) {
    const auto original = SecurityContext::forPrincipal(
        makePrincipal(), std::string{"test-adapter"});
    std::vector<std::future<bool>> readers;
    readers.reserve(8);
    for (int index = 0; index < 8; ++index) {
        readers.emplace_back(std::async(std::launch::async, [context = original] {
            for (int iteration = 0; iteration < 100; ++iteration) {
                const auto principal = context.principal();
                if (!principal || principal->id().subject != "user-123" ||
                    !principal->hasRole("editor") ||
                    principal->claims().get<std::string>("department") != "engineering" ||
                    context.authenticationMethod() != "test-adapter"
                ) {
                    return false;
                }
            }
            return true;
        }));
    }
    for (auto& reader : readers) {
        EXPECT_TRUE(reader.get());
    }
}