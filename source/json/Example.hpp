#include "Json.hpp"

void example1()
{
    struct Rect
    {
        Rect(int x, int y, int w, int h)
            : x(x), y(y), w(w), h(h) {}
        operator Json()
        {
            Json j;
            j["x"] = x;
            j["y"] = y;
            j["w"] = w;
            j["h"] = h;
            return j;
        }
        int x, y, w, h;
    };

    std::vector<Rect> rects;
    for (int i = 1; i <= 10; i++)
        rects.push_back(Rect(i * 5, i * 10, i * 15, i * 20));

    Json j;
    j["status"] = 1;
    j["coordinates"] = std::vector<Json>(rects.begin(), rects.end());

    j = Json{
        {"status", 1},
        {"coordinates", std::vector<Json>(rects.begin(), rects.end())}
    };
}

void example2()
{
    Json j;
    j["float"] = 3.14f;
    j["double"] = 3.14;
    j["uint8_t"] = (uint8_t)42;
    j["int8_t"] = (int8_t)-42;
    j["int64_t"] = (int64_t)42;
    j["bool"] = true;
    j["string"] = "hello world";
    j["null"] = nullptr;
    j["outer"]["inner"] = 42;
    j["vector"] = { 1, 2, 3 };
    j["object"] = { { "alfa", "beta" },{ "gamma", 4 } };

    float f = j["float"];
    double d = j["double"];
    uint8_t u8 = j["uint8_t"];
    int8_t i8 = j["int8_t"];
    int64_t i64 = j["int64_t"];
    bool b = j["bool"];
    std::string s = j["string"];
    int i = j["outer"]["inner"];

    f = j["int8_t"];
    i8 = j["double"];

    std::vector<Json> v = j["vector"];
    std::vector<int> vi(v.begin(), v.end());
    Json jo = j["object"];
    jo["gamma"] = "theta";
    s = jo["gamma"];
    i = j["object"]["gamma"];
}

void example3()
{
    Json j = {
        {"menu", {
            {"id", "file"},
            {"value", "File"},
            {"popup", {
                {"menuitem", {
                    {{"value", "New"}, {"onclick", "CreateNewDoc()"}},
                    {{"value", "Open"}, {"onclick", "OpenDoc()"}},
                    {{"value", "Close"}, {"onclick", "CloseDoc()"}}
                }}
            }}
        }}
    };

    std::string str = j["menu"]["popup"]["menuitem"][0]["onclick"];

    str = j.serialize("out.json");
    j = Json::deserialize(str);
    j = Json::deserialize("out.json");
}

void example4()
{
    Json map = {{"alfa", 42}, {"beta", -3.14}, {"gamma", "delta"}};
    Json list = {"alfa", 1, 2, "beta", "gamma", "theta"};

    for (auto& x : map)
        std::cout << x << " ";
    std::cout << std::endl;

    list.push_back(42);

    for (auto& x : list)
        std::cout << x << " ";
    std::cout << std::endl;

    map.insert("epsilon", true);

    for (auto it = map.begin(); it != map.end(); it++)
        std::cout << it.key() << " " << it.value() << std::endl;

    std::cout << "map: " << map.size() << std::endl;

    list.clear();

    if (list.empty())
        std::cout << "list: empty" << std::endl;
}
