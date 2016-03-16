#include"text_bounding.h"
vector <Rect> ImageProcess::textBounding(Mat *img)
{
	if (img->rows > 1000 || img->cols > 1000)
	{
		pyrDown(*img, *img, Size(img->cols / 2, img->rows / 2));
		pyrDown(*img, *img, Size(img->cols / 2, img->rows / 2));
	}
	vector<Rect> boundRect;
	Mat img_gray, img_sobel, img_threshold, element;

	GaussianBlur(*img, *img, Size(3, 3), 0, 0);

	cvtColor(*img, img_gray, CV_BGR2GRAY);

	Sobel(img_gray, img_sobel, CV_8U, 1, 0, 3, 1, 0, BORDER_DEFAULT);

	threshold(img_sobel, img_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

	element = getStructuringElement(MORPH_RECT, Size(50, 10));
	morphologyEx(img_threshold, img_threshold, CV_MOP_CLOSE, element);

	vector< vector< Point> > contours;
	findContours(img_threshold, contours, 0, 1);

	vector<vector<Point> > contours_poly(contours.size());
	for (int i = 0; i < contours.size(); i++)
		if (contours[i].size()>70)
		{
			approxPolyDP(Mat(contours[i]), contours_poly[i], 8, true);
			Rect appRect(boundingRect(Mat(contours_poly[i])));
			int offset = 10;
			if (appRect.x - offset < 0)
			{
				int temporaryOffset = appRect.x;
				appRect.x = 0;
				appRect.width += 2 * temporaryOffset;
			}
			else
			{
				appRect.x -= offset;
				appRect.width += 2 * offset;
			}
			if (appRect.y - offset < 0)
			{
				int temporaryOffset = appRect.y;
				appRect.y = 0;
				appRect.height += 2 * temporaryOffset;
			}
			else
			{
				appRect.y -= offset;
				appRect.height += 2 * offset;
			}
			boundRect.push_back(appRect);
		}
	return boundRect;
}