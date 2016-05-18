#ifndef PATR_FLASHCARD_ANALYSIS_RESPONSE_H
#define PATR_FLASHCARD_ANALYSIS_RESPONSE_H

#include <utility>
#include <string>

namespace Router {
    class RequestRouter;
}

namespace cv {
    class Mat;
}

std::pair<std::string, int> FlashcardAnalysisResponse(const std::string& body, cv::Mat(*ImageSource)(const std::string&));

void registerFlashcardAnalysisResponse(Router::RequestRouter& router);

#endif // PATR_FLASHCARD_ANALYSIS_RESPONSE_H
