#pragma once

#include "features/tasks/providers/task_provider.hpp"
#include "ui/portal_frame_navigation_support.hpp"

#include <drogular/render_context.hpp>

#include <algorithm>
#include <string>
#include <utility>

class PortalDashboardTasksSupport final {
public:
    static void apply(
        drogular::RenderContext& context
    ) {
        auto tasks =
            context.requireService<PortalTaskProvider>()->all();

        tasks.erase(
            std::remove_if(
                tasks.begin(),
                tasks.end(),
                [](const PortalTask& task) {
                    return task.status == PortalTaskStatus::Done;
                }
            ),
            tasks.end()
        );

        // Urgent tasks come first; stable task id breaks equal-priority ties.
        std::sort(
            tasks.begin(),
            tasks.end(),
            [](const PortalTask& left, const PortalTask& right) {
                if (left.priority != right.priority) {
                    return static_cast<int>(left.priority) >
                           static_cast<int>(right.priority);
                }
                return left.id < right.id;
            }
        );

        Json::Value viewTasks(Json::arrayValue);
        const auto count = std::min<std::size_t>(tasks.size(), 6);
        for (std::size_t index = 0; index < count; ++index) {
            const auto& task = tasks[index];
            Json::Value viewTask(Json::objectValue);
            viewTask["id"] = task.id;
            viewTask["title"] = task.title;
            viewTask["statusKey"] = portalTaskStatusKey(task.status);
            viewTask["priorityKey"] = portalTaskPriorityKey(task.priority);
            viewTask["url"] = "";
            viewTask["fragmentUrl"] = "";
            viewTask["statusValue"] = portalTaskStatusValue(task.status);
            viewTask["priorityValue"] = portalTaskPriorityValue(task.priority);
            viewTask["statusOptions"] = Json::Value(Json::arrayValue);
            for (const auto status : {PortalTaskStatus::Todo, PortalTaskStatus::InProgress,
                                      PortalTaskStatus::Blocked, PortalTaskStatus::Done}) {
                Json::Value option(Json::objectValue);
                option["value"] = portalTaskStatusValue(status);
                option["labelKey"] = portalTaskStatusKey(status);
                option["selected"] = task.status == status;
                viewTask["statusOptions"].append(std::move(option));
            }
            viewTask["priorityOptions"] = Json::Value(Json::arrayValue);
            for (const auto priority : {PortalTaskPriority::Low, PortalTaskPriority::Normal,
                                        PortalTaskPriority::High, PortalTaskPriority::Critical}) {
                Json::Value option(Json::objectValue);
                option["value"] = portalTaskPriorityValue(priority);
                option["labelKey"] = portalTaskPriorityKey(priority);
                option["selected"] = task.priority == priority;
                viewTask["priorityOptions"].append(std::move(option));
            }
            viewTask["priorityClass"] =
                task.priority == PortalTaskPriority::Critical ? "critical" :
                task.priority == PortalTaskPriority::High ? "high" :
                task.priority == PortalTaskPriority::Normal ? "normal" : "low";
            if (task.projectId > 0) {
                const auto url =
                    std::string("/projects/") + std::to_string(task.projectId);
                viewTask["url"] = url;
                viewTask["fragmentUrl"] =
                    PortalFrameNavigationSupport::fragmentUrl(url);
            }
            viewTasks.append(std::move(viewTask));
        }
        context.set("dashboardTasks", viewTasks);
    }

};