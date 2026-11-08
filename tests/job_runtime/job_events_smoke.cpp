#include <drogular/job_events.hpp>
#include <drogular/job_sse.hpp>
#include <drogular/job_runtime.hpp>

#include <cassert>
#include <chrono>
#include <future>
#include <thread>

using namespace drogular;
using namespace std::chrono_literals;

int main() {
    JobEventStream stream(3, 2);
    auto subscriber = stream.subscribe();
    JobSnapshot a; a.id = 7;
    JobSnapshot b; b.id = 8;
    stream.publish(JobEventKind::Queued, a);
    stream.publish(JobEventKind::Started, a);
    auto batch = subscriber.drain();
    assert(!batch.gap && batch.events.size() == 2);
    assert(batch.events[0].sequence == 1 && batch.events[1].sequence == 2);
    auto frame = encodeJobSse(batch.events[0]);
    assert(frame.find("id: 1\nevent: job.queued\ndata: ") == 0);
    assert(frame.ends_with("\n\n"));
    assert(encodeJobSseReset().find("job.reset") != std::string::npos);
    assert(encodeJobSseHeartbeat() == ": heartbeat\n\n");
    assert(!isTerminalJobEvent(JobEventKind::Progress));
    assert(isTerminalJobEvent(JobEventKind::Completed));
    assert(isTerminalJobEvent(JobEventKind::Failed));
    assert(isTerminalJobEvent(JobEventKind::Cancelled));
    assert(!isTerminalJobStatus(JobStatus::Running));
    assert(isTerminalJobStatus(JobStatus::Completed));
    auto filtered = stream.subscribe(7, 1);
    batch = filtered.drain();
    assert(!batch.gap && batch.events.size() == 1);
    assert(batch.events[0].job.id == 7);
    stream.publish(JobEventKind::Progress, b);
    stream.publish(JobEventKind::Progress, a);
    stream.publish(JobEventKind::Completed, a);
    batch = subscriber.drain();
    assert(batch.gap && batch.events.size() == 2);
    assert(batch.events[0].sequence == 4 && batch.events[1].sequence == 5);
    auto old = stream.subscribe(std::nullopt, 1);
    batch = old.drain();
    assert(batch.gap && batch.events.size() == 2);
    auto future = stream.subscribe(std::nullopt, 100);
    assert(future.drain().gap);
    batch = filtered.drain();
    assert(!batch.gap && batch.events.size() == 2);
    auto wake = stream.subscribe(8);
    std::thread writer([&] {
        std::this_thread::sleep_for(10ms);
        stream.publish(JobEventKind::Completed, b);
    });
    batch = wake.waitFor(1s);
    writer.join();
    assert(!batch.gap && batch.events.size() == 1);
    assert(batch.events.front().job.id == 8);

    JobRuntime runtime(1, 16);
    auto feed = runtime.events().subscribe();
    std::promise<void> release;
    const auto gate = release.get_future().share();
    const auto id = runtime.submit("task", [&](JobContext& ctx) {
        ctx.reportProgress(50, "half");
        gate.wait();
    });
    JobEventBatch received;
    for (int i = 0; i < 100; ++i) {
        auto item = feed.waitFor(20ms);
        received.events.insert(received.events.end(), item.events.begin(), item.events.end());
        if (received.events.size() >= 3) {
            break;
        }
    }
    assert(received.events.size() >= 3);
    assert(received.events[0].kind == JobEventKind::Queued);
    assert(received.events[1].kind == JobEventKind::Started);
    assert(received.events[2].kind == JobEventKind::Progress);
    release.set_value();
    for (int i = 0; i < 100 && runtime.find(id)->status != JobStatus::Completed; ++i) {
        std::this_thread::sleep_for(5ms);
    }
    assert(runtime.find(id)->status == JobStatus::Completed);
    auto last = feed.waitFor(1s);
    assert(last.events.size() == 1 && last.events[0].kind == JobEventKind::Completed);
    runtime.shutdown();
}