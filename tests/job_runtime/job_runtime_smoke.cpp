#include <drogular/job_runtime.hpp>
#include <drogular/runtime_diagnostics.hpp>

#include <cassert>
#include <chrono>
#include <future>
#include <stdexcept>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

int main() {
    RuntimeDiagnostics diagnostics;
    JobRuntime jobs(1, 16, &diagnostics);
    std::promise<void> started;
    std::promise<void> release;
    auto gate = release.get_future().share();
    auto running = jobs.submit("running", [&](JobContext& ctx) {
        started.set_value();
        gate.wait();
        ctx.throwIfCancellationRequested();
    });
    started.get_future().wait();
    auto queued = jobs.submit("queued", [](JobContext&) {
        assert(false);
    });

    assert(jobs.requestCancel(queued));
    assert(jobs.find(queued)->status == JobStatus::Cancelled);
    assert(jobs.requestCancel(running));
    assert(jobs.find(running)->cancellationRequested);
    release.set_value();
    auto failed = jobs.submit("failed", [](JobContext&) {
        throw std::runtime_error("boom");
    });
    auto complete = jobs.submit("completed", [](JobContext& ctx) {
        ctx.reportProgress(50, "half");
    });
    for (int n = 0; n < 500 && jobs.find(complete)->status != JobStatus::Completed; ++n) {
        std::this_thread::sleep_for(1ms);
    }
    assert(jobs.find(running)->status == JobStatus::Cancelled);
    assert(jobs.find(failed)->status == JobStatus::Failed);
    assert(jobs.find(failed)->error == "boom");
    assert(jobs.find(complete)->status == JobStatus::Completed);
    assert(jobs.find(complete)->progress == 100);
    auto s = diagnostics.snapshot().jobs;
    assert(s.submitted == 4 && s.started == 3);
    assert(s.completed == 1 && s.failed == 1 && s.cancelled == 2);
    assert(s.progressUpdates == 1 && s.cancellationRequests == 1);
    assert(!jobs.requestCancel(99999));
    jobs.shutdown();

    bool rejected = false;
    try {
        jobs.submit("late", [](JobContext&) {});
    }
    catch (const std::runtime_error&) {
        rejected = true;
    }
    assert(rejected);
}