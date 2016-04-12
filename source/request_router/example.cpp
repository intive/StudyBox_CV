#include <iostream>

#include "../httpserver/Server.h"
#include "../httpserver/Socket.h"
#include "RequestRouter.h"


void registerServicesExample(Router::RequestRouter& router)
{
    router.registerEndPointService("/api/ocr", [](const std::string& s)
    {
        return std::make_pair<std::string, int>(R"({"c":"d"})", 200);
    });
}


void routerExample()
{
    Router::RequestRouter router;
    registerServicesExample(router);

    try
    {
        Http::Server server("0.0.0.0", "8080", [&router](const Http::Request& r)
        {
            return router.routeRequest(r);
        });

        server.run();
    }
    catch (const Tcp::SocketError& e)
    {
        std::cerr << e.what() << std::endl;
    }
}