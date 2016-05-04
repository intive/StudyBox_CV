#ifndef LINUXPATHTESSDATA_H
#define LINUXPATHTESSDATA_H
#include <string>

std::string getPathTessData(std::string& path)
{
    std::string newPath = path + "/res/tessdata";
    return newPath;
}

std::string getAbsolutePath(std::string& path)
{
    std::string newPath = path + "/";
    return newPath;
}
#endif