#include "opencv2/opencv.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>

using namespace cv;
using namespace std;

static const int MAX_SIGMA = 100;
static const int MAX_KERNEL_SIZE = 100;
static int kernelSize = 7;
static double sigma = 1.5;

void trackbar_callback_sigma(int pos, void * userData)
{
	sigma = pos / 10;
}

void trackbar_callback_kernel(int pos, void * userData)
{
	if (pos % 2 == 0)
		kernelSize = pos + 1;
	else
		kernelSize = pos;

}

int main(int argc, char **argv)
{
	cv::VideoCapture c(0);

	if (!c.isOpened()) {
		//error
	}

	cv::namedWindow("gaussian");
	cv::namedWindow("canny");
	cv::Mat s1;
	cv::Mat s2;
	cv::Mat s3;

	int t1 = 10;
	int t2 = 30;

	int s = (int)(sigma * 10);
	cv::createTrackbar("Sigma: ", "gaussian", &s, MAX_SIGMA, trackbar_callback_sigma);
	cv::createTrackbar("Kernel: ", "gaussian", &kernelSize, MAX_KERNEL_SIZE, trackbar_callback_kernel);
	cv::createTrackbar("Threshold 1: ", "canny", &t1, 100);
	cv::createTrackbar("Threshold 2: ", "canny", &t2, 100);

	while (true)
	{
		bool ok=c.read(s1);
		
		if (!ok)
		{
			//error
		}
		cvtColor(s1, s1, CV_BGR2GRAY);
		GaussianBlur(s1, s2, cv::Size(kernelSize,kernelSize), sigma, 0);
		Canny(s2, s3, t1, t2, 3);
		imshow("gaussian", s2);
		imshow("szary", s1);
		imshow("canny", s3);

		if (cv::waitKey(1) != -1)
			break;
	}
	
	waitKey();

	return 0;
}