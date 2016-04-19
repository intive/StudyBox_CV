#ifndef SEGMENTATION_HPP
#define SEGMENTATION_HPP

#include <vector>

#include "opencv2/opencv.hpp"

#include "NonCopyable.hpp"

#include "Rectangle.hpp"
#include "RotatedRectangle.hpp"

class Segmentation : NonCopyable
{
public:
    Segmentation();
    ~Segmentation() = default;

    void SetImage(const cv::Mat& image);
    void ScaleImage(size_t scale = 1);
    void SetMorphEllipseSize(const cv::Size& mes);
    void SetMorphRectSize(const cv::Size& mrs);

    std::vector<Rectangle> CreateRectangles();
    std::vector<RotatedRectangle> CreateRotatedRectangles();

private:
    void Algorithm();

    cv::Mat grayImage;

    size_t usedScale;
    cv::Size morphEllipseSize;
    cv::Size morphRectSize;

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
};

#endif // SEGMENTATION_HPP
