#include <cstdlib>
#include <fstream>

#include "Segmentation.hpp"

int test()
{
	// SETTINGS
	const std::string fileName = "input1.jpg";
	cv::Mat orginalImage = cv::imread(fileName);
	const cv::Size morphEllipseSize = cv::Size(7, 4);
	const cv::Size morphRectSize = cv::Size(5, 3);


	if (orginalImage.data == nullptr)
	{
		std::cout << "Failed to load image." << std::endl;
		return EXIT_FAILURE;
	}


	Segmentation segmentation;
	segmentation.SetImage(orginalImage);
	segmentation.ScaleImage(2);
	segmentation.SetMorphEllipseSize(morphEllipseSize);
	segmentation.SetMorphRectSize(morphRectSize);


	std::vector<Rectangle> rectangles = segmentation.CreateRectangles();
	//std::vector<RotatedRectangle> rectangles = segmentation.CreateRotatedRectangles();
	
	for (size_t i = 0; i < rectangles.size(); i++)
	{
		int thickness = 3;
		cv::Scalar color = cv::Scalar(0, 255, 0);

		line(orginalImage, rectangles[i].topLeft(), rectangles[i].topRight(), color, thickness);
		line(orginalImage, rectangles[i].topLeft(), rectangles[i].bottomLeft(), color, thickness);
		line(orginalImage, rectangles[i].topRight(), rectangles[i].bottomRight(), color, thickness);
		line(orginalImage, rectangles[i].bottomLeft(), rectangles[i].bottomRight(), color, thickness);

		/*line(orginalImage, rectangles[i].P1(), rectangles[i].P2(), color, thickness);
		line(orginalImage, rectangles[i].P2(), rectangles[i].P3(), color, thickness);
		line(orginalImage, rectangles[i].P3(), rectangles[i].P4(), color, thickness);
		line(orginalImage, rectangles[i].P4(), rectangles[i].P1(), color, thickness);*/
	}

	cv::imwrite("outputTEST.png", orginalImage);

	// JSON test
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
