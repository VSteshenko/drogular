#include <drogular/pwa_scripts.hpp>

#include <gtest/gtest.h>

TEST(CorePwaScriptsTests, CreatesServiceWorkerRegistrationScript) {
    const auto script =
        drogular::PwaScripts::serviceWorkerRegistration();

    EXPECT_NE(
        script.find("navigator.serviceWorker.register"),
        std::string::npos
    );

    EXPECT_NE(
        script.find("/service-worker.js"),
        std::string::npos
    );
}

TEST(CorePwaScriptsTests, SupportsCustomServiceWorkerPath) {
    const auto script =
        drogular::PwaScripts::serviceWorkerRegistration(
            "/custom-worker.js"
        );

    EXPECT_NE(
        script.find("/custom-worker.js"),
        std::string::npos
    );
}