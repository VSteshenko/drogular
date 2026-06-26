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
                {"login.username", "Username"},
                {"login.password", "Password"},
                {"login.submit", "Login"},
                {"login.error.missing_credentials", "Please enter username and password."},
                {"login.error.invalid_credentials", "Invalid username or password."},
                {"dashboard.title", "Dashboard"},
                {"users.title", "Users"},
                {"users.error.validation", "Please enter a valid username and role."},
                {"users.error.access_denied", "Only administrators can create users."},
                {"users.error.duplicate_user", "A user with this username already exists."},
                {"users.username", "Username"},
                {"users.password", "Password"},
                {"users.role", "Role"},
                {"users.create", "Create User"},
                {"users.success.created", "User was created successfully."},
                {"admin.title", "Admin"},
                {"offline.title", "You are offline"},
                {"auth.login_required.title", "Login required"},
                {"auth.login_required.message", "Please login to continue."},
                {"auth.access_denied.title", "Access denied"},
                {"auth.access_denied.message", "Administrator role required."},
                {"header.signed_in_as", "Signed in as"},
                {"offline.message", "The portal is currently unavailable because there is no network connection."},
                {"offline.hint", "Please reconnect and reload the page."}
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
                {"login.username", "Benutzername"},
                {"login.password", "Passwort"},
                {"login.submit", "Anmelden"},
                {"language.english", "Englisch"},
                {"language.german", "Deutsch"},
                {"login.title", "Anmelden"},
                {"login.error.missing_credentials", "Bitte geben Sie Benutzername und Passwort ein."},
                {"login.error.invalid_credentials", "Ungültiger Benutzername oder Passwort."},
                {"dashboard.title", "Übersicht"},
                {"users.title", "Benutzer"},
                {"users.error.validation", "Bitte geben Sie einen gültigen Benutzernamen und eine Rolle ein."},
                {"users.error.access_denied", "Nur Administratoren können Benutzer erstellen."},
                {"users.error.duplicate_user", "Ein Benutzer mit diesem Namen existiert bereits."},
                {"users.username", "Benutzername"},
                {"users.password", "Passwort"},
                {"users.role", "Rolle"},
                {"users.create", "Benutzer erstellen"},
                {"users.success.created", "Der Benutzer wurde erfolgreich erstellt."},
                {"admin.title", "Admin"},
                {"offline.title", "Sie sind offline"},
                {"auth.login_required.title", "Anmeldung erforderlich"},
                {"auth.login_required.message", "Bitte melden Sie sich an, um fortzufahren."},
                {"auth.access_denied.title", "Zugriff verweigert"},
                {"auth.access_denied.message", "Administratorrechte sind erforderlich."},
                {"header.signed_in_as", "Angemeldet als"},
                {"offline.message", "Das Portal ist derzeit nicht verfügbar, weil keine Netzwerkverbindung besteht."},
                {"offline.hint", "Bitte stellen Sie die Verbindung wieder her und laden Sie die Seite neu."}
            }
        }
    };
};