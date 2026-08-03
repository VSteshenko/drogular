#pragma once

namespace drogular {

enum class ApplicationProfile {
    Development,
    Testing,
    Production
};

constexpr const char* toString(ApplicationProfile profile) {
    switch (profile) {
    case ApplicationProfile::Development:
        return "development";

    case ApplicationProfile::Testing:
        return "testing";

    case ApplicationProfile::Production:
        return "production";
    }

    return "production";
}

} // namespace drogular