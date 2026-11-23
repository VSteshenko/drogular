#include <drogular/job_runtime.hpp>
#include <drogular/security/principal_builder.hpp>

#include <cassert>
#include <chrono>
#include <future>
#include <memory>
#include <string>
#include <type_traits>

using namespace std::chrono_literals;

namespace {
drogular::security::SecurityContext identity(const char* subject) {
    auto built = drogular::security::PrincipalBuilder{}
        .issuer("test-provider")
        .subject(subject)
        .role("editor")
        .build();
    assert(built.has_value());
    return drogular::security::SecurityContext::forPrincipal(
        std::move(built.value()), std::string{"test"});
}

template <typename T>
T await(std::future<T>& result) {
    assert(result.wait_for(5s) == std::future_status::ready);
    return result.get();
}
}

int main() {
    static_assert(
        std::is_same_v<decltype(
            std::declval<const drogular::JobContext&>().initiator()),
            std::shared_ptr<const drogular::security::Principal>>
        );
    drogular::JobRuntime jobs(2, 8);
    std::promise<std::string> observedA;
    std::promise<std::string> observedB;
    std::promise<bool> anonymous;
    std::promise<std::string> observedResumable;
    auto resultA = observedA.get_future();
    auto resultB = observedB.get_future();
    auto resultAnonymous = anonymous.get_future();
    auto resultResumable = observedResumable.get_future();
    std::weak_ptr<const drogular::security::Principal> weak;
    {
        auto first = identity("alice");
        weak = first.principal();
        jobs.submit("alice", [&](drogular::JobContext& ctx) {
            const auto initiator = ctx.initiator();
            observedA.set_value(initiator ? initiator->id().subject : "missing");
        }, first);
    }
    assert(!weak.expired());
    jobs.submit("bob", [&](drogular::JobContext& ctx) {
        const auto initiator = ctx.initiator();
        observedB.set_value(initiator ? initiator->id().subject : "missing");
    }, identity("bob"));
    jobs.submit("anonymous", [&](drogular::JobContext& ctx) {
        anonymous.set_value(ctx.initiator() == nullptr);
    });
    jobs.submitResumable("resumable", [&](drogular::JobContext& ctx) {
        const auto initiator = ctx.initiator();
        observedResumable.set_value(initiator ? initiator->id().subject : "missing");
    }, identity("carol"));

    assert(await(resultA) == "alice");
    assert(await(resultB) == "bob");
    assert(await(resultAnonymous));
    assert(await(resultResumable) == "carol");
    jobs.shutdown();
}