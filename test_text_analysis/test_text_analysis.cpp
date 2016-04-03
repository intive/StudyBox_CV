#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>
#include <iostream>
#include "text_analysis.h"

using namespace std;

//Na czas testów zakładamy, że dane są public.

//Poprawna odpowiedz
BOOST_AUTO_TEST_CASE(SimplePositiveAnswerTest)
{
	cout << "Test nr 1"<<endl;
	vector<Markers> testing;
	testing = findQA("Moja wina.");
	Markers a = testing[0];
	BOOST_CHECK(a.start == 0 && a.end == 9 && a.type == answer);
	getchar();
}

//Brak spacji przy dlugim zdaniu
BOOST_AUTO_TEST_CASE(UnclasifiedTest)
{
	cout << "Test nr 2" << endl;
	vector<Markers> testing;
	testing = findQA("ojasdsaawiddfsafdfgdsna");
	Markers a = testing[0];
	BOOST_CHECK(a.type == unclasified);
	getchar();
}


//Poprawne pytanie
BOOST_AUTO_TEST_CASE(QuestionTest)
{
	cout << "Test nr 3" << endl;
	vector<Markers> testing;
	testing = findQA("Co tu sie dzieje?");
	Markers a = testing[0];
	BOOST_CHECK(a.start == 0 && a.end == 16 && a.percentage_chance == 100 && a.type == question);
	getchar();
}

//Brak samoglosek w zdaniu
BOOST_AUTO_TEST_CASE(VowelTest)
{
	cout << "Test nr 4" << endl;
	vector<Markers> testing;
	testing = findQA("Gsdl ptdsfgh lkrtf?");
	Markers a = testing[0];
	BOOST_CHECK(a.type == unclasified);
	getchar();
}

//Pare poprawnych zdan (nie wykrywa pierwszego zdania!)
BOOST_AUTO_TEST_CASE(FewSentence)
{
	cout << "Test nr 5" << endl;
	vector<Markers> testing;
	testing = findQA("Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	cout <<"size : " << testing.size() << endl;
	BOOST_CHECK(testing.size() == 4);

	// Ponieważ size() != 4 to tu będzie bład kompilacji, więc zakomentowałem dopóki algorytm źle działa.
	/*
	Markers a = testing[3];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 59 && a.end == 88 && a.type == question);
	getchar();
	a = testing[2];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 39 && a.end == 58 && a.type == answer);
	getchar();
	a = testing[1];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 18 && a.end == 38 && a.type == answer);
	getchar();
	a = testing[0];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 0 && a.end == 17 && a.type == question);
	getchar();
	*/
}

// Myślałem, że to błąd przy wykrywaniu zdania pytającego na początku ciągu zdań,
// lecz po tym teście widać, że algorytm tylko te zdanie nie wykrywa ("Dlaczego tak malo?")
BOOST_AUTO_TEST_CASE(FewSentence2)
{
	cout << "Test nr 6" << endl;
	vector<Markers> testing;
	testing = findQA("Tak ma byc. Dlaczego tak malo? Jutro bedzie wiosna. Mam alergie i kota. Jak i gdzie to sie wydarzylo?");
	cout << "size : " << testing.size() << endl;
	BOOST_CHECK(testing.size() == 5);

	// Ponieważ size() != 4 to tu będzie bład kompilacji, więc zakomentowałem dopóki algorytm źle działa.
	/*
	Markers a = testing[4];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 71 && a.end == 100 && a.type == answer);
	getchar();
	a = testing[3];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 51 && a.end == 70 && a.type == answer);
	getchar();
	a = testing[2];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 30 && a.end == 50 && a.type == question);
	getchar();
	a = testing[1];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 11 && a.end == 29 && a.type == answer);
	getchar();
	a = testing[0];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.start == 0 && a.end == 10 && a.type == answer);
	getchar();
	*/
}

//Tutaj już wykrywa, więc to bład w algorytmie.
BOOST_AUTO_TEST_CASE(OneSentence3)
{
	cout << "Test nr 7" << endl;
	vector<Markers> testing;
	testing = findQA("Dlaczego tak malo?");
	cout << "size : " << testing.size() << endl;
	BOOST_CHECK(testing.size() == 1);
	Markers a = testing[0];
	cout << a.start << " " << a.end << " " << a.type << " " << a.percentage_chance << endl;
	BOOST_CHECK(a.type == question);
	getchar();
}

