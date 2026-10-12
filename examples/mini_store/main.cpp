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
#include "features/staff/pages/products_page.hpp"
#include "features/staff/pages/product_form_page.hpp"
#include "features/staff/pages/categories_page.hpp"
#include "features/staff/pages/category_form_page.hpp"
#include "features/staff/pages/orders_page.hpp"
#include "features/staff/pages/order_details_page.hpp"
#include "features/staff/actions/save_product_action.hpp"
#include "features/staff/actions/save_category_action.hpp"
#include "features/staff/actions/advance_order_action.hpp"
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
    app.page<MiniStoreStaffProductsPage>("/staff/products");
    app.page<MiniStoreStaffProductFormPage>("/staff/products/new");
    app.page<MiniStoreStaffProductFormPage>("/staff/products/{id}/edit");
    app.page<MiniStoreStaffCategoriesPage>("/staff/categories");
    app.page<MiniStoreStaffCategoryFormPage>("/staff/categories/new");
    app.page<MiniStoreStaffCategoryFormPage>("/staff/categories/{id}/edit");
    app.page<MiniStoreStaffOrdersPage>("/staff/orders");
    app.page<MiniStoreStaffOrderDetailsPage>("/staff/orders/{id}");
    app.page<MiniStoreCatalogPage>("/catalog");
    app.page<MiniStoreProductDetailsPage>("/products/{id}");
    app.page<MiniStoreCartPage>("/cart");
    app.page<MiniStoreCheckoutPage>("/checkout");
    app.page<MiniStoreOrderConfirmationPage>("/order-confirmation");
    app.action<MiniStoreLanguageAction>("/language");
    app.action<MiniStoreSaveProductAction>("/staff/products/save");
    app.action<MiniStoreSaveCategoryAction>("/staff/categories/save");
    app.action<MiniStoreAdvanceOrderAction>("/staff/orders/advance");
    app.action<MiniStoreAddToCartAction>("/cart/add");
    app.action<MiniStoreUpdateCartAction>("/cart/update");
    app.action<MiniStoreCheckoutAction>("/checkout/place");

    app.run(8083);
    return 0;
}