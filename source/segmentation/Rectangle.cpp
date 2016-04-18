#include "Rectangle.hpp"

Rectangle::Rectangle(const cv::RotatedRect& rotatedRect)
	: RotatedRect(rotatedRect)
{
}

//Rectangle::Rectangle(const Json& json)
//{
//}

const cv::Point Rectangle::topLeft() const
{
	cv::Rect rect = boundingRect();
	return cv::Point(rect.x, rect.y);
}

const cv::Point Rectangle::topRight() const
{
	cv::Rect rect = boundingRect();
	return cv::Point(rect.x + rect.width, rect.y);
}

const cv::Point Rectangle::bottomLeft() const
{
	cv::Rect rect = boundingRect();
	return cv::Point(rect.x, rect.y + rect.height);
}

const cv::Point Rectangle::bottomRight() const
{
	cv::Rect rect = boundingRect();
	return cv::Point(rect.x + rect.width, rect.y + rect.height);
}

Rectangle::operator Json() const
{
	Json j;
	cv::Point2f topleft = topLeft();
	j[top_left_x_Key] = topleft.x;
	j[top_left_y_Key] = topleft.y;

	cv::Point2f bottomRight = topRight();
	j[bottom_right_x_Key] = bottomRight.x;
	j[bottom_right_y_Key] = bottomRight.y;

	return j;
}

Rectangle& Rectangle::operator * (const size_t& value)
{
	center.x *= value;
	center.y *= value;

	size.width *= value;
	size.height *= value;

	return *this;
}
