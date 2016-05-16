#include "SegmentationResponse.h"
#include "RequestRouter.h"
#include "../segmentation/Segmentation.hpp"
#include "../json/Json.hpp"
#include "../utility/DownloadFileFromHttp.h"

#include <algorithm>
#include <fstream>

#include "../utility/GetExePath.h"
#include "RestApiLiterals.h"
#include "RequestUtilities.h"

namespace {

    void SetSegmentationParameters(Segmentation& segmentation)
    {
        const cv::Size morphEllipseSize = cv::Size(7, 4);
        const cv::Size morphRectSize = cv::Size(5, 2);

        segmentation.ScaleImage(2);
        segmentation.SetMorphEllipseSize(morphEllipseSize);
        segmentation.SetMorphRectSize(morphRectSize);
    }

    Json GetSegmentsByImage(const cv::Mat& image)
    {
        Segmentation segmentation;
        segmentation.SetImage(image);
        SetSegmentationParameters(segmentation);
        
        auto rectangles = segmentation.CreateRectangles();
        
        std::vector<Json> rectangleJson(rectangles.size());
        std::transform(rectangles.begin(), rectangles.end(), rectangleJson.begin(), [](const decltype(rectangles)::value_type& r) { return static_cast<Json>(r); });
        auto result = Json(rectangleJson);

        return result;
    }

}

cv::Mat GetImageLocal(const std::string& path)
{
    return cv::imread(GetExePath() + path);
}

cv::Mat GetImageFromUrl(const std::string& url)
{
    std::vector<unsigned char> buffer;
    Utility::dlFileToBuffer(url, buffer);
    return imdecode(cv::Mat(buffer), 1);
}

std::pair<std::string, int> SegmentationResponse(const std::string& body, cv::Mat (*GetImageByUrl)(const std::string&))
{
    return GenericRequestErrorHandler(
        [&](Http::ResponseStatus& status, Json& response)
    {
        try
        {
            Json request = Json::deserialize(body);

            std::string url = request[Rest::Request::URL];
            std::string action = request[Rest::Request::ACTION];

            if (action != Rest::Request::SEGMENTATION_ACTION) // "action" jest niezgodne z api.
            {
                CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_ACTION);
                response[Rest::Response::STATUS] = Rest::Response::SEGMENTATION_STATUS_FAILURE;
            }
            else
            {
                auto image = GetImageByUrl(url);
                response[Rest::Response::SEGMENTATION_COORDINATES] = GetSegmentsByImage(image);
                response[Rest::Response::STATUS] = static_cast<int>(response[Rest::Response::SEGMENTATION_COORDINATES].size() > 0);
                status = Http::Response::Status::Ok;
            }

        }
        catch (const cv::Exception&) // nieprawidłowy obrazek
        {
            CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_IMAGE);
        }
    });
}

void registerSegmentationResponse(Router::RequestRouter& router)
{
    router.registerEndPointService(Rest::Endpoint::SEGMENTATION_ENDPOINT, [](const std::string& body)
    {
        return SegmentationResponse(body, GetImageFromUrl);
    });
}
