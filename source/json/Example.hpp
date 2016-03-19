#include "Json.hpp"

void example()
{
    Json j;
    j["null"] = nullptr;
    j["boolean"] = true;
    j["integral"] = 42;
    j["floating"] = 3.14;
    j["string"] = "hello world";
    j["array"] = { 1, 2, 3 };
    j["object"] = {
        {"key", "alfa"},
        {"value", 1}
    };

    Json j2 = {
        {"null", nullptr},
        {"boolean", true},
        {"integral", 42},
        {"floating", 3.14},
        {"string", "hello world"},
        {"array", {1, 2, 3}},
        {"object", {
            {"key", "alfa"},
            {"value", 1}
        }}
    };

    Json j3 = Json::deserialize("in.json");
    std::string str = j3.serialize("out.json");
}
