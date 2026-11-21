#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <variant>

namespace drogular {

// Explicitly distinguishes an error from a successful value, even if T == E.
template<class E>
class Unexpected final {
public:
    explicit Unexpected(E error) : error_(std::move(error)) {}

    const E& error() const & noexcept { return error_; }
    E& error() & noexcept { return error_; }
    const E&& error() const && noexcept { return std::move(error_); }
    E&& error() && noexcept { return std::move(error_); }

private:
    E error_;
};

template<class E>
Unexpected(E) -> Unexpected<E>;

template<class T, class E>
class Expected final {
    static_assert(!std::is_reference_v<T> && !std::is_void_v<T>,
                  "Expected<T, E> requires a non-void, non-reference T; use Expected<void, E> for void");
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>,
                  "Expected<T, E> requires a non-void, non-reference E");

public:
    Expected() requires std::default_initializable<T>
        : state_(std::in_place_index<0>) {}

    template<class U = T>
        requires (!std::same_as<std::remove_cvref_t<U>, Expected> &&
                  !std::same_as<std::remove_cvref_t<U>, Unexpected<E>> &&
                  std::constructible_from<T, U&&>)
    Expected(U&& value) : state_(std::in_place_index<0>, std::forward<U>(value)) {}

    template<class... Args>
        requires std::constructible_from<T, Args&&...>
    explicit Expected(std::in_place_t, Args&&... args)
        : state_(std::in_place_index<0>, std::forward<Args>(args)...) {}

    Expected(const Unexpected<E>& error) requires std::copy_constructible<E>
        : state_(std::in_place_index<1>, error.error()) {}
    Expected(Unexpected<E>&& error)
        : state_(std::in_place_index<1>, std::move(error).error()) {}

    Expected(const Expected&) = default;
    Expected(Expected&&) = default;
    Expected& operator=(const Expected&) = default;
    Expected& operator=(Expected&&) = default;

    [[nodiscard]] bool has_value() const noexcept { return state_.index() == 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }

    T& value() & { checkValue(); return std::get<0>(state_); }
    const T& value() const & { checkValue(); return std::get<0>(state_); }
    T&& value() && { checkValue(); return std::get<0>(std::move(state_)); }
    const T&& value() const && { checkValue(); return std::get<0>(std::move(state_)); }

    E& error() & { checkError(); return std::get<1>(state_); }
    const E& error() const & { checkError(); return std::get<1>(state_); }
    E&& error() && { checkError(); return std::get<1>(std::move(state_)); }
    const E&& error() const && { checkError(); return std::get<1>(std::move(state_)); }

    T& operator*() & { return value(); }
    const T& operator*() const & { return value(); }
    T&& operator*() && { return std::move(*this).value(); }
    const T&& operator*() const && { return std::move(*this).value(); }
    T* operator->() { return std::addressof(value()); }
    const T* operator->() const { return std::addressof(value()); }

    template<class U>
        requires std::copy_constructible<T> && std::convertible_to<U&&, T>
    T value_or(U&& fallback) const & {
        return has_value() ? value() : static_cast<T>(std::forward<U>(fallback));
    }
    template<class U>
        requires std::move_constructible<T> && std::convertible_to<U&&, T>
    T value_or(U&& fallback) && {
        return has_value() ? std::move(*this).value()
                           : static_cast<T>(std::forward<U>(fallback));
    }

private:
    void checkValue() const {
        if (!has_value()) throw std::logic_error("Expected has no value");
    }
    void checkError() const {
        if (state_.index() != 1) throw std::logic_error("Expected has no error");
    }
    std::variant<T, E> state_;
};

template<class E>
class Expected<void, E> final {
    static_assert(!std::is_reference_v<E> && !std::is_void_v<E>,
                  "Expected<void, E> requires a non-void, non-reference E");
public:
    Expected() = default;
    Expected(const Unexpected<E>& error) requires std::copy_constructible<E>
        : error_(error.error()) {}
    Expected(Unexpected<E>&& error) : error_(std::move(error).error()) {}

    [[nodiscard]] bool has_value() const noexcept { return !error_.has_value(); }
    [[nodiscard]] explicit operator bool() const noexcept { return has_value(); }
    void value() const {
        if (!has_value()) throw std::logic_error("Expected has no value");
    }
    E& error() & { checkError(); return *error_; }
    const E& error() const & { checkError(); return *error_; }
    E&& error() && { checkError(); return std::move(*error_); }
    const E&& error() const && { checkError(); return std::move(*error_); }
private:
    void checkError() const {
        if (!error_) throw std::logic_error("Expected has no error");
    }
    std::optional<E> error_;
};

} // namespace drogular