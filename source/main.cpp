#include <iostream>

#include "httpserver/Server.h"
#include "httpserver/Socket.h"
#include "request_router/RequestRouter.h"
#include "request_router/SegmentationResponse.h"
#include "request_router/TextAnalysisResponse.h"
#include "request_router/FlashcardsResponse.h"
#include "request_router/FlashcardAnalysisResponse.h"

#include "log/Logger.h"

void registerServices(Router::RequestRouter& router)
{
    registerSegmentationResponse(router);
    registerTextAnalysisResponse(router);
    registerFlashcardsResponse(router);
    registerFlashcardAnalysisResponse(router);
}


int main()
{
    LogManager manager(std::cout, LogConfig::severity >= LogConfig::LogLevel::Trace,
        [](Attributes&& attributes, Message&& message)
    {
        return attributes.timestamp.date.toIso8601() + '|' + std::to_string(attributes.thread.number()) + '|' + attributes.logger.name() +  ": " + message.what();
    },
        []
    {
        return std::time(nullptr);
    });
    Router::RequestRouter router(manager);
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
