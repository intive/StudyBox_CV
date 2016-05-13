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

Json::Array textToFlashcardJson(const std::string& txt)
{
    const std::string question = "question";
    const std::string answer = "answer";

    //uzywam funkcji analizy tekstu
    std::vector<Markers> markers = findQA(txt);

    Json array = Json::Array();
    
    for (size_t i = 0; i < markers.size(); ++i)
    {
        Json obj;
        //"jedź" do przodu aż znajdziesz pytanie albo koniec wektora
        if (markers[i].getType() != TextType::question)
            continue;
        std::string q = txt.substr(markers[i].getStart(), (markers[i].getEnd() - markers[i].getStart() + 1));
        if (i == markers.size() - 1 )
        {
            continue;
        }
        //jezeli następne zdanie po pytaniu to nie odpowiedz to pomin markers[i] szukaj nastepnego pytania
        if ((markers[++i]).getType() != TextType::answer)
        {
            continue;
        }
        
        std::string a = txt.substr(markers[i].getStart(), (markers[i].getEnd() - markers[i].getStart() + 1));

        obj = {
            { question, q },
            { answer, a }
        };
        
        
        array.push_back(obj);
    }

    return array;
}