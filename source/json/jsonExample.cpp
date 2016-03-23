#include <iostream>
#include <random>
#include <string>
#include "json.h"


std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(1, 100);

struct Coordinates
{
    int top_left_x     = 0;
    int top_left_y     = 0;
    int bottom_right_x = 0;
    int bottom_right_y = 0;

    Coordinates() = default;

    Coordinates(bool randomMock)
    {
        top_left_x       = dist(mt);
        top_left_y     = dist(mt);
        bottom_right_x = dist(mt);
        bottom_right_y = dist(mt);
    }

    Json::Object getJsonObject()
    {
        // Ró¿ne sposoby dodania w³asnoœci do obiektu

        auto jo = Json::Object()
            .Add("top_left_x", top_left_x);

        jo["top_left_y"] = top_left_y;

        jo.Add({
                {"bottom_right_y", bottom_right_y},
                {"bottom_right_x", bottom_right_x}
        });

        return jo;
    }

    void fromJsonObject(Json::Object& jsonObject)
    {
        bottom_right_x = jsonObject.at("bottom_right_x").getInt();
        bottom_right_y = jsonObject.at("bottom_right_y").getInt();
        top_left_x     = jsonObject["top_left_x"].getInt();
        top_left_y     = jsonObject["top_left_y"].getInt();
    }
};

class SegmentationResponseMock
{
    friend bool operator==(const SegmentationResponseMock& lhs, const SegmentationResponseMock& rhs);

    std::vector<Coordinates> coords;
    int status;
public:
    SegmentationResponseMock() = default;

    SegmentationResponseMock(bool randomMock)
    {
        status = 200;
        coords.push_back(Coordinates(true));
        coords.push_back(Coordinates(true));
    }


    Json::Object getJsonObject()
    {
        // Konstruowanie tablicy obiektów
        Json::Array jsonArr;
        for (auto& c : coords)
            jsonArr.Add(c.getJsonObject());


        auto jsonObj = Json::Object()
            .Add("status", status);
        
        jsonObj["coordinates"] = jsonArr;

        return jsonObj;
    }


    void fromJsonObject(Json::Object& jsonObject)
    {
        status = static_cast<int>(jsonObject.at("status").getInt());
        auto arrayOfCoordJsonArray = jsonObject["coordinates"].getArray();

        for (auto& item : arrayOfCoordJsonArray)
        {
            Json::Object coordJsonObject = item.getObject();
            Coordinates c;
            c.fromJsonObject(coordJsonObject);
            coords.push_back(c);
        }
    }
};

bool operator==(const Coordinates& lhs, const Coordinates& rhs)
{
    return
        lhs.bottom_right_x == rhs.bottom_right_x
        && lhs.bottom_right_y == rhs.bottom_right_y
        && lhs.top_left_x == rhs.top_left_x
        && lhs.top_left_y == rhs.top_left_y;
}

bool operator==(const SegmentationResponseMock& lhs, const SegmentationResponseMock& rhs)
{
    return lhs.status == rhs.status
        && lhs.coords == rhs.coords;
}


void jsonExamples()
{
    SegmentationResponseMock mock(true);                    // Tworzenie mocka dla odpowiedzi na ¿¹danie segmentacji obrazu.
    Json::Object mock_json = mock.getJsonObject();          // Uzyskanie Json::Object odpowiadaj¹cego danemu obiektowi.
    // std::cout << mock_json.serialize() << std::endl;     // Wypisanie jsona na std wyjœcie
    // mock_json.serializeToFile("mock.json");              // Lub zapis do pliku


    // Rekonstrukcja json -> obiekt
    SegmentationResponseMock dmock;
    dmock.fromJsonObject(mock_json);

    // Sprawdzenie czy pierwotny obiekt odpowiada zrekonstruowanemu ze struktury obiektów json.
    std::cout << std::boolalpha << (mock == dmock) << std::endl;


    auto bracketInitialized = Json::Array{
            "asdsad",
            Json::Object {
                    { "k1", 1},
                    { "k2", false },
                    { "k3", nullptr },
                    { "k4", Json::Array{ 1, 2, 3}}
            },
    };
    // std::cout << bracketInitialized.serialize() << std::endl;
    // bracketInitialized.serializeToFile("bracket.json");
}