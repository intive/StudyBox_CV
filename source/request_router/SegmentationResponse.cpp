#include "SegmentationResponse.h"
#include "RequestRouter.h"
#include "../segmentation/Segmentation.hpp"
#include "../json/Json.hpp"
#include "../storage/AzureStorageManager.h"

#include <algorithm>
#include <iostream>
#include <fstream>

#include "../utility/GetExePath.h"

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
        response["status"] = 2;
        response["error_description"] = errorMessage;
    }
}

cv::Mat GetImageLocal(const std::string & path)
{
    return cv::imread(GetExePath() + path);
}

cv::Mat GetImageFromAzure(const std::string& url)
{
    AzureStorageManager manager(GetExePath() + "keys.txt");
    auto buffer = manager.downloadToBuffer(url);
    return cv::imdecode(cv::Mat(1, buffer.size(), CV_8UC1, buffer.data()), 1);
}

std::pair<std::string, int> SegmentationResponse(const std::string& body, cv::Mat (*GetImageByUrl)(const std::string&))
{
    Json response;
    Http::Response::Status status;
    try
    {
        Json request = Json::deserialize(body);

        std::string url = request["url"];
        std::string action = request["action"];

        if (action != "Segmentation") // "action" jest niezgodne z api.
        {
            status = Http::Response::Status::BadRequest;
            response["status"] = 2;
            response["error_description"] = "unrecognised action for segment api";
        }
        else
        {
            auto image = GetImageByUrl(url);
            response["coordinates"] = GetSegmentsByImage(image);
            response["status"] = static_cast<int>(response["coordinates"].size() > 0);
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
    catch (const AzureStorageError& e) // problem z chmurą azure
    {
        CreateBadRequestError(status, response, std::string("azure storage error: ") + e.what());
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
    router.registerEndPointService("/api/segment", [](const std::string& body)
    {
        return SegmentationResponse(body, GetImageFromAzure);
    });
}