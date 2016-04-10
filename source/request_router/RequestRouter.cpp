#include <iostream>

#include "RequestRouter.h"
#include "../httpserver/Server.h"


namespace Router
{
    Http::Response RequestRouter::routeRequest(const Http::Request& request)
    {
        auto func = services.find(request.uri().raw());

        if (func == services.end())
        {
            return Http::Response(
                Http::Response::Status::NotFound,
                R"({"error":"no service for )" + request.uri().raw() + R"("})",
                "application/json");
        }


        try
        {
            std::string body;
            int response_code;
            std::tie(body, response_code) = func->second(request.body());
           
            return Http::Response(
                static_cast<Http::Response::Status>(response_code),
                body,
                "application/json");
        }
        catch (const std::exception& e)
        {
            if (emitExceptionsToStdcerr)
                std::cerr << "REQUEST ROUTER: Caught exception in execution handler for " + request.uri().raw() + " endpoint: "
                          << e.what()
                          << std::endl;

            return Http::Response(
                Http::Response::Status::InternalServerError,
                R"({"error":"internal server error"})",
                "application/json");
        }
    }


    void RequestRouter::registerEndPointService(const std::string& endPoint, EndpointHandler func)
    {
        auto lookup = services.find(endPoint);
        if (lookup != services.end())
        {
            lookup->second = std::move(func);
        }
        else
        {
            services.insert({ endPoint, std::move(func) });
        }
    };
}