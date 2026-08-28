#include "ssh/libssh_client.hpp"

#include <gtest/gtest.h>

#if SYSTEM_MONITOR_HAS_LIBSSH

TEST(LibsshClientTests, StartsDisconnected) {
    system_monitor::LibsshClient client;

    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Disconnected);
    EXPECT_FALSE(client.isConnected());
}

TEST(LibsshClientTests, DisconnectIsIdempotent) {
    system_monitor::LibsshClient client;

    client.disconnect();
    client.disconnect();

    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Disconnected);
}

TEST(LibsshClientTests, ExecuteRequiresConnection) {
    system_monitor::LibsshClient client;

    try {
        static_cast<void>(client.execute("hostname"));
        FAIL() << "expected execute to fail";
    } catch (const system_monitor::SshError& error) {
        EXPECT_EQ(error.code(), system_monitor::SshErrorCode::NotConnected);
    }
}

TEST(LibsshClientTests, RejectsEmptyHostBeforeNetworkAccess) {
    system_monitor::LibsshClient client;
    system_monitor::SshTargetOptions options;
    options.user = "monitor";

    EXPECT_THROW(client.connect(options), std::invalid_argument);
    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Disconnected);
}

TEST(LibsshClientTests, RejectsEmptyUserBeforeNetworkAccess) {
    system_monitor::LibsshClient client;
    system_monitor::SshTargetOptions options;
    options.host = "127.0.0.1";

    EXPECT_THROW(client.connect(options), std::invalid_argument);
    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Disconnected);
}

#endif