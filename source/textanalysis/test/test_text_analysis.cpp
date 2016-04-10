#include <boost/test/unit_test.hpp>
#include <iostream>
#include "../text_analysis.h"

using namespace std;


//Poprawna odpowiedz
BOOST_AUTO_TEST_CASE(SimplePositiveAnswerTest)
{
	vector<Markers> testing;
	testing = findQA("Moja wina.");
	Markers a = testing[0];
	BOOST_CHECK(a.getStart() == 0);
	BOOST_CHECK(a.getEnd() == 9);
	BOOST_CHECK(a.getType() == answer);
}

//Brak spacji przy dlugim zdaniu
BOOST_AUTO_TEST_CASE(UnclasifiedTest)
{
	vector<Markers> testing;
	testing = findQA("ojasdsaawiddfsafdfgdsna");
	Markers a = testing[0];
	BOOST_CHECK(a.getType() == unclasified);
}


//Poprawne pytanie
BOOST_AUTO_TEST_CASE(QuestionTest)
{
	vector<Markers> testing;
	testing = findQA("Co tu sie dzieje?");
	Markers a = testing[0];
	BOOST_CHECK(a.getStart() == 0);
	BOOST_CHECK(a.getEnd() == 16);
	BOOST_CHECK(a.getPercentageChance() == 100);
	BOOST_CHECK(a.getType() == question);
}

//Brak samoglosek w zdaniu
BOOST_AUTO_TEST_CASE(VowelTest)
{
	vector<Markers> testing;
	testing = findQA("Gsdl ptdsfgh lkrtf?");
	Markers a = testing[0];
	BOOST_CHECK(a.getType() == unclasified);
}

//Pare poprawnych zdan (zdanie pytające na początku)
BOOST_AUTO_TEST_CASE(FewSentence)
{
	vector<Markers> testing;
	testing = findQA("Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	BOOST_CHECK(testing.size() == 4);

	Markers a = testing[3];
	BOOST_CHECK(a.getStart() == 59);
	BOOST_CHECK(a.getEnd() == 88);
	BOOST_CHECK(a.getType() == question);

	a = testing[2];
	BOOST_CHECK(a.getStart() == 39);
	BOOST_CHECK(a.getEnd() == 58);
	BOOST_CHECK(a.getType() == answer);

	a = testing[1];
	BOOST_CHECK(a.getStart() == 18);
	BOOST_CHECK(a.getEnd() == 38);
	BOOST_CHECK(a.getType() == answer);

	a = testing[0];
	BOOST_CHECK(a.getStart() == 0);
	BOOST_CHECK(a.getEnd() == 17);
	BOOST_CHECK(a.getType() == question);
}

// Pare poprawnych zdań #2 (zdanie oznajmujące na początku)
BOOST_AUTO_TEST_CASE(FewSentence2)
{
	vector<Markers> testing;
	testing = findQA("Tak ma byc. Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	BOOST_REQUIRE(testing.size() == 5);
	
	Markers a = testing[4];
	BOOST_CHECK(a.getStart() == 71);
	BOOST_CHECK(a.getEnd() == 100);
	BOOST_CHECK(a.getType() == question);

	a = testing[3];
	BOOST_CHECK(a.getStart() == 51);
	BOOST_CHECK(a.getEnd() == 70);
	BOOST_CHECK(a.getType() == answer);

	a = testing[2];
	BOOST_CHECK(a.getStart() == 30);
	BOOST_CHECK(a.getEnd() == 50);
	BOOST_CHECK(a.getType() == answer);

	a = testing[1];
	BOOST_CHECK(a.getStart() == 11);
	BOOST_CHECK(a.getEnd() == 29);
	BOOST_CHECK(a.getType() == question);

	a = testing[0];
	BOOST_CHECK(a.getStart() == 0);
	BOOST_CHECK(a.getEnd() == 10);
	BOOST_CHECK(a.getType() == answer);
}

//Tutaj nie powinno wykryć dwóch zdań? Pierwsze "?" - start 0, end 0, type unclasified ; drugie "Tak było." - start 1, end 9 answer.
BOOST_AUTO_TEST_CASE(OneSentence1)
{
	vector<Markers> testing;
	testing = findQA("?Tak było.");
	BOOST_REQUIRE(testing.size() == 1);
	Markers a = testing[0];
	BOOST_CHECK(a.getStart() == 0);
	BOOST_CHECK(a.getEnd() == 9);
	BOOST_CHECK(a.getType() == answer);
}
