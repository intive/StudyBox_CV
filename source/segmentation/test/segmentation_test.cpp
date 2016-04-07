#define BOOST_TEST_MODULE SegmentationTest
#include "../Segmentation.hpp"
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(RectanglesCount1)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage1.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 1);
}

BOOST_AUTO_TEST_CASE(RectanglesCount2Dark)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage2Dark.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

BOOST_AUTO_TEST_CASE(RectanglesCount2Light)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage2Light.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

BOOST_AUTO_TEST_CASE(RectanglesCount1Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage1Grid.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 1);
}

BOOST_AUTO_TEST_CASE(RectanglesCount2Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage1Grid.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 2);
}

BOOST_AUTO_TEST_CASE(RectanglesCount4Grid)
{
    Segmentation testSeg;
    cv::Mat originalImage;
    cv::Size morphEllipseSize = cv::Size(7, 4);
    cv::Size morphRectSize = cv::Size(5, 2);
    originalImage = cv::imread("testImage4Grid.jpeg");
    BOOST_REQUIRE(originalImage.data != NULL);
    testSeg.SetImage(originalImage);
    testSeg.ScaleImage(2);
    testSeg.SetMorphEllipseSize(morphEllipseSize);
    testSeg.SetMorphRectSize(morphRectSize);
    std::vector<RotatedRectangle> rectangles = testSeg.GetRectangles();
    BOOST_REQUIRE(rectangles.size() != 0);
    BOOST_CHECK(rectangles.size() == 4);
}