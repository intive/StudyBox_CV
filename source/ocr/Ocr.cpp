#include "Ocr.hpp"

#include <memory>
#include <algorithm>

#include "../segmentation/Segmentation.hpp"

Ocr::Ocr(const std::string& datapath, const std::string& language, const std::string& dictpath)
    : dict(dictpath.empty() ? Json(nullptr) : Json::deserialize(dictpath))
{
    if (api.Init(datapath.c_str(), language.c_str()))
    {
        throw std::runtime_error("Could not init tesseract");
    }

    api.SetPageSegMode(tesseract::PageSegMode::PSM_SINGLE_BLOCK);
}

Ocr::~Ocr()
{
    api.End();
}

void Ocr::setImage(const cv::Mat& image)
{
    imageSize = cv::Point2i(image.cols, image.rows);
    api.SetImage(image.data, image.cols, image.rows, (int)image.elemSize(), (int)image.step);
}

std::string Ocr::recognize()
{
    api.SetRectangle(0, 0, imageSize.x, imageSize.y);
    return getText();
}

std::string Ocr::recognize(const cv::Mat& image)
{
    setImage(image);
    return getText();
}

std::string Ocr::recognize(const Rectangle& rect)
{
    setRectangle(rect);
    return getText();
}

std::string Ocr::recognize(const cv::Mat& image, const Rectangle& rect)
{
    setImage(image);
    setRectangle(rect);
    return getText();
}

std::string Ocr::getText()
{
    std::unique_ptr<char[]> buffer(api.GetUTF8Text());
    std::string text(buffer.get());
    if (!dict.isNull())
        fixErrors(text);
    return text;
}

void Ocr::setRectangle(const Rectangle& rect)
{
    api.SetRectangle((int)rect.topLeft().x, (int)rect.topLeft().y, (int)rect.size.width, (int)rect.size.height);
}

void Ocr::fixErrors(std::string& text) const
{
    const size_t end = std::string::npos;

    for (const Json& entry : dict)
    {
        const std::string& fix = entry.getKey();

        for (const std::string& err : entry.getValue())
        {
            for (size_t pos = text.find(err); pos != end; pos = text.find(err))
            {
                text.replace(pos, err.size(), fix);
            }
        }
    }
}

void Ocr::denoise(cv::Mat& image, const float strength)
{
    cv::fastNlMeansDenoisingColored(image, image, strength, strength, 7, 21);
}

void Ocr::resize(cv::Mat& image, const size_t size)
{
    const double scale = 1.0 / cv::max((float)image.cols / size, (float)image.rows / size);
    cv::resize(image, image, cv::Size(), scale, scale, CV_INTER_CUBIC);
}

std::vector<Rectangle> Ocr::segment(const cv::Mat& image, const int elemSize)
{
    Segmentation segmentator;
    segmentator.SetImage(image);
    segmentator.SetMorphEllipseSize(cv::Size(elemSize, elemSize));
    segmentator.SetMorphRectSize(cv::Size(2 * elemSize + 1, elemSize - 1));
    std::vector<Rectangle> rects = segmentator.CreateRectangles();

    rects.erase(std::remove_if(rects.begin(), rects.end(), [=](const Rectangle& rect)
    {
        int threshold = 2 * elemSize;
        if (rect.size.width < threshold || rect.size.height < threshold)
            return true;
        else
            return false;
    }), rects.end());

    for (auto& rect : rects)
    {
        if (rect.angle < -45.0f)
        {
            rect.angle = 90.0f + rect.angle;
            rect.size = cv::Size2f(rect.size.height, rect.size.width);
        }
    }

    std::reverse(rects.begin(), rects.end());

    return rects;
}

cv::Mat Ocr::deskew(const cv::Mat& source, const Rectangle& rect)
{
    cv::Mat image;
    const cv::Mat tm = cv::getRotationMatrix2D(rect.center, rect.angle, 1.0);
    cv::warpAffine(source, image, tm, source.size(), CV_INTER_CUBIC);
    cv::getRectSubPix(image, rect.size, rect.center, image);
    return image;
}

void Ocr::binarize(cv::Mat& image, const int parts)
{
    if (image.type() != CV_8UC1)
        cv::cvtColor(image, image, CV_BGR2GRAY);

    for (int i = 0; i < parts; i++)
    {
        int step = image.cols / parts + image.cols % parts;
        cv::Rect rect(i * step, 0, step, image.rows);

        int off = rect.x + rect.width - image.cols;
        if (off > 0)
        {
            rect.width -= off;
        }

        const cv::Mat roi(image, rect);
        cv::threshold(roi, roi, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
    }

    const cv::Mat kernel = cv::getStructuringElement(CV_SHAPE_RECT, cv::Size(2, 2));
    cv::morphologyEx(image, image, cv::MorphTypes::MORPH_ERODE, kernel, cv::Point(-1, -1), 1);
}
