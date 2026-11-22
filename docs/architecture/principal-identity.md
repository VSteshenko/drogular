# Principal & Identity

The `drogular::security` types provide a framework-independent identity model.
They do not authenticate credentials or grant permissions.

```cpp
#include <drogular/security/principal_builder.hpp>

auto result = drogular::security::PrincipalBuilder{}
    .issuer("https://identity.example")
    .subject("user-123")
    .type(drogular::security::PrincipalType::User)
    .role("editor")
    .build();

if (!result) {
    // Handle result.error().code / result.error().message.
}
```

`PrincipalId` pairs issuer and subject; the subject is not globally unique.
Issuer normalization and identity verification are responsibilities of a future
trusted authentication adapter. `PrincipalBuilder` only validates structural
invariants, and does not make caller-provided values trustworthy.

`Principal` is immutable through its public interface and can be copied or moved.
Roles are unscoped identity metadata, **not** authorization grants; repeated
roles are deduplicated preserving first occurrence. Blank identity values and
roles produce typed errors through the C++20 `drogular::Expected` API.

## Typed claims and attributes

`Principal::claims()` and `Principal::attributes()` expose separate, read-only
snapshots. Both use `ClaimValue = std::variant<bool, std::int64_t, double,
std::string, std::vector<std::string>>`. `get<T>(name)` performs an **exact**
type lookup and returns `std::optional<T>` (no conversions); `find(name)`
returns a pointer to the variant or `nullptr` if the name is absent.

```cpp
using namespace drogular::security;
auto result = PrincipalBuilder{}
    .issuer("identity-provider").subject("user-42")
    .claim("department", std::string{"finance"})
    .attribute("locale", std::string{"de"})
    .build();
if (result) {
    auto department = result->claims().get<std::string>("department");
    auto locale = result->attributes().get<std::string>("locale");
}
```

Empty/whitespace-only names and duplicate names within either namespace are
rejected with `PrincipalError`. The reserved identity claims `iss` and `sub`
are permitted only as strings matching the PrincipalId values exactly.
The two namespaces do not overlap: a name may appear in both.

**Trust boundary:** `Claims` contains caller-supplied values, not automatically
verified claims. Only a future authentication adapter may establish their
trustworthiness; `Attributes` must never be implicitly used as trusted identity
claims. Neither class offers public mutations after construction.
