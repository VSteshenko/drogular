#pragma once

#include "system/monitor_target.hpp"
#include "system/system_reader.hpp"

#include <stdexcept>
#include <string>
#include <string_view>

namespace system_monitor {

enum class SshConnectionState {
    Disconnected,
    Connecting,
    Connected
};

enum class SshErrorCode {
    ConnectionFailed,
    AuthenticationFailed,
    HostKeyRejected,
    NotConnected,
    Timeout,
    TransportError
};

class SshError final : public std::runtime_error {
public:
    SshError(SshErrorCode code, std::string message)
        : std::runtime_error(std::move(message)),
          code_(code) {}

    [[nodiscard]] SshErrorCode code() const noexcept {
        return code_;
    }

private:
    SshErrorCode code_;
};

class SshClient {
public:
    virtual ~SshClient() = default;

    virtual void connect(const SshTargetOptions& options) = 0;
    virtual void disconnect() noexcept = 0;

    [[nodiscard]] virtual SshConnectionState state() const noexcept = 0;

    [[nodiscard]] bool isConnected() const noexcept {
        return state() == SshConnectionState::Connected;
    }

    [[nodiscard]] virtual CommandResult execute(std::string_view command) = 0;
};

} // namespace system_monitor