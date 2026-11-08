#include <drogular/developer_tools/job_registry_contributor.hpp>

#include <json/json.h>

#include <stdexcept>
#include <utility>

namespace drogular {

JobRegistryContributor::JobRegistryContributor(std::shared_ptr<JobRegistry> registry,
                                               std::string launchEndpoint)
    : registry_(std::move(registry)), launchEndpoint_(std::move(launchEndpoint))
{
    if (!registry_) {
        throw std::invalid_argument("JobRegistryContributor requires a registry");
    }
}

void JobRegistryContributor::contribute(ApplicationInspection& inspection) const {
    Json::Value data(Json::objectValue);
    Json::Value types(Json::arrayValue);
    for (const auto& job : registry_->list()) {
        Json::Value item(Json::objectValue);
        item["id"] = job.id;
        item["name"] = job.name;
        item["description"] = job.description;
        item["cancellable"] = job.cancellable;

        Json::Value parameters(Json::arrayValue);
        for (const auto& p : job.parameters) {
            Json::Value parameter(Json::objectValue);
            parameter["name"] = p.name;
            parameter["type"] = p.type == JobParameterType::Integer ? "integer" :
                                p.type == JobParameterType::Boolean ? "boolean" : "string";
            parameter["required"] = p.required;
            parameter["default"] = p.defaultValue;

            if (p.minimum) {
                parameter["minimum"] = Json::Int64(*p.minimum);
            }
            if (p.maximum) {
                parameter["maximum"] = Json::Int64(*p.maximum);
            }
            parameters.append(std::move(parameter));
        }
        item["parameters"] = std::move(parameters);
        types.append(std::move(item));
    }
    data["types"] = std::move(types);

    if (!launchEndpoint_.empty()) {
        data["launchEndpoint"] = launchEndpoint_;
    }
    inspection.addSection({
        "job-registry",
        "Available Jobs",
        "drogular.job-registry",
        std::move(data)
    });
}

} // namespace drogular