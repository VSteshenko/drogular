#include <drogular/job_runtime.hpp>
#include <drogular/job_events.hpp>
#include <drogular/runtime_diagnostics.hpp>

#include <cassert>
#include <chrono>
#include <future>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

int main() {
    RuntimeDiagnostics diagnostics;
    {
        JobRuntime jobs(1, 8, &diagnostics);
        assert(diagnostics.snapshot().jobs.workerCapacity == 1);
        std::promise<void> release;
        auto gate = release.get_future().share();
        const auto first = jobs.submit("blocking", [gate](JobContext& ctx) {
            ctx.reportProgress(40);
            gate.wait();
        });
        for (int i = 0; i < 1000 && jobs.find(first)->status != JobStatus::Running; ++i) {
            std::this_thread::sleep_for(1ms);
        }
        assert(jobs.find(first)->status == JobStatus::Running);
        const auto second = jobs.submit("queued", [](JobContext&) {});
        auto snap = diagnostics.snapshot().jobs;
        assert(snap.running == 1 && snap.queued == 1 && snap.workerCapacity == 1);
        assert(snap.queueCapacity == 8);
        assert(snap.peakQueued >= 1 && snap.peakRunning == 1);
        assert(snap.workerUtilizationBps == 10000);
        assert(snap.queueSaturationBps == 1250);
        assert(snap.queueWaitSamples == 1);
        assert(snap.queueWaitAverageMs == snap.queueWaitTotalMs);
        assert(snap.executionSamples == 0);

        assert(jobs.requestCancel(second));
        snap = diagnostics.snapshot().jobs;
        assert(snap.running == 1 && snap.queued == 0 && snap.cancelled == 1);
        release.set_value();
        for (int i = 0; i < 1000 && jobs.find(first)->status != JobStatus::Completed; ++i) {
            std::this_thread::sleep_for(1ms);
        }
        assert(jobs.find(first)->status == JobStatus::Completed);
        for (int i = 0; i < 1000 && diagnostics.snapshot().jobs.completed == 0; ++i) {
            std::this_thread::sleep_for(1ms);
        }
        snap = diagnostics.snapshot().jobs;
        assert(snap.running == 0 && snap.queued == 0);
        assert(snap.completed == 1 && snap.cancelled == 1);
        assert(snap.queueWaitSamples == 1);
        assert(snap.executionSamples == 1);
        assert(snap.queueWaitMaxMs == snap.queueWaitTotalMs);
        assert(snap.executionMaxMs == snap.executionTotalMs);
        assert(snap.executionAverageMs == snap.executionTotalMs);
        assert(snap.workerUtilizationBps == 0 && snap.queueSaturationBps == 0);
        assert(snap.eventsPublished >= 5 && snap.progressUpdates == 1);
        assert(snap.activeSubscriptions == 0);
        {
            auto subscriber = jobs.events().subscribe();
            assert(diagnostics.snapshot().jobs.activeSubscriptions == 1);
        }
        assert(diagnostics.snapshot().jobs.activeSubscriptions == 0);
        {
            auto old = jobs.events().subscribe();
            auto replacement = jobs.events().subscribe();
            old = std::move(replacement);
            assert(diagnostics.snapshot().jobs.activeSubscriptions == 1);
        }
        assert(diagnostics.snapshot().jobs.activeSubscriptions == 0);
        jobs.shutdown();
        assert(diagnostics.snapshot().jobs.workerCapacity == 0);
        assert(diagnostics.snapshot().jobs.queueCapacity == 0);
        jobs.shutdown();
        assert(diagnostics.snapshot().jobs.workerCapacity == 0);
        assert(diagnostics.snapshot().jobs.queueCapacity == 0);
    }
    {
        JobEventStream stream(2, 1, &diagnostics);
        auto subscriber = stream.subscribe();
        JobSnapshot job; job.id = 1;
        stream.publish(JobEventKind::Queued, job);
        stream.publish(JobEventKind::Started, job);
        assert(subscriber.drain().gap);
        assert(diagnostics.snapshot().jobs.eventGaps >= 1);
    }
    assert(diagnostics.snapshot().jobs.activeSubscriptions == 0);
    // Explicit sample counts keep averages correct even when queued jobs cancel.
    diagnostics.recordJobQueueWait(20);
    diagnostics.recordJobQueueWait(40);
    diagnostics.recordJobExecution(50);
    diagnostics.recordJobExecution(90);
    const auto final = diagnostics.snapshot().jobs;
    assert(final.queueWaitSamples == 3 && final.queueWaitMaxMs >= 40);
    assert(final.executionSamples == 3 && final.executionMaxMs >= 90);
    assert(final.queueWaitAverageMs == final.queueWaitTotalMs / 3);
    assert(final.executionAverageMs == final.executionTotalMs / 3);
    return 0;
}