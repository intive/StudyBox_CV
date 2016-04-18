#include "Segmentation.hpp"

Segmentation::Segmentation()
    : usedScale(1)
    , morphEllipseSize(1, 1)
    , morphRectSize(1, 1)
{
}

void Segmentation::SetImage(const cv::Mat& image)
{
    cv::cvtColor(image, grayImage, CV_BGR2GRAY);
}

void Segmentation::ScaleImage(size_t scale)
{
    if (scale < 1)
        return;

    usedScale = scale;
    for (size_t i = 1; i < scale; i++)
        cv::pyrDown(grayImage, grayImage);
}

void Segmentation::SetMorphEllipseSize(const cv::Size& mes)
{
    morphEllipseSize = mes;
}

void Segmentation::SetMorphRectSize(const cv::Size& mrs)
{
    morphRectSize = mrs;
}

std::vector<Rectangle> Segmentation::CreateRectangles()
{
    Algorithm();
    size_t s = usedScale > 1 ? 1 << (usedScale - 1) : 1;
    std::vector<Rectangle> rectnagles;

    for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
    {
        Rectangle rect = Rectangle(minAreaRect(contours[idx]));
        rectnagles.push_back(rect * s);
    }

    return rectnagles;
}

std::vector<RotatedRectangle> Segmentation::CreateRotatedRectangles()
{
    Algorithm();
    size_t s = usedScale > 1 ? 1 << (usedScale - 1) : 1;
    std::vector<RotatedRectangle> rotatedRectangles;

    for (int idx = 0; idx >= 0; idx = hierarchy[idx][0])
    {
        cv::RotatedRect rr = minAreaRect(contours[idx]);
        cv::Point2f p[4];
        rr.points(p);

        std::cout << rr.size << std::endl;

        rotatedRectangles.push_back(RotatedRectangle(p[0], p[1], p[2], p[3]) * s);
    }

    return rotatedRectangles;
}

void Segmentation::Algorithm()
{
    contours.clear(); hierarchy.clear();

    cv::Mat morphKernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, morphEllipseSize);

    cv::Mat morphGradient;
    morphologyEx(grayImage, morphGradient, cv::MORPH_GRADIENT, morphKernel);

    cv::Mat binarize;
    cv::threshold(morphGradient, binarize, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);

    morphKernel = cv::getStructuringElement(cv::MORPH_RECT, morphRectSize);

    cv::Mat chor;
    cv::morphologyEx(binarize, chor, cv::MORPH_CLOSE, morphKernel);

    findContours(chor, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
}

