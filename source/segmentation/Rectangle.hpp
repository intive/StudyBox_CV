#ifndef RECTANGLE_HPP
#define RECTANGLE_HPP

#include <string>

#include "opencv2/core/types.hpp"

#include "../json/Json.hpp"

class Rectangle : public cv::RotatedRect
{
public:
    Rectangle(const cv::RotatedRect& rotatedRect);
    //Rectangle(const Json& json);
    ~Rectangle() = default;

    const cv::Point topLeft() const;
    const cv::Point topRight() const;
    const cv::Point bottomLeft() const;
    const cv::Point bottomRight() const;

    operator Json() const;
    Rectangle(const Rectangle& rectangle) = default;
    Rectangle& operator = (const Rectangle& rectangle) = default;
    Rectangle& operator * (const size_t& value);

private:
    static constexpr const char* const top_left_x_Key = "top_left_x";
    static constexpr const char* const top_left_y_Key = "top_left_y";
    static constexpr const char* const bottom_right_x_Key = "bottom_right_x";
    static constexpr const char* const bottom_right_y_Key = "bottom_right_y";
};

#endif // RECTANGLE_HPP
