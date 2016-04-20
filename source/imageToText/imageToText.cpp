#include "imageToText.h"


//std::string imageToString(const cv::Mat& image)
void imageToText(const cv::Mat& image)
{
    cv::Mat imgg;
    imgg = binarizeImage(image);
    Segmentation seg;
    seg.SetImage(imgg);
    seg.ScaleImage(2);
    seg.SetMorphEllipseSize(cv::Size(7, 4));
    seg.SetMorphRectSize(cv::Size(30, 15));
    std::vector<RotatedRectangle> rectangles = seg.CreateRotatedRectangles();
        //std::string pathToData = ABSOLUTE_PATH;
    //pathToData = pathToData + "C:\\Users\\Mateusz\\Documents\\Visual Studio 2015\\Projects\\blstream\\StudyBox_CV\\res\\tessdata";
   // Ocr *ocr = new Ocr(pathToData);
   // ocr->setImage(image);

    //return "nope";
}

cv::Mat binarizeImage(const cv::Mat& image)
{
    cv::Mat img;
    cv::Mat img2;
    //cv::imshow("nie", *img);
    if (image.empty())
        cv::imwrite("output2.jpg", image);
    else if (image.channels() > 1)
        cv::cvtColor(image, img, CV_BGR2GRAY);
    else img = image;
    img2 = img.clone();
    cv::imwrite("ooo.jpg",img);
    cv::cvtColor(img, img2, cv::COLOR_BGR2GRAY);
    cv::threshold(img, img2, 150, 255, CV_THRESH_BINARY); // proba wywolania zwraca pusty obraz
    cv::cvtColor(img2, img2, CV_BGR2GRAY);
    //cv::adaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 2);
    //cv::adaptiveThreshold(image,img, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 11, 2);
    return img2;
}
