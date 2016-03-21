//Jak Dawid wrzuci kod, to dostosuje testy do jego kodu.
#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

//Klasa markerów do zaznaczania początku, końca oraz typu (answer/question) tekstu, 
//podobną będzie miał Dawid w swoim kodzie.
class Markers
{
//Tylko na czas testów public:
public:
    int start;
    int end;
    string type;
public:
    Markers(int s, int e, string t) :start(s), end(e), type(t) {};
};

//Uboga funkcja tylko do testów odpowiedzi
vector<Markers> findA(string text)
{
    vector<Markers> markersVector;
    Markers answer(0, 9, "answer");
    markersVector.push_back(answer);
    return markersVector;
}

//Uboga funkcja tylko do testów pytań
vector<Markers> findQ(string text)
{
    vector<Markers> markersVector;
    Markers answer(0, 11, "question");
    markersVector.push_back(answer);
    return markersVector;
}

//Uboga funkcja tylko do testów odpowiedzi i pytania
vector<Markers> findQA(string text)
{
    vector<Markers> markersVector;
    Markers answer(0, 9, "answer");
    markersVector.push_back(answer);
    Markers question(10, 21, "question");
    markersVector.push_back(question);
    return markersVector;
}

//Test odpowiedzi
BOOST_AUTO_TEST_CASE(SimplePositiveAnswerTest)
{
    vector<Markers> testing;
    testing = findA("Moja wina.");
    Markers a = testing[0];
    BOOST_CHECK(a.start == 0 && a.end == 9 && a.type == "answer");
}

//Test pytania
BOOST_AUTO_TEST_CASE(SimplePositiveQuestionTest)
{
    vector<Markers> testing;
    testing = findQ("Czy to moje?");
    Markers a = testing[0];
    BOOST_CHECK(a.start == 0 && a.end == 11 && a.type == "question");
}

//Test odpowiedź-pytanie
BOOST_AUTO_TEST_CASE(SimpleMixedTest)
{
    vector<Markers> testing;
    testing = findQA("Moja wina.Czy to moje?");
    Markers a = testing[0];
    BOOST_CHECK(a.start == 0 && a.end == 9 && a.type == "answer");
    Markers b = testing[1];
    BOOST_CHECK(b.start == 10 && b.end == 21 && b.type == "question");
}

//Test błędów
BOOST_AUTO_TEST_CASE(SimpleErrorTest)
{
    vector<Markers> testing;
    testing = findQ("Moja wina.");
    Markers a = testing[0];
	//Użyłem findQ zamiast findA, więc nie będzie się zgadzać a.end oraz a.type
    BOOST_CHECK(a.start == 0 && a.end == 9 && a.type == "answer");
	//Test a.start jest równy 0, więc wyrzuci błąd
    BOOST_CHECK_EQUAL(a.start , 5);
    getchar();
	//Jeżeli a.start nie równa się 4 to automatycznie sfailuje
    BOOST_REQUIRE(a.start == 4);
}