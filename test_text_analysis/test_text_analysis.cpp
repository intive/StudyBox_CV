#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include "text_analysis.h"

using namespace std;

//Na czas testów zakładamy, że dane są public.

//Poprawna odpowiedz
BOOST_AUTO_TEST_CASE(SimplePositiveAnswerTest)
{
	vector<Markers> testing;
	testing = findQA("Moja wina.");
	Markers a = testing[0];
	BOOST_CHECK(a.start == 0);
	BOOST_CHECK(a.end == 9);
	BOOST_CHECK(a.type == answer);
}

//Brak spacji przy dlugim zdaniu
BOOST_AUTO_TEST_CASE(UnclasifiedTest)
{
	vector<Markers> testing;
	testing = findQA("ojasdsaawiddfsafdfgdsna");
	Markers a = testing[0];
	BOOST_CHECK(a.type == unclasified);
}


//Poprawne pytanie
BOOST_AUTO_TEST_CASE(QuestionTest)
{
	vector<Markers> testing;
	testing = findQA("Co tu sie dzieje?");
	Markers a = testing[0];
	BOOST_CHECK(a.start == 0);
	BOOST_CHECK(a.end == 16);
	BOOST_CHECK(a.percentage_chance == 100);
	BOOST_CHECK(a.type == question);
}

//Brak samoglosek w zdaniu
BOOST_AUTO_TEST_CASE(VowelTest)
{
	vector<Markers> testing;
	testing = findQA("Gsdl ptdsfgh lkrtf?");
	Markers a = testing[0];
	BOOST_CHECK(a.type == unclasified);
}

//Pare poprawnych zdan (zdanie pytające na początku)
BOOST_AUTO_TEST_CASE(FewSentence)
{
	vector<Markers> testing;
	testing = findQA("Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	BOOST_CHECK(testing.size() == 4);

	Markers a = testing[3];
	BOOST_CHECK(a.start == 59);
	BOOST_CHECK(a.end == 88);
	BOOST_CHECK(a.type == question);

	a = testing[2];
	BOOST_CHECK(a.start == 39);
	BOOST_CHECK(a.end == 58);
	BOOST_CHECK(a.type == answer);

	a = testing[1];
	BOOST_CHECK(a.start == 18);
	BOOST_CHECK(a.end == 38);
	BOOST_CHECK(a.type == answer);

	a = testing[0];
	BOOST_CHECK(a.start == 0);
	BOOST_CHECK(a.end == 17);
	BOOST_CHECK(a.type == question);
}

// Pare poprawnych zdań #2 (zdanie oznajmujące na początku)
BOOST_AUTO_TEST_CASE(FewSentence2)
{
	vector<Markers> testing;
	testing = findQA("Tak ma byc. Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	BOOST_REQUIRE(testing.size() == 5);
	
	Markers a = testing[4];
	BOOST_CHECK(a.start == 71);
	BOOST_CHECK(a.end == 100);
	BOOST_CHECK(a.type == question);

	a = testing[3];
	BOOST_CHECK(a.start == 51);
	BOOST_CHECK(a.end == 70);
	BOOST_CHECK(a.type == answer);

	a = testing[2];
	BOOST_CHECK(a.start == 30);
	BOOST_CHECK(a.end == 50);
	BOOST_CHECK(a.type == answer);

	a = testing[1];
	BOOST_CHECK(a.start == 11);
	BOOST_CHECK(a.end == 29);
	BOOST_CHECK(a.type == question);

	a = testing[0];
	BOOST_CHECK(a.start == 0);
	BOOST_CHECK(a.end == 10);
	BOOST_CHECK(a.type == answer);
}

//Tutaj nie powinno wykryć dwóch zdań? Pierwsze "?" - start 0, end 0, type unclasified ; drugie "Tak było." - start 1, end 9 answer.
BOOST_AUTO_TEST_CASE(OneSentence1)
{
	vector<Markers> testing;
	testing = findQA("?Tak było.");
	BOOST_REQUIRE(testing.size() == 1);
	Markers a = testing[0];
	BOOST_CHECK(a.start == 0);
	BOOST_CHECK(a.end == 9);
	BOOST_CHECK(a.type == answer);
}