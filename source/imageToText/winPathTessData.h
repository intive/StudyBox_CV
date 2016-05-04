#ifndef WINPATHTESSDATA_H
#define WINPATHTESSDATA_H
#include <string>

std::string getPathTessData(std::string& path)
{
    int charNumber = 0;
    std::string newPath = path;
	size_t originalSize = path.length();
    while((size_t)charNumber < originalSize)
    {
        if(path[charNumber] == '/')
        {
            newPath[charNumber] = '\\';
        }
        else
        {
            newPath[charNumber] = path[charNumber];
        }
        charNumber++;
    }
    newPath = newPath + "\\res\\tessdata";
    return newPath;
}
std::string getAbsolutePath(std::string& path)
{
    int charNumber = 0;
    std::string newPath = path;
    size_t originalSize = path.length();
    while ((size_t)charNumber < originalSize)
    {
        if (path[charNumber] == '/')
        {
            newPath[charNumber] = '\\';
        }
        else
        {
            newPath[charNumber] = path[charNumber];
        }
        charNumber++;
    }
    newPath = newPath + "\\";
    return newPath;
}

#endif