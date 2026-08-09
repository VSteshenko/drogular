#include <drogular/action_response.hpp>
#include <drogular/static_file_response.hpp>
#include <drogular/action_validation_error.hpp>

namespace drogular {

namespace {

void applyCookies(
    const ActionResult& result,
    const drogon::HttpResponsePtr& response
) {
    for (const auto& cookie : result.cookies()) {
        drogon::Cookie drogonCookie(
            cookie.name,
            cookie.value
        );

        drogonCookie.setPath(cookie.path);
        drogonCookie.setHttpOnly(cookie.httpOnly);
        drogonCookie.setSecure(cookie.secure);

        if (cookie.maxAge.has_value()) {
            drogonCookie.setMaxAge(*cookie.maxAge);
        }

        switch (cookie.sameSite) {
            case CookieSameSite::Unspecified:
                break;

            case CookieSameSite::Lax:
                drogonCookie.setSameSite(drogon::Cookie::SameSite::kLax);
                break;

            case CookieSameSite::Strict:
                drogonCookie.setSameSite(drogon::Cookie::SameSite::kStrict);
                break;

            case CookieSameSite::None:
                drogonCookie.setSameSite(drogon::Cookie::SameSite::kNone);
                break;
        }

        response->addCookie(drogonCookie);
    }
}

} // namespace

drogon::HttpResponsePtr toHttpResponse(
    const ActionResult& result
) {
    switch (result.type()) {
        case ActionResultType::Empty: {
            auto response =
                drogon::HttpResponse::newHttpResponse();

            applyCookies(result, response);

            return response;
        }

        case ActionResultType::Redirect: {
            auto response =
                drogon::HttpResponse::newRedirectionResponse(
                    result.location()
                );

            applyCookies(result, response);
            return response;
        }

        case ActionResultType::Html: {
            auto response =
                drogon::HttpResponse::newHttpResponse();

            response->setContentTypeCode(
                drogon::CT_TEXT_HTML
            );

            response->setBody(result.body());

            applyCookies(result, response);
            return response;
        }

        case ActionResultType::Json: {
            auto response =
                drogon::HttpResponse::newHttpJsonResponse(result.json());

            applyCookies(result, response);
            return response;
        }

        case ActionResultType::File: {
            auto response =
                StaticFileResponse::create(
                    result.fileInfo().path
                );

            if (result.fileInfo().forceDownload) {
                response->addHeader(
                    "Content-Disposition",
                    "attachment; filename=\"" +
                        result.fileInfo().downloadName +
                        "\""
                );
            }

            applyCookies(result, response);

            return response;
        }
    }

    return drogon::HttpResponse::newHttpResponse();
}

drogon::HttpResponsePtr toHttpErrorResponse(
    const std::exception& error
) {
    if (const auto* validationError = dynamic_cast<const ActionValidationError*>(&error)) {
        auto response = drogon::HttpResponse::newHttpResponse();

        response->setStatusCode(drogon::k400BadRequest);
        response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
        response->setBody(validationError->what());

        return response;
    }

    auto response = drogon::HttpResponse::newHttpResponse();

    response->setStatusCode(drogon::k500InternalServerError);
    response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    response->setBody("Internal Server Error");

    return response;
}

drogon::HttpResponsePtr toHttpErrorResponse() {
    auto response = drogon::HttpResponse::newHttpResponse();

    response->setStatusCode(drogon::k500InternalServerError);
    response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
    response->setBody("Internal Server Error");

    return response;
}

} // namespace drogular