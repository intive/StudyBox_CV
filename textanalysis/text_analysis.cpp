#include"text_analysis.h"
#include<vector>

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
std::vector<Markers> findQA(const std::string& text)
{
	std::vector<Markers> markersVector;
	if (text.empty())
		return markersVector;
	/*Analiza opiera sie na interpunkcji w zdaniu*/
	for (int i = 0; i<text.size(); ++i)
	{
		//Jesli wykryto znak zapytania
		if (text[i] == '?') 
		{
			//Wykrywanie przypadku gdy zdanie z niewiadomych przyczyn zaczyna sie znakiem zapytania
			if (i == 0)
			{
				continue;
			}
			int startingMarker = i - 1;

			//"Cofanie siê" dopoki nie znajdzie sie znaku interpunkcyjnego lub poczatku
			while (startingMarker != 0)
			{
				if (text[startingMarker] == '.' || text[startingMarker] == '!' || text[startingMarker] == '?')
					break;
				startingMarker--;
			}

			//Sprawdzanie sytuacji typu ". ? " badz z jakims jednym znakiem pomiedzy
			if (i - startingMarker + 1 < 2)
			{
				Markers unclasified(startingMarker, i, unclasified, 80);
				markersVector.push_back(unclasified);
				continue;
			}
			else
			{
				int found=std::string::npos;
				int k = 0;

				//Wyszukiwanie w zdaniu jednego ze slow kluczowych zdefiniowanych w dictionary, charakteryzujacych pytanie
				while (k < dictionary.size())
				{
					int tmp= text.find(dictionary[k], startingMarker);
					if (found != std::string::npos && tmp < found && tmp >= startingMarker)
					{
						found = tmp;
					}
					else if (found == std::string::npos)
					{
						found = text.find(dictionary[k], startingMarker);
					}
					k++;
				}
				//Jezeli znaleziono takie slowo kluczowe na poczatku zdania to na 100% jest to pytanie
				if (found - startingMarker < 4 && found!=std::string::npos && found < i)
				{
					Markers question(startingMarker == 0 ? 0 : startingMarker + 1, i, question, 100);
					markersVector.push_back(question);
				}
				else if (found - startingMarker > 4 && found < i)  //w przeciwnym wypadku jesli znaleziono gdzies bardziej w srodku to tylko na 99%
				{
					Markers question(startingMarker == 0 ? 0 : startingMarker + 1, i, question, 99);
					markersVector.push_back(question);
				}
				else               //jesli nie znaleziono 
				{

					//Sprawdzam czy zdanie ma jakakolwiek samogloske. Jesli jej nie ma to prawie na pewno jest to blad
					int found2 = text.find_first_of("eyuioa", startingMarker);
					if (found2 == std::string::npos || found2 > i)
					{
						Markers unclasified(startingMarker == 0 ? 0 : startingMarker + 1, i, unclasified, 95);
						markersVector.push_back(unclasified);
					}
					else
					{
						//Sprawdzam czy zdanie ma jakas spacje. Jesli nie ma, a jest bardzo dlugie to prawdopodobnie jest to zlepek roznych liter
						int found3 = text.find_first_of(" ", startingMarker+2);
						if (i - startingMarker > 15 && (found3 > i || found3 == std::string::npos))
						{
							Markers unclasified(startingMarker == 0 ? 0 : startingMarker + 1, i, unclasified, 70);
							markersVector.push_back(unclasified);
						}
						else
						{
							Markers question(startingMarker == 0 ? 0 : startingMarker + 1, i, question, 95);
							markersVector.push_back(question);
						}
					}
				}
			}
		}
		else if (text[i] == '!' || text[i] == '.' || ((i==text.size()-1) && (!markersVector.empty())))
		{
			//Wykrywanie przypadku gdy zdanie z niewiadomych przyczyn zaczyna sie znakiem interpunkcyjnym
			if (i == 0)
			{
				continue;
			}

			//"Cofanie siê" dopoki nie znajdzie sie znaku interpunkcyjnego lub poczatku
			int startingMarker = i - 1;
			while (startingMarker != 0)
			{
				if (text[startingMarker] == '.' || text[startingMarker] == '!' || text[startingMarker] == '?')
					break;
				startingMarker--;
			}
			//Wykrywanie sytuacji " ? . " i podobnych
			if (i - startingMarker + 1 < 2)
			{
				Markers unclasified(startingMarker, i, unclasified, 60);
				markersVector.push_back(unclasified);
				continue;
			}
			else
			{

				//Sprawdzam czy zdanie ma jakakolwiek samogloske. Jesli jej nie ma to prawie na pewno jest to blad
				int found2 = text.find_first_of("eyuioa", startingMarker);
				if (found2 == std::string::npos || found2 > i)
				{
					Markers unclasified(startingMarker == 0 ? 0 : startingMarker + 1, i, unclasified, 95);
					markersVector.push_back(unclasified);
				}
				else
				{

					//Sprawdzam czy zdanie ma spacje. Jesli jej nie ma i jest bardzo dlugie to prawdopodobnie jest to zlepek liter
					int found3 = text.find_first_of(" ", startingMarker+2);
					if (i - startingMarker > 15 && (found3 > i || found3 == std::string::npos))
					{
						Markers unclasified(startingMarker == 0 ? 0 : startingMarker + 1, i, unclasified, 70);
						markersVector.push_back(unclasified);
					}
					else
					{
						Markers answer(startingMarker == 0 ? 0 : startingMarker + 1, i, answer, 95);
						markersVector.push_back(answer);
					}
				}
			}
		}
	}
	/*W przypadku gdy tekst jest bez zadnej interpunkcji wykonuje siê ta bardziej "zaawansowana" czesc analizy.
	Jednakze analizowany tekst jest traktowany jako jedno zdanie, a nie wieksza ilosc zdan gdyz niemozliwe jest okreslenie
	kiedy potencjalnie zaczyna sie nowe zdanie, gdy nie mamy interpunkcji*/
	if (markersVector.empty())
	{
		//Zaczynam od sprawdzania czy to jest pytanie
		int found = std::string::npos;
		int k = 0;
		while (found == std::string::npos && k < dictionary.size())
		{
			found = text.find(dictionary[k]);
			k++;
		}

		//Jesli znaleziono slowo kluczowe na poczatku to bardzo prawdopodobne ze to pytanie
		if (found == 0)
		{
			Markers question(0, text.size() - 1, question, 90);
			markersVector.push_back(question);
		}
		else
		{
			//Sprawdzam czy sa samogloski. Jesli nie ma to prawdopodobnie bledne zdanie
			int found2 = text.find_first_of("eyuioa");
			if (found2 == std::string::npos)
			{
				Markers unclasified(0, text.size() - 1, unclasified, 99);
				markersVector.push_back(unclasified);
			}
			else
			{
				//Szukam spacji. Jesli nie ma i zdanie jest dlugie to prawdopodobnie zlepek liter
				int found3 = text.find_first_of(" ");
				if (found3 == std::string::npos && text.size()>15)
				{
					Markers unclasified(0, text.size() - 1, unclasified, 70);
					markersVector.push_back(unclasified);
				}
				else
				{
					Markers answer(0, text.size() - 1, answer, 90);
					markersVector.push_back(answer);
				}
			}
		}
	}
	return markersVector;
}