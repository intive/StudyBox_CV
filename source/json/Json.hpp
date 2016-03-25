#ifndef PATR_JSON_HPP
#define PATR_JSON_HPP

#include <map>
#include <vector>

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
        Value(const std::string& arg)
            : string(new std::string(arg)) { }
        Value(const std::vector<Json>& arg)
            : array(new std::vector<Json>(arg)) { }
        Value(const std::map<std::string, Json>& arg)
            : object(new std::map<std::string, Json>(arg)) { }
    };

    ///////////// Reguła pięciu /////////////

    // Konstruktor domyślny
    Json();

    // Konstruktor kopiujący
    Json(const Json& arg);

    // Konstruktor przenoszący
    Json(Json&& arg);

    // Operator przypisania
    Json& operator=(Json arg);

    // Destruktor
    ~Json();

    /////////////////////////////////////////

    // Konstruktor obiektu pustego
    Json(std::nullptr_t);

    // Konstruktor obiektów Boolowskich
    Json(bool arg);

    // Konstruktor obiektów łańcuchów znaków
    Json(const std::string& arg);

    // Konstruktor obiektów łańcuchów znaków
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
            std::is_constructible<uint64_t, T>::value, T>::type* = nullptr>
    Json(const T arg)
        : type(Type::Uinteger)
        , value(static_cast<uint64_t>(arg))
    {

    }

    // Konstruktor obiektów tablicowych
    Json(const std::vector<Json>& arg);

    // Konstruktor obiektów tablicowych
    Json(const std::initializer_list<Json>& arg);

    // Operator zwracający obiekt o podanym indeksie
    template <typename T, typename std::enable_if<
        std::is_integral<T>::value>::type* = nullptr>
    Json& operator[](const T arg)
    {
        if (type == Type::Null)
        {
            type = Type::Array;
            value.array = new std::vector<Json>();
        }

        if (type != Type::Array)
            throw std::domain_error("type is not array");
        return (*value.array)[arg];
    }

    // Operator zwracający obiekt o podanej nazwie
    Json& operator[](const char* arg);

    // Operator zwracający obiekt o podanej nazwie
    Json& operator[](const std::string& arg);

    // Operator rzutujący na obiekt Boolowski
    /*explicit*/ operator bool();

    // Operator rzutujący na obiekt łańcucha znaków
    operator std::string();

    // Operator rzutujący na obiekt zmienno-przecinkowy
    template <typename T,
        typename = std::enable_if<
            std::is_constructible<T, double>::value &&
            std::is_floating_point<T>::value>::type>
    operator T()
    {
        if (type != Type::Floating)
            throw std::domain_error("type is not floating");
        return static_cast<T>(value.floating);
    }

    // Operator rzutujący na obiekt całkowity ze znakiem
    template <typename T,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, bool>::value &&
            !std::is_same<T, std::string::value_type>::value &&
            std::is_constructible<T, int64_t>::value, T>::type* = nullptr>
    operator T()
    {
        if (type != Type::Integer)
            throw std::domain_error("type is not integer");
        return static_cast<T>(value.integer);
    }

    // Operator rzutujący na obiekt całkowity bez znaku
    template <typename T,
        typename std::enable_if<
            std::is_unsigned<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, bool>::value &&
            std::is_constructible<T, uint64_t>::value, T>::type* = nullptr>
    operator T()
    {
        if (type != Type::Uinteger)
            throw std::domain_error("type is not uinteger");
        return static_cast<T>(value.uinteger);
    }

    // Operator rzutujący na obiekt tablicowy
    operator std::vector<Json>();

    // Metoda zwraca łańcuch znaków z usuniętymi nadmiarowymi znakami białymi zgodnie z regułami JSON
    static std::string minify(std::string str);

    // Metoda deserializująca JSON z łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku z rozszerzeniem ".json"
    static Json deserialize(std::string str);

    // Metoda serializująca JSON do łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku
    std::string serialize(const std::string& path = "");

protected:
    Type type;
    Value value;
};

#endif // PATR_JSON_HPP
