#pragma once

#include "ssh/ssh_client.hpp"

#include <memory>

namespace system_monitor {

class LibsshClient final : public SshClient {
public:
    LibsshClient();
    ~LibsshClient() override;

    LibsshClient(const LibsshClient&) = delete;
    LibsshClient& operator=(const LibsshClient&) = delete;
    LibsshClient(LibsshClient&&) = delete;
    LibsshClient& operator=(LibsshClient&&) = delete;

    void connect(const SshTargetOptions& options) override;
    void disconnect() noexcept override;

    [[nodiscard]] SshConnectionState state() const noexcept override;
    [[nodiscard]] CommandResult execute(std::string_view command) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace system_monitor