#include <drogular/route_pattern.hpp>

#include <sstream>

namespace drogular {

RoutePattern::RoutePattern(
    std::string pattern
)
    : segments_(parse(pattern))
{
}

bool RoutePattern::match(
    const std::string& path,
    std::unordered_map<std::string, std::string>& parameters
) const {
    const auto pathSegments =
        parse(path);

    if (pathSegments.size() != segments_.size()) {
        return false;
    }

    std::unordered_map<std::string, std::string> matched;

    for (std::size_t index = 0; index < segments_.size(); ++index) {
        const auto& patternSegment =
            segments_[index];

        const auto& pathSegment =
            pathSegments[index];

        if (patternSegment.parameter) {
            matched[patternSegment.value] =
                pathSegment.value;

            continue;
        }

        if (patternSegment.value != pathSegment.value) {
            return false;
        }
    }

    parameters.insert(
        matched.begin(),
        matched.end()
    );

    return true;
}

std::vector<RoutePattern::Segment> RoutePattern::parse(
    const std::string& value
) {
    std::vector<Segment> result;

    std::stringstream stream(value);
    std::string part;

    while (std::getline(stream, part, '/')) {
        if (part.empty()) {
            continue;
        }

        const auto isParameter =
            part.size() >= 3 &&
            part.front() == '{' &&
            part.back() == '}';

        if (isParameter) {
            result.push_back({
                part.substr(1, part.size() - 2),
                true
            });
        } else {
            result.push_back({
                part,
                false
            });
        }
    }

    return result;
}

} // namespace drogular