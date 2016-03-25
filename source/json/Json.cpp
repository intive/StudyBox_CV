#include "Json.hpp"

#include <string>
#include <fstream>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

// Konstruktor domyślny
Json::Json()
    : type(Type::Null)
{

}

// Konstruktor kopiujący
Json::Json(const Json& arg)
    : type(arg.type)
{
    switch (type)
    {
        case Type::Array:
            value = *arg.value.array;
            break;
        case Type::Boolean:
            value = arg.value.boolean;
            break;
        case Type::Floating:
            value = arg.value.floating;
            break;
        case Type::Integer:
            value = arg.value.integer;
            break;
        case Type::Null:
            value = nullptr;
            break;
        case Type::Object:
            value = *arg.value.object;
            break;
        case Type::String:
            value = *arg.value.string;
            break;
        case Type::Uinteger:
            value = arg.value.uinteger;
            break;
        default:
            break;
    }
}

// Konstruktor przenoszący
Json::Json(Json&& arg)
    : type(std::move(arg.type))
    , value(std::move(arg.value))
{
    arg.type = Type::Null;
    arg.value = {};
}

// Operator przypisania
Json& Json::operator=(Json arg)
{
    std::swap(type, arg.type);
    std::swap(value, arg.value);
    return *this;
}

// Destruktor
Json::~Json()
{
    switch (type)
    {
    case Type::Array:
        delete value.array;
        break;
    case Type::Object:
        delete value.object;
        break;
    case Type::String:
        delete value.string;
        break;
    default:
        break;
    }
}

// Konstruktor obiektu pustego
Json::Json(std::nullptr_t)
    : type(Type::Null)
{

}

// Konstruktor obiektów Boolowskich
Json::Json(bool arg)
    : type(Type::Boolean)
    , value(arg)
{

}

// Konstruktor obiektów łańcuchów znaków
Json::Json(const std::string& arg)
    : type(Type::String)
    , value(arg)
{

}

// Konstruktor obiektów tablicowych
Json::Json(const std::vector<Json>& arg)
    : type(Type::Array)
    , value(arg)
{

}

// Konstruktor obiektów JSON z listy inicjującej
Json::Json(const std::initializer_list<Json>& arg)
{
    bool isObject = true;
    for (auto x : arg)
    {
        if (x.type != Type::Array || x.size() != 2 || x[0].type != Type::String)
        {
            isObject = false;
            break;
        }
    }

    if (isObject)
    {
        type = Type::Object;
        value.object = new std::map<std::string, Json>();
        for (auto x : arg)
            (*value.object)[x[0]] = x[1];
    }
    else
    {
        type = Type::Array;
        value.array = new std::vector<Json>(std::move(arg));
    }
}

// Operator zwracający obiekt o podanej nazwie
Json& Json::operator[](const char* arg)
{
    if (type == Type::Null)
    {
        type = Type::Object;
        value.object = new std::map<std::string, Json>();
    }

    if (type != Type::Object)
        throw std::domain_error("type is not object");
    return (*value.object)[arg];
}

// Operator zwracający obiekt o podanej nazwie
Json& Json::operator[](const std::string& arg)
{
    return (*this)[arg.c_str()];
}

// Operator rzutujący na obiekt Boolowski
Json::operator bool()
{
    if (type != Type::Boolean)
        throw std::domain_error("type is not boolean");
    return value.boolean;
}

// Operator rzutujący na obiekt łańcucha znaków
Json::operator std::string()
{
    if (type != Type::String)
        throw std::domain_error("type is not string");
    return *value.string;
}

// Operator rzutujący na obiekt tablicowy
Json::operator std::vector<Json>()
{
    if (type != Type::Array)
        throw std::domain_error("type is not array");
    return *value.array;
}

// Metoda zwraca ilość elementów w obiekcie
size_t Json::size()
{
    if (type == Type::Array)
        return value.array->size();
    else if (type == Type::Object)
        return value.object->size();
    else
        throw std::domain_error("object does not have size");
}

// Metoda zwraca łańcuch znaków z usuniętymi nadmiarowymi znakami białymi zgodnie z regułami JSON
std::string Json::minify(std::string str)
{
    size_t begin = 0;
    while (true)
    {
        size_t end = str.find('\"', begin);
        if (end == std::string::npos)
            end = str.size();
        else if (end > 0 && str[end - 1] == '\\')
            continue;

        size_t length = str.length();
        str.erase(std::remove_if(str.begin() + begin, str.begin() + end,
            [](char c) { return (c == ' ' || c == '\t' || c == '\r' || c == '\n'); }),
            str.begin() + end);
        length -= str.length();

        begin = end - length + 1;
        do
        {
            begin = str.find('\"', begin);
            if (begin == std::string::npos)
                return str;
        } while (str[begin++ - 1] == '\\');
    }
}

// Metoda deserializująca JSON z łańcucha znaków
// lub pliku jeśli podano ścieżkę do pliku z rozszerzeniem ".json"
Json Json::deserialize(std::string str)
{
    static const std::string ext = ".json";

    bool isPath = false;
    if (str.size() > ext.size())
        isPath = str.substr(str.size() - ext.size()) == ext;

    if (isPath)
    {
        std::ifstream file(str);
        if (file.good())
            str = std::string(std::istreambuf_iterator<char>(file),
                std::istreambuf_iterator<char>());
        else
            throw std::exception(("Couldn't open file: " + str).c_str());
    }

    str = minify(str);

    Json j;

    // Wykorzystać Recursive Descent Parsing

    return j;
}

// Metoda serializująca JSON do łańcucha znaków
// lub pliku jeśli podano ścieżkę do pliku
std::string Json::serialize(const std::string& path)
{
    std::string str;

    // Wykorzystać Recursive Descent Parsing

    if (!path.empty())
    {
        std::ofstream file(path);
        if (file.good())
            file << str;
        else
            throw std::exception(("Couldn't open file: " + path).c_str());
    }

    return str;
}
