#include "startup/portal_demo_startup.hpp"

#include <gtest/gtest.h>

TEST(PortalDemoStartupTests, UsesDefaultPort) {
    char executable[] =
        "portal_demo";

    char* argv[] = {
        executable
    };

    const auto options =
        PortalDemoStartup::parse(
            1,
            argv
        );

    EXPECT_EQ(
        options.port,
        8083
    );

    EXPECT_FALSE(
        options.showHelp
    );
}

TEST(PortalDemoStartupTests, ParsesShortPortOption) {
    char executable[] =
        "portal_demo";

    char portOption[] =
        "-p";

    char portValue[] =
        "9000";

    char* argv[] = {
        executable,
        portOption,
        portValue
    };

    const auto options =
        PortalDemoStartup::parse(
            3,
            argv
        );

    EXPECT_EQ(
        options.port,
        9000
    );
}

TEST(PortalDemoStartupTests, RejectsInvalidPort) {
    char executable[] =
        "portal_demo";

    char portOption[] =
        "-p";

    char portValue[] =
        "invalid";

    char* argv[] = {
        executable,
        portOption,
        portValue
    };

    EXPECT_THROW(
        PortalDemoStartup::parse(
            3,
            argv
        ),
        std::invalid_argument
    );
}

TEST(PortalDemoStartupTests, BuildsBrowserAddress) {
    PortalDemoOptions options;
    options.port = 9000;

    EXPECT_EQ(
        PortalDemoStartup::browserAddress(
            options
        ),
        "http://localhost:9000/"
    );
}