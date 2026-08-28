#include "libssh_client.hpp"

#include <libssh/libssh.h>

#include <array>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

namespace system_monitor {

namespace {

[[nodiscard]] std::string sessionError(ssh_session session, std::string_view prefix) {
    std::string message(prefix);
    if (session != nullptr) {
        const char* detail = ssh_get_error(session);
        if (detail != nullptr && *detail != '\0') {
            message += ": ";
            message += detail;
        }
    }

    return message;
}

void requireOption(ssh_session session,
                   enum ssh_options_e option,
                   const void* value,
                   std::string_view name
) {
    if (ssh_options_set(session, option, value) != SSH_OK) {
        throw SshError(
            SshErrorCode::ConnectionFailed,
            sessionError(
                session,
                std::string("failed to configure SSH ") + std::string(name)
            )
        );
    }
}

void verifyKnownHost(ssh_session session) {
    switch (ssh_session_is_known_server(session)) {
        case SSH_KNOWN_HOSTS_OK:
            return;

        case SSH_KNOWN_HOSTS_CHANGED:
            throw SshError(
                SshErrorCode::HostKeyRejected,
                "SSH host key changed; refusing connection");

        case SSH_KNOWN_HOSTS_OTHER:
            throw SshError(
                SshErrorCode::HostKeyRejected,
                "SSH host key type differs from known_hosts; refusing connection");

        case SSH_KNOWN_HOSTS_NOT_FOUND:
            throw SshError(
                SshErrorCode::HostKeyRejected,
                "SSH known_hosts file was not found; refusing unverified connection");

        case SSH_KNOWN_HOSTS_UNKNOWN:
            throw SshError(
                SshErrorCode::HostKeyRejected,
                "SSH host is not present in known_hosts; refusing unverified connection");

        case SSH_KNOWN_HOSTS_ERROR:
        default:
            throw SshError(
                SshErrorCode::HostKeyRejected,
                sessionError(session, "failed to verify SSH host key"));
    }
}

class Channel final {
public:
    explicit Channel(ssh_session session)
        : session_(session),
          channel_(ssh_channel_new(session))
    {
        if (channel_ == nullptr) {
            throw SshError(
                SshErrorCode::TransportError,
                sessionError(session_, "failed to create SSH channel"));
        }
    }

    ~Channel() {
        if (channel_ != nullptr) {
            if (ssh_channel_is_open(channel_)) {
                (void)ssh_channel_close(channel_);
            }
            ssh_channel_free(channel_);
        }
    }

    [[nodiscard]] ssh_channel get() const noexcept {
        return channel_;
    }

private:
    ssh_session session_{nullptr};
    ssh_channel channel_{nullptr};
};

void appendAvailable(ssh_session session,
                     ssh_channel channel,
                     int isStderr,
                     std::string& output,
                     bool& progressed) {
    std::array<char, 4096> buffer{};

    while (true) {
        const int read = ssh_channel_read_nonblocking(
            channel,
            buffer.data(),
            static_cast<std::uint32_t>(buffer.size()),
            isStderr);

        if (read > 0) {
            output.append(buffer.data(), static_cast<std::size_t>(read));
            progressed = true;
            continue;
        }

        if (read == SSH_ERROR) {
            throw SshError(
                SshErrorCode::TransportError,
                sessionError(session, "failed to read SSH channel"));
        }

        return;
    }
}

} // namespace

class LibsshClient::Impl final {
public:
    ~Impl() {
        closeSession();
    }

    void connect(const SshTargetOptions& options) {
        validateOptions(options);

        closeSession();
        reconnectOptions_.reset();

        try {
            establishConnection(options);
            reconnectOptions_ = options;
        } catch (...) {
            closeSession();
            throw;
        }
    }

    void disconnect() noexcept {
        reconnectOptions_.reset();
        closeSession();
    }

    [[nodiscard]] SshConnectionState state() const noexcept {
        return state_;
    }

    [[nodiscard]] CommandResult execute(std::string_view command) {
        ensureConnected();

        try {
            return executeConnected(command);
        } catch (const SshError& error) {
            if (error.code() == SshErrorCode::TransportError ||
                error.code() == SshErrorCode::NotConnected
            ) {
                closeSession();
            }
            throw;
        }
    }

private:
    static void validateOptions(const SshTargetOptions& options) {
        if (options.host.empty()) {
            throw std::invalid_argument("SSH host must not be empty");
        }
        if (options.user.empty()) {
            throw std::invalid_argument("SSH user must not be empty");
        }
    }

    void establishConnection(const SshTargetOptions& options) {
        state_ = SshConnectionState::Connecting;
        session_ = ssh_new();
        if (session_ == nullptr) {
            state_ = SshConnectionState::Disconnected;
            throw SshError(SshErrorCode::ConnectionFailed, "failed to allocate SSH session");
        }

        try {
            requireOption(session_, SSH_OPTIONS_HOST, options.host.c_str(), "host");
            requireOption(session_, SSH_OPTIONS_USER, options.user.c_str(), "user");

            const unsigned int port = options.port;
            requireOption(session_, SSH_OPTIONS_PORT, &port, "port");

            if (!options.identityFile.empty()) {
                requireOption(
                    session_,
                    SSH_OPTIONS_IDENTITY,
                    options.identityFile.c_str(),
                    "identity file");
            }

            if (!options.knownHostsFile.empty()) {
                requireOption(
                    session_,
                    SSH_OPTIONS_KNOWNHOSTS,
                    options.knownHostsFile.c_str(),
                    "known_hosts file");
            }

            if (ssh_connect(session_) != SSH_OK) {
                throw SshError(
                    SshErrorCode::ConnectionFailed,
                    sessionError(session_, "SSH connection failed"));
            }

            verifyKnownHost(session_);

            if (ssh_userauth_publickey_auto(session_, nullptr, nullptr) != SSH_AUTH_SUCCESS) {
                throw SshError(
                    SshErrorCode::AuthenticationFailed,
                    sessionError(session_, "SSH public-key authentication failed"));
            }

            state_ = SshConnectionState::Connected;
        } catch (...) {
            closeSession();
            throw;
        }
    }

    void ensureConnected() {
        if (state_ == SshConnectionState::Connected &&
            session_ != nullptr &&
            ssh_is_connected(session_)) {
            return;
        }

        closeSession();

        if (!reconnectOptions_) {
            throw SshError(SshErrorCode::NotConnected, "SSH client is not connected");
        }

        establishConnection(*reconnectOptions_);
    }

    [[nodiscard]] CommandResult executeConnected(std::string_view command) {
        if (state_ != SshConnectionState::Connected ||
            session_ == nullptr ||
            !ssh_is_connected(session_)) {
            throw SshError(SshErrorCode::NotConnected, "SSH client is not connected");
        }

        Channel channel(session_);
        if (ssh_channel_open_session(channel.get()) != SSH_OK) {
            throw SshError(
                SshErrorCode::TransportError,
                sessionError(session_, "failed to open SSH channel"));
        }

        const std::string commandText(command);
        if (ssh_channel_request_exec(channel.get(), commandText.c_str()) != SSH_OK) {
            throw SshError(
                SshErrorCode::TransportError,
                sessionError(session_, "failed to execute SSH command"));
        }

        CommandResult result;
        while (ssh_channel_is_open(channel.get()) && !ssh_channel_is_eof(channel.get())) {
            bool progressed = false;
            appendAvailable(session_, channel.get(), 0, result.standardOutput, progressed);
            appendAvailable(session_, channel.get(), 1, result.standardError, progressed);

            if (!progressed) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        bool progressed = false;
        appendAvailable(session_, channel.get(), 0, result.standardOutput, progressed);
        appendAvailable(session_, channel.get(), 1, result.standardError, progressed);

        const int exitCode = ssh_channel_get_exit_status(channel.get());
        if (exitCode == SSH_ERROR) {
            throw SshError(
                SshErrorCode::TransportError,
                sessionError(session_, "failed to read SSH command exit status"));
        }

        result.exitCode = exitCode;
        return result;
    }

    void closeSession() noexcept {
        if (session_ != nullptr) {
            if (ssh_is_connected(session_)) {
                ssh_disconnect(session_);
            }
            ssh_free(session_);
            session_ = nullptr;
        }
        state_ = SshConnectionState::Disconnected;
    }

    ssh_session session_{nullptr};
    SshConnectionState state_{SshConnectionState::Disconnected};
    std::optional<SshTargetOptions> reconnectOptions_;
};

LibsshClient::LibsshClient()
    : impl_(std::make_unique<Impl>()) {}

LibsshClient::~LibsshClient() = default;

void LibsshClient::connect(const SshTargetOptions& options) {
    impl_->connect(options);
}

void LibsshClient::disconnect() noexcept {
    impl_->disconnect();
}

SshConnectionState LibsshClient::state() const noexcept {
    return impl_->state();
}

CommandResult LibsshClient::execute(std::string_view command) {
    return impl_->execute(command);
}

} // namespace system_monitor