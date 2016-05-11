#ifndef TEXT2FLASHCARD
#define TEXT2FLASHCARD
#include "../json/Json.hpp"
#include "../textanalysis/text_analysis.h"

Json::Array textToMarkersJson(const std::string& txt);

Json::Array textToFlashcardJson(const std::string& txt);

#endif // TEXT2FLASHCARD