#ifndef FLASHCARD_RESPONSE_H
#define FLASHCARD_RESPONSE_H

#include <string>
#include <utility>

namespace Router
{
    class RequestRouter;
}


// Tworzy odpowiedŸ na zapytanie zamiany obrazka / pliku tekstowego na fiszki
std::pair<std::string, int> FlashcardsResponse(const std::string& body, std::string(*textFetcher)(const std::string&));

// Dodaje obs³ugê ¿¹dania przetwarzania obrazka / pliku tekstowego na fiszki do RequestRouter
void registerFlashcardsResponse(Router::RequestRouter& router);


// Pobiera plik tekstowy z url
std::string getTextFromHttp(const std::string& url);

// Wczytuje plik tekstowy z dysku (plik w katalogu z .exe)
std::string getTextFromDisk(const std::string& filename);


#endif // FLASHCARD_RESPONSE_H