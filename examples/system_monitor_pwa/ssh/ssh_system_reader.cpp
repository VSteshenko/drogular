#include "ssh_system_reader.hpp"

#include <stdexcept>
#include <string>

namespace system_monitor {

namespace {

std::string shellQuote(std::string_view value) {
    std::string quoted;
    quoted.reserve(value.size() + 2);
    quoted.push_back('\'');

    for (const char ch : value) {
        if (ch == '\'') {
            quoted += "'\\''";
        } else {
            quoted.push_back(ch);
        }
    }

    quoted.push_back('\'');
    return quoted;
}

std::string readFailureMessage(std::string_view path, const CommandResult& result) {
    std::string message = "failed to read remote file ";
    message += path;
    message += " (exit code ";
    message += std::to_string(result.exitCode);
    message += ')';

    const auto& detail = result.standardError.empty()
        ? result.standardOutput
        : result.standardError;
    if (!detail.empty()) {
        message += ": ";
        message += detail;
    }

    return message;
}

} // namespace

SshSystemReader::SshSystemReader(std::shared_ptr<SshClient> client)
    : client_(std::move(client))
{
    if (!client_) {
        throw std::invalid_argument("SshSystemReader requires an SshClient");
    }
}

std::string SshSystemReader::readFile(std::string_view path) {
    const auto result = client_->execute("cat -- " + shellQuote(path));
    if (!result.succeeded()) {
        throw std::runtime_error(readFailureMessage(path, result));
    }

    return result.standardOutput;
}

CommandResult SshSystemReader::execute(std::string_view command) {
    return client_->execute(command);
}

} // namespace system_monitor