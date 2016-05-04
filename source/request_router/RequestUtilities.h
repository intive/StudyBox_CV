#ifndef REQUEST_UTILITIES_H
#define REQUEST_UTILITIES_H

#include "../httpserver/ServerUtilities.h"

#include <string>
#include <functional>

class Json;

// Ustawia status na Http::Response::Status::BadRequest i response[Rest::Response::ERROR_DESCRIPTION] na errorMessage.
void CreateBadRequestError(Http::Response::Status& status, Json& response, const std::string& errorMessage);
// Łapie wszystkie wyjątki. Dodatkowo dodaje diagnostykę dla wyjątków związanych z klasami Json, PropertyTree (zgodnie z Rest::Response::ErrorStrings) i std::exception (std::exception::what()).
std::pair<std::string, int> GenericRequestErrorHandler(std::function<void(Http::ResponseStatus&, Json&)> targetFunction);


#endif // REQUEST_UTILITIES_H
