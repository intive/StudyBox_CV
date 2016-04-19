#ifndef ROTATEDRECTANGLE_HPP
#define ROTATEDRECTANGLE_HPP

#include <string>

#include "opencv2/core/types.hpp"

#include "../json/Json.hpp"

class RotatedRectangle
{
public:
    RotatedRectangle() = default;
    RotatedRectangle(const Json& json);
    RotatedRectangle(const cv::Point2f& p1, const cv::Point2f& p2, const cv::Point2f& p3, const cv::Point2f& p4);
    ~RotatedRectangle() = default;

    const cv::Point2f& P1() const;
    const cv::Point2f& P2() const;
    const cv::Point2f& P3() const;
    const cv::Point2f& P4() const;

    operator Json() const;
    RotatedRectangle(const RotatedRectangle& rotatedRectangle);
    RotatedRectangle& operator = (const RotatedRectangle& rotatedRectangle);
    RotatedRectangle& operator * (const size_t& value);

private:
    cv::Point2f p1;
    cv::Point2f p2;
    cv::Point2f	p3;
    cv::Point2f p4;
	
    static constexpr const char* const p1_x_Key = "p1.x";
    static constexpr const char* const p1_y_Key = "p1.y";
    static constexpr const char* const p2_x_Key = "p2.x";
    static constexpr const char* const p2_y_Key = "p2.y";
    static constexpr const char* const p3_x_Key = "p3.x";
    static constexpr const char* const p3_y_Key = "p3.y";
    static constexpr const char* const p4_x_Key = "p4.x";
    static constexpr const char* const p4_y_Key = "p4.y";
};

#endif // ROTATEDRECTANGLE_HPP
