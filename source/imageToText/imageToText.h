#ifndef IMAGETOTEXT_H
#define IMAGETOTEXT_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../ocr/Ocr.hpp"
#include "../segmentation/Segmentation.hpp"
#include <opencv2/imgproc/imgproc.hpp>

/// Czytanie tekstu z obrazu
/**
* Odczytuje tekst zamieszczony na obrazie
* param image obraz, z ktorego ma byc odczytany tekst
* return string odczytany z obrazu tekst
*/
//std::string imageToText(const cv::Mat& image);
void imageToText(const cv::Mat& image);

/// Binaryzacja obrazu 
/**
* Przygotowywanie obrazu do odczytywania z niego tekstu
* param image obraz do obrobienia
* return cv::Mat zbinaryzowany, ktory ma ulatwic odczytanie z niego tekstu
*/
cv::Mat binarizeImage(const cv::Mat& image);
#endif
