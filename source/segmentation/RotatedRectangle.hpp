#ifndef ROTATEDRECTANGLE_HPP
#define ROTATEDRECTANGLE_HPP

#include "opencv2/core/types.hpp"

#include "../json/Json.hpp"

class RotatedRectangle
{
public:
	RotatedRectangle();
	RotatedRectangle(const Json& json);
	RotatedRectangle(cv::Point2f p1, cv::Point2f p2, cv::Point2f p3, cv::Point2f p4);
	~RotatedRectangle() = default;

	const cv::Point2f& P1() const;
	const cv::Point2f& P2() const;
	const cv::Point2f& P3() const;
	const cv::Point2f& P4() const;

	operator Json();
	RotatedRectangle(const RotatedRectangle& rotatedRectangle);
	RotatedRectangle& operator = (const RotatedRectangle& rotatedRectangle);
	RotatedRectangle& operator * (const size_t& value);

private:
	cv::Point2f p1, p2, p3, p4;
};

#endif // ROTATEDRECTANGLE_HPP
