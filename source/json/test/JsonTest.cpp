#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <fstream>

#include "../../json/Json.hpp"

BOOST_AUTO_TEST_SUITE(JsonTest)

BOOST_AUTO_TEST_CASE(ConstrucAndCompare)
{
	Json nullJson;
	if (nullJson.getType() == Json::Type::Null)
		BOOST_TEST(true);
	else
		BOOST_TEST(false);

	/******/

	Json toCopy("testValue");
	Json destination(toCopy);
	Json destination2 = toCopy;
	Json moveDestination;
	BOOST_TEST(destination.serialize() == toCopy.serialize());
	BOOST_TEST(destination2.serialize() == "\"testValue\"");
	moveDestination = std::move(toCopy);
	BOOST_TEST(moveDestination.isNull() == false);
	BOOST_TEST(toCopy.isNull() == true);

	/******/

	bool boolValue = true;
	std::string stringValue = "testValue";
	std::vector<Json> arrayVector;
		arrayVector.push_back(Json(1));
		arrayVector.push_back(Json(2));
	std::map < std::string, Json> objectMap;
		objectMap.insert(std::pair<std::string, Json>("testValue1", 1));
		objectMap.insert(std::pair<std::string, Json>("testValue2", 2));
	float floatValue = 1.43f;
	int integerValue = 1;
	unsigned int uintegerValue = 32;

	Json nullJson2(nullptr);
	BOOST_TEST(nullJson2.isNull() == true);
	BOOST_TEST(nullJson2.isNumeric() == false);
	BOOST_TEST(nullJson2.serialize() == "null");

	Json boolJson(true);
	BOOST_TEST(boolJson.isBool() == true);
	BOOST_TEST(boolJson.isNumeric() == false);
	BOOST_TEST(boolJson.serialize() == "true");

	Json stringJson(stringValue);
	BOOST_TEST(stringJson.isString() == true);
	BOOST_TEST(stringJson.isNumeric() == false);
	BOOST_TEST(stringJson.serialize() == "\"testValue\"");

	Json constCharJson(stringValue.c_str());
	BOOST_TEST(constCharJson.isString() == true);
	BOOST_TEST(constCharJson.isNumeric() == false);
	BOOST_TEST(constCharJson.serialize() == "\"testValue\"");

	Json arrayJson(arrayVector);
	BOOST_TEST(arrayJson.isArray() == true);
	BOOST_TEST(arrayJson.isNumeric() == false);
	BOOST_TEST(arrayJson.serialize() == "[1,2]");

	Json objectJson(objectMap);
	BOOST_TEST(objectJson.isObject() == true);
	BOOST_TEST(objectJson.isNumeric() == false);
	BOOST_TEST(objectJson.serialize() == "{\"testValue1\":1,\"testValue2\":2}");

	Json listValues({ 1,2,3,"testValue" });
	BOOST_TEST(listValues.isArray() == true);
	BOOST_TEST(listValues.isNumeric() == false);
	BOOST_TEST(listValues.serialize() == "[1,2,3,\"testValue\"]");

	Json listValues2({ {"testValue1", 1}, {"testValue2", 2} });
	BOOST_TEST(listValues2.isObject() == true);
	BOOST_TEST(listValues2.isNumeric() == false);
	BOOST_TEST(listValues2.serialize() == "{\"testValue1\":1,\"testValue2\":2}");

	Json floatJson(floatValue);
	BOOST_TEST(floatJson.isFloating() == true);
	BOOST_TEST(floatJson.isNumeric() == true);

	Json integerJson(integerValue);
	BOOST_TEST(integerJson.isInteger() == true);
	BOOST_TEST(integerJson.isNumeric() == true);
	BOOST_TEST(integerJson.serialize() == "1");

	Json uintegerJson(uintegerValue);
	BOOST_TEST(uintegerJson.isUinteger() == true);
	BOOST_TEST(uintegerJson.isNumeric() == true);
	BOOST_TEST(uintegerJson.serialize() == "32");
}

BOOST_AUTO_TEST_CASE(DataAccess)
{
	Json testBatch = { 
		{ "alfa", 42 },
		{ "beta", -3.14 },
		{ "gamma", "delta" } 
	};
	Json testBatch2(nullptr);
	Json testBatch3(123);
	Json testBatch4 = { 1, 4, 55, "test" };

	int integer;
	std::string str;
	std::ostringstream oss;

	BOOST_REQUIRE_NO_THROW(
		integer = testBatch["alfa"];
		BOOST_TEST(integer == 42);
	);

	BOOST_REQUIRE_NO_THROW(
		oss << testBatch2["d"];
		BOOST_TEST(oss.str() == "null");
	);

	BOOST_REQUIRE_NO_THROW(
		str = testBatch4[3];
		integer = testBatch4[2];
		BOOST_TEST(str == "test");
		BOOST_TEST(integer == 55);
	);
	
	BOOST_REQUIRE_THROW(
		(integer = testBatch3[2]), std::domain_error
	);

	BOOST_REQUIRE_THROW(
		(integer = testBatch3["alfa"]), std::domain_error
		);
	
	/*
	BOOST_REQUIRE_THROW(
		(integer = testBatch4[4]), std::out_of_range
		);
	*/

	BOOST_REQUIRE_THROW(
		(integer = testBatch4.at(4)), std::out_of_range
		);
}

BOOST_AUTO_TEST_CASE(Output)
{
	Json testBatch = {
		{ "alfa", 42 },
		{ "beta", -3.14 },
		{ "gamma", "delta" }
	};
	std::string testBatchExpect = "{\"alfa\":42,\"beta\":-3.14,\"gamma\":\"delta\"}";
	std::ostringstream oss1, oss2;
	
	oss1 << testBatch;
	oss2 << testBatch.serialize();
	testBatch.serialize("test.json");
	
	std::ifstream file("test.json");
	if (file)
	{
		std::stringstream ss;
		ss << file.rdbuf();
		BOOST_TEST(ss.str() == testBatchExpect);
	}
	BOOST_TEST(oss1.str() == testBatchExpect);
	BOOST_TEST(oss2.str() == testBatchExpect);
}

BOOST_AUTO_TEST_SUITE_END()
