#include "imageToText.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../ocr/Ocr.hpp"
#include "../segmentation/Segmentation.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "../utility/DownloadFileFromHttp.h"


//std::string imageToString(const cv::Mat& image)
std::string imageToText(const std::string& address)
{
    std::string textFromPage;
    std::string tmpName = std::tmpnam(nullptr);
    Utility::dlFileToFile(address, tmpName); // zapis do pliku
    std::string text;
    cv::Mat textImage;
    textImage = cv::imread(tmpName);
    textImage = binarizeImage(textImage);
    Segmentation seg;
    seg.SetImage(textImage);
    seg.ScaleImage(2);
    seg.SetMorphEllipseSize(cv::Size(7, 4)); // dane bêd¹ modyfikowalne
    seg.SetMorphRectSize(cv::Size(30, 15));
    std::vector<Rectangle> rectangles = seg.CreateRectangles();
    //std::string pathToData = ABSOLUTE_PATH;
    //pathToData = pathToData + "C:\\Users\\Mateusz\\Documents\\Visual Studio 2015\\Projects\\blstream\\StudyBox_CV\\res\\tessdata";
    //cv::cvtColor(imgg, imgg, CV_BGR2GRAY);
     Ocr *ocr = new Ocr("C:\\Users\\Mateusz\\Documents\\Visual Studio 2015\\Projects\\blstream\\StudyBox_CV\\res\\tessdata");
     ocr->setImage(textImage);
     for (size_t i = 0; i < rectangles.size();i++)
     {
         text = ocr->recognize(rectangles[i]);
         textFromPage = textFromPage + text;
     }
     std::cout << text << std::endl;
    return textFromPage;
}

cv::Mat binarizeImage(const cv::Mat& image)
{
    cv::Mat img;
    cv::Mat img2;
    if (image.empty())
        return image;
    /*if (image.empty())
        cv::imwrite("output2.jpg", image);
    else if (image.channels() > 1)
        cv::cvtColor(image, img, CV_BGR2GRAY);
    else img = image;
    img2 = img.clone();*/
    img = image;
    cv::cvtColor(img, img2, cv::COLOR_BGR2GRAY);
    cv::threshold(img2, img2, 150, 255, CV_THRESH_BINARY); // proba wywolania zwraca pusty obraz
    //cv::adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 2);
    //cv::adaptiveThreshold(image,img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 2);
    return img2;
}
