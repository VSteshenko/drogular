#pragma once

#include <drogular/developer_tools/application_inspection.hpp>
#include <drogular/job_registry.hpp>

#include <memory>
#include <string>

namespace drogular {

/** Read-only discovery adapter. launchEndpoint is opt-in and must enforce
 * authorization independently of this contributor.
 */
class JobRegistryContributor final : public DeveloperToolsContributor {
public:
    explicit JobRegistryContributor(std::shared_ptr<JobRegistry> registry,
                                    std::string launchEndpoint = {});
    void contribute(ApplicationInspection& inspection) const override;

private:
    std::shared_ptr<JobRegistry> registry_;
    std::string launchEndpoint_;
};

} // namespace drogular