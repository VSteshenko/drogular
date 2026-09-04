#include "configuration/runtime_options.hpp"

#include <charconv>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace system_monitor {

namespace {

struct RawOptions {
    std::optional<std::string> target;
    std::optional<std::string> sshHost;
    std::optional<std::string> sshPort;
    std::optional<std::string> sshUser;
    std::optional<std::string> sshIdentityFile;
    std::optional<std::string> sshKnownHostsFile;
    std::optional<std::string> i2cScanBuses;
    bool showHelp{false};
};

[[nodiscard]] std::optional<std::string> defaultEnvironmentLookup(
    std::string_view name) {
    const std::string key(name);
    const char* value = std::getenv(key.c_str());
    if (value == nullptr || *value == '\0') {
        return std::nullopt;
    }

    return std::string(value);
}

void loadEnvironment(RawOptions& options, const EnvironmentLookup& environment) {
    options.target = environment("SYSTEM_MONITOR_TARGET");
    options.sshHost = environment("SYSTEM_MONITOR_SSH_HOST");
    options.sshPort = environment("SYSTEM_MONITOR_SSH_PORT");
    options.sshUser = environment("SYSTEM_MONITOR_SSH_USER");
    options.sshIdentityFile = environment("SYSTEM_MONITOR_SSH_IDENTITY_FILE");
    options.sshKnownHostsFile = environment("SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE");
    options.i2cScanBuses = environment("SYSTEM_MONITOR_I2C_SCAN_BUSES");
}

[[nodiscard]] std::string requireOptionValue(
    int argc,
    const char* const argv[],
    int& index,
    std::string_view option,
    std::optional<std::string_view> inlineValue) {
    if (inlineValue.has_value()) {
        if (inlineValue->empty()) {
            throw std::invalid_argument(std::string(option) + " requires a value");
        }
        return std::string(*inlineValue);
    }

    if (index + 1 >= argc) {
        throw std::invalid_argument(std::string(option) + " requires a value");
    }

    ++index;
    const std::string_view value(argv[index]);
    if (value.empty() || value.starts_with("--")) {
        throw std::invalid_argument(std::string(option) + " requires a value");
    }

    return std::string(value);
}

void applyCommandLine(
    RawOptions& options,
    int argc,
    const char* const argv[]) {
    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);
        if (argument == "--help" || argument == "-h") {
            options.showHelp = true;
            continue;
        }
        if (!argument.starts_with("--")) {
            throw std::invalid_argument(
                "unexpected positional argument: " + std::string(argument));
        }

        const auto equals = argument.find('=');
        const auto name = argument.substr(0, equals);
        const std::optional<std::string_view> inlineValue =
            equals == std::string_view::npos
                ? std::nullopt
                : std::optional<std::string_view>(argument.substr(equals + 1));

        auto value = [&] {
            return requireOptionValue(argc, argv, index, name, inlineValue);
        };

        if (name == "--target") {
            options.target = value();
        } else if (name == "--ssh-host") {
            options.sshHost = value();
        } else if (name == "--ssh-port") {
            options.sshPort = value();
        } else if (name == "--ssh-user") {
            options.sshUser = value();
        } else if (name == "--ssh-identity-file") {
            options.sshIdentityFile = value();
        } else if (name == "--ssh-known-hosts-file") {
            options.sshKnownHostsFile = value();
        } else if (name == "--i2c-scan-buses") {
            options.i2cScanBuses = value();
        } else {
            throw std::invalid_argument("unknown option: " + std::string(name));
        }
    }
}

[[nodiscard]] std::uint16_t parseSshPort(const std::optional<std::string>& value) {
    if (!value.has_value()) {
        return 22;
    }

    unsigned int port = 0;
    const std::string_view text(*value);
    const auto [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), port);
    if (error != std::errc{} || end != text.data() + text.size() ||
        port == 0 || port > 65535) {
        throw std::invalid_argument(
            "SSH port (--ssh-port / SYSTEM_MONITOR_SSH_PORT) must be between 1 and 65535");
    }

    return static_cast<std::uint16_t>(port);
}

[[nodiscard]] std::vector<std::uint32_t> parseI2cScanBuses(
    const std::optional<std::string>& value) {
    if (!value.has_value()) {
        return {};
    }

    std::vector<std::uint32_t> buses;
    std::string_view remaining(*value);
    while (!remaining.empty()) {
        const auto comma = remaining.find(',');
        auto token = remaining.substr(0, comma);
        const auto first = token.find_first_not_of(" \t");
        const auto last = token.find_last_not_of(" \t");
        if (first == std::string_view::npos) {
            throw std::invalid_argument(
                "I2C scan buses (--i2c-scan-buses / SYSTEM_MONITOR_I2C_SCAN_BUSES) contains an empty bus number");
        }
        token = token.substr(first, last - first + 1);

        std::uint32_t bus = 0;
        const auto [end, error] =
            std::from_chars(token.data(), token.data() + token.size(), bus);
        if (error != std::errc{} || end != token.data() + token.size()) {
            throw std::invalid_argument(
                "I2C scan buses (--i2c-scan-buses / SYSTEM_MONITOR_I2C_SCAN_BUSES) must be a comma-separated list of bus numbers");
        }
        buses.push_back(bus);

        if (comma == std::string_view::npos) {
            break;
        }
        remaining.remove_prefix(comma + 1);
    }

    return buses;
}

[[nodiscard]] RuntimeOptions finalizeOptions(const RawOptions& raw) {
    RuntimeOptions options;
    options.showHelp = raw.showHelp;
    options.i2cScanBuses = parseI2cScanBuses(raw.i2cScanBuses);

    const std::string target = raw.target.value_or("local");
    if (target == "local") {
        options.target = MonitorTarget::local();
        return options;
    }
    if (target != "ssh") {
        throw std::invalid_argument(
            "target (--target / SYSTEM_MONITOR_TARGET) must be 'local' or 'ssh'");
    }

    if (!raw.sshHost.has_value() || !raw.sshUser.has_value() ||
        !raw.sshIdentityFile.has_value() || !raw.sshKnownHostsFile.has_value()) {
        throw std::invalid_argument(
            "SSH target requires --ssh-host, --ssh-user, --ssh-identity-file and "
            "--ssh-known-hosts-file (or their SYSTEM_MONITOR_SSH_* environment variables)");
    }

    SshTargetOptions ssh;
    ssh.host = *raw.sshHost;
    ssh.port = parseSshPort(raw.sshPort);
    ssh.user = *raw.sshUser;
    ssh.identityFile = *raw.sshIdentityFile;
    ssh.knownHostsFile = *raw.sshKnownHostsFile;
    options.target = MonitorTarget::remote(std::move(ssh));

    return options;
}

} // namespace

RuntimeOptions parseRuntimeOptions(
    int argc,
    const char* const argv[],
    EnvironmentLookup environment) {
    if (!environment) {
        environment = defaultEnvironmentLookup;
    }

    RawOptions raw;
    loadEnvironment(raw, environment);
    applyCommandLine(raw, argc, argv);
    if (raw.showHelp) {
        RuntimeOptions options;
        options.showHelp = true;
        return options;
    }
    return finalizeOptions(raw);
}

std::string runtimeOptionsHelp(std::string_view executableName) {
    std::ostringstream output;
    output
        << "Drogular System Monitor\n\n"
        << "Usage: " << executableName << " [options]\n\n"
        << "Command-line options override matching environment variables.\n\n"
        << "Options:\n"
        << "  --target <local|ssh>          Monitoring target\n"
        << "  --ssh-host <host>             Remote SSH hostname or address\n"
        << "  --ssh-port <port>             Remote SSH port (default: 22)\n"
        << "  --ssh-user <user>             Remote SSH user\n"
        << "  --ssh-identity-file <path>    Private key path\n"
        << "  --ssh-known-hosts-file <path> Strict known_hosts path\n"
        << "  --i2c-scan-buses <list>       Comma-separated buses allowed for active scan\n"
        << "  -h, --help                    Show this help and exit\n\n"
        << "Environment variables:\n"
        << "  SYSTEM_MONITOR_TARGET\n"
        << "  SYSTEM_MONITOR_SSH_HOST\n"
        << "  SYSTEM_MONITOR_SSH_PORT\n"
        << "  SYSTEM_MONITOR_SSH_USER\n"
        << "  SYSTEM_MONITOR_SSH_IDENTITY_FILE\n"
        << "  SYSTEM_MONITOR_SSH_KNOWN_HOSTS_FILE\n"
        << "  SYSTEM_MONITOR_I2C_SCAN_BUSES\n";
    return output.str();
}

} // namespace system_monitor