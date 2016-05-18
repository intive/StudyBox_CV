#include <iostream>
#include <iterator>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/imgproc.hpp>

#include "../segmentation/Rectangle.hpp"
#include "../request_router/SegmentationResponse.h"
#include "../ocr/Ocr.hpp"
#include "../json/Json.hpp"


namespace
{
    // Granice kolorów w Hue
    // (OpenCV koduje H na 0 - 179)
    constexpr unsigned char RedGreen = 30;
    constexpr unsigned char GreenBlue = 90;
    constexpr unsigned char BlueRed = 150;
    constexpr unsigned char SaturValueThreshold = 127;

    constexpr unsigned char PixelMaxValue = 255;

    constexpr auto QuestionJsonField = "question";
    constexpr auto AnswerJsonField = "answer";
    constexpr auto TipsJsonField = "tips";

    constexpr auto NoQuestionsError = "No question frames were detected";
    constexpr auto NoAnswersError = "No answer frames were detected";
    constexpr auto QuestionAnswerAmountMismatch = "Amount of questions does not match amount of answers";

    constexpr auto NewLine = '\n';

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
        Rectangle question;
        std::vector<Rectangle> tips;
        Rectangle answer;

        FlashcardRectangles() :
            question({}), answer({}) { }

        FlashcardRectangles(const Rectangle& q, std::vector<Rectangle> t, const Rectangle& a) :
            question(q), tips(std::move(t)), answer(a) { }
    };


    struct Flashcard
    {
        std::string question;
        std::string answer;
        std::vector<std::string> tips;

        template<typename Str, typename StrVec>
        Flashcard(Str&& q, Str&& a, StrVec&& t)
            : question(std::forward<Str>(q)), answer(std::forward<Str>(a)), tips(std::forward<StrVec>(t)) { }


        Json::Object getJson() const
        {
            Json::Object flashcard;
            flashcard[QuestionJsonField] = question;
            flashcard[AnswerJsonField] = answer;
            flashcard[TipsJsonField] = Json::Array{};
            std::for_each(begin(tips), end(tips), [&](const std::string& s) {flashcard[TipsJsonField].push_back(s); });
            return flashcard;
        }
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



    // Funkcje pomocniczne

    // Sortuje wektory prostokątów wg współrzędnej pionowej lewego górnego rogu
    void sortFlashcardsInOrder(Rects& questions, Rects& tips, Rects& answers);

    // Oblicza szerokość obramowania na podstawie binarnego obrazu zawierającego tylko ramkę/i
    int calculateFrameWidth(const cv::Mat& img);

    std::vector<cv::Mat> preprocess(const cv::Mat& source, const int cols)
    {
        const auto rects = Ocr::segment(source);
        std::vector<cv::Mat> images;
        for (const auto& rect : rects)
        {
            cv::Mat image = Ocr::deskew(source, rect);
            const int parts = 1 + 9 * image.cols / cols;
            Ocr::binarize(image, parts);
            images.push_back(image);
        }
        return images;
    }

    std::string recognize(Ocr& ocr, const cv::Mat& source, const cv::Mat& filter, const Rectangle& rectangle)
    {
        cv::Mat mask;
        mask = cv::Mat(filter, rectangle.boundingRect());
        cv::floodFill(mask, cv::Point{}, cv::Scalar(255));
        mask = 255 - mask;

        cv::Mat image;
        cv::Mat(source, rectangle.boundingRect()).copyTo(image, mask);
        cv::floodFill(image, cv::Point(), cv::mean(image, mask));

        std::string text;
        for (auto& x : preprocess(image, source.cols))
        {
            text += ocr.recognize(x);
        }

        return text;
    }

    std::vector<Flashcard> inspectImage(cv::Mat& img)
    {
        Ocr::resize(img);

        auto frames = getMatricesWithFrames(img);
        auto rectangles = detectRectangles(frames);

        Ocr ocr;

        std::vector<Flashcard> flashcards;

        for (const auto& fcr : rectangles)
        {
            auto question = recognize(ocr, img, frames.HR, fcr.question);
            auto answer = recognize(ocr, img, frames.VB, fcr.answer);

            std::vector<std::string> tips;
            for (const auto& tip : fcr.tips)
            {
                tips.emplace_back(recognize(ocr, img, frames.SG, tip));
            }

            flashcards.emplace_back( std::move(question), std::move(answer), std::move(tips) );
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
            throw std::runtime_error(NoQuestionsError);
        if (answers.size() == 0)
            throw std::runtime_error(NoAnswersError);
        if (questions.size() != answers.size())
            throw std::runtime_error(QuestionAnswerAmountMismatch);


        if (questions.size() == 1)
            return std::vector<FlashcardRectangles>{FlashcardRectangles(std::move(questions[0]), std::move(tips), std::move(answers[0]))};

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
        {
            auto r = minAreaRect(contours[idx]);
            if (r.size.width > 0 && r.size.height > 0)
                rects.emplace_back(r);
        }

        return rects;
    }





    void sortFlashcardsInOrder(Rects& questions, Rects& tips, Rects& answers)
    {
        auto sort = [](const Rectangle& r1, const Rectangle& r2) -> bool
        {
                return r1.topLeft().y < r2.topLeft().y;
        };
        
        std::sort(begin(questions), end(questions), sort);
        std::sort(begin(tips), end(tips), sort);
        std::sort(begin(answers), end(answers), sort);
    }


    std::vector<FlashcardRectangles> groupRectsIntoFlashcards(Rects& questions, Rects& tips, Rects& answers)
    {
        size_t questions_amount = questions.size();
        std::vector<FlashcardRectangles> v;

        for (size_t i = 0; i < questions_amount; i++)
        {
            FlashcardRectangles fcr;
            fcr.question = questions[i];
            fcr.answer = answers[i];


            float question_Y = questions[i].center.y;
            if (i == questions_amount - 1)
            {
                // Ostatnia fiszka, kopiujemy wszystkie podpowiedzi pod obecnym pytaniem
                std::copy_if(begin(tips), end(tips), std::back_inserter(fcr.tips), [&](const Rectangle& r) -> bool { return question_Y < r.center.y; });
            }
            else
            {
                // Wyłuskiwanie podpowiedzi leżących na osi Y między obecnie przetwarzanym pytaniem a następnym
                float next_question_Y = questions[i + 1].center.y;
                std::copy_if(begin(tips), end(tips), std::back_inserter(fcr.tips), [&](const Rectangle& r) -> bool { return question_Y < r.center.y &&  r.center.y < next_question_Y; });
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


}

Json::Array framedFlashcardsToJson(const cv::Mat& img)
{
    auto copy = img.clone();
    auto flashcards = inspectImage(copy);
    Json::Array result;
    for (const auto& f : flashcards)
        result.emplace_back(f.getJson());

    return result;
}
