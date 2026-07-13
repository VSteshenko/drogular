#pragma once

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

struct PortalDemoOptions {
    unsigned short port = 8083;
    bool showHelp = false;
};

class PortalDemoStartup {
public:
    static PortalDemoOptions parse(
        int argc,
        char* argv[]
    ) {
        PortalDemoOptions options;

        for (int index = 1; index < argc; ++index) {
            const std::string_view argument(
                argv[index]
            );

            if (argument == "-h" ||
                argument == "--help") {
                options.showHelp = true;
                continue;
            }

            if (argument == "-p" ||
                argument == "--port") {
                if (index + 1 >= argc) {
                    throw std::invalid_argument(
                        "Missing port after " +
                        std::string(argument)
                    );
                }

                options.port =
                    parsePort(argv[++index]);

                continue;
            }

            throw std::invalid_argument(
                "Unknown argument: " +
                std::string(argument)
            );
        }

        return options;
    }

    static void printBanner(
        const PortalDemoOptions& options,
        std::ostream& output = std::cout
    ) {
        const auto address =
            browserAddress(options);

        output
            << '\n'
            << "========================================\n"
            << " Drogular Portal Demo\n"
            << "========================================\n"
            << " Open in browser:\n"
            << "   " << address << '\n'
            << '\n'
            << " Administration:\n"
            << "   " << address << "admin\n"
            << '\n'
            << " Press Ctrl+C to stop the server.\n"
            << "========================================\n"
            << std::endl;
    }

    static void printHelp(
        std::ostream& output = std::cout
    ) {
        output
            << "Drogular Portal Demo\n"
            << '\n'
            << "Usage:\n"
            << "  portal_demo [options]\n"
            << '\n'
            << "Options:\n"
            << "  -p, --port <port>  HTTP port "
               "(default: 8083)\n"
            << "  -h, --help         Show this help\n";
    }

    static std::string browserAddress(
        const PortalDemoOptions& options
    ) {
        return
            "http://localhost:" +
            std::to_string(options.port) +
            "/";
    }

private:
    static unsigned short parsePort(
        std::string_view value
    ) {
        unsigned int port = 0;

        const auto begin =
            value.data();

        const auto end =
            value.data() + value.size();

        const auto result =
            std::from_chars(
                begin,
                end,
                port
            );

        if (result.ec != std::errc{} ||
            result.ptr != end ||
            port == 0 ||
            port > 65535) {
            throw std::invalid_argument(
                "Invalid port: " +
                std::string(value)
            );
        }

        return static_cast<unsigned short>(
            port
        );
    }
};