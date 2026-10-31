#include "mini_store_application_test_host.hpp"
#include "support/html_test_support.hpp"

#include "features/cart/actions/add_to_cart_action.hpp"
#include "features/cart/actions/checkout_action.hpp"
#include "features/cart/actions/update_cart_action.hpp"
#include "features/cart/pages/cart_page.hpp"
#include "features/cart/pages/checkout_page.hpp"
#include "features/catalog/pages/catalog_page.hpp"
#include "features/catalog/pages/product_details_page.hpp"
#include "features/staff/actions/advance_order_action.hpp"
#include "features/staff/pages/order_details_page.hpp"
#include "features/staff/pages/orders_page.hpp"
#include "features/staff/pages/product_form_page.hpp"
#include "features/staff/pages/products_page.hpp"

#include <drogular/action_result.hpp>

#include <gtest/gtest.h>

TEST(MiniStoreApplicationTests, CatalogRendersLocalizedFilteredStateAndCanonicalHistoryUrl) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto html = app.render<MiniStoreCatalogPage>(
        {
            {"q", "Kaffee"},
            {"category", "coffee-tea"},
            {"sort", "price-asc"}
        },
        {},
        "/catalog"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, "Hausmischung Kaffee"));
    EXPECT_FALSE(HtmlTestSupport::containsText(html, "Digital Kitchen Scale"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(name="q")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="Kaffee")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(dg-history-url="/catalog?q=Kaffee&amp;category=coffee-tea&amp;sort=price-asc")"
    ));
}

TEST(MiniStoreApplicationTests, ProductDetailsPreserveSafeCatalogReturnUrl) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto html = app.render<MiniStoreProductDetailsPage>(
        {
            {"returnUrl", "/catalog?category=kitchen"}
        },
        {
            {"id", "3"}
        },
        "/products/3"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, "Kaffeekanne aus Glas"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(href="/catalog?category=kitchen")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(name="productId")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="3")"));
}

TEST(MiniStoreApplicationTests, ProductDetailsRejectUnsafeReturnUrl) {
    MiniStoreApplicationTestHost app;

    const auto html = app.render<MiniStoreProductDetailsPage>(
        {
            {"returnUrl", "https://example.test"}
        },
        {
            {"id", "1"}
        },
        "/products/1"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(href="/catalog")"));
    EXPECT_FALSE(HtmlTestSupport::containsText(html, "https://example.test"));
}

TEST(MiniStoreApplicationTests, CartPersistsSessionAndRendersLocalizedItem) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto add = app.post<MiniStoreAddToCartAction>({
        {"productId", "1"},
        {"quantity", "2"},
        {"returnUrl", "/cart"}
    });
    ASSERT_EQ(add.type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(add.location(), "/cart");
    app.adoptSession(add);

    const auto html = app.render<MiniStoreCartPage>(
        {},
        {},
        "/cart");
    EXPECT_TRUE(HtmlTestSupport::containsText(html, "Hausmischung Kaffee"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, "€25.80"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="2")"));
}

TEST(MiniStoreApplicationTests, CartInteractionUpdateReturnsFragmentAndClampsStock) {
    MiniStoreApplicationTestHost app;

    const auto add = app.post<MiniStoreAddToCartAction>({
        {"productId", "5"},
        {"quantity", "1"},
        {"returnUrl", "/cart"}
    });
    app.adoptSession(add);

    const auto result = app.postInteraction<MiniStoreUpdateCartAction>({
        {"productId", "5"},
        {"quantity", "99"}
    });

    EXPECT_EQ(result.type(), drogular::ActionResultType::Html);
    EXPECT_EQ(result.statusCode(), drogon::k200OK);
    EXPECT_TRUE(HtmlTestSupport::containsText(result.body(), "Digital Kitchen Scale"));
    EXPECT_TRUE(HtmlTestSupport::containsText(result.body(), R"(value="4")"));
    EXPECT_FALSE(HtmlTestSupport::containsText(result.body(), "<!doctype html>"));
}

TEST(MiniStoreApplicationTests, CheckoutRejectsInvalidCustomerWithoutCreatingOrder) {
    MiniStoreApplicationTestHost app;
    const auto before = app.store()->orders().size();

    const auto result = app.post<MiniStoreCheckoutAction>({
        {"name", "Ada"},
        {"email", "invalid"}
    });

    EXPECT_EQ(result.type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(result.location(), "/checkout?error=customer");
    EXPECT_EQ(app.store()->orders().size(), before);
}

TEST(MiniStoreApplicationTests, CheckoutCreatesLocalizedOrderAndClearsCart) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");
    const auto before = app.store()->orders().size();

    const auto add = app.post<MiniStoreAddToCartAction>({
        {"productId", "1"},
        {"quantity", "2"},
        {"returnUrl", "/cart"}
    });
    app.adoptSession(add);

    const auto result = app.post<MiniStoreCheckoutAction>({
        {"name", "Ada Example"},
        {"email", "ada@example.test"}
    });

    ASSERT_EQ(result.type(), drogular::ActionResultType::Redirect);
    EXPECT_EQ(app.store()->orders().size(), before + 1);
    EXPECT_TRUE(result.location().starts_with("/order-confirmation?id="));

    const auto orders = app.store()->orders();
    const auto created = std::find_if(
        orders.begin(), orders.end(),
        [](const auto& order) {
            return order.customer.email == "ada@example.test";
        }
    );
    ASSERT_NE(created, orders.end());
    ASSERT_EQ(created->items.size(), 1U);
    EXPECT_EQ(created->items.front().productName, "Hausmischung Kaffee");

    const auto cartHtml = app.render<MiniStoreCartPage>(
        {},
        {},
        "/cart");
    EXPECT_TRUE(HtmlTestSupport::containsText(cartHtml, "Warenkorb ist leer"));
}

TEST(MiniStoreApplicationTests, CheckoutPageRendersCustomerAndStockErrors) {
    MiniStoreApplicationTestHost app;
    const auto add = app.post<MiniStoreAddToCartAction>({
        {"productId", "1"},
        {"quantity", "1"},
        {"returnUrl", "/cart"}
    });
    app.adoptSession(add);

    const auto customer = app.render<MiniStoreCheckoutPage>(
        {
            {"error", "customer"}
        },
        {},
        "/checkout"
    );
    const auto stock = app.render<MiniStoreCheckoutPage>(
        {
            {"error", "stock"}
        },
        {},
        "/checkout"
    );

    const auto clean = app.render<MiniStoreCheckoutPage>(
        {}, {}, "/checkout"
    );

    constexpr auto errorAlert = R"(class="dg-alert dg-alert-danger")";
    EXPECT_TRUE(HtmlTestSupport::containsText(customer, errorAlert));
    EXPECT_TRUE(HtmlTestSupport::containsText(stock, errorAlert));
    EXPECT_FALSE(HtmlTestSupport::containsText(clean, errorAlert));
}

TEST(MiniStoreApplicationTests, StaffProductsRenderStockAndStatusSemantics) {
    MiniStoreApplicationTestHost app;

    auto inactive = app.store()->product(1);
    ASSERT_TRUE(inactive);
    inactive->active = false;
    ASSERT_TRUE(app.store()->saveProduct(*inactive));

    const auto html = app.render<MiniStoreStaffProductsPage>(
        {},
        {},
        "/staff/products"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(class="mini-store-staff-stock-state is-low")"
    ));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(class="mini-store-staff-stock-state is-out")"
    ));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(class="mini-store-staff-stock-value">4</strong>)"
    ));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(class="mini-store-staff-stock-value">0</strong>)"
    ));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(href="/staff/products/1/edit")"
    ));
}

TEST(MiniStoreApplicationTests, StaffProductFormRendersExistingLocalizedValues) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto html = app.render<MiniStoreStaffProductFormPage>(
        {},
        {
            {"id", "1"}
        },
        "/staff/products/1/edit"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, "Produkt bearbeiten"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="COF-001")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="House Blend Coffee")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="Hausmischung Kaffee")"));
}

TEST(MiniStoreApplicationTests, StaffOrdersFilterAndPreserveCanonicalReturnUrl) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto html = app.render<MiniStoreStaffOrdersPage>(
        {
            {"status", "processing"}
        },
        {},
        "/staff/orders"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, "#1007"));
    EXPECT_FALSE(HtmlTestSupport::containsText(html, "#1008"));
    EXPECT_FALSE(HtmlTestSupport::containsText(html, "#1006"));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        "returnUrl=%2Fstaff%2Forders%3Fstatus%3Dprocessing"
    ));
    EXPECT_TRUE(HtmlTestSupport::containsText(
        html,
        R"(dg-history-url="/staff/orders?status=processing")"
    ));
}

TEST(MiniStoreApplicationTests, StaffOrderDetailsRenderSnapshotAndTransitionTarget) {
    MiniStoreApplicationTestHost app;
    app.setLocale("de");

    const auto html = app.render<MiniStoreStaffOrderDetailsPage>(
        {
            {"returnUrl", "/staff/orders?status=new"}
        },
        {
            {"id", "1008"}
        },
        "/staff/orders/1008"
    );

    EXPECT_TRUE(HtmlTestSupport::containsText(html, "Mia Weber"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, "mia@example.test"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, "In Bearbeitung"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(name="returnUrl")"));
    EXPECT_TRUE(HtmlTestSupport::containsText(html, R"(value="/staff/orders?status=new")"));
}

TEST(MiniStoreApplicationTests, StaffOrderInteractionAdvancesAndReturnsDetailsFragment) {
    MiniStoreApplicationTestHost app;

    const auto result = app.postInteraction<MiniStoreAdvanceOrderAction>({
        {"id", "1008"},
        {"returnUrl", "/staff/orders?status=new"}
    });

    EXPECT_EQ(result.type(), drogular::ActionResultType::Html);
    EXPECT_EQ(result.statusCode(), drogon::k200OK);
    EXPECT_TRUE(HtmlTestSupport::containsText(result.body(), "Mia Weber"));
    EXPECT_TRUE(HtmlTestSupport::containsText(result.body(), "Processing"));
    EXPECT_FALSE(HtmlTestSupport::containsText(result.body(), "<!doctype html>"));

    const auto order = app.store()->order(1008);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, MiniStoreOrderStatus::Processing);
}

TEST(MiniStoreApplicationTests, CompletedOrderRejectsFurtherInteractionTransition) {
    MiniStoreApplicationTestHost app;

    const auto result = app.postInteraction<MiniStoreAdvanceOrderAction>({
        {"id", "1006"},
        {"returnUrl", "/staff/orders?status=completed"}
    });

    EXPECT_EQ(result.type(), drogular::ActionResultType::Html);
    EXPECT_EQ(result.statusCode(), drogon::k422UnprocessableEntity);
    EXPECT_TRUE(HtmlTestSupport::containsText(result.body(), "Completed"));

    const auto order = app.store()->order(1006);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, MiniStoreOrderStatus::Completed);
}