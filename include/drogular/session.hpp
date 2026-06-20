#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace drogular {

class Session {
public:
    void set(
        std::string key,
        std::string value
    );

    std::optional<std::string> get(
        const std::string& key
    ) const;

    bool has(
        const std::string& key
    ) const;

    void remove(
        const std::string& key
    );

    void clear();

private:
    std::unordered_map<
        std::string,
        std::string
    > values_;
};

} // namespace drogular