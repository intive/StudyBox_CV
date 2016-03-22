#ifndef TEXT_ANALYSIS
#define TEXT_ANALYSIS
#include<string>
using namespace std;
/* Typ enumeracyjny uzywany przy markerach*/
enum textType
{
	question,
	answer
};
/* Klasa ta przechowuje indeks poczatku (int start), indeks konca (int end) oraz wyzej wspomniany typ enum okreslajacy czy to jest question czy answer*/
class Markers
{
	int start;
	int end;
	textType type;
public:
	Markers(int s, int e, textType t) :start(s), end(e), type(t) {};
};
#endif