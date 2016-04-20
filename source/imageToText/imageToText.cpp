#include "imageToText.h"


//std::string imageToString(const cv::Mat& image)
void imageToText(const cv::Mat& image)
{
    std::string text;
    cv::Mat imgg;
    imgg = binarizeImage(image);
    Segmentation seg;
    seg.SetImage(imgg);
    seg.ScaleImage(2);
    seg.SetMorphEllipseSize(cv::Size(7, 4));
    seg.SetMorphRectSize(cv::Size(30, 15));
    std::vector<Rectangle> rectangles = seg.CreateRectangles();
        //std::string pathToData = ABSOLUTE_PATH;
    //pathToData = pathToData + "C:\\Users\\Mateusz\\Documents\\Visual Studio 2015\\Projects\\blstream\\StudyBox_CV\\res\\tessdata";
    cv::cvtColor(imgg, imgg, CV_BGR2GRAY);
     Ocr *ocr = new Ocr("C:\\Users\\Mateusz\\Documents\\Visual Studio 2015\\Projects\\blstream\\StudyBox_CV\\res\\tessdata");
     ocr->setImage(image);
     for (size_t i = 0; i < rectangles.size();i++)
     {
         text = ocr->recognize(rectangles[i]);
         std::cout << text << std::endl;
     }
     std::cout << text << std::endl;
    cv::imwrite("kkk.jpg",imgg);
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
    cv::threshold(img, img2, 150, 255, CV_THRESH_BINARY); // proba wywolania zwraca pusty obraz
    //cv::adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 2);
    //cv::adaptiveThreshold(image,img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 2);
    return img2;
}
