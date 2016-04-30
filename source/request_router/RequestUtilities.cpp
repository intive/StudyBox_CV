#include "RequestUtilities.h"
#include "../json/Json.hpp"
#include "RestApiLiterals.h"

void CreateBadRequestError(Http::Response::Status& status, Json& response, const std::string& errorMessage)
{
    status = Http::Response::Status::BadRequest;
    response[Rest::Response::ERROR_DESCRIPTION] = errorMessage;
}

std::pair<std::string, int> GenericRequestErrorHandler(std::function<void(Http::Response::Status&, Json&)> targetFunction)
{
    Json response;
    Http::Response::Status status;
    try
    {
        targetFunction(status, response);
    }
    catch (const std::domain_error&) // nieprawidłowe typy
    {
        CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_JSON_TYPE);
    }
    catch (const std::out_of_range&) // odwołanie poza zasięg drzewa lub nieprawidłowa/niewspierana składnia JSON powodująca wyjście poza zasięg drzewa.
    {
        CreateBadRequestError(status, response, Rest::Response::ErrorStrings::UNKNOWN_JSON_FIELDS);
        status = Http::Response::Status::InternalServerError;
    }
    catch (const std::range_error&) // nieprawidłowa składnia
    {
        CreateBadRequestError(status, response, Rest::Response::ErrorStrings::BAD_JSON);
    }
    catch (const std::exception& e) // nierozpoznany błąd
    {
        CreateBadRequestError(status, response, std::string(Rest::Response::ErrorStrings::UNKNOWN_ELABORATE) + e.what());
        status = Http::Response::Status::InternalServerError;
    }
    catch (...) // nierozpoznany błąd (bez diagnostyki)
    {
        CreateBadRequestError(status, response, Rest::Response::ErrorStrings::UNKNOWN);
        status = Http::Response::Status::InternalServerError;
    }

    return std::make_pair(response.serialize(), static_cast<int>(status));
}
