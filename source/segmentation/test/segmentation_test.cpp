#ifndef SEGMENTATION_TEST_CPP
#define SEGMENTATION_TEST_CPP
#define BOOST_TEST_DYN_LINK
#include "../Segmentation.hpp"
#include <boost/test/unit_test.hpp>
#include "../../httpserver/Predef.h"
#include <string>

#if defined(PATR_OS_WINDOWS)
#include "winPath.h"
#else
#include "linuxPath.h"
#endif


BOOST_AUTO_TEST_SUITE(SegmentationTest)

/// szara kartka, jeden napis
BOOST_AUTO_TEST_CASE(RectanglesCount1)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage1.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 1);
}

/// ciemna kartka, dwa napisy
BOOST_AUTO_TEST_CASE(RectanglesCount2Dark)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage2Dark.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

/// jasna kartka, dwa napisy
BOOST_AUTO_TEST_CASE(RectanglesCount2Light)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage2Light.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

/// kartka w kratke, jeden napis
BOOST_AUTO_TEST_CASE(RectanglesCount1Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage1Grid.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 1);
}

/// kartka w kratke, dwa napisy
BOOST_AUTO_TEST_CASE(RectanglesCount2Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage2Grid.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

/// kartka w kratke, 4 napisy
BOOST_AUTO_TEST_CASE(RectanglesCount4Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "testImage4Grid.jpeg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 4);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
