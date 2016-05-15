#include <iostream>
#include <iterator>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

#include "../segmentation/Rectangle.hpp"
#include "../request_router/SegmentationResponse.h"
#include "../ocr/Ocr.hpp"



namespace
{
    // Granice kolorów w Hue
    // (OpenCV koduje H na 0 - 179)
    constexpr unsigned char RedGreen = 30;
    constexpr unsigned char GreenBlue = 90;
    constexpr unsigned char BlueRed = 150;
    constexpr unsigned char SaturValueThreshold = 127;

    constexpr unsigned char PixelMaxValue = 255;

    const std::string language = "pol+eng";
    const std::string datapath = std::string(ABSOLUTE_PATH) + "/res/tessdata";
    const std::string dictpath = datapath + "/custom.json";

    using Rects = std::vector<Rectangle>;


    struct V3M
    {
        cv::Mat HR;
        cv::Mat SG;
        cv::Mat VB;
    };

    struct FlashcardRectangles
    {
        FlashcardRectangles() :
            Question({}), Answer({}) { }

        FlashcardRectangles(const Rectangle& Q, const std::vector<Rectangle>& T, const Rectangle& A) :
            Question(Q), Answer(A), Tips(T) { }

        Rectangle Question;
        Rectangle Answer;
        std::vector<Rectangle> Tips;
    };


    struct sort_rectangle_by_y
    {
        inline bool operator() (const Rectangle& r1, const Rectangle& r2)
        {
            return (r1.topLeft().y < r2.topLeft().y);
        }
    };
}

class Flashcard
{
    std::string question;
    std::string answer;
    std::vector<std::string> tips;
public:
    Flashcard(const std::string& q, const std::string& a, const std::vector<std::string>& t)
        :question(q)
        , answer(a)
        , tips(t)
    {};
    std::string getQuestion() const { return question; };
    std::string getAnswer() const { return answer; };
    std::vector<std::string> getTips() const { return tips; };
};


// Wejściowa funkcja dla analizy obrazka z obramowanymi fiszkami
std::vector<Flashcard> inspectImage(cv::Mat& img);


// Utworzenie 3 binarnych obrazów dla każdego zakresu kolorów z zaznaczonymi ramkami
V3M getMatricesWithFrames(const cv::Mat& img);

// Zwraca strukturę z 3 macierzami, które zawierają kolejno kanały Hue, Saturation i Value przestrzeni HSV.
V3M getHSV(const cv::Mat& img);



// Zwraca wektor struktur z Rectangle przyporządkowanymi do odpowiedniej kategorii (Pyt, Odp, Tip)
std::vector<FlashcardRectangles> detectRectangles(const V3M& frames);

// Wykorzystuje findContours z OpenCV do znalezienia współrzędnych ramek.
std::vector<Rectangle> getRectangles(const cv::Mat& img);

// Grupuje wykryte ramki w fiszki w przypadku wielu fiszek na jednym obrazku
// Funkcjonuje wg założenia, że podpowiedzi są pod pytaniem
std::vector<FlashcardRectangles> groupRectsIntoFlashcards(Rects& questions, Rects& tips, Rects& answers);

// Przetwarza otrzymane fiszki, by uzyskać prostokąty z obrotem 0 stopni.
// Dodatkowo zmniejsza obszar, tak by Rectangle nie obejmował ramki 
// (czasem OCR nie chciał współpracować jeśli tekst był w ramce)
Rectangle getRectangleWithoutFrame(const Rectangle& r, int width);



// Funkcje pomocniczne

// Sortuje wektory prostokątów wg współrzędnej pionowej lewego górnego rogu
void sortFlashcardsInOrder(Rects& questions, Rects& tips, Rects& answers);

// Oblicza szerokość obramowania na podstawie binarnego obrazu zawierającego tylko ramkę/i
int calculateFrameWidth(const cv::Mat& img);


void testFunc()
{
    try
    {
        cv::Mat img = GetImageLocal("lat_5_skewed.jpg");
        auto fsc = inspectImage(img);

        for (const auto& fc : fsc)
        {
            std::cout << "Pyt: " << fc.getQuestion() << std::endl
                << "Odp: " << fc.getAnswer() << std::endl;

            for (const auto& t : fc.getTips())
                std::cout << "Tip: " << t << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
}


std::vector<Flashcard> inspectImage(cv::Mat& img)
{
    auto frames = getMatricesWithFrames(img);
    auto rectangles = detectRectangles(frames);

    cv::Rect first_question(rectangles[0].Question.topLeft(), rectangles[0].Question.bottomRight());
    cv::Mat bin_img_of_first_question(frames.HR, first_question);
    int frame_width = calculateFrameWidth(bin_img_of_first_question);


    Ocr ocr{ datapath, language, dictpath };
    ocr.setImage(img);

    std::vector<Flashcard> flashcards;

    for (const auto& fcr : rectangles)
    {
        auto question = ocr.recognize(getRectangleWithoutFrame(fcr.Question, frame_width));
        auto answer = ocr.recognize(getRectangleWithoutFrame(fcr.Answer, frame_width));

        std::vector<std::string> tips;
        for (const auto& tip : fcr.Tips)
            tips.emplace_back(ocr.recognize(getRectangleWithoutFrame(tip, frame_width)));

        flashcards.emplace_back(Flashcard{ question, answer, tips });
    }

    return flashcards;
}




V3M getMatricesWithFrames(const cv::Mat& img)
{
    V3M hsv = getHSV(img);

    int rows = img.rows;
    int cols = img.cols;
    int elements = rows * cols;

    V3M rgb;
    rgb.HR = cv::Mat::zeros(rows, cols, CV_8UC1);
    rgb.SG = cv::Mat::zeros(rows, cols, CV_8UC1);
    rgb.VB = cv::Mat::zeros(rows, cols, CV_8UC1);

    for (int i = 0; i < elements; i++)
    {
        // Warunkiem złapania ramki są: Saturation > 0.5 i Value > 0.5
        unsigned char pixel_saturation = hsv.SG.at<unsigned char>(i);
        unsigned char pixel_value = hsv.VB.at<unsigned char>(i);

        if (pixel_saturation > SaturValueThreshold && pixel_value > SaturValueThreshold)
        {
            unsigned char pixel_hue = hsv.HR.at<unsigned char>(i);
            if (pixel_hue < RedGreen || pixel_hue > BlueRed)
            {
                // Czerwony: 0 - 30  & 150 - 179
                rgb.HR.at<unsigned char>(i) = PixelMaxValue;
            }
            else if (pixel_hue > RedGreen && pixel_hue < GreenBlue)
            {
                // Zielony: 30 - 90
                rgb.SG.at<unsigned char>(i) = PixelMaxValue;
            }
            else
            {
                // Niebieski: 90 - 150
                rgb.VB.at<unsigned char>(i) = PixelMaxValue;
            }
        }
    }

    return rgb;
}

V3M getHSV(const cv::Mat& img)
{
    cv::Mat img_hsv;
    cv::cvtColor(img, img_hsv, CV_BGR2HSV);

    std::vector<cv::Mat> channels;
    cv::split(img_hsv, channels);
    V3M hsv;
    hsv.HR = channels[0];
    hsv.SG = channels[1];
    hsv.VB = channels[2];

    return hsv;
}




std::vector<FlashcardRectangles> detectRectangles(const V3M& frames)
{
    auto questions = getRectangles(frames.HR);
    auto tips = getRectangles(frames.SG);
    auto answers = getRectangles(frames.VB);

    if (questions.size() == 0)
        throw std::runtime_error("No question frames were detected");
    if (answers.size() == 0)
        throw std::runtime_error("No answer frames were detected");
    if (questions.size() != answers.size())
        throw std::runtime_error("Amount of questions does not match amount of answers");


    if (questions.size() == 1)
        return std::vector<FlashcardRectangles>{FlashcardRectangles(questions[0], tips, answers[0])};

    sortFlashcardsInOrder(questions, tips, answers);

    return groupRectsIntoFlashcards(questions, tips, answers);
}


std::vector<Rectangle> getRectangles(const cv::Mat& img)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    auto copy = img.clone();
    findContours(copy, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

    std::vector<Rectangle> rects;

    if (contours.size() < 1) return rects;

    for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
        rects.emplace_back(Rectangle(minAreaRect(contours[idx])));

    return rects;
}


Rectangle getRectangleWithoutFrame(const Rectangle& r, int width)
{
    return Rectangle{ cv::RotatedRect(
        r.center,
        cv::Size2f((float)(r.bottomRight().x - r.topLeft().x - 3 * width), (float)(r.bottomRight().y - r.topLeft().y - 3 * width)),
        0) };
}


void sortFlashcardsInOrder(Rects& questions, Rects& tips, Rects& answers)
{
    std::sort(begin(questions), end(questions), sort_rectangle_by_y());
    std::sort(begin(tips), end(tips), sort_rectangle_by_y());
    std::sort(begin(answers), end(answers), sort_rectangle_by_y());
}


std::vector<FlashcardRectangles> groupRectsIntoFlashcards(Rects& questions, Rects& tips, Rects& answers)
{
    size_t questions_amount = questions.size();
    std::vector<FlashcardRectangles> v;

    for (size_t i = 0; i < questions_amount; i++)
    {
        FlashcardRectangles fcr;
        fcr.Question = questions[i];
        fcr.Answer = answers[i];


        float question_Y = questions[i].center.y;
        if (i == questions_amount - 1)
        {
            // Ostatnia fiszka, kopiujemy wszystkie podpowiedzi pod obecnym pytaniem
            std::copy_if(begin(tips), end(tips), std::back_inserter(fcr.Tips), [&](const Rectangle& r) -> bool { return question_Y < r.center.y; });
        }
        else
        {
            // Wyłuskiwanie podpowiedzi leżących na osi Y między obecnie przetwarzanym pytaniem a następnym
            float next_question_Y = questions[i + 1].center.y;
            std::copy_if(begin(tips), end(tips), std::back_inserter(fcr.Tips), [&](const Rectangle& r) -> bool { return question_Y < r.center.y &&  r.center.y < next_question_Y; });
        }

        v.emplace_back(fcr);
    }

    return v;
}



int calculateFrameWidth(const cv::Mat& img)
{
    int rows = img.rows;
    int mid = img.cols / 2;
    int edge_width = 0;
    bool detected_edge = false;

    for (int i = 0; i < rows; i++)
    {
        unsigned char pix_val = img.at<unsigned char>(i, mid);
        if (pix_val == 0)
        {
            if (!detected_edge)
                continue;
            else
                break;
        }
        edge_width++;
        detected_edge = true;
    }

    return edge_width;
}

