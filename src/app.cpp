#include <drogular/app.hpp>

#include <drogon/drogon.h>

namespace drogular {

App::App() {
    services_.setOptions(&options_);
}

void App::run(unsigned short port) {
    drogon::app()
        .addListener("0.0.0.0", port)
        .run();
}

} // namespace drogular
