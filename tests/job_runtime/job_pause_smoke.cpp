#include <drogular/job_runtime.hpp>
#include <drogular/runtime_diagnostics.hpp>
#include <cassert>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

template<class Pred> void until(Pred pred) {
    for (int i=0; i<1000 && !pred(); ++i) {
        std::this_thread::sleep_for(2ms);
    }
    assert(pred());
}

int main() {
    RuntimeDiagnostics diagnostics;
    JobRuntime jobs(1, 8, &diagnostics);
    std::atomic<int> steps{0};
    std::promise<void> started;
    const auto id = jobs.submit("pause", [&](JobContext& ctx) {
        started.set_value();
        for (int i=0; i<40; ++i) {
            ctx.waitIfPaused();
            ++steps;
            std::this_thread::sleep_for(5ms);
        }
    });
    started.get_future().wait();
    assert(jobs.requestPause(id));
    until([&] {
        return jobs.find(id)->status == JobStatus::Paused;
    });
    const auto before = steps.load();
    std::this_thread::sleep_for(25ms);
    assert(steps.load() == before);
    assert(jobs.find(id)->pauseRequested);
    assert(diagnostics.snapshot().jobs.paused == 1);
    assert(diagnostics.snapshot().jobs.running == 1); // Occupies worker.
    assert(jobs.requestResume(id));
    until([&] {
        return jobs.find(id)->status == JobStatus::Completed;
    });
    assert(jobs.find(id)->pausedDuration.count() > 0);
    assert(diagnostics.snapshot().jobs.paused == 0);

    std::promise<void> gateStarted;
    std::promise<void> release;
    auto future = release.get_future().share();
    auto busy = jobs.submit("busy", [&](JobContext&) {
        gateStarted.set_value();
        future.wait();
    });
    gateStarted.get_future().wait();
    std::atomic<bool> queuedRan{false};
    auto queued = jobs.submit("queued", [&](JobContext&) {
        queuedRan = true;
    });
    assert(jobs.requestPause(queued));
    release.set_value();
    until([&] {
        return jobs.find(busy)->status == JobStatus::Completed;
    });
    std::this_thread::sleep_for(20ms);
    assert(!queuedRan && jobs.find(queued)->status == JobStatus::Queued);
    assert(jobs.requestResume(queued));
    until([&] {
        return jobs.find(queued)->status == JobStatus::Completed;
    });
    assert(queuedRan);

    std::atomic<bool> reached{ false };
    auto cancellable = jobs.submit("cancel-paused", [&](JobContext& ctx) {
        reached = true;
        for (;;) { ctx.waitIfPaused(); std::this_thread::sleep_for(1ms); }
    });
    until([&] {
        return reached.load();
    });
    assert(jobs.requestPause(cancellable));
    until([&] {
        return jobs.find(cancellable)->status == JobStatus::Paused;
    });
    assert(jobs.requestCancel(cancellable));
    until([&] {
        return jobs.find(cancellable)->status == JobStatus::Cancelled;
    });
    assert(diagnostics.snapshot().jobs.paused == 0);

    auto halted = jobs.submit("shutdown-paused", [](JobContext& ctx) {
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
    });
    until([&] {
        return jobs.find(halted)->status == JobStatus::Running;
    });
    assert(jobs.requestPause(halted));
    until([&] {
        return jobs.find(halted)->status == JobStatus::Paused;
    });
    jobs.shutdown();
    assert(jobs.find(halted)->status == JobStatus::Cancelled);
    assert(diagnostics.snapshot().jobs.paused == 0);
}