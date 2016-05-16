#ifndef PATR_FLASHCARDS_ANALYSIS_H
#define PATR_FLASHCARDS_ANALYSIS_H
#include "opencv2/opencv.hpp"
#include "../json/Json.hpp"

Json::Array framedFlashcardsToJson(const cv::Mat& img);

#endif
