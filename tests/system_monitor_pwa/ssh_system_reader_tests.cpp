#include "ssh/ssh_system_reader.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

namespace {

class FakeSshClient final : public system_monitor::SshClient {
public:
    void connect(const system_monitor::SshTargetOptions&) override {
        state_ = system_monitor::SshConnectionState::Connected;
    }

    void disconnect() noexcept override {
        state_ = system_monitor::SshConnectionState::Disconnected;
    }

    [[nodiscard]] system_monitor::SshConnectionState state() const noexcept override {
        return state_;
    }

    [[nodiscard]] system_monitor::CommandResult execute(std::string_view command) override {
        commands.emplace_back(command);
        if (throwTransportError) {
            throw system_monitor::SshError(
                system_monitor::SshErrorCode::TransportError,
                "transport failed");
        }
        return nextResult;
    }

    system_monitor::SshConnectionState state_{
        system_monitor::SshConnectionState::Connected};
    system_monitor::CommandResult nextResult;
    bool throwTransportError{false};
    std::vector<std::string> commands;
};

} // namespace

TEST(SshSystemReaderTests, RequiresClient) {
    EXPECT_THROW(
        system_monitor::SshSystemReader(nullptr),
        std::invalid_argument);
}

TEST(SshSystemReaderTests, ReadFileUsesQuotedRemoteCat) {
    auto client = std::make_shared<FakeSshClient>();
    client->nextResult.standardOutput = "MemTotal: 1024 kB\n";

    system_monitor::SshSystemReader reader(client);

    EXPECT_EQ(reader.readFile("/proc/meminfo"), "MemTotal: 1024 kB\n");
    ASSERT_EQ(client->commands.size(), 1U);
    EXPECT_EQ(client->commands.front(), "cat -- '/proc/meminfo'");
}

TEST(SshSystemReaderTests, ReadFileShellQuotesPath) {
    auto client = std::make_shared<FakeSshClient>();
    client->nextResult.standardOutput = "value";

    system_monitor::SshSystemReader reader(client);

    EXPECT_EQ(reader.readFile("/tmp/it's safe"), "value");
    ASSERT_EQ(client->commands.size(), 1U);
    EXPECT_EQ(client->commands.front(), "cat -- '/tmp/it'\\''s safe'");
}

TEST(SshSystemReaderTests, ReadFileReportsRemoteCommandFailure) {
    auto client = std::make_shared<FakeSshClient>();
    client->nextResult.exitCode = 1;
    client->nextResult.standardError = "permission denied";

    system_monitor::SshSystemReader reader(client);

    try {
        (void)reader.readFile("/root/secret");
        FAIL() << "expected readFile to fail";
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find("/root/secret"), std::string::npos);
        EXPECT_NE(message.find("exit code 1"), std::string::npos);
        EXPECT_NE(message.find("permission denied"), std::string::npos);
    }
}

TEST(SshSystemReaderTests, ExecuteDelegatesWithoutChangingCommandResult) {
    auto client = std::make_shared<FakeSshClient>();
    client->nextResult = {
        .exitCode = 7,
        .standardOutput = "stdout",
        .standardError = "stderr",
    };

    system_monitor::SshSystemReader reader(client);
    const auto result = reader.execute("df -P -B1 /");

    ASSERT_EQ(client->commands.size(), 1U);
    EXPECT_EQ(client->commands.front(), "df -P -B1 /");
    EXPECT_EQ(result.exitCode, 7);
    EXPECT_EQ(result.standardOutput, "stdout");
    EXPECT_EQ(result.standardError, "stderr");
}

TEST(SshSystemReaderTests, PreservesSshTransportErrors) {
    auto client = std::make_shared<FakeSshClient>();
    client->throwTransportError = true;

    system_monitor::SshSystemReader reader(client);

    try {
        (void)reader.execute("hostname");
        FAIL() << "expected execute to fail";
    } catch (const system_monitor::SshError& error) {
        EXPECT_EQ(error.code(), system_monitor::SshErrorCode::TransportError);
    }
}