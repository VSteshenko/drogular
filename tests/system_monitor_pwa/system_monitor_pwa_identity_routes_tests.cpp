#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string readFile(const std::filesystem::path& path) {
    std::ifstream stream(path);
    std::ostringstream buffer;
    buffer << stream.rdbuf();

    return buffer.str();
}

} // namespace

TEST(SystemMonitorPwaIdentityRoutesTests, RegistersNestedIconDirectoryAsStaticMapping) {
    const auto main = readFile(
        std::filesystem::path(DROGULAR_SOURCE_DIR) /
        "examples/system_monitor_pwa/main.cpp");

    EXPECT_NE(main.find("\"/assets/icons\""), std::string::npos);
    EXPECT_NE(
        main.find("\"examples/system_monitor_pwa/public/icons\""),
        std::string::npos
    );
}