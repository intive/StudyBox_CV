#include "FlashcardAnalysisResponse.h"

#include "RequestRouter.h"
#include "RestApiLiterals.h"
#include "RequestUtilities.h"

#include "SegmentationResponse.h"

#include "../json/Json.hpp"

#include <opencv2/opencv.hpp>

static Json::Array ImageToFlashcardJson(const cv::Mat& image)
{
    return Json::Array();
}

std::pair<std::string, int> FlashcardAnalysisResponse(const std::string& body, cv::Mat(*ImageSource)(const std::string&))
{
    return GenericRequestErrorHandler([&](Http::ResponseStatus& status, Json& response)
    {
        try
        {
            Json request = Json::deserialize(body);

            std::string url = request[Rest::Request::URL];
            std::string action = request[Rest::Request::ACTION];

            if (action != Rest::Request::FLASHCARD_ANALYSIS_ACTION) // "action" jest niezgodne z api.
            {
                CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_ACTION);
                response[Rest::Response::STATUS] = Rest::Response::FLASHCARD_RESPONSE_STATUS_FAILURE;
            }
            else
            {
                auto image = ImageSource(url);
                response[Rest::Response::FLASHCARD_ANALYSIS_FLASHCARDS] = ImageToFlashcardJson(image);
                response[Rest::Response::STATUS] = Rest::Response::FLASHCARD_RESPONSE_STATUS_SUCCESS;
                status = Http::Response::Status::Ok;
            }

        }
        catch (const cv::Exception&) // nieprawidłowy obrazek
        {
            CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_IMAGE);
        }
    });
}

void registerFlashcardAnalysisResponse(Router::RequestRouter& router)
{
    router.registerEndPointService(Rest::Endpoint::FLASHCARD_ANALYSIS_ENDPOINT, [](const std::string& body)
    {
        return FlashcardAnalysisResponse(body, GetImageFromUrl);
    });
}
