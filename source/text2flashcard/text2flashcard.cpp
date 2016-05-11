#include "text2flashcard.h"
#include <vector>

Json::Array textToMarkersJson(const std::string& txt)
{
    
    const std::string begin = "begin";
    const std::string end = "end";
    const std::string type = "type";
    const std::string percentage_chance = "percentage_chance";

    //uzywam funkcji analizy tekstu
    std::vector<Markers> markers = findQA(txt);

    //pakuję wyniki do obiektu JSON
    Json array = Json::Array();
    for (size_t i = 0; i < markers.size(); i++)
    {
        Json object = {
            { begin , markers[i].getStart() },
            { end , markers[i].getEnd() },
            { type , (int)markers[i].getType() },
            { percentage_chance , markers[i].getPercentageChance() }
        };
        array.push_back(object);
    }

    return array;
}