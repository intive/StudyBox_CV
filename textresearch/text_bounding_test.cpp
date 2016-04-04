#include "text_bounding.h"
#define BOOST_TEST_MODULE TestWykrywania
#include <boost/test/unit_test.hpp>
using namespace cv;
//Funkcja findRectangles NIE rysuje niczego ani nie wyœwietla na ekranie. Zwraca jedynie wektor Rect'ów, w którym 
//s¹ wspó³rzêdne lewego dolnego wierzcho³ka prostok¹ta.


BOOST_AUTO_TEST_CASE(Test1)
{
	ImageProcess Bounding;
	Mat src = imread("picture1.jpg", 1);
	BOOST_REQUIRE(src.data!=NULL);
	std::vector<Rect> boxes = Bounding.findRectangles(&src);
	BOOST_CHECK(boxes.size()==4);
	
}

BOOST_AUTO_TEST_CASE(Test2)
{
	ImageProcess Bounding;
	Mat src = imread("picture2.png", 1);
	BOOST_REQUIRE(src.data != NULL);
	std::vector<Rect> boxes = Bounding.findRectangles(&src);
	BOOST_CHECK(boxes.size() == 3);
}

BOOST_AUTO_TEST_CASE(Test3)
{
	ImageProcess Bounding;
	Mat src = imread("picture3.jpg", 1);
	BOOST_REQUIRE(src.data != NULL);
	std::vector<Rect> boxes = Bounding.findRectangles(&src);
	BOOST_CHECK(boxes.size() == 4);
}