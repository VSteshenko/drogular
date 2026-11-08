#pragma once

#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

struct JobsDemoOptions {
    unsigned short port = 8084;
    bool showHelp = false;
};

class JobsDemoStartup {
public:
    static JobsDemoOptions parse(int argc, char* argv[]) {
        JobsDemoOptions options;
        for (int index = 1; index < argc; ++index) {
            const std::string_view argument(argv[index]);
            if (argument == "-h" || argument == "--help") {
                options.showHelp = true;
                continue;
            }
            if (argument == "-p" || argument == "--port") {
                if (++index == argc) {
                    throw std::invalid_argument("Missing port after " + std::string(argument));
                }
                options.port = parsePort(argv[index]);
                continue;
            }
            throw std::invalid_argument("Unknown argument: " + std::string(argument));
        }
        return options;
    }

    static void printHelp(std::ostream& output = std::cout) {
        output << "Drogular JobsDemo\n\n"
               << "Usage:\n  jobs_demo [options]\n\n"
               << "Options:\n"
               << "  -p, --port <port>  HTTP port (default: 8084)\n"
               << "  -h, --help         Show this help\n";
    }

    static void printBanner(const JobsDemoOptions& options,
                            std::ostream& output = std::cout) {
        output << "\n========================================\n"
               << " Drogular JobsDemo\n"
               << "========================================\n"
               << " Open in browser:\n   http://localhost:" << options.port << "/\n"
               << " Developer Tools:\n   http://localhost:" << options.port << "/__drogular\n"
               << " Press Ctrl+C to stop the server.\n"
               << "========================================\n" << std::endl;
    }

private:
    static unsigned short parsePort(std::string_view text) {
        unsigned int value = 0;
        const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
            value == 0 || value > 65535
        ) {
            throw std::invalid_argument("Invalid port: " + std::string(text));
        }
        return static_cast<unsigned short>(value);
    }
};