#ifndef _TEXT_BOUNDING_HPP
#define _TEXT_BOUNDING_HPP


#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
using namespace cv;
//Klasa zawieraj¹ca (narazie) funkcjê, która buduje obramowanie wokó³ bloków tekstu. 
class ImageProcess
{
public:
	std::vector <Rect> findRectangles(Mat *img);
};
#endif
