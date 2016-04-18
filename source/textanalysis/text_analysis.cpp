#include"text_analysis.h"
#include<vector>
#include<cctype>
#include<algorithm>
const std::vector<std::string> dictionary = {
	"czy",
	"ile",
	"gdzie",
	"jak",
	"dlaczego",
	"po co",
	"czemu",
	"z kim",
	"z czym",
	"na czym",
	"kiedy",
	"kim",
	"w którym","w ktorym",
	"kogo",
	"za co",
	"co",
	"u kogo",
	"jakiego",
	"dla jakiego",
	"dla kogo",
	"komu"
};

//Funkcja wracaj¹ca od punktu end do najbli¿szego znaku interpunkcyjnego

size_t findPreviousPunctuationMark(const std::string& text, size_t end)
{
	size_t startingMarker = end - 1;
	while (true)
	{
		if (text[startingMarker] == '.' || text[startingMarker] == '!' || text[startingMarker] == '?')
		{
			startingMarker++;
			break;
		}
		if (startingMarker != 0)
			startingMarker--;
		else
			break;
	}
	return startingMarker;
}

//Funkcja sprawdzajaca czy zdanie jest puste albo z jednym znakiem

bool checkIfEmptySentence(std::vector<Markers>& markersVector, const std::string& text, size_t start, size_t end)
{
	if (end - start < 2)
	{
		Markers unclasified(start, end, TextType::unclasified, 80);
		markersVector.push_back(unclasified);
		return true;
	}
	else
	{
		return false;
	}
}

//Funkcja sprawdzaj¹ca czy s¹ samog³oski w zdaniu

bool checkIfNoVowels(std::vector<Markers>& markersVector, const std::string& text, size_t start, size_t end)
{
	auto found2 = text.find_first_of("eyuioa", start);
	if (found2 == std::string::npos || found2 > end)
	{
		Markers unclasified(start, end, TextType::unclasified, 95);
		markersVector.push_back(unclasified);
		return true;
	}
	else
		return false;
}

//Funkcja sprawdzaj¹ca spacje w zdaniu

bool checkIfNoSpaces(std::vector<Markers>& markersVector, const std::string& text, size_t start, size_t end)
{
	auto found3 = text.find_first_of(" ", start + 1);
	if (end - start > 15 && (found3 > end || found3 == std::string::npos))
	{
		Markers unclasified(start, end, TextType::unclasified, 70);
		markersVector.push_back(unclasified);
		return true;
	}
	else
		return false;
}

std::vector<Markers> findQA(std::string text)
{
	std::transform(text.begin(), text.end(),text.begin(), std::tolower);
	std::vector<Markers> markersVector;
	if (text.empty())
		return markersVector;
	/*Analiza opiera sie na interpunkcji w zdaniu*/
	for (std::size_t i = 0; i<text.size(); ++i)
	{
		//Jesli wykryto znak zapytania
		if (text[i] == '?')
		{
			//Wykrywanie przypadku gdy zdanie z niewiadomych przyczyn zaczyna sie znakiem zapytania

			if (i == 0)
			{
				continue;
			}
			
			//"Cofanie siê" dopoki nie znajdzie sie znaku interpunkcyjnego lub poczatku

			std::size_t startingMarker = findPreviousPunctuationMark(text, i);

			//Sprawdzanie sytuacji typu ". ? " badz z jakims jednym znakiem pomiedzy

			if (checkIfEmptySentence(markersVector, text, startingMarker, i))
				continue;

			auto found = std::string::npos;
			std::size_t k = 0;
			//Wyszukiwanie w zdaniu jednego ze slow kluczowych zdefiniowanych w dictionary, charakteryzujacych pytanie
			while (k < dictionary.size())
			{
				auto tmp = text.find(dictionary[k], startingMarker);
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
			if (found - startingMarker < 4 && found != std::string::npos && found < i)
			{
				Markers question(startingMarker, i, TextType::question, 100);
				markersVector.push_back(question);
			}
			else if (found - startingMarker > 4 && found < i)  //w przeciwnym wypadku jesli znaleziono gdzies bardziej w srodku to tylko na 99%
			{
				Markers question(startingMarker, i, TextType::question, 99);
				markersVector.push_back(question);
			}
			else               //jesli nie znaleziono 
			{
				//Sprawdzam czy zdanie ma jakakolwiek samogloske. Jesli jej nie ma to prawie na pewno jest to blad
				
				if (checkIfNoVowels(markersVector,text,startingMarker,i))
				{
					continue;
				}
				else
				{
					//Sprawdzam czy zdanie ma jakas spacje. Jesli nie ma, a jest bardzo dlugie to prawdopodobnie jest to zlepek roznych liter

					if (checkIfNoSpaces(markersVector, text, startingMarker, i))
					{
						continue;
					}
					else
					{
						Markers question(startingMarker, i, TextType::question, 95);
						markersVector.push_back(question);
					}
				}
			}
		}
		else if (text[i] == '!' || text[i] == '.' || ((i == text.size() - 1) && (!markersVector.empty())))
		{
			//Wykrywanie przypadku gdy zdanie z niewiadomych przyczyn zaczyna sie znakiem interpunkcyjnym
			if (i == 0)
			{
				continue;
			}

			//"Cofanie siê" dopoki nie znajdzie sie znaku interpunkcyjnego lub poczatku

			size_t startingMarker = findPreviousPunctuationMark(text, i);

			//Wykrywanie sytuacji " ? . " i podobnych

			if (checkIfEmptySentence(markersVector, text, startingMarker, i))
				continue;

			//Sprawdzam czy zdanie ma jakakolwiek samogloske. Jesli jej nie ma to prawie na pewno jest to blad
			
			if (checkIfNoVowels(markersVector, text, startingMarker, i))
			{
				continue;
			}
			else
			{
				//Sprawdzam czy zdanie ma spacje. Jesli jej nie ma i jest bardzo dlugie to prawdopodobnie jest to zlepek liter
				if (checkIfNoSpaces(markersVector, text, startingMarker, i))
				{
					continue;
				}
				else
				{
					Markers answer(startingMarker, i, TextType::answer, 95);
					markersVector.push_back(answer);
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
		size_t found = std::string::npos;
		std::size_t k = 0;
		while (found == std::string::npos && k < dictionary.size())
		{
			found = text.find(dictionary[k]);
			k++;
		}

		//Jesli znaleziono slowo kluczowe na poczatku to bardzo prawdopodobne ze to pytanie
		if (found == 0)
		{
			Markers question(0, text.size() - 1, TextType::question, 90);
			markersVector.push_back(question);
		}
		else
		{
			//Sprawdzam czy sa samogloski. Jesli nie ma to prawdopodobnie bledne zdanie
			if (checkIfNoVowels(markersVector, text, 0, text.size() - 1))
			{
				return markersVector;
			}
			else
			{
				//Szukam spacji. Jesli nie ma i zdanie jest dlugie to prawdopodobnie zlepek liter
				if (checkIfNoSpaces(markersVector, text, 0, text.size()))
				{
					return markersVector;
				}
				else
				{
					Markers answer(0, text.size() - 1, TextType::answer, 90);
					markersVector.push_back(answer);
				}
			}
		}
	}
	return markersVector;
}

size_t Markers::getStart() const
{
	return start;
}
size_t Markers::getEnd() const
{
	return end;
}
TextType Markers::getType() const
{
	return type;
}
int Markers::getPercentageChance() const
{
	return percentage_chance;
}
