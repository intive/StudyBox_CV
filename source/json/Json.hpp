#ifndef PATR_JSON_HPP
#define PATR_JSON_HPP

#include <map>
#include <vector>
#include <limits>
#include <stdexcept>
#include <type_traits>

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
        Value(const char *arg)
            : string(new std::string(arg)) { }
        Value(const std::string& arg)
            : string(new std::string(arg)) { }
        Value(const std::vector<Json>& arg)
            : array(new std::vector<Json>(arg)) { }
        Value(const std::map<std::string, Json>& arg)
            : object(new std::map<std::string, Json>(arg)) { }
    };

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

    // Konstruktor obiektu pustego
    Json(std::nullptr_t);

    // Konstruktor obiektów Boolowskich
    Json(const bool arg);

    // Konstruktor obiektów łańcuchów znaków
    Json(const std::string& arg);

    // Konstruktor obiektów łańcuchów znaków
    Json(const char* arg);

    // Konstruktor obiektów tablicowych
    Json(const std::vector<Json>& arg);

    // Konstruktor obiektów tablicowych
    Json(const std::initializer_list<Json>& arg);

    // Konstruktor obiektów zmienno-przecinkowych
    template <typename T,
        typename std::enable_if<
            std::is_constructible<double, T>::value &&
            std::is_floating_point<T>::value>::type* = nullptr>
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

    // Operator zwracający obiekt o podanej nazwie
    Json& operator[](const char* arg);

    // Operator zwracający obiekt o podanej nazwie
    Json& operator[](const std::string& arg);

    // Operator zwracający obiekt o podanej nazwie
    const Json& operator[](const char* arg) const;

    // Operator zwracający obiekt o podanej nazwie
    const Json& operator[](const std::string& arg) const;

    // Operator zwracający obiekt o podanym indeksie
    template <typename T,
        typename std::enable_if<
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

    // Operator zwracający obiekt o podanym indeksie
    template <typename T,
        typename std::enable_if<
            std::is_integral<T>::value>::type* = nullptr>
    const Json& operator[](const T arg) const
    {
        if (type != Type::Array)
            throw std::domain_error("type is not array");

        return (*value.array)[arg];
    }

    // Operator rzutujący na obiekt Boolowski
    operator bool() const;

    // Operator rzutujący na obiekt łańcucha znaków
    operator std::string&() const;

    // Operator rzutujący na obiekt tablicowy
    operator std::vector<Json>&() const;

    // Operator rzutujący obiekt na typ numeryczny
    template <typename T,
        typename std::enable_if<
            std::is_arithmetic<T>::value &&
            !std::is_same<T, std::string::value_type>::value>::type* = nullptr>
    operator T() const
    {
        switch (type)
        {
        case Type::Floating:
            return numericCast<T>(value.floating);
        case Type::Integer:
            return numericCast<T>(value.integer);
        case Type::Uinteger:
            return numericCast<T>(value.uinteger);
        default:
            throw std::domain_error("object is not number");
        }
    }

    // Operator strumienia wyjścia
    friend std::ostream& operator<<(std::ostream& out, const Json& arg);

    // Metoda zwraca ilość elementów w obiekcie
    size_t size() const;

    // Metoda zwraca łańcuch znaków z usuniętymi nadmiarowymi znakami białymi zgodnie z regułami JSON
    static std::string minify(std::string str);

    // Metoda deserializująca JSON z łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku z rozszerzeniem ".json"
    static Json deserialize(std::string str);

    // Metoda serializująca JSON do łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku
    std::string serialize(const std::string& path = "") const;

protected:

    // Metoda rzutuje wartości numeryczne z kontrolą przepełnienia
    template <typename T, typename U,
        typename std::enable_if<
            std::is_floating_point<T>::value, T>::type* = nullptr>
    const T numericCast(U arg) const
    {
        if (arg > std::numeric_limits<T>::max() || arg < std::numeric_limits<T>::lowest())
            throw std::domain_error("overflow");

        return static_cast<T>(arg);
    }

    // Metoda rzutuje wartości numeryczne z kontrolą przepełnienia
    template <typename T, typename U,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_signed<U>::value &&
            std::is_integral<T>::value>::type* = nullptr>
    const T numericCast(U arg) const
    {
        if (arg < std::numeric_limits<T>::min() || arg > std::numeric_limits<T>::max())
            throw std::domain_error("overflow");

        return static_cast<T>(arg);
    }

    // Metoda rzutuje wartości numeryczne z kontrolą przepełnienia
    template <typename T, typename U,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_unsigned<U>::value &&
            std::is_integral<T>::value, T>::type* = nullptr>
    const T numericCast(U arg) const
    {
        if (arg > (std::make_unsigned<T>::type)std::numeric_limits<T>::max())
            throw std::domain_error("overflow");

        return static_cast<T>(arg);
    }

    // Metoda rzutuje wartości numeryczne z kontrolą przepełnienia
    template <typename T, typename U,
        typename std::enable_if<
            std::is_unsigned<T>::value &&
            std::is_integral<T>::value>::type* = nullptr>
    const T numericCast(U arg) const
    {
        if (arg < 0 || arg > std::numeric_limits<T>::max())
            throw std::domain_error("overflow");

        return static_cast<T>(arg);
    }

    Type type;
    Value value;
};

#endif // PATR_JSON_HPP
