#ifndef IMAGETOTEXT_H
#define IMAGETOTEXT_H

#include <opencv2/core/core.hpp>

/// Czytanie tekstu z obrazu
/**
* Odczytuje tekst zamieszczony na obrazie
* param image obraz, z ktorego ma byc odczytany tekst
* return string odczytany z obrazu tekst
*/
//std::string imageToText(const cv::Mat& image);
std::string imageToText(const std::string& address);

/// Binaryzacja obrazu 
/**
* Przygotowywanie obrazu do odczytywania z niego tekstu
* param image obraz do obrobienia
* return cv::Mat zbinaryzowany, ktory ma ulatwic odczytanie z niego tekstu
*/
cv::Mat binarizeImage(const cv::Mat& image);
/// Generowanie losowej nazwy pliku
/**
* return std::string losowa nazwa pliku
*/
std::string getRandomName();
#endif
