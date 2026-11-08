#include <drogular/job_registry.hpp>

#include <charconv>
#include <set>
#include <utility>

namespace drogular {
namespace {

bool validIdentifier(std::string_view value) {
    if (value.empty()) {
        return false;
    }
    for (const unsigned char c : value) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '.' || c == '-' || c == '_')
        ) {
            return false;
        }
    }
    return true;
}

void validateValue(const JobParameterDefinition& param, const std::string& value) {
    if (param.type == JobParameterType::Boolean) {
        if (value != "true" && value != "false") {
            throw std::invalid_argument("Invalid boolean parameter: " + param.name);
        }
    } else if (param.type == JobParameterType::Integer) {
        std::int64_t number = 0;
        const auto [end, error] =
            std::from_chars(value.data(), value.data() + value.size(), number);
        if (error != std::errc{} || end != value.data() + value.size() ||
            (param.minimum && number < *param.minimum) || (param.maximum && number > *param.maximum)
        ) {
            throw std::invalid_argument("Invalid integer parameter: " + param.name);
        }
    }
}

} // namespace

JobRegistry::JobRegistry(JobRuntime& runtime) noexcept : runtime_(runtime) {}

void JobRegistry::registerJob(JobDefinition definition, Factory factory) {
    if (!validIdentifier(definition.id) || definition.name.empty() || !factory) {
        throw std::invalid_argument("Invalid job registration");
    }
    std::set<std::string> names;
    for (const auto& param : definition.parameters) {
        if (!validIdentifier(param.name) || !names.insert(param.name).second ||
            (param.minimum && param.maximum && *param.minimum > *param.maximum)
        ) {
            throw std::invalid_argument("Invalid job parameter definition");
        }
        if (!param.defaultValue.empty()) {
            validateValue(param, param.defaultValue);
        }
    }
    std::lock_guard lock(mutex_);
    const std::string key = definition.id;
    if (!registered_.emplace(key, Registered{std::move(definition), std::move(factory)}).second) {
        throw std::invalid_argument("Duplicate job type: " + key);
    }
}

std::vector<JobDefinition> JobRegistry::list() const {
    std::lock_guard lock(mutex_);
    std::vector<JobDefinition> result;
    result.reserve(registered_.size());
    for (const auto& [key, entry] : registered_) {
        (void)key; result.push_back(entry.definition);
    }
    return result;
}

std::optional<JobDefinition> JobRegistry::find(std::string_view id) const {
    std::lock_guard lock(mutex_);
    const auto it = registered_.find(id);
    if (it == registered_.end()) {
        return std::nullopt;
    }
    return it->second.definition;
}

JobId JobRegistry::launch(std::string_view id, const JobParameters& parameters) {
    Registered entry;
    {
        std::lock_guard lock(mutex_);
        const auto it = registered_.find(id);
        if (it == registered_.end()) {
            throw std::invalid_argument("Unknown job type");
        }
        entry = it->second;
    }
    JobParameters args;
    for (const auto& parameter : entry.definition.parameters) {
        const auto found = parameters.find(parameter.name);
        if (found == parameters.end() && parameter.required && parameter.defaultValue.empty()) {
            throw std::invalid_argument("Missing job parameter: " + parameter.name);
        }
        const auto value = found == parameters.end() ? parameter.defaultValue : found->second;
        if (found != parameters.end() || !value.empty()) {
            validateValue(parameter, value);
            args.emplace(parameter.name, value);
        }
    }
    for (const auto& [name, value] : parameters) {
        (void)value;
        if (!args.contains(name)) {
            throw std::invalid_argument("Unknown job parameter: " + name);
        }
    }
    return runtime_.submit(entry.definition.name,
        [factory = std::move(entry.factory), args = std::move(args)](JobContext& context) {
            factory(context, args);
        });
}

} // namespace drogular