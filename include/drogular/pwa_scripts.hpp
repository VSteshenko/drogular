#pragma once

#include <string>

namespace drogular {

/**
 * Provides small reusable JavaScript snippets for PWA features.
 */
class PwaScripts {
public:
    /**
     * Creates a service worker registration script.
     *
     * The default path is /service-worker.js because service workers
     * usually need to be served from the application root to control
     * the whole application scope.
     */
    static std::string serviceWorkerRegistration(
        const std::string& path = "/service-worker.js"
    );

    /**
     * Creates a small script that persists an input value
     * in localStorage and restores it on page load.
     */
    static std::string inputPersistence(
        const std::string& inputId,
        const std::string& storageKey
    );
};

} // namespace drogular