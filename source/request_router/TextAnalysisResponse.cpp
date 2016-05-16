#include "RequestRouter.h"
#include "../httpserver/ServerUtilities.h"
#include "../textanalysis/text_analysis.h"
#include "../json/Json.hpp"
#include "../text2flashcard/text2flashcard.h"

#include "RestApiLiterals.h"
#include "RequestUtilities.h"

std::pair<std::string, int> TextAnalysisResponse(const std::string& body)
{
    return GenericRequestErrorHandler(
        [&](Http::ResponseStatus& status, Json& response)
    {
        Json request = Json::deserialize(body);
        status = Http::Response::Status::Ok;
        response[Rest::Response::TEXT_ANALYSIS_RESULTS] = textToMarkersJson(request[Rest::Request::TEXT_ANALYSIS_TEXT_FOR_ANALYSIS]);
    });
}

void registerTextAnalysisResponse(Router::RequestRouter& router)
{
    router.registerEndPointService(Rest::Endpoint::TEXT_ANALYSIS_ENDPOINT, [](const std::string& body)
    {
        return TextAnalysisResponse(body);
    });
}

