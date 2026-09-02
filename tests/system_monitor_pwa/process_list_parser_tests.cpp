#include "system/process_list_parser.hpp"

#include <gtest/gtest.h>

TEST(ProcessListParserTests, ParsesPsRowsAndResidentMemory) {
    const auto processes = system_monitor::parseProcessList(
        "  42 alice 12.5 1.2 2048 worker /usr/bin/worker --serve\n"
        "   7 root   0.0 0.1  512 sshd   /usr/sbin/sshd -D\n");

    ASSERT_EQ(processes.size(), 2U);
    EXPECT_EQ(processes[0].pid, 42);
    EXPECT_EQ(processes[0].user, "alice");
    EXPECT_EQ(processes[0].name, "worker");
    EXPECT_EQ(processes[0].command, "/usr/bin/worker --serve");
    EXPECT_DOUBLE_EQ(processes[0].cpuPercent, 12.5);
    EXPECT_DOUBLE_EQ(processes[0].memoryPercent, 1.2);
    EXPECT_EQ(processes[0].residentBytes, 2048ULL * 1024ULL);
}

TEST(ProcessListParserTests, SkipsMalformedRows) {
    const auto processes = system_monitor::parseProcessList(
        "broken row\n"
        "9 bob 1.0 2.0 100 app\n");
    ASSERT_EQ(processes.size(), 1U);
    EXPECT_EQ(processes[0].command, "app");
}