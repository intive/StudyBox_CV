#include "text2flashcard.h"
#include <vector>

void textToJson(const std::string& txt)
{
	std::vector<Markers> markers = findQA(txt);
	Json array = Json::Array();
	for (size_t i = 0; i < markers.size(); i++)
	{
		Json object = {
			{ "begin" , markers[i].getStart() },
			{ "end" , markers[i].getEnd() },
			{ "type" , (int)markers[i].getType() },
			{ "percentage_chance" , markers[i].getPercentageChance() }
		};
		array.push_back(object);
	}
	Json final = {
		{ "result" , array }
	};
	final.serialize("fiszka.txt");
}