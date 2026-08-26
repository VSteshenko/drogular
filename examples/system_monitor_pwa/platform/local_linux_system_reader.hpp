#pragma once

#include "system/system_reader.hpp"

namespace system_monitor {

class LocalLinuxSystemReader final : public SystemReader {
public:
    [[nodiscard]] std::string readFile(std::string_view path) override;
    [[nodiscard]] CommandResult execute(std::string_view command) override;
};

} // namespace system_monitor