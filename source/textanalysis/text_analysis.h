#ifndef TEXT_ANALYSIS
#define TEXT_ANALYSIS
#include<string>
#include<vector>

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
    size_t start;
    size_t end;
    TextType type;
    int percentage_chance;
public:
    Markers(size_t s, size_t e, TextType t, int percent) 
        :start(s)
        , end(e)
        , type(t)
        , percentage_chance(percent) 
    {};
    size_t getStart() const;
    size_t getEnd() const;
    TextType getType() const;
    int getPercentageChance() const;
};
std::vector<Markers> findQA(std::string text);
#endif