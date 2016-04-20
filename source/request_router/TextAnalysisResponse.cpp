#include "RequestRouter.h"
#include "../textanalysis/text_analysis.h"
#include "../json/Json.hpp"

#include <algorithm>
#include <sstream>
#include <iterator>
#include <vector>
#include <iostream>
#include <fstream>





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
			response["error_message"] = "Nie mo¿na by³o przeanalizowaæ tekstu";
			std::make_pair<std::string, int>(response.serialize(), static_cast<int>(status));
		}
		else
		{
			std::vector <Markers> v_text;
			v_text = findQA(text);
			
			
			if (!v_text.empty())
			{
				std::vector <int> result;
				for (int i = 0; i < v_text.size(); i++)
				{
					result.push_back(v_text[i].getStart());
					result.push_back(v_text[i].getEnd());
					result.push_back(v_text[i].getType());
					result.push_back(v_text[i].getEnd());
					result.push_back(v_text[i].getPercentageChance());
				}

				std::ostringstream convert_text;
				copy(result.begin(), result.end() - 1,std::ostream_iterator<int>(convert_text, " "));
				convert_text << result.back();
				std::string results = convert_text.str();
				response["results"] = results;
				response["status"] = static_cast<int>(response["results"].size() > 0);
				status = Http::Response::Status::Ok;
			}
		}

	}


	catch (const std::exception& e) // nierozpoznany b³¹d
	{
		CreateBadRequestError(status, response, std::string("server could not handle request, reason: ") + e.what());
		status = Http::Response::Status::InternalServerError;
	}
	catch (...) // nierozpoznany b³¹d (bez diagnostyki)
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
