#include "Json.hpp"

#include <string>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

#include "../propertytree/PropertyTree.h"
#include "../propertytree/PropertyTreeReader.h"
#include "../propertytree/PropertyTreeWriter.h"

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
Json::Json(Null)
    : type(Type::Null)
{

}

// Konstruktor obiektów Boolowskich
Json::Json(const bool arg)
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

// Konstruktor obiektów łańcuchów znaków
Json::Json(const char* arg)
    : type(Type::String)
    , value(arg)
{

}

// Konstruktor obiektów tablicowych
Json::Json(const Array& arg)
    : type(Type::Array)
    , value(arg)
{

}

// Konstruktor obiektów typu obiektowego
Json::Json(const Object& arg)
    : type(Type::Object)
    , value(arg)
{

}

// Konstruktor obiektów JSON z listy inicjującej
Json::Json(const std::initializer_list<Json>& arg)
{
    bool isValid = true;
    for (const Json& x : arg)
    {
        if (!x.isArray() || x.size() != 2 || !x[0].isString())
        {
            isValid = false;
            break;
        }
    }

    if (isValid)
    {
        type = Type::Object;
        value.object = new Object();
        for (auto x : arg)
            (*value.object)[x[0]] = x[1];
    }
    else
    {
        type = Type::Array;
        value.array = new Array(std::move(arg));
    }
}

// Operator zwracający obiekt o podanej nazwie
Json& Json::operator[](const char* arg)
{
    if (isNull())
    {
        type = Type::Object;
        value.object = new Object();
    }

    if (!isObject())
        throw std::domain_error("type is not object");

    return (*value.object)[arg];
}

// Operator zwracający obiekt o podanej nazwie
Json& Json::operator[](const std::string& arg)
{
    return (*this)[arg.c_str()];
}

// Operator zwracający obiekt o podanej nazwie
const Json& Json::operator[](const char* arg) const
{
    if (!isObject())
        throw std::domain_error("type is not object");

    return (*value.object)[arg];
}

// Operator zwracający obiekt o podanej nazwie
const Json& Json::operator[](const std::string& arg) const
{
    return (*this)[arg.c_str()];
}

// Operator rzutujący na obiekt Boolowski
Json::operator Boolean() const
{
    if (!isBool())
        throw std::domain_error("type is not boolean");

    return value.boolean;
}

// Operator rzutujący na obiekt łańcucha znaków
Json::operator String() const
{
    if (!isString())
        throw std::domain_error("type is not string");

    return *value.string;
}

// Operator rzutujący na obiekt tablicowy
Json::operator Array() const
{
    if (!isArray())
        throw std::domain_error("type is not array");

    return *value.array;
}

// Operator rzutujący na liczbę całkowitą z znakiem
Json::operator Integer() const
{
    if (!isInteger())
        throw std::domain_error("type is not valid");

    return value.integer;
}

// Operator rzutujący na liczbę całkowitą bez znaku
Json::operator Uinteger() const
{
    if (!isUinteger())
        throw std::domain_error("type is not valid");

    return value.uinteger;
}

// Operator rzutujący na liczbę zmiennoprzecinkową
Json::operator Floating() const
{
    if (!isFloating())
        throw std::domain_error("type is not valid");

    return value.floating;
}

// Operator rzutujący na kontener obiektów
Json::operator Object() const
{
    if (!isObject())
        throw std::domain_error("type is not valid");

    return *value.object;
}

// Operator strumienia wyjścia
std::ostream& operator<<(std::ostream& out, const Json& arg)
{
    switch (arg.type)
    {
    case Json::Type::Array:
    {
        auto& array = *arg.value.array;
        auto it = array.begin();
        out << "[";
        if (!array.empty())
        {
            if (it != array.end())
                out << *it;
            while (++it != array.end())
                out << "," << *it;
        }
        out << "]";
        break;
    }
    case Json::Type::Object:
    {
        auto& objects = *arg.value.object;
        auto it = objects.begin();
        out << "{";
        if (!objects.empty())
        {
            if (it != objects.end())
                out << "\"" << it->first << "\":" << it->second;
            while (++it != objects.end())
                out << ",\"" << it->first << "\":" << it->second;
        }
        out << "}";
        break;
    }
    case Json::Type::String:
        out << "\"" << *arg.value.string << "\"";
        break;
    case Json::Type::Boolean:
        out << std::boolalpha << arg.value.boolean;
        break;
    case Json::Type::Floating:
        if (std::fmod(arg.value.floating, 1.0))
            out << std::setprecision(std::numeric_limits<double>::digits10)
            << arg.value.floating;
        else
            out << arg.value.floating << ".0";
        break;
    case Json::Type::Integer:
        out << arg.value.integer;
        break;
    case Json::Type::Uinteger:
        out << arg.value.uinteger;
        break;
    case Json::Type::Null:
        out << "null";
        break;
    default:
        break;
    }
    return out;
}

// Metoda zwraca ilość elementów w obiekcie
const size_t Json::size() const
{
    if (isArray())
        return value.array->size();
    else if (isObject())
        return value.object->size();
    else
        throw std::domain_error("object does not have size");
}

// Metoda sprawdza czy kontener jest pusty
const bool Json::empty() const
{
    return size() == 0;
}

// Metoda opróżnia kontener
void Json::clear()
{
    switch (type)
    {
    case Type::Array:
        value.array->clear();
        break;
    case Type::Object:
        value.object->clear();
        break;
    default:
        throw std::domain_error("invalid type");
    }
}

// Metoda dodaje obiekt do listy
void Json::push_back(const Json& arg)
{
    if (isNull())
    {
        type = Type::Array;
        value.array = new Array();
    }

    if (isArray())
        value.array->push_back(arg);
    else
        throw std::domain_error("type is not array");
}


// Metoda dodaje obiekt do listy
void Json::push_back(Json&& arg)
{
    if (isNull())
    {
        type = Type::Array;
        value.array = new Array();
    }

    if (isArray())
        value.array->push_back(std::move(arg));
    else
        throw std::domain_error("type is not array");
}

// Metoda dodaje obiekt do obiektów
void Json::insert(const std::string& key, const Json& arg)
{
    if (isNull())
    {
        type = Type::Object;
        value.object = new Object();
    }

    if (isObject())
        (*value.object)[key] = arg;
    else
        throw std::domain_error("type is not object");
}

// Metoda dodaje obiekt do obiektów
void Json::insert(const std::string& key, Json&& arg)
{
    if (isNull())
    {
        type = Type::Object;
        value.object = new Object();
    }

    if (isObject())
        (*value.object)[key] = std::move(arg);
    else
        throw std::domain_error("type is not object");
}

// Metoda usuwa obiekt z listy
void Json::erase(const size_t arg)
{
    if (isArray())
        value.array->erase(value.array->begin() + arg);
    else
        throw std::domain_error("type is not object");
}

// Metoda usuwa obiekt z obiektów
void Json::erase(const std::string& arg)
{
    if (isObject())
        value.object->erase(arg);
    else
        throw std::domain_error("type is not object");
}

// Metoda usuwa ostatni obiekt z listy
void Json::pop_back()
{
    if (isArray())
        value.array->pop_back();
    else
        throw std::domain_error("type is not object");
}

// Metoda zwraca obiekt z listy
const Json& Json::at(std::size_t arg)
{
    if (isArray())
        return value.array->at(arg);
    else
        throw std::domain_error("type is not object");
}

// Metoda zwraca obiekt z obiektów
const Json& Json::at(const std::string& arg)
{
    if (isObject())
        return value.object->at(arg);
    else
        throw std::domain_error("type is not object");
}

// Metoda zwraca iterator na początek kontenera
Json::iterator Json::begin() const
{
    switch (type)
    {
    case Type::Array:
        return iterator(value.array->begin());
    case Type::Object:
        return iterator(value.object->begin());
    default:
        throw std::domain_error("invalid type");
    }
}

// Metoda zwraca iterator na koniec kontenera
Json::iterator Json::end() const
{
    switch (type)
    {
    case Type::Array:
        return iterator(value.array->end());
    case Type::Object:
        return iterator(value.object->end());
    default:
        throw std::domain_error("invalid type");
    }
}

// Metoda zwraca stały iterator na początek kontenera
Json::const_iterator Json::cbegin() const
{
    switch (type)
    {
    case Type::Array:
        return const_iterator(value.array->begin());
    case Type::Object:
        return const_iterator(value.object->begin());
    default:
        throw std::domain_error("invalid type");
    }
}

// Metoda zwraca stały iterator na koniec kontenera
Json::const_iterator Json::cend() const
{
    switch (type)
    {
    case Type::Array:
        return const_iterator(value.array->end());
    case Type::Object:
        return const_iterator(value.object->end());
    default:
        throw std::domain_error("invalid type");
    }
}

// Metoda zwraca typ obiektu
const Json::Type Json::getType() const
{
    return type;
}

// Metoda sprawdza czy obiekt jest typu listowego
const bool Json::isArray() const
{
    return type == Json::Type::Array;
}

// Metoda sprawdza czy obiekt jest typu obiektowego
const bool Json::isObject() const
{
    return type == Json::Type::Object;
}

// Metoda sprawdza czy obiekt jest typu Boolowskiego
const bool Json::isBool() const
{
    return type == Json::Type::Boolean;
}

// Metoda sprawdza czy obiekt jest typu pustego
const bool Json::isNull() const
{
    return type == Json::Type::Null;
}

// Metoda sprawdza czy obiekt jest typu tekstowego
const bool Json::isString() const
{
    return type == Json::Type::String;
}

// Metoda sprawdza czy obiekt jest typu zmiennoprzecinkowego
const bool Json::isFloating() const
{
    return type == Json::Type::Floating;
}

// Metoda sprawdza czy obiekt jest typu całkowitego z znakiem
const bool Json::isInteger() const
{
    return type == Json::Type::Integer;
}

// Metoda sprawdza czy obiekt jest typu całkowitego bez znaku
const bool Json::isUinteger() const
{
    return type == Json::Type::Uinteger;
}

// Metoda sprawdza czy obiekt jest typu numerycznego
const bool Json::isNumeric() const
{
    return isFloating() || isInteger() || isUinteger();
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
            throw std::runtime_error(("Couldn't open file: " + str).c_str());
    }

    PropertyTree tree;
    std::istringstream istr(str);
    ReadJson(tree, istr);
    return ToJsonObject(tree);
}

// Metoda serializująca JSON do łańcucha znaków
// lub pliku jeśli podano ścieżkę do pliku
std::string Json::serialize(const std::string& path) const
{
    std::ostringstream oss;
    oss << *this;
    std::string str = oss.str();

    if (!path.empty())
    {
        std::ofstream file(path);
        if (file.good())
            file << str;
        else
            throw std::runtime_error(("Couldn't open file: " + path).c_str());
    }

    return str;
}
