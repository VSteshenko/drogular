#include <drogular/action_response.hpp>

namespace drogular {

namespace {

void applyCookies(
    const drogular::ActionResult& result,
    const drogon::HttpResponsePtr& response
) {
    for (const auto& cookie : result.cookies()) {
        drogon::Cookie drogonCookie(
            cookie.name,
            cookie.value
        );

        drogonCookie.setPath(cookie.path);

        if (cookie.httpOnly) {
            drogonCookie.setHttpOnly(true);
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
            return drogon::HttpResponse::newHttpResponse();
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
    }

    return drogon::HttpResponse::newHttpResponse();
}

} // namespace drogular
