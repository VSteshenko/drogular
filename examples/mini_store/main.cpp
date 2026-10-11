#include "data/mini_store_store.hpp"
#include "pages/staff_home_page.hpp"
#include "pages/storefront_home_page.hpp"
#include "features/catalog/pages/catalog_page.hpp"
#include "features/catalog/pages/product_details_page.hpp"
#include "features/localization/actions/language_action.hpp"
#include "features/cart/actions/add_to_cart_action.hpp"
#include "features/cart/actions/update_cart_action.hpp"
#include "features/cart/actions/checkout_action.hpp"
#include "features/cart/pages/cart_page.hpp"
#include "features/cart/pages/checkout_page.hpp"
#include "features/cart/pages/order_confirmation_page.hpp"
#include "support/mini_store_expression_functions.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>
#include <drogular/file_translation_provider.hpp>
#include <drogular/session_store.hpp>
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
    app.services().add<drogular::SessionStore>(drogular::ServiceLifetime::Singleton);

    app.page<MiniStoreStorefrontHomePage>("/");
    app.page<MiniStoreStaffHomePage>("/staff");
    app.page<MiniStoreCatalogPage>("/catalog");
    app.page<MiniStoreProductDetailsPage>("/products/{id}");
    app.page<MiniStoreCartPage>("/cart");
    app.page<MiniStoreCheckoutPage>("/checkout");
    app.page<MiniStoreOrderConfirmationPage>("/order-confirmation");
    app.action<MiniStoreLanguageAction>("/language");
    app.action<MiniStoreAddToCartAction>("/cart/add");
    app.action<MiniStoreUpdateCartAction>("/cart/update");
    app.action<MiniStoreCheckoutAction>("/checkout/place");

    app.run(8083);
    return 0;
}