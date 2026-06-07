#include <drogular/app.hpp>

#include <drogon/drogon.h>

namespace drogular {

void App::run(unsigned short port) {
    drogon::app()
        .addListener("0.0.0.0", port)
        .run();
}

} // namespace drogular
