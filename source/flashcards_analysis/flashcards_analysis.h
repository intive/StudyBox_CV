#ifndef FLASHCARDS_ANALYSIS
#define FLASHCARDS_ANALYSIS
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "../json/Json.hpp"

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
    const std::string& getQuestion() const;
    const std::string& getAnswer() const;
    const std::vector<std::string>& getTips() const;
};

Json::Array flashcardsToJson(const cv::Mat& img);

#endif