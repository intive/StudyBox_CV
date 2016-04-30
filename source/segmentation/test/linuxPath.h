#ifndef LINUXPATH_H
#define LINUXPATH_H
#include <string>

std::string getPath(std::string& path)
{
    std::string newPath = path + "/source/segmentation/test/";
    return newPath;
}

#endif