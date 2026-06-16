#include <drogular/action_response.hpp>

namespace drogular {

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

            return response;
        }

        case ActionResultType::Html: {
            auto response =
                drogon::HttpResponse::newHttpResponse();

            response->setContentTypeCode(
                drogon::CT_TEXT_HTML
            );

            response->setBody(result.body());

            return response;
        }

        case ActionResultType::Json: {
            return drogon::HttpResponse::newHttpJsonResponse(
                result.json()
            );
        }
    }

    return drogon::HttpResponse::newHttpResponse();
}

} // namespace drogular
