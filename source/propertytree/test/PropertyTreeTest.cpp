#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "../PropertyTree.h"
#include "../PropertyTreeReader.h"
#include "../PropertyTreeWriter.h"

#include "../../json/json.h"


BOOST_AUTO_TEST_SUITE(PropertyTreeValidity)

struct TreeTest
{
    TreeTest()
    {
        inout = "{\"firstName\":\"John\",\"lastName\":\"Smith\",\"isAlive\":true,\"age\":25,\"address\":{\"streetAddress\":\"21 2nd Street\",\"city\":\"New York\",\"state\":\"NY\",\"postalCode\":\"10021 - 3100\"},\"phoneNumbers\":[{\"type\":\"home\",\"number\":\"212 555 - 1234\"},{\"type\":\"office\",\"number\":\"646 555 - 4567\"}],\"children\":[],\"spouse\":null}";

        tree.put("firstName", "John");
        tree.put("lastName", "Smith");
        tree.put("isAlive", true);
        tree.put("age", 25);

        PropertyTree address;
        address.put("streetAddress", "21 2nd Street");
        address.put("city", "New York");
        address.put("state", "NY");
        address.put("postalCode", "10021 - 3100");
        tree.put("address", address);
        std::vector<PropertyTree> phones(2);
        phones[0].put("type", "home");
        phones[0].put("number", "212 555 - 1234");
        phones[1].put("type", "office");
        phones[1].put("number", "646 555 - 4567");
        tree.put("phoneNumbers", phones);
        tree.put("children", std::vector<nullptr_t>());
        tree.put("spouse", nullptr);
    }

    std::string inout;
    PropertyTree tree;
};

BOOST_AUTO_TEST_CASE(PropertyTreeFields)
{
    TreeTest t;
    auto& tree = t.tree;
    BOOST_REQUIRE_NO_THROW(
        BOOST_TEST(tree.find("isAlive").get<bool>() == true)
    );
    BOOST_REQUIRE_THROW(tree.find("phoneNumbers.type", 2), std::out_of_range);
    BOOST_REQUIRE_THROW(tree.find("phoneNumbers.children"), std::out_of_range);
    BOOST_REQUIRE_NO_THROW(
        BOOST_TEST(tree.find("phoneNumbers.type", 1).get<std::string>() == "office")
    );
    BOOST_TEST(tree.find("spouse").get<std::string>() == "null");
}


BOOST_AUTO_TEST_CASE(PropertyTreeToJson)
{
    TreeTest t;
    auto& tree = t.tree;
    auto& expectedOutput = t.inout;

    std::ostringstream ss;
    WriteJson(tree, ss, false);
    BOOST_TEST(ss.str() == expectedOutput);
}

BOOST_AUTO_TEST_CASE(JsonToPropertyTree)
{
    TreeTest t;
    auto& tree = t.tree;
    auto& input = t.inout;
    std::istringstream iss(input);
    PropertyTree out;
    ReadJson(out, iss);
    BOOST_TEST((tree == out));

}

BOOST_AUTO_TEST_CASE(ConversionIntegrity)
{
    TreeTest t;
    auto& tree = t.tree;
    auto& input = t.inout;
    std::ostringstream oss;
    PropertyTree out;
    WriteJson(tree, oss, true);
    std::istringstream iss(oss.str());
    BOOST_REQUIRE_NO_THROW(ReadJson(out, iss));
    BOOST_TEST((tree == out));
    out = PropertyTree();
    oss = std::ostringstream();
    WriteJson(tree, oss, false);
    iss = std::istringstream(oss.str());
    BOOST_REQUIRE_NO_THROW(ReadJson(out, iss));
    BOOST_TEST((tree == out));
}

BOOST_AUTO_TEST_CASE(JsonObjectInterop)
{
    TreeTest t;
    auto& tree = t.tree;
    auto& expectedOutput = t.inout;

    auto json = ToJsonObject(tree);
    // Ze wzglêdu na brak zachowania kolejnoœci pól Json::Object, potrzebna jest ponowna serializacja do przejœcia testu.
    std::istringstream istr(json.serialize());
    tree = PropertyTree();
    ReadJson(tree, istr);
    std::ostringstream ostr;
    WriteJson(tree, ostr, false);
    auto json2 = ToJsonObject(tree);
    BOOST_TEST((json2.serialize() == ostr.str()));
    BOOST_TEST((json.serialize() == ostr.str()));
}

BOOST_AUTO_TEST_SUITE_END()
