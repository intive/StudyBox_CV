#include <iostream>

#include "httpserver/Server.h"
#include "httpserver/Socket.h"
#include "request_router/RequestRouter.h"


void registerServices(Router::RequestRouter& router)
{
    router.registerEndPointService("/api/test", [](const std::string& s)
    {
        return std::make_pair<std::string, int>(R"({"test":"ok"})", 200);
    });
}


int main()
{
    Router::RequestRouter router;
    registerServices(router);

    try
    {
        Http::Server server("0.0.0.0", "80", [&router](const Http::Request& r)
        {
            return router.routeRequest(r);
        });

        server.run();
    }
    catch (const Tcp::SocketError& e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}