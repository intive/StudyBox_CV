﻿#include <fstream>
#include <iterator>
#include <opencv2/opencv.hpp>

#include "FlashcardsResponse.h"
#include "RestApiLiterals.h"
#include "RequestUtilities.h"
#include "RequestRouter.h"
#include "../json/Json.hpp"
#include "../text2flashcard/text2flashcard.h"
#include "../utility/DownloadFileFromHttp.h"
#include "../utility/GetExePath.h"
#include "SegmentationResponse.h"
#include "../ocr/Ocr.hpp"


std::string getTextFromDisk(const std::string& filename)
{
    std::ifstream file(GetExePath() + filename, std::ios::binary);
    if (!file.is_open())
    {
        return "";
    }

    file >> std::noskipws;
    std::string text;
    std::copy(std::istream_iterator<char> {file}, std::istream_iterator<char>{}, std::back_inserter(text));

    return text;
}

std::string getTextFromHttp(const std::string& url)
{
    std::vector<unsigned char> buffer;
    Utility::dlFileToBuffer(url, buffer);
    return std::string(buffer.begin(), buffer.end());
}


std::pair<std::string, int> FlashcardsResponse(const std::string& body, std::string(*textFetcher)(const std::string&))
{
    return GenericRequestErrorHandler([&](Http::ResponseStatus& status, Json& response)
    {
        response[Rest::Response::STATUS] = Rest::Response::FLASHCARDS_STATUS_FAILURE;

        Json request = Json::deserialize(body);

        std::string url = request[Rest::Request::URL];
        std::string action = request[Rest::Request::ACTION];

        if (action != Rest::Request::IMG_TO_FLASHCARD && action != Rest::Request::TEXT_TO_FLASHCARD)
        {
            CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_ACTION);
            response[Rest::Response::STATUS] = Rest::Response::FLASHCARDS_STATUS_FAILURE;
            return;
        }

        std::string text;
        if (action == Rest::Request::IMG_TO_FLASHCARD)
        {
            cv::Mat source = GetImageFromUrl(url);
            const std::vector<cv::Mat> images = Ocr::preprocess(source);

            Ocr ocr;
            for (auto& image : images)
            {
                text += ocr.recognize(image);
            }
        }
        else
            text = textFetcher(url);

        status = Http::Response::Status::Ok;
        response[Rest::Response::FLASHCARDS] = textToFlashcardJson(text);
        response[Rest::Response::STATUS] = response[Rest::Response::FLASHCARDS].size() > 0 ? 1 : 2;
    });
}


void registerFlashcardsResponse(Router::RequestRouter& router)
{
    router.registerEndPointService(Rest::Endpoint::FLASHCARDS_ENDPOINT, [](const std::string& body)
    {
        return FlashcardsResponse(body, getTextFromHttp);
    });
}
