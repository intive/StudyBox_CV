#include "imageToText.h"
//#include <string>
int main(int argc, char* argv[])
{
    cv::Mat imgg = cv::imread("scn1.jpg");
    //imgg = binarizeImage(imgg);
    imageToText(imgg);
    cv::waitKey(1);
    return 0;
}