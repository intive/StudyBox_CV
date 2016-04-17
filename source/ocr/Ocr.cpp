#include "Ocr.hpp"

Ocr::Ocr(const std::string& datapath, const std::string& language, const std::string& dictpath)
    : dict(dictpath.empty() ? nullptr : Json::deserialize(dictpath))
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
    char* buffer = api.GetUTF8Text();
    std::string text(buffer);
    delete[] buffer;
    if (!dict.isNull())
        fixErrors(text);
    return text;
}

void Ocr::setRectangle(const Rectangle& rect)
{
    api.SetRectangle((int)rect.topLeft().x, (int)rect.topLeft().y, rect.size.width, rect.size.height);
}

void Ocr::fixErrors(std::string& text)
{
    const size_t end = std::string::npos;

    for (const Json& entry : dict)
    {
        const std::string& fix = entry.cbegin().key();

        for (const std::string& err : entry.cbegin().value())
        {
            for (size_t pos = text.find(err); pos != end; pos = text.find(err))
            {
                text.replace(pos, err.size(), fix);
            }
        }
    }
}
