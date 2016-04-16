#ifndef ROTATEDRECTANGLE_HPP
#define ROTATEDRECTANGLE_HPP

#include <string>

#include "opencv2/core/types.hpp"

#include "../json/Json.hpp"

class RotatedRectangle
{
public:
	RotatedRectangle();
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
	
	const std::string p1_x_Key;
	const std::string p1_y_Key;
	const std::string p2_x_Key;
	const std::string p2_y_Key;
	const std::string p3_x_Key;
	const std::string p3_y_Key;
	const std::string p4_x_Key;
	const std::string p4_y_Key;
};

#endif // ROTATEDRECTANGLE_HPP
