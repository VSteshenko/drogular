#pragma once

#include <utility>

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
    }

private:
    T value_{};
};

} // namespace drogular