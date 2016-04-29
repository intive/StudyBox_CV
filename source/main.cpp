#include <iostream>

#include "httpserver/Server.h"
#include "httpserver/Socket.h"
#include "request_router/RequestRouter.h"
#include "request_router/SegmentationResponse.h"
#include "request_router/TextAnalysisResponse.h"


void registerServices(Router::RequestRouter& router)
{
    registerSegmentationResponse(router);
    registerTextAnalysisResponse(router);
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
