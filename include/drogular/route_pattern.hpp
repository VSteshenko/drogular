#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace drogular {

/**
 * Matches route patterns with optional named parameters.
 *
 * Example:
 * /projects/{id} matches /projects/42 and extracts id=42.
 */
class RoutePattern {
public:
    explicit RoutePattern(
        std::string pattern
    );

    bool match(
        const std::string& path,
        std::unordered_map<std::string, std::string>& parameters
    ) const;

private:
    struct Segment {
        std::string value;
        bool parameter = false;
    };

    static std::vector<Segment> parse(
        const std::string& value
    );

    std::vector<Segment> segments_;
};

} // namespace drogular