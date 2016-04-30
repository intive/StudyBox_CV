#ifndef TEXT_ANALYSIS_RESPONSE_H
#define TEXT_ANALYSIS_RESPONSE_H

#include <string>
#include <utility>

namespace Router {
    class RequestRouter;
}

//Tworzy odpowiedź na zapytanie analizy tekstu
std::pair<std::string, int> TextAnalysisResponse(const std::string& body);

//Dodaje odpowiedź na analizę tekstu do Router::RequestRouter
void registerTextAnalysisResponse(Router::RequestRouter& router);


#endif // TEXT_ANALYSIS_RESPONSE_H

