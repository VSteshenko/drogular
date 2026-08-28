#pragma once

#include "ssh/ssh_client.hpp"
#include "system/system_reader.hpp"

#include <memory>

namespace system_monitor {

class SshSystemReader final : public SystemReader {
public:
    explicit SshSystemReader(std::shared_ptr<SshClient> client);

    [[nodiscard]] std::string readFile(std::string_view path) override;
    [[nodiscard]] CommandResult execute(std::string_view command) override;

private:
    std::shared_ptr<SshClient> client_;
};

} // namespace system_monitor