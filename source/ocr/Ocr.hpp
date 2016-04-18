#ifndef PATR_OCR_HPP
#define PATR_OCR_HPP

#include <string>
#include <opencv2/opencv.hpp>

#define Rectangle IT_IS_VERY_ERRONEOUS
#include <tesseract/baseapi.h>
#undef Rectangle

#include "../json/Json.hpp"
#include "../segmentation/Rectangle.hpp"

class Ocr
{
public:
    // Inicjalizuje silnik przy użyciu podanych wyuczonych danych i językach
    // Można wybrać kilka języków łącząc je znakiem plusa np. "pol+eng" (domyślnie)
    // Można opcjonalnie dodać słownik poprawiający zdefiniowane błędy
    Ocr(const std::string& datapath, const std::string& language = "pol+eng", const std::string& dictpath = "");

    // Destruktor
    ~Ocr();

    // Konstruktor kopiujący
    Ocr(const Ocr&) = delete;

    // Operator przypisania kopiującego
    Ocr& operator=(const Ocr&) = delete;

    // Ustawia aktualnie przetwarzany obraz
    // Obraz musi być zawierać tekst w idealnie horyzontalnej linii
    // Obraz musi być odpowiednio przetworzony tj. idealny czarny tekst na białym tle
    void setImage(const cv::Mat& image);

    // Zwraca rozpoznany ciąg znaków z całego obrazu
    std::string recognize();

    // Zwraca rozpoznany ciąg znaków z całego danego obrazu
    std::string recognize(const cv::Mat& image);

    // Zwraca rozpoznany ciąg znaków z określonego obszaru obrazu
    std::string recognize(const Rectangle& rect);

    // Zwraca rozpoznany ciąg znaków z określonego obszaru danego obrazu
    std::string recognize(const cv::Mat& image, const Rectangle& rect);

protected:
    // Zwraca ciąg znaków rozpoznany przez tesseract
    std::string getText();

    // Ustawia obszar przetwarzania przez tesseract
    void setRectangle(const Rectangle& rect);

    // Poprawia błędy korzystając z zdefiniowanego słownika
    void fixErrors(std::string& text) const;

    const Json dict;
    cv::Point2i imageSize;
    tesseract::TessBaseAPI api;
};

#endif // PATR_OCR_HPP
