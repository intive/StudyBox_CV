#ifndef WINPATH_H
#define WINPATH_H
#include <string>

std::string getPath(std::string& path)
{
    int charNumber = 0;
    std::string newPath = path;
    int originalSize = path.length();
    while(charNumber < originalSize)
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
    newPath = newPath + "\\source\\segmentation\\test\\";
    return newPath;
}

#endif