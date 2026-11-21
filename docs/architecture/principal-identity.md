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
