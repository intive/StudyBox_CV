#ifndef PATR_OCR_HPP
#define PATR_OCR_HPP

#include <string>
#include <opencv2/opencv.hpp>
#include <tesseract/baseapi.h>

#include "../json/Json.hpp"
#include "../segmentation/Rectangle.hpp"

class Ocr
{
public:
    // Ścieżka wskazująca na standardowe miejsce przechowywania danych wyuczonych
    static constexpr auto TESSDATA_PATH = ABSOLUTE_PATH "/res/tessdata";

    // Ścieżka wskazująca na standardowe miejsce przechowywania danych wyuczonych
    static constexpr auto DICT_PATH = ABSOLUTE_PATH "/res/tessdata/custom.json";

    // Domyślny zestaw języków
    static constexpr auto DEFAULT_LANGUAGE = "pol+eng";

    // Inicjializuje silnik zestawem argumentów domyślnych
    Ocr();

    // Inicjalizuje silnik przy użyciu podanych wyuczonych danych i językach
    // Zmienna datapath powinna wskazywać na KATALOG (najczęściej "tessdata") z danymi wyuczonymi (".traineddata")
    // Można wybrać kilka języków łącząc je znakiem plusa np. "pol+eng" (domyślnie),
    // kody języków można wywnioskować z nazw plików z danymi wyuczonymi (".traineddata")
    // Można opcjonalnie dodać słownik poprawiający zdefiniowane błędy korzystając z ustalonego formatu json,
    // czyli listy obiektów, gdzie nazwą jest tekst poprawny, a wartością jest lista z tekstami błędnymi
    Ocr(const std::string& datapath, const std::string& language = DEFAULT_LANGUAGE, const std::string& dictpath = "");

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

    // Funkcja usuwa szum z obrazu
    static void denoise(cv::Mat& image, const float strength = 3.0f);

    // Funkcja skaluje obraz do określonego rozmiaru
    static void resize(cv::Mat& image, const size_t size = 1944);

    // Funkcja segmentuje obraz
    // Rozmiar elementu strukturującego dla standardowej wielkości czcionki można wyznaczyć
    // na podstawie wzoru: cv::max(image.cols, image.rows) / 120
    static std::vector<Rectangle> segment(const cv::Mat& image, const int elemSize = 16);

    // Funkcja eliminuje krzywiznę tekstu
    static cv::Mat deskew(const cv::Mat& source, const Rectangle& rect);

    // Funkcja przeprowadza binaryzację obrazu
    // Argument parts określa ilość przedziałów binaryzacji
    static void binarize(cv::Mat& image, const int parts = 1);

    // Funkcja przetwarza obraz dla OCR
    static std::vector<cv::Mat> preprocess(cv::Mat& source);

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
