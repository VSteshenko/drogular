#pragma once

#include <drogular/action_result.hpp>

#include <drogon/HttpResponse.h>

namespace drogular {

/**
 * Converts ActionResult to Drogon HttpResponse.
 */
drogon::HttpResponsePtr toHttpResponse(
    const ActionResult& result
);

} // namespace drogular
