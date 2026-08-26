#include "ssh/ssh_client.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

class FakeSshClient final : public system_monitor::SshClient {
public:
    void connect(const system_monitor::SshTargetOptions& options) override {
        lastOptions = options;
        connectionState = system_monitor::SshConnectionState::Connected;
    }

    void disconnect() noexcept override {
        connectionState = system_monitor::SshConnectionState::Disconnected;
    }

    system_monitor::SshConnectionState state() const noexcept override {
        return connectionState;
    }

    system_monitor::CommandResult execute(std::string_view command) override {
        if (!isConnected()) {
            throw system_monitor::SshError(
                system_monitor::SshErrorCode::NotConnected,
                "SSH client is not connected");
        }

        commands.emplace_back(command);
        return nextResult;
    }

    system_monitor::SshTargetOptions lastOptions;
    system_monitor::SshConnectionState connectionState{
        system_monitor::SshConnectionState::Disconnected};
    system_monitor::CommandResult nextResult{0, "ok\n", {}};
    std::vector<std::string> commands;
};

} // namespace

TEST(SshClientTests, ConnectReceivesTargetOptionsAndUpdatesState) {
    FakeSshClient client;
    system_monitor::SshTargetOptions options;
    options.host = "raspberrypi.local";
    options.port = 2222;
    options.user = "monitor";
    options.identityFile = "/keys/id_ed25519";
    options.knownHostsFile = "/keys/known_hosts";

    EXPECT_FALSE(client.isConnected());
    client.connect(options);

    EXPECT_TRUE(client.isConnected());
    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Connected);
    EXPECT_EQ(client.lastOptions.host, "raspberrypi.local");
    EXPECT_EQ(client.lastOptions.port, 2222);
    EXPECT_EQ(client.lastOptions.user, "monitor");
    EXPECT_EQ(client.lastOptions.identityFile, "/keys/id_ed25519");
    EXPECT_EQ(client.lastOptions.knownHostsFile, "/keys/known_hosts");
}

TEST(SshClientTests, DisconnectReturnsClientToDisconnectedState) {
    FakeSshClient client;
    client.connect({.host = "raspberrypi.local", .user = "monitor"});

    client.disconnect();

    EXPECT_FALSE(client.isConnected());
    EXPECT_EQ(client.state(), system_monitor::SshConnectionState::Disconnected);
}

TEST(SshClientTests, ExecuteReturnsStructuredCommandResult) {
    FakeSshClient client;
    client.connect({.host = "raspberrypi.local", .user = "monitor"});
    client.nextResult = {7, "stdout", "stderr"};

    const auto result = client.execute("uname -m");

    ASSERT_EQ(client.commands.size(), 1U);
    EXPECT_EQ(client.commands.front(), "uname -m");
    EXPECT_EQ(result.exitCode, 7);
    EXPECT_EQ(result.standardOutput, "stdout");
    EXPECT_EQ(result.standardError, "stderr");
    EXPECT_FALSE(result.succeeded());
}

TEST(SshClientTests, TransportFailuresUseTypedSshError) {
    FakeSshClient client;

    try {
        (void)client.execute("hostname");
        FAIL() << "Expected SshError";
    } catch (const system_monitor::SshError& error) {
        EXPECT_EQ(error.code(), system_monitor::SshErrorCode::NotConnected);
        EXPECT_STREQ(error.what(), "SSH client is not connected");
    }
}