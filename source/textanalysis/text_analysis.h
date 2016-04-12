#ifndef TEXT_ANALYSIS
#define TEXT_ANALYSIS
#include<string>
#include<vector>
//Slowniczek slow kluczowych pomocnych przy szukaniu pytan
extern const std::vector<std::string> dictionary;
/* Typ enumeracyjny uzywany przy markerach*/
enum TextType
{
	unclasified,
	question,
	answer
};
/* Klasa ta przechowuje indeks poczatku (int start), indeks konca (int end) oraz wyzej wspomniany typ enum okreslajacy czy to jest question czy answer*/
class Markers
{
	int start;
	int end;
	TextType type;
	int percentage_chance;
public:
	Markers(int s, int e, TextType t, int percent) 
	    : start(s)
	    , end(e)
	    , type(t)
	    , percentage_chance(percent) 
	{};
	int getStart() const;
	int getEnd() const;
	TextType getType() const;
	int getPercentageChance() const;
};
std::vector<Markers> findQA(const std::string& text);
#endif
