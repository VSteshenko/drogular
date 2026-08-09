#pragma once

#include <drogular/action_result.hpp>

#include <drogon/HttpResponse.h>

#include <exception>

namespace drogular {

/**
 * Converts ActionResult to Drogon HttpResponse.
 */
drogon::HttpResponsePtr toHttpResponse(
    const ActionResult& result
);

/**
 * Converts an exception raised during action execution into a safe HTTP response.
 * ActionValidationError becomes 400 Bad Request. Other exceptions become
 * 500 Internal Server Error without exposing their message to the client.
 */
drogon::HttpResponsePtr toHttpErrorResponse(
    const std::exception& error
);

drogon::HttpResponsePtr toHttpErrorResponse();

} // namespace drogular