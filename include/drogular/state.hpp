#pragma once

#include <utility>
#include <functional>
#include <vector>

namespace drogular {

/**
 * Stores a mutable value.
 *
 * State<T> is the smallest building block for
 * future component and application state.
 */
template <typename T>
class State {
public:
    using Callback = std::function<void(const T&)>;

    State() = default;

    explicit State(T value)
        : value_(std::move(value)) {
    }

    const T& value() const {
        return value_;
    }

    T& value() {
        return value_;
    }

    void set(T value) {
        value_ = std::move(value);

        for (auto& callback : subscribers_) {
            callback(value_);
        }
    }

    void subscribe(Callback callback) {
        subscribers_.push_back(
            std::move(callback)
        );
    }

private:
    T value_{};

    std::vector<Callback> subscribers_;
};

} // namespace drogular