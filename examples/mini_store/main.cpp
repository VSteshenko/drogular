#include "data/mini_store_store.hpp"
#include "pages/staff_home_page.hpp"
#include "pages/storefront_home_page.hpp"
#include "features/catalog/pages/catalog_page.hpp"
#include "features/catalog/pages/product_details_page.hpp"
#include "features/localization/actions/language_action.hpp"
#include "support/mini_store_expression_functions.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>
#include <drogular/file_translation_provider.hpp>
#include <drogular/translation_provider.hpp>

int main() {
    drogular::App app;

    app.ui()
       .templateRoot("examples/mini_store/templates")
       .staticFiles("/assets", "examples/mini_store/public")
       .templateCache(false)
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .profile(drogular::ApplicationProfile::Development);

    app.expressionFunction("t", miniStoreTranslationExpressionFunction());

    app.services().addFactory<drogular::TranslationProvider>(
        drogular::ServiceLifetime::Singleton,
        [] {
            return std::make_shared<drogular::FileTranslationProvider>(
                "examples/mini_store/resources/localization",
                "en"
            );
        }
    );

    app.services().add<MiniStoreStore>(drogular::ServiceLifetime::Singleton);

    app.page<MiniStoreStorefrontHomePage>("/");
    app.page<MiniStoreStaffHomePage>("/staff");
    app.page<MiniStoreCatalogPage>("/catalog");
    app.page<MiniStoreProductDetailsPage>("/products/{id}");
    app.action<MiniStoreLanguageAction>("/language");

    app.run(8083);
    return 0;
}