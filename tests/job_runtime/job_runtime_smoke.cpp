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
        ctx.setResult(std::string("ok"));
    });
    for (int n = 0; n < 500 && jobs.find(complete)->status != JobStatus::Completed; ++n) {
        std::this_thread::sleep_for(1ms);
    }
    assert(jobs.find(running)->status == JobStatus::Cancelled);
    assert(jobs.find(failed)->status == JobStatus::Failed);
    assert(jobs.find(failed)->error == "boom");
    assert(jobs.find(failed)->failure->code == "exception");
    assert(!jobs.find(failed)->result.has_value());
    assert(jobs.find(complete)->status == JobStatus::Completed);
    assert(jobs.find(complete)->progress == 100);
    assert(jobs.find(complete)->resultAs<std::string>() != nullptr);
    assert(*jobs.find(complete)->resultAs<std::string>() == "ok");
    assert(jobs.find(complete)->resultAs<int>() == nullptr);
    assert(jobs.find(complete)->queueDuration.has_value());
    assert(jobs.find(complete)->executionDuration.has_value());
    assert(!jobs.find(queued)->queueDuration.has_value());
    assert(!jobs.find(running)->result.has_value());
    auto coded = jobs.submit("coded", [](JobContext& ctx) {
        ctx.setResult(23);
        throw JobFailure("bad_input", "Input rejected");
    });
    for (int n = 0; n < 500 && jobs.find(coded)->status != JobStatus::Failed; ++n) {
        std::this_thread::sleep_for(1ms);
    }
    assert(jobs.find(coded)->failure->code == "bad_input");
    assert(jobs.find(coded)->failure->message == "Input rejected");
    assert(!jobs.find(coded)->result.has_value());
    auto s = diagnostics.snapshot().jobs;
    assert(s.submitted == 5 && s.started == 4);
    assert(s.completed == 1 && s.failed == 2 && s.cancelled == 2);
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