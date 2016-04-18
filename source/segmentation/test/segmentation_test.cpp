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
BOOST_AUTO_TEST_CASE(Scan1)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "Scan1.jpg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 3);
}

/// ciemna kartka, dwa napisy
BOOST_AUTO_TEST_CASE(Scan2)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "Scan2.jpg";
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

/// jasna kartka, dwa napisy
BOOST_AUTO_TEST_CASE(Scan3)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "Scan3.jpg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 7);
}

/// kartka w kratke, jeden napis
BOOST_AUTO_TEST_CASE(Scan4)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(30, 10);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "Scan4.jpg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 3);
}

/// kartka w kratke, dwa napisy
BOOST_AUTO_TEST_CASE(Scan5)
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
    BOOST_CHECK(rectangles.size() == 7);
}

/// kartka w kratke, 4 napisy
BOOST_AUTO_TEST_CASE(Scan6)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    std::string path = ABSOLUTE_PATH;
    path = getPath(path);
    path = path + "Scan6.jpg";
    originalImage = cv::imread(path);
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.CreateRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 9);
}

BOOST_AUTO_TEST_SUITE_END()

#endif
