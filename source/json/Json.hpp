#ifndef PATR_JSON_HPP
#define PATR_JSON_HPP

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

// Klasa odpowiadająca za obsługę obiektów JSON
class Json
{
public:
    // Enumerator definiujący typ obiektu
    enum class Type
    {
        Null,
        Array,
        Object,
        String,
        Boolean,
        Integer,
        Uinteger,
        Floating
    };

    // Unia definiująca możliwe wartości obiektu
    union Value
    {
        bool boolean;
        double floating;
        int64_t integer;
        uint64_t uinteger;
        std::string* string;
        std::vector<Json>* array;
        std::map<std::string, Json>* object;

        // Konstruktory nadające wartość unii
        Value() = default;
        Value(std::nullptr_t) { }
        Value(const bool arg) : boolean(arg) { }
        Value(const double arg) : floating(arg) { }
        Value(const int64_t arg) : integer(arg) { }
        Value(const uint64_t arg) : uinteger(arg) { }
        Value(const std::string& arg) : string(new std::string(arg)) { }
        Value(const std::vector<Json>& arg)
            : array(new std::vector<Json>(arg)) { }
        Value(const std::map<std::string, Json>& arg)
            : object(new std::map<std::string, Json>(arg)) { }
    };

    ///////////// Reguła pięciu /////////////

    // Konstruktor domyślny
    Json() = default;

    // Konstruktor kopiujący
    Json(const Json& arg)
        : type(arg.type)
    {
        if (type == Type::Array)
            value = *arg.value.array;
        else if (type == Type::Boolean)
            value = arg.value.boolean;
        else if (type == Type::Floating)
            value = arg.value.floating;
        else if (type == Type::Integer)
            value = arg.value.integer;
        else if (type == Type::Null)
            value = nullptr;
        else if (type == Type::Object)
            value = *arg.value.object;
        else if (type == Type::String)
            value = *arg.value.string;
        else if (type == Type::Uinteger)
            value = arg.value.uinteger;
    }

    // Konstruktor przenoszący
    Json(Json&& arg)
        : type(std::move(arg.type))
        , value(std::move(arg.value))
    {
        arg.type = Type::Null;
        arg.value = { };
    }

    // Operator przypisania
    Json& operator=(Json arg)
    {
        std::swap(type, arg.type);
        std::swap(value, arg.value);
        return *this;
    }

    // Destruktor
    ~Json()
    {
        if (type == Type::Array)
            delete value.array;
        else if (type == Type::Object)
            delete value.object;
        else if (type == Type::String)
            delete value.string;
    }

    /////////////////////////////////////////

    // Konstruktor obiektu pustego
    Json(std::nullptr_t)
        : type(Type::Null)
    {

    }

    // Konstruktor obiektów Boolowskich
    Json(bool arg)
        : type(Type::Boolean)
        , value(arg)
    {

    }

    // Konstruktor obiektów łancuchów znaków
    Json(const std::string& arg)
        : type(Type::String)
        , value(arg)
    {

    }

    // Konstruktor obiektów łancuchów znaków
    template <typename T,
              typename = std::enable_if<
                  std::is_constructible<std::string, T>::value>::type>
    Json(const T& arg)
        : type(Type::String)
        , value(std::string(arg))
    {

    }

    // Konstruktor obiektów zmienno-przecinkowych
    template <typename T,
              typename = std::enable_if<
                  std::is_constructible<double, T>::value &&
                  std::is_floating_point<T>::value>::type>
    Json(const T arg)
        : type(Type::Floating)
        , value(static_cast<double>(arg))
    {

    }

    // Konstruktor obiektów całkowitych ze znakiem
    template <typename T,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, bool>::value &&
            std::is_constructible<int64_t, T>::value, T>::type* = nullptr>
        Json(const T arg)
        : type(Type::Integer)
        , value(static_cast<int64_t>(arg))
    {

    }

    // Konstruktor obiektów całkowitych bez znaku
    template <typename T,
        typename std::enable_if<
            std::is_unsigned<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, bool>::value &&
            std::is_constructible<int64_t, T>::value, T>::type* = nullptr>
        Json(const T arg)
        : type(Type::Uinteger)
        , value(static_cast<uint64_t>(arg))
    {

    }

    // Konstruktor obiektów tablicowych
    Json(const std::vector<Json>& arg)
        : type(Type::Array)
        , value(arg)
    {

    }

    // Konstruktor obiektów tablicowych
    Json(const std::initializer_list<Json>& arg)
        : type(Type::Array)
        , value(std::vector<Json>(arg))
    {

    }

    // Operator zwracający obiekt o podanej nazwie
    Json& operator[](const std::string& arg)
    {
        return *this;
    }

    // Metoda zwraca łańcuch znaków z usuniętymi nadmiarowymi znakami białymi zgodnie z regułami JSON
    static std::string minify(std::string str)
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
    static Json deserialize(std::string str)
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
    std::string serialize(const std::string& path = "")
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

protected:
    Type type;
    Value value;
};

#endif // PATR_JSON_HPP
