#include <cstdlib>
#include <fstream>

#include "Segmentation.hpp"

int main(int, char* argv[])
{
	// SETTINGS
	const std::string fileName = argv[1];
	cv::Mat orginalImage = cv::imread(fileName);
	const cv::Size morphEllipseSize = cv::Size(7, 4);
	const cv::Size morphRectSize = cv::Size(5, 2);

	Segmentation segmentation;
	segmentation.SetImage(orginalImage);
	segmentation.ScaleImage(2);
	segmentation.SetMorphEllipseSize(morphEllipseSize);
	segmentation.SetMorphRectSize(morphRectSize);

	std::vector<RotatedRectangle> rectangles = segmentation.CreateRectangles();
	for (size_t i = 0; i < rectangles.size(); i++)
	{
		int thickness = 3;
		cv::Scalar color = cv::Scalar(0, 255, 0);

		line(orginalImage, rectangles[i].P1(), rectangles[i].P2(), color, thickness);
		line(orginalImage, rectangles[i].P2(), rectangles[i].P3(), color, thickness);
		line(orginalImage, rectangles[i].P3(), rectangles[i].P4(), color, thickness);
		line(orginalImage, rectangles[i].P4(), rectangles[i].P1(), color, thickness);
	}

	cv::imwrite("outputTEST.png", orginalImage);

	// Json test
	Json j =
	{
		{ "status", 1 },
		{ "coordinates", Json::Array(rectangles.begin(), rectangles.end()) }
	};

	std::fstream file;
	file.open("outputTEST.json", std::ios::out);
	file << j;
	file.close();

	return EXIT_SUCCESS;
}
