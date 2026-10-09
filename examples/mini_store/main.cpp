#include "data/mini_store_store.hpp"
#include "pages/staff_home_page.hpp"
#include "pages/storefront_home_page.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

int main() {
    drogular::App app;

    app.ui()
       .templateRoot("examples/mini_store/templates")
       .staticFiles("/assets", "examples/mini_store/public")
       .templateCache(false)
       .staticFileCacheProfile(drogular::StaticFileCacheProfile::Development)
       .profile(drogular::ApplicationProfile::Development);

    app.services().add<MiniStoreStore>(drogular::ServiceLifetime::Singleton);

    app.page<MiniStoreStorefrontHomePage>("/");
    app.page<MiniStoreStaffHomePage>("/staff");

    app.run(8083);
    return 0;
}