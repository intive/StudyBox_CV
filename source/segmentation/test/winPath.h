#ifndef WINPATH_H
#define WINPATH_H
#include <string>

std::string getPath(std::string& path)
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
    newPath = newPath + "\\source\\segmentation\\test\\";
    return newPath;
}

#endif