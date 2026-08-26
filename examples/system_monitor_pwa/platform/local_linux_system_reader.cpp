#include "local_linux_system_reader.hpp"

#if !defined(__linux__)
#error "LocalLinuxSystemReader can only be built on Linux"
#endif

#include <array>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <sys/wait.h>

namespace system_monitor {

std::string LocalLinuxSystemReader::readFile(std::string_view path) {
    std::ifstream input(std::string(path), std::ios::binary);
    if (!input) {
        throw std::system_error(errno ? errno : ENOENT,
            std::generic_category(),
            std::string("failed to open ") + std::string(path));
    }

    std::ostringstream output;
    output << input.rdbuf();
    if (!input.good() && !input.eof()) {
        throw std::runtime_error(std::string("failed to read ") + std::string(path));
    }

    return output.str();
}

CommandResult LocalLinuxSystemReader::execute(std::string_view command) {
    const std::string shellCommand = std::string(command) + " 2>&1";
    FILE* pipe = ::popen(shellCommand.c_str(), "r");
    if (pipe == nullptr) {
        throw std::system_error(errno, std::generic_category(), "popen failed");
    }

    std::array<char, 4096> buffer{};
    std::string output;
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output.append(buffer.data());
    }

    const int status = ::pclose(pipe);
    CommandResult result;
    result.standardOutput = std::move(output);
    if (status == -1) {
        result.exitCode = -1;
    } else if (WIFEXITED(status)) {
        result.exitCode = WEXITSTATUS(status);
    } else {
        result.exitCode = status;
    }

    return result;
}

} // namespace system_monitor