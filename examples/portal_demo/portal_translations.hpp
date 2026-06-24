#pragma once

#include <string>
#include <unordered_map>

class PortalTranslations {
public:
    std::string get(
        const std::string& locale,
        const std::string& key
    ) const {
        const auto localeFound =
            values_.find(locale);

        if (localeFound == values_.end()) {
            return fallback(key);
        }

        const auto keyFound =
            localeFound->second.find(key);

        if (keyFound == localeFound->second.end()) {
            return fallback(key);
        }

        return keyFound->second;
    }

private:
    std::string fallback(
        const std::string& key
    ) const {
        const auto english =
            values_.find("en");

        if (english == values_.end()) {
            return key;
        }

        const auto found =
            english->second.find(key);

        if (found == english->second.end()) {
            return key;
        }

        return found->second;
    }

    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::string>
    > values_{
        {
            "en",
            {
                {"app.title", "Drogular Portal Demo"},
                {"nav.dashboard", "Dashboard"},
                {"nav.users", "Users"},
                {"nav.admin", "Admin"},
                {"nav.logout", "Logout"},
                {"nav.login", "Login"},
                {"language.english", "English"},
                {"language.german", "German"},
                {"login.title", "Login"},
                {"dashboard.title", "Dashboard"},
                {"users.title", "Users"},
                {"admin.title", "Admin"},
                {"offline.title", "You are offline"}
            }
        },
        {
            "de",
            {
                {"app.title", "Drogular Portal Demo"},
                {"nav.dashboard", "Übersicht"},
                {"nav.users", "Benutzer"},
                {"nav.admin", "Admin"},
                {"nav.logout", "Abmelden"},
                {"nav.login", "Anmelden"},
                {"language.english", "Englisch"},
                {"language.german", "Deutsch"},
                {"login.title", "Anmelden"},
                {"dashboard.title", "Übersicht"},
                {"users.title", "Benutzer"},
                {"admin.title", "Admin"},
                {"offline.title", "Sie sind offline"}
            }
        }
    };
};