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
        assert(jobs.requestCancel(second));
        snap = diagnostics.snapshot().jobs;
        assert(snap.running == 1 && snap.queued == 0 && snap.cancelled == 1);
        release.set_value();
        for (int i = 0; i < 1000 && jobs.find(first)->status != JobStatus::Completed; ++i) {
            std::this_thread::sleep_for(1ms);
        }
        assert(jobs.find(first)->status == JobStatus::Completed);
        snap = diagnostics.snapshot().jobs;
        assert(snap.running == 0 && snap.queued == 0);
        assert(snap.completed == 1 && snap.cancelled == 1);
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
        jobs.shutdown();
        assert(diagnostics.snapshot().jobs.workerCapacity == 0);
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

    return 0;
}