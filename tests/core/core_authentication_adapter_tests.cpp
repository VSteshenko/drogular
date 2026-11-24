#include <drogular/security/authentication_adapter.hpp>
#include <drogular/security/principal_builder.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

using namespace drogular::security;

namespace {

class Request final : public AuthenticationRequest {
public:
    std::optional<std::string_view> header(std::string_view name) const override {
        if (name == "Authorization") {
            return authorization;
        }
        return std::nullopt;
    }

    std::optional<std::string_view> sessionValue(std::string_view) const override {
        return std::nullopt;
    }

    std::optional<std::string_view> authorization;
};

SecurityContext makeIdentity() {
    auto result = PrincipalBuilder{}
        .issuer("test.issuer")
        .subject("user-1")
        .build();
    if (!result) {
        throw std::runtime_error("test principal failed");
    }
    return SecurityContext::forPrincipal(std::move(result.value()), std::string{"bearer"});
}

class Adapter final : public AuthenticationAdapter {
public:
    enum class Mode {
        Skip,
        Accept,
        Reject,
        Anonymous,
        Throw
    };

    explicit Adapter(Mode mode, int& calls) : mode_(mode), calls_(calls) {}

    AuthenticationResult authenticate(const AuthenticationRequest& request) const override {
        ++calls_;
        static_cast<void>(request.header("Authorization"));
        switch (mode_) {
        case Mode::Skip:
            return std::optional<SecurityContext>{};
        case Mode::Accept:
            return std::optional<SecurityContext>{makeIdentity()};
        case Mode::Reject:
            return drogular::Unexpected{AuthenticationError{AuthenticationErrorCode::InvalidCredentials}};
        case Mode::Anonymous:
            return std::optional<SecurityContext>{SecurityContext::anonymous()};
        case Mode::Throw:
            throw std::runtime_error("sensitive details must not escape");
        }
        return std::optional<SecurityContext>{};
    }

private:
    Mode mode_;
    int& calls_;
};

}

TEST(CoreAuthenticationAdapter, EmptyAndSkippedPipelineAreAnonymous) {
    Request request;
    AuthenticationPipeline pipeline;
    auto empty = pipeline.authenticate(request);
    ASSERT_TRUE(empty);
    EXPECT_FALSE(empty->authenticated());

    int calls = 0;
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Skip, calls));
    auto skipped = pipeline.authenticate(request);
    ASSERT_TRUE(skipped);
    EXPECT_FALSE(skipped->authenticated());
    EXPECT_EQ(calls, 1);
}

TEST(CoreAuthenticationAdapter, FirstSuccessWinsAndPreservesIdentity) {
    Request request;
    request.authorization = "Bearer token";
    AuthenticationPipeline pipeline;
    int skipped = 0, accepted = 0, ignored = 0;
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Skip, skipped));
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Accept, accepted));
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Reject, ignored));
    auto result = pipeline.authenticate(request);

    ASSERT_TRUE(result);
    ASSERT_TRUE(result->authenticated());
    EXPECT_EQ(result->principal()->id().subject, "user-1");
    EXPECT_EQ(result->authenticationMethod(), std::optional<std::string_view>{"bearer"});
    EXPECT_EQ(skipped, 1);
    EXPECT_EQ(accepted, 1);
    EXPECT_EQ(ignored, 0);
}

TEST(CoreAuthenticationAdapter, RejectionNeverFallsThrough) {
    Request request;
    AuthenticationPipeline pipeline;
    int rejected = 0, ignored = 0;
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Reject, rejected));
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Accept, ignored));
    auto result = pipeline.authenticate(request);

    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, AuthenticationErrorCode::InvalidCredentials);
    EXPECT_EQ(rejected, 1);
    EXPECT_EQ(ignored, 0);
}

TEST(CoreAuthenticationAdapter, AnonymousSuccessIsInvalid) {
    Request request;
    AuthenticationPipeline pipeline;
    int invalid = 0;
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Anonymous, invalid));
    auto result = pipeline.authenticate(request);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, AuthenticationErrorCode::InvalidAdapterResult);
}

TEST(CoreAuthenticationAdapter, ExceptionsFailClosed) {
    Request request;
    AuthenticationPipeline pipeline;
    int thrown = 0;
    pipeline.add(std::make_shared<Adapter>(Adapter::Mode::Throw, thrown));
    auto result = pipeline.authenticate(request);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.error().code, AuthenticationErrorCode::InternalError);
}