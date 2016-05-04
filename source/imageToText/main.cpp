#include "imageToText.h"
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

//#include <string>
int main(int argc, char* argv[])
{
    std::string str = imageToText("https://i.imgur.com/0VwUJ7q.jpg");
    std::cout << str << std::endl;
    getchar();
    return 0;
}