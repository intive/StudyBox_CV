#include "imageToText.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../ocr/Ocr.hpp"
#include "../segmentation/Segmentation.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "../utility/DownloadFileFromHttp.h"
#include <time.h>
#include "../httpserver/Predef.h"

#if defined(PATR_OS_WINDOWS)
#include "winPathTessData.h"
#else
#include "linuxPathTessData.h"
#endif


//std::string imageToString(const cv::Mat& image)
std::string imageToText(const std::string& address)
{
    std::string tessData;
    std::string textFromPage;
    std::string tmpName = ABSOLUTE_PATH;
    tmpName = getAbsolutePath(tmpName);
    tmpName += getRandomName();
    Utility::dlFileToFile(address, tmpName); // zapis do pliku
    std::string text;
    cv::Mat textImage;
    textImage = cv::imread(tmpName);
    Segmentation seg;
    seg.SetImage(textImage);
    seg.ScaleImage(2);
    seg.SetMorphEllipseSize(cv::Size(7, 4)); // dane bêd¹ modyfikowalne
    seg.SetMorphRectSize(cv::Size(30, 15));
    std::vector<Rectangle> rectangles = seg.CreateRectangles();

    textImage = binarizeImage(textImage);

    tessData = ABSOLUTE_PATH;
    tessData = getPathTessData(tessData);
    Ocr *ocr = new Ocr(tessData);
    ocr->setImage(textImage);
    for (size_t i = 0; i < rectangles.size();i++)
    {
        text = ocr->recognize(rectangles[i]);
        textFromPage = textFromPage + text;
    }
    return textFromPage;
}

cv::Mat binarizeImage(const cv::Mat& image)
{
    cv::Mat img;
    cv::Mat img2;
    if (image.empty())
        return image;
    img = image;
    cv::cvtColor(img, img2, cv::COLOR_BGR2GRAY);
    cv::threshold(img2, img2, 150, 255, CV_THRESH_BINARY);
    //cv::adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 2);
    //cv::adaptiveThreshold(image,img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 2);
    return img2;
}


std::string getRandomName()
{
    srand(time(NULL));
    std::string name;
    for(int i=0;i<10;i++)
    {
        name = name + (char)('a'+rand() % 26);
    }
    return name;
}