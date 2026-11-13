#include <drogular/job_runtime.hpp>
#include <drogular/job_events.hpp>

#include <cassert>
#include <chrono>
#include <future>
#include <thread>
#include <vector>

using namespace drogular;
using namespace std::chrono_literals;

int main() {
    JobRuntime jobs(1, 8, nullptr, 2);
    assert(!jobs.isStopping());
    std::promise<void> started;
    std::promise<void> release;
    auto gate = release.get_future().share();
    const auto running = jobs.submit("running", [&](JobContext& ctx) {
        started.set_value();
        gate.wait();
        ctx.setResult(77);
    });
    started.get_future().wait();
    const auto cancelled = jobs.submit("cancelled", [](JobContext&) {
        assert(false);
    });
    assert(jobs.requestCancel(cancelled));
    assert(jobs.find(cancelled)->status == JobStatus::Cancelled);
    const auto pending = jobs.submit("pending", [](JobContext& ctx) {
        ctx.setResult(42);
    });
    assert(jobs.find(running).has_value());
    assert(jobs.find(pending)->status == JobStatus::Queued);
    release.set_value();
    for (int i = 0; i < 1000; ++i) {
        const auto snapshot = jobs.find(pending);
        if (snapshot && snapshot->status == JobStatus::Completed) {
            break;
        }
        std::this_thread::sleep_for(1ms);
    }
    assert(jobs.find(pending)->status == JobStatus::Completed);
    assert(jobs.find(cancelled)->status == JobStatus::Cancelled);
    // Two newest terminal IDs remain; the older running Job is evicted.
    assert(!jobs.find(running));
    const auto saved = jobs.find(pending);
    assert(saved && saved->resultAs<int>() && *saved->resultAs<int>() == 42);
    assert(jobs.list().size() == 2);
    assert(jobs.pruneTerminalJobs(0) == 2);
    assert(jobs.list().empty());

    // Eviction never removes an active or queued job even with keepLatest=0.
    std::promise<void> startedAgain;
    std::promise<void> releaseAgain;
    auto gateAgain = releaseAgain.get_future().share();
    const auto active = jobs.submit("active", [&](JobContext&) {
        startedAgain.set_value(); gateAgain.wait();
    });
    startedAgain.get_future().wait();
    const auto queued = jobs.submit("queued", [](JobContext&) {});
    assert(jobs.pruneTerminalJobs(0) == 0);
    assert(jobs.find(active) && jobs.find(queued));
    releaseAgain.set_value();
    jobs.shutdown();
    assert(jobs.isStopping());
}