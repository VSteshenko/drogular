#include <drogular/router.hpp>
#include <drogular/services.hpp>
#include <drogular/page.hpp>
#include <drogular/action_response.hpp>
#include <drogular/static_file_resolver.hpp>
#include <drogular/static_file_response.hpp>
#include <drogular/static_file_etag.hpp>
#include <drogular/static_file_last_modified.hpp>
#include <drogular/render_context.hpp>
#include <drogular/component_renderer.hpp>
#include <drogular/route_pattern.hpp>
#include <drogular/error.hpp>
#include <drogular/detail/request_context_state.hpp>

#include <drogon/drogon.h>

#include <filesystem>
#include <unordered_map>
#include <string_view>

namespace drogular {

namespace {

std::string regexEscape(const std::string& value) {
    std::string escaped;
    for (const char ch : value) {
        if (std::string_view(R"(\.^$|()[]{}*+?)").find(ch) !=
            std::string_view::npos) {
            escaped.push_back('\\');
        }
        escaped.push_back(ch);
    }

    return escaped;
}

void recordActionResponseDiagnostics(
    RuntimeDiagnostics* diagnostics,
    bool interaction,
    const drogon::HttpResponsePtr& response
) {
    if (diagnostics == nullptr || response == nullptr) {
        return;
    }

    const auto status =
        static_cast<int>(response->statusCode());

    if (status >= 300 && status < 400) {
        diagnostics->recordRedirect();

        if (interaction) {
            diagnostics->recordInteractionRedirect();
        }

        return;
    }

    if (!interaction) {
        return;
    }

    if (status >= 200 && status < 300) {
        diagnostics->recordInteractionSuccessfulResponse();
    } else if (status >= 400 && status < 500) {
        diagnostics->recordInteractionClientErrorResponse();
    }
}

} // namespace

Router::Router(
    ApplicationServices* services,
    RuntimeDiagnostics* diagnostics
)
    : services_(services),
      diagnostics_(diagnostics) {
}

void Router::page(
    const std::string& path,
    PageFactory factory,
    std::string target
) {
    routes_.push_back({
        path,
        RouteKind::Page,
        "GET",
        std::move(target)
    });

    auto* services = services_;
    auto* diagnostics = diagnostics_;
    const RoutePattern pattern(path);

    drogon::app().registerHandler(
        path,
        [factory = std::move(factory), services, diagnostics, pattern](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            if (diagnostics != nullptr) {
                diagnostics->recordRequest();
            }

            auto state =
                std::make_shared<detail::RequestContextState>(
                    request,
                    services,
                    diagnostics
                );

            std::unordered_map<std::string, std::string> routeParams;

            pattern.match(
                request->path(),
                routeParams
            );

            for (const auto& [name, value] : routeParams) {
                state->setRouteParam(
                    name,
                    value
                );
            }

            RenderContext context(state);

            auto page = factory();

            auto response = drogon::HttpResponse::newHttpResponse();
            response->setContentTypeCode(drogon::CT_TEXT_HTML);
            response->setBody(
                component_renderer::renderComponentTree(*page, context)
            );

            callback(response);
        }
    );
}

void Router::action(
    const std::string& path,
    ActionFactory factory,
    std::string target,
    ActionMethod method
) {
    const auto methodName =
        method == ActionMethod::Get ? "GET" : "POST";
    const auto drogonMethod =
        method == ActionMethod::Get ? drogon::Get : drogon::Post;

    routes_.push_back({
        path,
        RouteKind::Action,
        methodName,
        std::move(target)
    });

    auto* services = services_;
    auto* diagnostics = diagnostics_;
    const RoutePattern pattern(path);

    drogon::app().registerHandler(
        path,
        [factory = std::move(factory), services, diagnostics, pattern, method](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            if (diagnostics != nullptr) {
                diagnostics->recordRequest();
                diagnostics->recordAction();
            }

            auto state =
                std::make_shared<detail::RequestContextState>(
                    request,
                    services,
                    diagnostics
                );

            std::unordered_map<std::string, std::string> routeParams;

            pattern.match(
                request->path(),
                routeParams
            );

            for (const auto& [name, value] : routeParams) {
                state->setRouteParam(
                    name,
                    value
                );
            }

            ActionContext context(state);
            const auto interaction = context.isInteraction();

            if (interaction && diagnostics != nullptr) {
                diagnostics->recordInteractionRequest();

                if (method == ActionMethod::Get) {
                    diagnostics->recordInteractionGetRequest();
                } else {
                    diagnostics->recordInteractionPostRequest();
                }
            }

            try {
                auto action = factory();

                if (action == nullptr) {
                    throw DrogularError(
                        "Action factory returned nullptr"
                    );
                }

                const auto result =
                    action->handle(context);

                auto response = toHttpResponse(result);

                recordActionResponseDiagnostics(
                    diagnostics,
                    interaction,
                    response
                );

                callback(response);
            } catch (const std::exception& error) {
                LOG_ERROR << "Drogular action failed: " << error.what();

                auto response = toHttpErrorResponse(error);

                recordActionResponseDiagnostics(
                    diagnostics,
                    interaction,
                    response
                );

                callback(response);
            } catch (...) {
                LOG_ERROR << "Drogular action failed with an unknown exception";

                auto response = toHttpErrorResponse();

                recordActionResponseDiagnostics(
                    diagnostics,
                    interaction,
                    response
                );

                callback(response);
            }
        },
        {drogonMethod}
    );
}

void Router::staticFiles(
    const std::string& routePrefix,
    const std::filesystem::path& directory,
    bool recursive
) {
    routes_.push_back({
        routePrefix,
        RouteKind::StaticFiles,
        "GET",
        directory.string()
    });

    auto normalizedPrefix = routePrefix;

    if (!normalizedPrefix.empty() &&
        normalizedPrefix.back() == '/') {
        normalizedPrefix.pop_back();
        }

    const auto rootDirectory =
        std::filesystem::weakly_canonical(directory);

    auto* options =
        services_ != nullptr
            ? services_->options()
            : nullptr;

    // A single capture group provides the entire relative path, including
    // nested segments. The resolver remains the security boundary.
    const auto pattern =
        "^" + regexEscape(normalizedPrefix) +
        (recursive ? "/(.+)$" : "/([^/]+)$");

    auto handler = [rootDirectory, options, recursive](
            const drogon::HttpRequestPtr& request,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
            const std::string& filePath
        ) {
            StaticFileResolver resolver(rootDirectory);
            const auto resolved = resolver.resolve(filePath, recursive);

            if (!resolved.has_value()) {
                auto response =
                    drogon::HttpResponse::newHttpResponse();
                response->setStatusCode(drogon::k404NotFound);
                callback(response);
                return;
            }

            StaticFileResponseOptions responseOptions;

            if (options != nullptr) {
                responseOptions.cacheEnabled =
                    options->staticFileCacheEnabled();

                responseOptions.maxAge =
                    options->staticFileCacheMaxAge();

                responseOptions.etagEnabled =
                    options->staticFileEtagEnabled();

                responseOptions.lastModifiedEnabled =
                    options->staticFileLastModifiedEnabled();
            }

            if (responseOptions.etagEnabled) {
                const auto etag =
                    StaticFileEtag::create(
                        *resolved
                    );

                const auto requestEtag =
                    request->getHeader("If-None-Match");

                if (requestEtag == etag) {
                    callback(
                        StaticFileResponse::notModified(etag)
                    );

                    return;
                }
            }

            if (responseOptions.lastModifiedEnabled) {
                const auto lastModified =
                    drogular::StaticFileLastModified::create(
                        *resolved
                    );

                const auto requestLastModified =
                    request->getHeader("If-Modified-Since");

                if (requestLastModified == lastModified) {
                    callback(
                        drogular::StaticFileResponse::notModified(
                            responseOptions.etagEnabled
                                ? drogular::StaticFileEtag::create(*resolved)
                                : ""
                        )
                    );

                    return;
                }
            }

            auto response =
                StaticFileResponse::create(
                    *resolved,
                    responseOptions
                );

            callback(response);
        };

    if (recursive) {
        drogon::app().registerHandlerViaRegex(
            pattern, handler, {drogon::Get}
        );
    } else {
        // Retain the original route registration and its precedence.
        drogon::app().registerHandler(
            normalizedPrefix + "/{filePath}", handler, {drogon::Get}
        );
    }
}

void Router::serviceWorker(
    const std::filesystem::path& path
) {
    routes_.push_back({
        "/service-worker.js",
        RouteKind::ServiceWorker,
        "GET",
        path.string()
    });

    drogon::app().registerHandler(
        "/service-worker.js",
        [path](
            const drogon::HttpRequestPtr&,
            std::function<void(const drogon::HttpResponsePtr&)>&& callback
        ) {
            auto response =
                drogular::StaticFileResponse::create(path);

            response->setContentTypeString(
                "application/javascript"
            );

            response->addHeader(
                "Service-Worker-Allowed",
                "/"
            );

            callback(response);
        },
        {drogon::Get}
    );
}

} // namespace drogular