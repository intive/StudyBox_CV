#include "SegmentationResponse.h"
#include "RequestRouter.h"
#include "../segmentation/Segmentation.hpp"
#include "../json/Json.hpp"
#include "../utility/DownloadFileFromHttp.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include "../utility/GetExePath.h"
#include "RestApiLiterals.h"

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

    void CreateBadRequestError(Http::Response::Status& status, Json& response, const std::string& errorMessage)
    {
        status = Http::Response::Status::BadRequest;
        response[Rest::Response::STATUS] = 2;
        response[Rest::Response::ERROR_DESCRIPTION] = errorMessage;
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
    Json response;
    Http::Response::Status status;
    try
    {
        Json request = Json::deserialize(body);

        std::string url = request[Rest::Request::URL];
        std::string action = request[Rest::Request::ACTION];

        if (action != Rest::Request::SEGMENTATION_ACTION) // "action" jest niezgodne z api.
        {
            status = Http::Response::Status::BadRequest;
            response[Rest::Response::STATUS] = 2;
            response[Rest::Response::ERROR_DESCRIPTION] = "unrecognised action for segment api";
        }
        else
        {
            auto image = GetImageByUrl(url);
            response[Rest::Response::SEGMENTATION_COORDINATES] = GetSegmentsByImage(image);
            response[Rest::Response::STATUS] = static_cast<int>(response[Rest::Response::SEGMENTATION_COORDINATES].size() > 0);
            status = Http::Response::Status::Ok;
        }

    }
    catch (const std::domain_error&) // nieprawidłowe typy
    {
        CreateBadRequestError(status, response, "request body contains invalid field types");
    }
    catch (const std::out_of_range&) // odwołanie poza zasięg drzewa lub nieprawidłowa/niewspierana składnia JSON powodująca wyjście poza zasięg drzewa.
    {
        CreateBadRequestError(status, response, "server could not handle request, possibly unsupported syntax");
        status = Http::Response::Status::InternalServerError;
    }
    catch (const std::range_error&) // nieprawidłowa składnia
    {
        CreateBadRequestError(status, response, "request body could not be read as valid json");
    }
    catch (const cv::Exception&) // nieprawidłowy obrazek
    {
        CreateBadRequestError(status, response, "invalid or unsupported image format");
    }
    catch (const std::exception& e) // nierozpoznany błąd
    {
        CreateBadRequestError(status, response, std::string("server could not handle segmentation request, reason: ") + e.what());
        status = Http::Response::Status::InternalServerError;
    }
    catch (...) // nierozpoznany błąd (bez diagnostyki)
    {
        CreateBadRequestError(status, response, "server could not handle segmentation request, error unkown");
        status = Http::Response::Status::InternalServerError;
    }

    return std::make_pair(response.serialize(), static_cast<int>(status));
}

void registerSegmentationResponse(Router::RequestRouter& router)
{
    router.registerEndPointService(Rest::Endpoint::SEGMENTATION_ENDPOINT, [](const std::string& body)
    {
        return SegmentationResponse(body, GetImageFromUrl);
    });
}
