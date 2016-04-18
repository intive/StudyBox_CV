#include "RotatedRectangle.hpp"

RotatedRectangle::RotatedRectangle(const Json& json)
    : p1(json[p1_x_Key], json[p1_y_Key])
    , p2(json[p2_x_Key], json[p2_y_Key])
    , p3(json[p3_x_Key], json[p3_y_Key])
    , p4(json[p4_x_Key], json[p4_y_Key])
{
}

RotatedRectangle::RotatedRectangle(const cv::Point2f& p1, const cv::Point2f& p2, const cv::Point2f& p3, const cv::Point2f& p4)
    : p1(p1)
    , p2(p2)
    , p3(p3)
    , p4(p4)
{
	RotatedRectangle();
}

const cv::Point2f& RotatedRectangle::P1() const
{
    return p1;
}

const cv::Point2f& RotatedRectangle::P2() const
{
    return p2;
}

const cv::Point2f& RotatedRectangle::P3() const
{
    return p3;
}

const cv::Point2f& RotatedRectangle::P4() const
{
    return p4;
}

RotatedRectangle::operator Json() const
{
    Json j;
    j[p1_x_Key] = p1.x;
    j[p1_y_Key] = p1.y;

    j[p2_x_Key] = p2.x;
    j[p2_y_Key] = p2.y;

    j[p3_x_Key] = p3.x;
    j[p3_y_Key] = p3.y;

    j[p4_x_Key] = p4.x;
    j[p4_y_Key] = p4.y;

    return j;
}

RotatedRectangle::RotatedRectangle(const RotatedRectangle& rotatedRectangle)
    : p1(rotatedRectangle.p1)
    , p2(rotatedRectangle.p2)
    , p3(rotatedRectangle.p3)
    , p4(rotatedRectangle.p4)
{
}

RotatedRectangle& RotatedRectangle::operator = (const RotatedRectangle& rotatedRectangle)
{
    p1 = rotatedRectangle.p1;
    p2 = rotatedRectangle.p2;
    p3 = rotatedRectangle.p3;
    p4 = rotatedRectangle.p4;

    return *this;
}

RotatedRectangle& RotatedRectangle::operator * (const size_t& value)
{
    p1.x *= value;
    p1.y *= value;

    p2.x *= value;
    p2.y *= value;

    p3.x *= value;
    p3.y *= value;

    p4.x *= value;
    p4.y *= value;

    return *this;
}
