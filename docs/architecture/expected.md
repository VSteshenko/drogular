# Expected — C++20 Core Result Type

`<drogular/expected.hpp>` provides `drogular::Expected<T, E>` for operations that either produce a value or return 
a typed error without using exceptions for normal failure paths. The component is header-only, requires C++20, and 
has no additional dependencies.

```cpp
#include <drogular/expected.hpp>

struct ValidationError { std::string message; };

drogular::Expected<int, ValidationError> parsePositive(int input) {
    if (input <= 0)
        return drogular::Unexpected{ValidationError{"must be positive"}};
    return input;
}

auto result = parsePositive(10);
if (!result) {
    // result.error()
} else {
    // result.value() / *result
}
```

- Success: construct from `T` or `std::in_place`, or default-construct when `T` is default constructible.
- Error: construct explicitly from `drogular::Unexpected<E>`; this remains unambiguous when `T` and `E` are the same type.
- Check `has_value()` or use the explicit boolean conversion before `value()`, `operator*`, or `operator->`.
- Incorrect `value()` / `error()` access throws `std::logic_error`; it is a programming error, not an expected failure path.
- `Expected<void, E>` represents operations that succeed without a returned value.
- Move-only value and error types are supported; copying is available only when the underlying types allow it.
- Unlike C++23 `std::expected`, this is a deliberately small C++20 subset: monadic operations (`and_then`, `transform`, `or_else`) and reference-valued results are not supported.
