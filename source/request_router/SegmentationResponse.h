#ifndef PATR_SEGMENTATION_RESPONSE_H
#define PATR_SEGMENTATION_RESPONSE_H

#include <utility>
#include <string>

namespace cv {
    class Mat;
}
namespace Router {
    class RequestRouter;
}

/// Zwraca obrazek ze ścieżki względnej od położenia pliku wykonywalnego.
cv::Mat GetImageLocal(const std::string& path);
/// Zwraca obrazek pobrany z podanego url.
cv::Mat GetImageFromUrl(const std::string& path);
/// Tworzy odpowiedź na zapytanie o segmentację obrazka.
std::pair<std::string, int> SegmentationResponse(const std::string& body, cv::Mat(*GetImageaFromString)(const std::string&));
/// Dodaje odpowiedź na segmentację obrazka do Router::RequestRouter.
void registerSegmentationResponse(Router::RequestRouter& router);

#endif // PATR_SEGMENTATION_RESPONSE_H
