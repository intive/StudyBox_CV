#include"text_analysis.h"
#include<vector>
vector<Markers> findQA(string text)
{
	vector<Markers> markersVector;
	if (text.empty())
		return markersVector;
	for (int i = 0; i<text.size(); ++i)
	{
		if (text[i] == '?')
		{
			if (i == 0)
			{
				continue;
			}
			int startingMarker = i - 1;
			while (startingMarker != 0)
			{
				if (text[startingMarker] == '.' || text[startingMarker] == '!' || text[startingMarker] == '?')
					break;
				startingMarker--;
			}
			if (startingMarker + 1 == i)
			{
				continue;
			}
			Markers question(startingMarker == 0 ? 0 : startingMarker + 1, i, question);
			markersVector.push_back(question);
		}
		else if (text[i] == '!' || text[i] == '.')
		{
			if (i == 0)
			{
				continue;
			}
			int startingMarker = i - 1;
			while (startingMarker != 0)
			{
				if (text[startingMarker] == '.' || text[startingMarker] == '!' || text[startingMarker] == '?')
					break;
				startingMarker--;
			}
			if (startingMarker + 1 == i)
			{
				continue;
			}
			Markers answer(startingMarker == 0 ? 0 : startingMarker + 1, i, answer);
			markersVector.push_back(answer);
		}
	}
	return markersVector;
}