#pragma once

#include <map>
#include <string>
#include <utility>

namespace drogular {

/** Controls whether a read interaction may use an offline representation. */
enum class OfflineReadPolicy {
    Disabled,
    Exact
};

/** Identifies the kind of server-rendered representation being stored. */
enum class RepresentationKind {
    Fragment,
    Shell
};

/** Describes the lifetime/security boundary of a cached representation. */
enum class RepresentationScopeKind {
    Session,
    AuthenticatedPrincipal,
    Application,
    Custom
};

struct RepresentationScope {
    RepresentationScopeKind kind = RepresentationScopeKind::Session;
    std::string key;

    friend bool operator==(const RepresentationScope&, const RepresentationScope&) = default;
};

/**
 * Rendering dimensions that affect representation identity.
 *
 * Locale is intentionally a first-class dimension because the same request
 * may produce different server-rendered HTML for different locales. Additional
 * application-defined dimensions can be added without changing the identity
 * model.
 */
struct RepresentationContext {
    std::string locale;
    std::map<std::string, std::string> dimensions;

    friend bool operator==(const RepresentationContext&, const RepresentationContext&) = default;
};

/**
 * Stable logical identity for an offline representation.
 *
 * requestKey is the normalized request identity (for example a normalized GET
 * path plus sorted query parameters). Storage adapters decide how this value is
 * serialized; callers must not depend on a particular IndexedDB key format.
 */
struct RepresentationIdentity {
    RepresentationKind kind = RepresentationKind::Fragment;
    std::string requestKey;
    RepresentationContext context;
    RepresentationScope scope;

    friend bool operator==(const RepresentationIdentity&, const RepresentationIdentity&) = default;
};

enum class ConnectionState {
    Live,
    Offline,
    Reconnecting
};

enum class DataState {
    Live,
    Cached,
    Unavailable
};

enum class InteractionCapability {
    ReadWrite,
    ReadOnly
};

/** Snapshot of the independent runtime states used by offline-capable UI. */
struct OfflineState {
    ConnectionState connection = ConnectionState::Live;
    DataState data = DataState::Live;
    InteractionCapability capability = InteractionCapability::ReadWrite;

    friend bool operator==(const OfflineState&, const OfflineState&) = default;
};

} // namespace drogular