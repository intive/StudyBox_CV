#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../Ocr.hpp"

const std::string language = "pol+eng";
const std::string datapath = std::string(ABSOLUTE_PATH) + "/res/tessdata";
const std::string dictpath = datapath + "/custom.json";

const std::string imagepath = std::string(ABSOLUTE_PATH) + "/res/test/ocr_test.png";
const std::string imagetext = "This sentence is quite long\nand moderately complex.\n";
const std::string regiontext = imagetext.substr(0, imagetext.find_first_of("\n") + 1);

/// Testy sprawdzające poprawność OCR
BOOST_AUTO_TEST_SUITE(OcrTest)

/// Sprawdza poprawność inicjalizacji OCR dla niepoprawnej ścieżki z danymi wyuczonymi
BOOST_AUTO_TEST_CASE(SetupFailureData)
{
    BOOST_CHECK_THROW(Ocr("invalid_datapath"), std::runtime_error);
}

/// Sprawdza poprawność inicjalizacji OCR dla niepoprawnego języka
BOOST_AUTO_TEST_CASE(SetupFailureLanguage)
{
    BOOST_CHECK_THROW(Ocr(datapath, "invalid_language"), std::runtime_error);
}

/// Sprawdza poprawność inicjalizacji OCR dla niepoprawnej ścieżki z słownikiem
BOOST_AUTO_TEST_CASE(SetupFailureDict)
{
    BOOST_CHECK_THROW(Ocr(datapath, language, "invalid_dictpath"), std::runtime_error);
}

/// Sprawdza poprawność inicjalizacji OCR
BOOST_AUTO_TEST_CASE(SetupSuccess)
{
    BOOST_CHECK_NO_THROW(Ocr(datapath, language, dictpath));
}

/// Sprawdza czy działa rozpoznawanie tekstu na całym obrazie
BOOST_AUTO_TEST_CASE(RecognizeImage)
{
    const cv::Mat image = cv::imread(imagepath);
    Ocr ocr(datapath, language, dictpath);
    BOOST_CHECK_EQUAL(ocr.recognize(image), imagetext);
}

/// Sprawdza czy działa rozpoznawanie tekstu na całym ustawionym obrazie
BOOST_AUTO_TEST_CASE(RecognizeSetImage)
{
    const cv::Mat image = cv::imread(imagepath);
    Ocr ocr(datapath, language, dictpath);
    ocr.setImage(image);
    BOOST_CHECK_EQUAL(ocr.recognize(), imagetext);
}

/// Sprawdza czy działa rozpoznawanie tekstu w wyznaczonym regionie obrazu
BOOST_AUTO_TEST_CASE(RecognizeImageRegion)
{
    const cv::Mat image = cv::imread(imagepath);
    Ocr ocr(datapath, language, dictpath);
    const Rectangle rect = cv::RotatedRect(cv::Point2f(0.f, 0.f),
        cv::Point2f((float)image.cols, 0.f),
        cv::Point2f((float)image.cols, image.rows / 2.f));
    BOOST_CHECK_EQUAL(ocr.recognize(image, rect), regiontext);
}

/// Sprawdza czy działa rozpoznawanie tekstu w wyznaczonym regionie ustawionego obrazu
BOOST_AUTO_TEST_CASE(RecognizeSetImageRegion)
{
    const cv::Mat image = cv::imread(imagepath);
    Ocr ocr(datapath, language, dictpath);
    ocr.setImage(image);
    const Rectangle rect = cv::RotatedRect(cv::Point2f(0.f, 0.f),
        cv::Point2f((float)image.cols, 0.f),
        cv::Point2f((float)image.cols, image.rows / 2.f));
    BOOST_CHECK_EQUAL(ocr.recognize(rect), regiontext);
}

/// Sprawdza czy działa rozpoznawanie tekstu w wyznaczonych regionach ustawionego obrazu
BOOST_AUTO_TEST_CASE(RecognizeSetImageRegions)
{
    const cv::Mat image = cv::imread(imagepath);
    Ocr ocr(datapath, language, dictpath);
    ocr.setImage(image);
    const Rectangle rect1 = cv::RotatedRect(cv::Point2f(0.f, 0.f),
        cv::Point2f((float)image.cols, 0.f),
        cv::Point2f((float)image.cols, image.rows / 2.f));
    const Rectangle rect2 = cv::RotatedRect(cv::Point2f(0.f, image.rows / 2.f),
        cv::Point2f((float)image.cols, image.rows / 2.f),
        cv::Point2f((float)image.cols, (float)image.rows));
    BOOST_CHECK_EQUAL(ocr.recognize(rect1) + ocr.recognize(rect2), imagetext);
}

BOOST_AUTO_TEST_SUITE_END()
