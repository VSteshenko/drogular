#pragma once

#include <string>
#include <string_view>

namespace system_monitor {

struct CommandResult {
    int exitCode{0};
    std::string standardOutput;
    std::string standardError;

    [[nodiscard]] bool succeeded() const noexcept {
        return exitCode == 0;
    }
};

class SystemReader {
public:
    virtual ~SystemReader() = default;

    [[nodiscard]] virtual std::string readFile(std::string_view path) = 0;
    [[nodiscard]] virtual CommandResult execute(std::string_view command) = 0;
};

} // namespace system_monitor