#include "RequestRouter.h"
#include "../textanalysis/text_analysis.h"
#include "../json/Json.hpp"


#include <algorithm>
#include <sstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <fstream>

#include "RestApiLiterals.h"



	void CreateBadRequestError(Http::Response::Status& status, Json& response, const std::string& errorMessage)
	{
		status = Http::Response::Status::BadRequest;
		response[Rest::Response::STATUS] = 2;
		response[Rest::Response::ERROR_DESCRIPTION] = errorMessage;
	}


std::pair<std::string, int> TextAnalysisResponse(const std::string& body)
{
	Json response;
	Http::Response::Status status;
	try
	{
		Json request = Json::deserialize(body);

		std::string text_for_analysis = request["text_for_analysis"];


		if (text_for_analysis.empty()) // "text_for_analysis jest pusty
		{
			status = Http::Response::Status::BadRequest;
			response["status"] = 0;
			response["error_message"] = "Nie można było przeanalizować tekstu";
			std::make_pair<std::string, int>(response.serialize(), static_cast<int>(status));
		}
		else
		{
			const std::string begin = "begin";
			const std::string end = "end";
			const std::string type = "type";
			const std::string percentage_chance = "percentage_chance";
			std::vector <Markers> markers;
			markers = findQA(text_for_analysis);


			if (!markers.empty())
			{
				Json result = Json::Array();
				for (auto i = 0; i < markers.size(); i++)
				{
					Json object = {
						{ begin , markers[i].getStart() },
						{ end , markers[i].getEnd() },
						{ type , (int)markers[i].getType() },
						{ percentage_chance , markers[i].getPercentageChance() }
					};
					result.push_back(object);
				}
				
				Json final = {
					{ "result" , result }
				};
				
				std::string results = final.serialize();
				response["results"] = results;
				response["status"] = static_cast<int>(response["results"].size() > 0);
				status = Http::Response::Status::Ok;
			}
		}

	}


	catch (const std::exception& e) // nierozpoznany błąd
	{
		CreateBadRequestError(status, response, std::string("server could not handle request, reason: ") + e.what());
		status = Http::Response::Status::InternalServerError;
	}
	catch (...) // nierozpoznany błąd (bez diagnostyki)
	{
		CreateBadRequestError(status, response, "server could not handle request, error unkown");
		status = Http::Response::Status::InternalServerError;
	}

	return std::make_pair(response.serialize(), static_cast<int>(status));
}

void registerTextAnalysisResponse(Router::RequestRouter& router)
{
	router.registerEndPointService("/api/analysis", [](const std::string& body)
	{
		return TextAnalysisResponse(body);
	});
}
