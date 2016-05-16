#ifndef FLASHCARDS_ANALYSIS
#define FLASHCARDS_ANALYSIS
#include "opencv2/opencv.hpp"
#include "../json/Json.hpp"

Json::Array flashcardsToJson(const cv::Mat& img);

#endif