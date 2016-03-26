#ifndef TEXT_ANALYSIS
#define TEXT_ANALYSIS
#include<string>
#include<vector>
//Slowniczek slow kluczowych pomocnych przy szukaniu pytan
const std::vector<std::string> dictionary = {
	"Czy","czy",
	"Ile","ile",
	"Gdzie","gdzie",
	"Jak","jak",
	"Dlaczego","dlaczego",
	"Po co","po co",
	"Czemu","czemu",
	"Z kim","z kim",
	"Z czym","z czym",
	"Na czym","na czym",
	"Kiedy","kiedy",
	"Kim","kim",
	"W którym","w którym","W ktorym","w ktorym",
	"Kogo","kogo",
	"Za co","za co",
	"Co","co",
	"U kogo","u kogo",
	"Jakiego","jakiego",
	"Dla jakiego","dla jakiego",
	"Dla kogo","dla kogo",
	"Komu","komu"
};
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
	Markers(int s, int e, textType t, int percent) :start(s), end(e), type(t), percentage_chance(percent) {};
};
std::vector<Markers> findQA(const std::string& text);
#endif
