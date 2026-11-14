#include <drogular/job_runtime.hpp>
#include <drogular/job_events.hpp>
#include <drogular/runtime_diagnostics.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

// Keep test checks active in Release builds where NDEBUG disables assert().
#define CHECK(expr) do { \
    if (!(expr)) { \
        std::cerr << "CHECK failed: " #expr << " at " << __FILE__ \
                  << ':' << __LINE__ << '\n'; \
        std::abort(); \
    } \
} while (false)

template<class Pred> void until(Pred pred) {
    for (int i = 0; i < 1000; ++i) {
        if (pred()) return;
        std::this_thread::sleep_for(2ms);
    }
    CHECK(pred());
}

int main() {
    RuntimeDiagnostics diagnostics;
    JobRuntime jobs(1, 8, &diagnostics);
    std::atomic<int> steps{0};
    std::promise<void> started;
    const auto id = jobs.submit("pause", [&](JobContext& ctx) {
        started.set_value();
        for (int i = 0; i < 40; ++i) {
            ctx.waitIfPaused();
            ++steps;
            std::this_thread::sleep_for(5ms);
        }
    });
    started.get_future().wait();
    CHECK(jobs.requestPause(id));
    until([&] {
        return jobs.find(id)->status == JobStatus::Paused;
    });
    const auto before = steps.load();
    std::this_thread::sleep_for(25ms);
    CHECK(steps.load() == before);
    CHECK(jobs.find(id)->pauseRequested);
    CHECK(diagnostics.snapshot().jobs.paused == 1);
    CHECK(diagnostics.snapshot().jobs.running == 1); // Occupies worker.
    CHECK(jobs.requestResume(id));
    until([&] {
        return jobs.find(id)->status == JobStatus::Completed;
    });
    CHECK(jobs.find(id)->pausedDuration.count() > 0);
    CHECK(diagnostics.snapshot().jobs.paused == 0);

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
    CHECK(jobs.requestPause(queued));
    release.set_value();
    until([&] {
        return jobs.find(busy)->status == JobStatus::Completed;
    });
    std::this_thread::sleep_for(20ms);
    CHECK(!queuedRan && jobs.find(queued)->status == JobStatus::Queued);
    CHECK(jobs.requestResume(queued));
    until([&] {
        return jobs.find(queued)->status == JobStatus::Completed;
    });
    CHECK(queuedRan);

    std::atomic<bool> reached{ false };
    auto cancellable = jobs.submit("cancel-paused", [&](JobContext& ctx) {
        reached = true;
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
    });
    until([&] {
        return reached.load();
    });
    CHECK(jobs.requestPause(cancellable));
    until([&] {
        return jobs.find(cancellable)->status == JobStatus::Paused;
    });
    CHECK(jobs.requestCancel(cancellable));
    until([&] {
        return jobs.find(cancellable)->status == JobStatus::Cancelled;
    });
    CHECK(diagnostics.snapshot().jobs.paused == 0);

    auto halted = jobs.submit("shutdown-paused", [](JobContext& ctx) {
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
    });
    until([&] {
        return jobs.find(halted)->status == JobStatus::Running;
    });
    CHECK(jobs.requestPause(halted));
    until([&] {
        return jobs.find(halted)->status == JobStatus::Paused;
    });
    // New resumable API: pause frees the only worker; resume requeues.
    RuntimeDiagnostics releasedDiagnostics;
    JobRuntime released(1, 8, &releasedDiagnostics);
    struct Checkpoint {
        std::atomic<int> step{0};
    };
    auto checkpoint = std::make_shared<Checkpoint>();
    const auto longId = released.submitResumable("long", [checkpoint](JobContext& ctx) {
        while (checkpoint->step.load() < 80) {
            ctx.waitIfPaused();
            ++checkpoint->step;
            std::this_thread::sleep_for(3ms);
        }
    });
    until([&] {
        return checkpoint->step.load() >= 3;
    });
    CHECK(released.requestPause(longId));
    until([&] {
        return released.find(longId)->status == JobStatus::Paused;
    });
    CHECK(releasedDiagnostics.snapshot().jobs.running == 0);
    CHECK(releasedDiagnostics.snapshot().jobs.paused == 1);

    const auto urgent = released.submit("urgent", [](JobContext&) {});
    until([&] {
        return released.find(urgent)->status == JobStatus::Completed;
    });
    CHECK(released.find(longId)->status == JobStatus::Paused);
    auto resumeEvents = released.events().subscribe(longId);
    CHECK(released.requestResume(longId));
    until([&] {
        return released.find(longId)->status == JobStatus::Completed;
    });
    const auto resumeBatch = resumeEvents.drain();
    const auto resumeCount = std::count_if(resumeBatch.events.begin(),
        resumeBatch.events.end(), [](const auto& event) {
            return event.kind == JobEventKind::Resumed;
        });
    CHECK(resumeCount == 1);
    CHECK(releasedDiagnostics.snapshot().jobs.paused == 0);
    CHECK(releasedDiagnostics.snapshot().jobs.running == 0);
    CHECK(releasedDiagnostics.snapshot().jobs.queued == 0);
    // Paused resumable jobs can be cancelled without consuming a worker.
    const auto toCancel = released.submitResumable("cancel", [](JobContext& ctx) {
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
    });
    until([&] {
        return released.find(toCancel)->status == JobStatus::Running;
    });
    CHECK(released.requestPause(toCancel));
    until([&] {
        return released.find(toCancel)->status == JobStatus::Paused;
    });
    CHECK(released.requestCancel(toCancel));
    CHECK(released.find(toCancel)->status == JobStatus::Cancelled);
    CHECK(releasedDiagnostics.snapshot().jobs.paused == 0);
    released.shutdown();

    // Shutdown cancels resumable jobs parked without workers.
    JobRuntime shutdownRuntime(1, 4);
    const auto parking = shutdownRuntime.submitResumable("parking", [](JobContext& ctx) {
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
    });
    until([&] {
        return shutdownRuntime.find(parking)->status == JobStatus::Running;
    });
    CHECK(shutdownRuntime.requestPause(parking));
    until([&] {
        return shutdownRuntime.find(parking)->status == JobStatus::Paused;
    });
    shutdownRuntime.shutdown();
    CHECK(shutdownRuntime.find(parking)->status == JobStatus::Cancelled);

    // Сancelling a queued job releases bounded FIFO
    // capacity immediately, even if the only worker remains blocked.
    RuntimeDiagnostics capacityDiagnostics;
    JobRuntime capacityJobs(1, 1, &capacityDiagnostics);
    std::promise<void> capacityStarted;
    std::promise<void> capacityRelease;
    const auto capacityGate = capacityRelease.get_future().share();
    const auto capacityBusy = capacityJobs.submit("busy", [&](JobContext&) {
        capacityStarted.set_value();
        capacityGate.wait();
    });
    capacityStarted.get_future().wait();
    const auto cancelledQueued = capacityJobs.submit("cancelled-queued", [](JobContext&) {});
    CHECK(capacityJobs.requestCancel(cancelledQueued));
    CHECK(capacityJobs.find(cancelledQueued)->status == JobStatus::Cancelled);
    const auto replacement = capacityJobs.submit("replacement", [](JobContext&) {});
    CHECK(capacityDiagnostics.snapshot().jobs.queued == 1);
    capacityRelease.set_value();
    until([&] {
        return capacityJobs.find(replacement)->status == JobStatus::Completed;
    });
    until([&] {
        return capacityJobs.find(capacityBusy)->status == JobStatus::Completed;
    });
    CHECK(capacityDiagnostics.snapshot().jobs.queued == 0);
    capacityJobs.shutdown();

    jobs.shutdown();
    CHECK(jobs.find(halted)->status == JobStatus::Cancelled);
    CHECK(diagnostics.snapshot().jobs.paused == 0);
}