#pragma once

#include <drogular/job_runtime.hpp>

#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace drogular {

enum class JobParameterType {
    Integer,
    Boolean,
    String
};

struct JobParameterDefinition {
    std::string name;
    JobParameterType type = JobParameterType::String;
    bool required = false;
    std::string defaultValue;
    std::optional<std::int64_t> minimum;
    std::optional<std::int64_t> maximum;
};

struct JobDefinition {
    std::string id;
    std::string name;
    std::string description;
    bool cancellable = true;
    std::vector<JobParameterDefinition> parameters;
};

using JobParameters = std::map<std::string, std::string>;

/** Transport-neutral catalog of explicitly registered jobs.
 * Registration does not expose an HTTP endpoint or grant permission to launch.
 */
class JobRegistry {
public:
    using Factory = std::function<void(JobContext&, const JobParameters&)>;

    explicit JobRegistry(JobRuntime& runtime) noexcept;
    void registerJob(JobDefinition definition, Factory factory);
    std::vector<JobDefinition> list() const;
    std::optional<JobDefinition> find(std::string_view id) const;
    // Throws invalid_argument for invalid/unknown parameters or an unknown type.
    // The caller MUST authorize the operation before invoking launch().
    JobId launch(std::string_view id, const JobParameters& parameters = {});

private:
    struct Registered {
        JobDefinition definition;
        Factory factory;
    };
    JobRuntime& runtime_;
    mutable std::mutex mutex_;
    std::map<std::string, Registered, std::less<>> registered_;
};

} // namespace drogular