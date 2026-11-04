#include "features/tasks/providers/portal_memory_task_provider.hpp"
#include "data/demo_dataset.hpp"

#include <gtest/gtest.h>
#include <algorithm>

TEST(PortalTaskProviderTests, SeedsFirstClassTasks) {
    PortalMemoryTaskProvider provider;

    const auto tasks = provider.all();
    ASSERT_EQ(tasks.size(), 4U);
    EXPECT_EQ(tasks[0].status, PortalTaskStatus::InProgress);
    EXPECT_EQ(tasks[0].priority, PortalTaskPriority::High);
    EXPECT_EQ(tasks[0].projectId, 1);
    ASSERT_TRUE(provider.findById(3).has_value());
    EXPECT_EQ(provider.findById(3)->priority, PortalTaskPriority::Critical);
    EXPECT_FALSE(provider.findById(999).has_value());
}

TEST(PortalTaskProviderTests, SupportsIndependentAndEmptyTasks) {
    PortalMemoryTaskProvider provider({
        {
            42,
            "Unlinked task",
            PortalTaskStatus::Todo,
            PortalTaskPriority::Low,
            0
        }
    });

    ASSERT_EQ(provider.all().size(), 1U);
    ASSERT_TRUE(provider.findById(42).has_value());
    EXPECT_EQ(provider.findById(42)->projectId, 0);
    EXPECT_STREQ(portalTaskStatusKey(PortalTaskStatus::Todo), "dashboard.task.status.todo");
    EXPECT_STREQ(portalTaskPriorityKey(PortalTaskPriority::Low), "dashboard.task.priority.low");
    PortalMemoryTaskProvider empty(std::vector<PortalTask>{});
    EXPECT_TRUE(empty.all().empty());
}

TEST(PortalTaskProviderTests, SharedDemoDatasetKeepsTaskProjectReferencesValid) {
    const auto dataset = DemoDataset::create();
    PortalMemoryTaskProvider tasks(dataset.tasks());

    ASSERT_EQ(tasks.all().size(), 4U);
    for (const auto& task : tasks.all()) {
        if (task.projectId == 0) continue;
        const auto& projects = dataset.projects();
        EXPECT_NE(std::find_if(projects.begin(), projects.end(),
            [&](const PortalProject& project) { return project.id == task.projectId; }), projects.end());
    }
}