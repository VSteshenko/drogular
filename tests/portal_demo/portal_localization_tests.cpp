#include "portal_application_test_host.hpp"

#include "data/demo_dataset.hpp"
#include "features/localization/actions/language_action.hpp"
#include "features/localization/support/portal_error_translator.hpp"
#include "features/localization/support/portal_translations.hpp"

#include <drogular/action_context.hpp>
#include <drogular/action_validation_error.hpp>
#include <drogular/locale_support.hpp>
#include <drogular/render_context.hpp>

#include <drogon/HttpRequest.h>
#include <gtest/gtest.h>

namespace {

drogular::ActionContext makeActionContext(
    PortalApplicationTestHost& app,
    const std::unordered_map<std::string, std::string>& form
) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Post);

    for (const auto& [name, value] : form) {
        request->setParameter(name, value);
    }

    return drogular::ActionContext(
        request,
        &app.services()
    );
}

drogular::RenderContext makeRenderContext(
    PortalApplicationTestHost& app,
    const std::string& language = ""
) {
    auto request = drogon::HttpRequest::newHttpRequest();
    request->setMethod(drogon::Get);

    if (!language.empty()) {
        request->addCookie("lang", language);
    }

    drogular::RenderContext context;
    context.setServices(&app.services());
    context.setRequest(request);
    return context;
}

} // namespace

TEST(PortalLocalizationTests, TranslationsReturnEnglishValue) {
    PortalTranslations translations;

    EXPECT_EQ(
        translations.get("en", "dashboard.title"),
        "Dashboard"
    );
}

TEST(PortalLocalizationTests, TranslationsReturnGermanValue) {
    PortalTranslations translations;

    EXPECT_EQ(
        translations.get("de", "dashboard.title"),
        "Übersicht"
    );
}

TEST(PortalLocalizationTests, UnknownLocaleFallsBackToEnglish) {
    PortalTranslations translations;

    EXPECT_EQ(
        translations.get("fr", "login.title"),
        "Login"
    );
}

TEST(PortalLocalizationTests, MissingGermanKeyFallsBackToEnglish) {
    PortalTranslations translations;

    EXPECT_EQ(
        translations.get("de", "missing.key"),
        "missing.key"
    );
}

TEST(PortalLocalizationTests, UnknownKeyFallsBackToKey) {
    PortalTranslations translations;

    EXPECT_EQ(
        translations.get("en", "unknown.translation.key"),
        "unknown.translation.key"
    );
}

TEST(PortalLocalizationTests, RenderContextUsesLanguageCookie) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeRenderContext(app, "de");

    EXPECT_EQ(
        drogular::LocaleSupport::current(context),
        "de"
    );
    EXPECT_EQ(
        context.translate("dashboard.title"),
        "Übersicht"
    );
}

TEST(PortalLocalizationTests, UnsupportedLanguageCookieFallsBackToEnglish) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeRenderContext(app, "fr");

    EXPECT_EQ(
        drogular::LocaleSupport::current(context),
        "en"
    );
    EXPECT_EQ(
        context.translate("dashboard.title"),
        "Dashboard"
    );
}

TEST(PortalLocalizationTests, LanguageActionStoresGermanAndPreservesLocalRedirect) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeActionContext(
        app,
        {
            {"language", "de"},
            {"redirect", "/projects?page=2"}
        }
    );

    PortalLanguageAction action;
    const auto result = action.handle(context);

    EXPECT_EQ(result.location(), "/projects?page=2");
    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].name, "lang");
    EXPECT_EQ(result.cookies()[0].value, "de");
    EXPECT_EQ(result.cookies()[0].path, "/");
}

TEST(PortalLocalizationTests, LanguageActionNormalizesUnsupportedLanguageToEnglish) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeActionContext(
        app,
        {{"language", "fr"}}
    );

    PortalLanguageAction action;
    const auto result = action.handle(context);

    EXPECT_EQ(result.location(), "/dashboard");
    ASSERT_EQ(result.cookies().size(), 1);
    EXPECT_EQ(result.cookies()[0].value, "en");
}

TEST(PortalLocalizationTests, LanguageActionRejectsExternalRedirect) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeActionContext(
        app,
        {
            {"language", "de"},
            {"redirect", "https://example.com"}
        }
    );

    PortalLanguageAction action;
    const auto result = action.handle(context);

    EXPECT_EQ(result.location(), "/dashboard");
}

TEST(PortalLocalizationTests, LanguageActionRejectsProtocolRelativeRedirect) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeActionContext(
        app,
        {
            {"language", "de"},
            {"redirect", "//example.com"}
        }
    );

    PortalLanguageAction action;
    const auto result = action.handle(context);

    EXPECT_EQ(result.location(), "/dashboard");
}

TEST(PortalLocalizationTests, LanguageActionRequiresLanguageField) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeActionContext(app, {});

    PortalLanguageAction action;

    EXPECT_THROW(
        action.handle(context),
        drogular::ActionValidationError
    );
}

TEST(PortalLocalizationTests, ErrorTranslatorUsesCurrentLocale) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeRenderContext(app, "de");

    EXPECT_EQ(
        PortalErrorTranslator::loginError(
            context,
            "invalid_credentials"
        ),
        "Ungültiger Benutzername oder Passwort."
    );
    EXPECT_EQ(
        PortalErrorTranslator::usersError(
            context,
            "duplicate_user"
        ),
        "Ein Benutzer mit diesem Namen existiert bereits."
    );
    EXPECT_EQ(
        PortalErrorTranslator::projectsSuccess(
            context,
            "project_created"
        ),
        "Das Projekt wurde erfolgreich erstellt."
    );
    EXPECT_EQ(
        PortalErrorTranslator::projectTypesError(
            context,
            "project_type_in_use"
        ),
        "Dieser Projekttyp wird von einem oder mehreren Projekten verwendet und kann nicht gelöscht werden."
    );
    EXPECT_EQ(
        PortalErrorTranslator::rolesSuccess(
            context,
            "role_created"
        ),
        "Die Rolle wurde erfolgreich erstellt."
    );
}

TEST(PortalLocalizationTests, ErrorTranslatorReturnsEmptyForUnknownCode) {
    PortalApplicationTestHost app(DemoDataset::create());
    auto context = makeRenderContext(app);

    EXPECT_TRUE(
        PortalErrorTranslator::loginError(
            context,
            "unknown"
        ).empty()
    );
    EXPECT_TRUE(
        PortalErrorTranslator::usersSuccess(
            context,
            "unknown"
        ).empty()
    );
    EXPECT_TRUE(
        PortalErrorTranslator::projectsError(
            context,
            "unknown"
        ).empty()
    );
    EXPECT_TRUE(
        PortalErrorTranslator::projectTypesSuccess(
            context,
            "unknown"
        ).empty()
    );
    EXPECT_TRUE(
        PortalErrorTranslator::rolesError(
            context,
            "unknown"
        ).empty()
    );
}
