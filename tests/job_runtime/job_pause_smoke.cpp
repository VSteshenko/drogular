#include <drogular/job_runtime.hpp>
#include <drogular/job_events.hpp>
#include <drogular/runtime_diagnostics.hpp>
#include <algorithm>
#include <cassert>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

template<class Pred> void until(Pred pred) {
    for (int i = 0; i < 1000 && !pred(); ++i) {
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
        for (int i = 0; i < 40; ++i) {
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
        for (;;) {
            ctx.waitIfPaused();
            std::this_thread::sleep_for(1ms);
        }
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
    assert(released.requestPause(longId));
    until([&] {
        return released.find(longId)->status == JobStatus::Paused;
    });
    assert(releasedDiagnostics.snapshot().jobs.running == 0);
    assert(releasedDiagnostics.snapshot().jobs.paused == 1);

    const auto urgent = released.submit("urgent", [](JobContext&) {});
    until([&] {
        return released.find(urgent)->status == JobStatus::Completed;
    });
    assert(released.find(longId)->status == JobStatus::Paused);
    auto resumeEvents = released.events().subscribe(longId);
    assert(released.requestResume(longId));
    until([&] {
        return released.find(longId)->status == JobStatus::Completed;
    });
    const auto resumeBatch = resumeEvents.drain();
    const auto resumeCount = std::count_if(resumeBatch.events.begin(),
        resumeBatch.events.end(), [](const auto& event) {
            return event.kind == JobEventKind::Resumed;
        });
    assert(resumeCount == 1);
    assert(releasedDiagnostics.snapshot().jobs.paused == 0);
    assert(releasedDiagnostics.snapshot().jobs.running == 0);
    assert(releasedDiagnostics.snapshot().jobs.queued == 0);
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
    assert(released.requestPause(toCancel));
    until([&] {
        return released.find(toCancel)->status == JobStatus::Paused;
    });
    assert(released.requestCancel(toCancel));
    assert(released.find(toCancel)->status == JobStatus::Cancelled);
    assert(releasedDiagnostics.snapshot().jobs.paused == 0);
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
    assert(shutdownRuntime.requestPause(parking));
    until([&] {
        return shutdownRuntime.find(parking)->status == JobStatus::Paused;
    });
    shutdownRuntime.shutdown();
    assert(shutdownRuntime.find(parking)->status == JobStatus::Cancelled);

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
    assert(capacityJobs.requestCancel(cancelledQueued));
    assert(capacityJobs.find(cancelledQueued)->status == JobStatus::Cancelled);
    const auto replacement = capacityJobs.submit("replacement", [](JobContext&) {});
    assert(capacityDiagnostics.snapshot().jobs.queued == 1);
    capacityRelease.set_value();
    until([&] {
        return capacityJobs.find(replacement)->status == JobStatus::Completed;
    });
    until([&] {
        return capacityJobs.find(capacityBusy)->status == JobStatus::Completed;
    });
    assert(capacityDiagnostics.snapshot().jobs.queued == 0);
    capacityJobs.shutdown();

    jobs.shutdown();
    assert(jobs.find(halted)->status == JobStatus::Cancelled);
    assert(diagnostics.snapshot().jobs.paused == 0);
}