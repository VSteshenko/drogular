#include <drogular/job_events.hpp>
#include <drogular/runtime_diagnostics.hpp>

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace drogular {

struct JobEventStream::Subscriber {
    explicit Subscriber(std::optional<JobId> filter) : job(filter) {}
    std::optional<JobId> job;
    std::deque<JobEvent> pending;
    bool gap = false;
    bool closed = false;
    std::condition_variable ready;
};

struct JobEventStream::State {
    State(std::size_t replayLimit, std::size_t subscriberLimit, RuntimeDiagnostics* diag)
        : diagnostics(diag), replayCapacity(replayLimit), subscriberCapacity(subscriberLimit)
    {
    }

    RuntimeDiagnostics* diagnostics;
    mutable std::mutex mutex;
    const std::size_t replayCapacity;
    const std::size_t subscriberCapacity;
    std::uint64_t sequence = 0;
    std::deque<JobEvent> replay;
    std::vector<std::weak_ptr<Subscriber>> subscribers;
};

JobEventStream::JobEventStream(
    std::size_t replayCapacity,
    std::size_t subscriberCapacity,
    RuntimeDiagnostics* diagnostics
) {
    if (!replayCapacity || !subscriberCapacity) {
        throw std::invalid_argument("Event capacities must be positive");
    }
    state_ = std::make_shared<State>(replayCapacity, subscriberCapacity, diagnostics);
}

JobEventStream::Subscription::Subscription(std::shared_ptr<State> state,
                                            std::shared_ptr<Subscriber> subscriber)
    : state_(std::move(state)), subscriber_(std::move(subscriber))
{
}

JobEventStream::Subscription& JobEventStream::Subscription::operator=(Subscription&& other) noexcept {
    if (this != &other) {
        if (subscriber_) {
            std::lock_guard lock(state_->mutex);
            if (!subscriber_->closed) {
                subscriber_->closed = true;
                if (state_->diagnostics) {
                    state_->diagnostics->recordJobSubscriptionClosed();
                }
                subscriber_->ready.notify_all();
            }
        }
        state_ = std::move(other.state_);
        subscriber_ = std::move(other.subscriber_);
    }
    return *this;
}

JobEventStream::Subscription::~Subscription() {
    if (subscriber_) {
        std::lock_guard lock(state_->mutex);
        if (!subscriber_->closed) {
            subscriber_->closed = true;
            if (state_->diagnostics) {
                state_->diagnostics->recordJobSubscriptionClosed();
            }
        }
        subscriber_->ready.notify_all();
    }
}

JobEventBatch JobEventStream::Subscription::drain() {
    JobEventBatch batch;
    if (!subscriber_) {
        return batch;
    }

    std::lock_guard lock(state_->mutex);
    batch.gap = std::exchange(subscriber_->gap, false);
    batch.events.assign(std::make_move_iterator(subscriber_->pending.begin()),
                        std::make_move_iterator(subscriber_->pending.end()));
    subscriber_->pending.clear();

    return batch;
}

JobEventBatch JobEventStream::Subscription::waitFor(std::chrono::milliseconds timeout) {
    if (!subscriber_) {
        return {};
    }

    {
        std::unique_lock lock(state_->mutex);
        subscriber_->ready.wait_for(lock, timeout, [this] {
            return subscriber_->closed || subscriber_->gap || !subscriber_->pending.empty();
        });
    }

    return drain();
}

JobEventStream::Subscription JobEventStream::subscribe(
    std::optional<JobId> job,
    std::optional<std::uint64_t> afterSequence
) {
    auto subscriber = std::make_shared<Subscriber>(job);
    std::lock_guard lock(state_->mutex);
    if (afterSequence) {
        const auto oldest = state_->replay.empty()
            ? state_->sequence + 1
            : state_->replay.front().sequence;
        // A future cursor is invalid; treat it as a gap rather than suppressing events.
        subscriber->gap =
            (*afterSequence > state_->sequence) || (*afterSequence < oldest - 1);
        for (const auto& event : state_->replay) {
            if (event.sequence > *afterSequence && (!job || event.job.id == *job)) {
                if (subscriber->pending.size() == state_->subscriberCapacity) {
                    subscriber->pending.pop_front();
                    subscriber->gap = true;
                    if (state_->diagnostics) {
                        state_->diagnostics->recordJobEventGap();
                    }
                }
                subscriber->pending.push_back(event);
            }
        }
    }
    state_->subscribers.emplace_back(subscriber);
    if (state_->diagnostics) {
        state_->diagnostics->recordJobSubscriptionOpened();
        if (subscriber->gap) {
            state_->diagnostics->recordJobEventGap();
        }
    }

    return Subscription(state_, std::move(subscriber));
}

std::uint64_t JobEventStream::latestSequence() const {
    std::lock_guard lock(state_->mutex);
    return state_->sequence;
}

void JobEventStream::publish(JobEventKind kind, JobSnapshot snapshot) {
    std::lock_guard lock(state_->mutex);
    JobEvent event{++state_->sequence, kind, std::move(snapshot)};
    if (state_->diagnostics) {
        state_->diagnostics->recordJobEventPublished();
    }
    state_->replay.push_back(event);
    if (state_->replay.size() > state_->replayCapacity) {
        state_->replay.pop_front();
    }
    auto& subscribers = state_->subscribers;
    subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(),
        [&](const std::weak_ptr<Subscriber>& weak) {
            const auto target = weak.lock();
            if (!target || target->closed) {
                return true;
            }
            if (target->job && *target->job != event.job.id) {
                return false;
            }
            if (target->pending.size() == state_->subscriberCapacity) {
                target->pending.pop_front();
                target->gap = true;
                if (state_->diagnostics) state_->diagnostics->recordJobEventGap();
            }
            target->pending.push_back(event);
            target->ready.notify_one();
            return false;
        }), subscribers.end());
}

} // namespace drogular