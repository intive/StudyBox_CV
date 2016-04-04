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
    // Deklaracje wprzód
    template <bool> class Iterator;

    // Aliasy
    typedef bool Boolean;
    typedef double Floating;
    typedef int64_t Integer;
    typedef uint64_t Uinteger;
    typedef std::string String;
    typedef std::nullptr_t Null;
    typedef std::vector<Json> Array;
    typedef std::map<std::string, Json> Object;
    typedef class Iterator<false> iterator;
    typedef class Iterator<true> const_iterator;

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
        Boolean boolean;
        Floating floating;
        Integer integer;
        Uinteger uinteger;
        String* string;
        Array* array;
        Object* object;

        // Konstruktory nadające wartość unii
        Value() = default;
        Value(Null) { }
        Value(const Boolean arg) : boolean(arg) { }
        Value(const Floating arg) : floating(arg) { }
        Value(const Integer arg) : integer(arg) { }
        Value(const Uinteger arg) : uinteger(arg) { }
        Value(const char *arg)
            : string(new String(arg)) { }
        Value(const String& arg)
            : string(new String(arg)) { }
        Value(const Array& arg)
            : array(new Array(arg)) { }
        Value(const Object& arg)
            : object(new Object(arg)) { }
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
    Json(Null);

    // Konstruktor obiektów Boolowskich
    Json(const bool arg);

    // Konstruktor obiektów łańcuchów znaków
    Json(const std::string& arg);

    // Konstruktor obiektów łańcuchów znaków
    Json(const char* arg);

    // Konstruktor obiektów tablicowych
    Json(const Array& arg);

    // Konstruktor obiektów typu obiektowego
    Json(const Object& arg);

    // Konstruktor obiektów tablicowych
    Json(const std::initializer_list<Json>& arg);

    // Konstruktor obiektów zmienno-przecinkowych
    template <typename T,
        typename std::enable_if<
            std::is_constructible<Floating, T>::value &&
            std::is_floating_point<T>::value>::type* = nullptr>
    Json(const T arg)
        : type(Type::Floating)
        , value(static_cast<Floating>(arg))
    {

    }

    // Konstruktor obiektów całkowitych ze znakiem
    template <typename T,
        typename std::enable_if<
            std::is_signed<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, Boolean>::value &&
            std::is_constructible<Integer, T>::value, T>::type* = nullptr>
    Json(const T arg)
        : type(Type::Integer)
        , value(static_cast<Integer>(arg))
    {

    }

    // Konstruktor obiektów całkowitych bez znaku
    template <typename T,
        typename std::enable_if<
            std::is_unsigned<T>::value &&
            std::is_integral<T>::value &&
            !std::is_same<T, Boolean>::value &&
            std::is_constructible<Uinteger, T>::value, T>::type* = nullptr>
    Json(const T arg)
        : type(Type::Uinteger)
        , value(static_cast<Uinteger>(arg))
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
        if (isNull())
        {
            type = Type::Array;
            value.array = new Array();
        }

        if (!isArray())
            throw std::domain_error("type is not array");

        return (*value.array)[arg];
    }

    // Operator zwracający obiekt o podanym indeksie
    template <typename T,
        typename std::enable_if<
            std::is_integral<T>::value>::type* = nullptr>
    const Json& operator[](const T arg) const
    {
        if (!isArray())
            throw std::domain_error("type is not array");

        return (*value.array)[arg];
    }

    // Operator rzutujący
    template <typename T,
        typename std::enable_if<
            !std::is_pointer<T>::value &&
            !std::is_same<T, char>::value &&
            !std::is_same<T, std::initializer_list<char>>::value>::type* = nullptr>
    operator T() const
    {
        return get(static_cast<T*>(nullptr));
    }

    // Operator strumienia wyjścia
    friend std::ostream& operator<<(std::ostream& out, const Json& arg);

    // Metoda zwraca ilość elementów w obiekcie
    const size_t size() const;

    // Metoda sprawdza czy kontener jest pusty
    const bool empty() const;

    // Metoda opróżnia kontener
    void clear();

    // Metoda dodaje obiekt do listy
    void push_back(const Json& arg);

    // Metoda dodaje obiekt do listy
    void push_back(Json&& arg);

    // Metoda dodaje obiekt do obiektów
    void insert(const std::string& key, const Json& arg);

    // Metoda dodaje obiekt do obiektów
    void insert(const std::string& key, Json&& arg);

    // Metoda usuwa obiekt z listy
    void erase(const size_t arg);

    // Metoda usuwa obiekt z obiektów
    void erase(const std::string& arg);

    // Metoda usuwa ostatni obiekt z listy
    void pop_back();

    // Metoda zwraca obiekt z listy
    const Json& at(std::size_t arg);

    // Metoda zwraca obiekt z obiektów
    const Json& at(const std::string& arg);

    // Metoda zwraca iterator na początek kontenera
    iterator begin() const;

    // Metoda zwraca iterator na koniec kontenera
    iterator end() const;

    // Metoda zwraca stały iterator na początek kontenera
    const_iterator cbegin() const;

    // Metoda zwraca stały iterator na koniec kontenera
    const_iterator cend() const;

    // Metoda zwraca typ obiektu
    const Type getType() const;

    // Metoda sprawdza czy obiekt jest typu listowego
    const bool isArray() const;

    // Metoda sprawdza czy obiekt jest typu obiektowego
    const bool isObject() const;

    // Metoda sprawdza czy obiekt jest typu Boolowskiego
    const bool isBool() const;

    // Metoda sprawdza czy obiekt jest typu pustego
    const bool isNull() const;

    // Metoda sprawdza czy obiekt jest typu tekstowego
    const bool isString() const;

    // Metoda sprawdza czy obiekt jest typu zmiennoprzecinkowego
    const bool isFloating() const;

    // Metoda sprawdza czy obiekt jest typu całkowitego z znakiem
    const bool isInteger() const;

    // Metoda sprawdza czy obiekt jest typu całkowitego bez znaku
    const bool isUinteger() const;

    // Metoda sprawdza czy obiekt jest typu numerycznego
    const bool isNumeric() const;

    // Metoda zwraca łańcuch znaków z usuniętymi nadmiarowymi znakami białymi zgodnie z regułami JSON
    static std::string minify(std::string str);

    // Metoda deserializująca JSON z łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku z rozszerzeniem ".json"
    static Json deserialize(std::string str);

    // Metoda serializująca JSON do łańcucha znaków
    // lub pliku jeśli podano ścieżkę do pliku
    std::string serialize(const std::string& path = "") const;

protected:
    Type type;
    Value value;

public:
    // Klasa definiująca (stały) iterator
    template <bool isConst = false>
    class Iterator
    {
    public:
        friend class Json;
        friend class Iterator<true>;

        typedef typename std::conditional<isConst, const Json*, Json*>::type Pointer;
        typedef typename std::conditional<isConst, const Json&, Json&>::type Reference;

        // Konstruktor domyślny
        Iterator()
            : type(Type::Null)
        {

        }

        // Destruktor
        ~Iterator()
        {
            switch (type)
            {
            case Type::Array:
                delete iter.array;
                break;
            case Type::Object:
                delete iter.object;
                break;
            default:
                break;
            }
        }

    protected:
        // Konstruktor z iteratora listy
        Iterator(const Array::iterator& arg)
            : type(Type::Array)
            , iter(arg)
        {

        }

        // Konstruktor z iteratora obiektów
        Iterator(const Object::iterator& arg)
            : type(Type::Object)
            , iter(arg)
        {

        }

    public:
        // Konstruktor kopiujący
        Iterator(const Iterator& arg)
            : type(arg.type)
        {
            switch (type)
            {
            case Type::Array:
                iter = *arg.iter.array;
                break;
            case Type::Object:
                iter = *arg.iter.object;
                break;
            default:
                break;
            }
        }

        // Operator przypisania kopiującego
        Reference operator=(Iterator arg)
        {
            std::swap(type, arg.type);
            std::swap(iter, arg.iter);
            return *this;
        }

        // Operator dereferencji gwiazdka
        Reference operator*() const
        {
            switch (type)
            {
            case Type::Array:
                return **iter.array;
            case Type::Object:
                return (*iter.object)->second;
            default:
                throw std::domain_error("invalid type");
            }
        }

        // Operator dereferencji strzałka
        Pointer operator->() const
        {
            switch (type)
            {
            case Type::Array:
                return &**iter.array;
            case Type::Object:
                return &(*iter.object)->second;
            default:
                throw std::domain_error("invalid type");
            }
        }

        // Operator pre-inkrementacji
        Iterator& operator++()
        {
            switch (type)
            {
            case Type::Array:
                ++(*iter.array);
                break;
            case Type::Object:
                ++(*iter.object);
                break;
            default:
                throw std::domain_error("invalid type");
            }
            return *this;
        }

        // Operator post-inkrementacji
        Iterator operator++(int)
        {
            auto result = *this;
            ++(*this);
            return result;
        }

        // Operator pre-dekrementacji
        Iterator& operator--()
        {
            switch (type)
            {
            case Type::Array:
                --(*iter.array);
                break;
            case Type::Object:
                --(*iter.object);
                break;
            default:
                throw std::domain_error("invalid type");
            }
            return *this;
        }

        // Operator post-dekrementacji
        Iterator operator--(int)
        {
            auto result = *this;
            --(*this);
            return result;
        }

        // Operator równości
        bool operator==(const Iterator& arg) const
        {
            if (type != arg.type)
                throw std::domain_error("invalid type");

            switch (type)
            {
            case Type::Array:
                return *iter.array == *arg.iter.array;
            case Type::Object:
                return *iter.object == *arg.iter.object;
            default:
                throw std::domain_error("invalid type");
            }
        }

        // Operator nierówności
        bool operator!=(const Iterator& arg) const
        {
            return !operator==(arg);
        }

        // Operator mniejszości
        bool operator<(const Iterator& arg) const
        {
            if (type != arg.type)
                throw std::domain_error("invalid type");

            switch (type)
            {
            case Type::Array:
                return *iter.array < *arg.iter.array;
            default:
                throw std::domain_error("invalid type");
            }
        }

        // Operator mniejszości lub równości
        bool operator<=(const Iterator& arg) const
        {
            return !(arg < *this);
        }

        // Operator większości
        bool operator>(const Iterator& arg) const
        {
            return !(*this <= arg);
        }

        // Operator większości lub równości
        bool operator>=(const Iterator& arg) const
        {
            return !(*this < arg);
        }

        // Operator przypisania z dodawaniem
        Iterator& operator+=(std::ptrdiff_t arg)
        {
            if (isArray())
                (*iter.array) += arg;
            else
                throw std::domain_error("invalid type");
            return *this;
        }

        // Operator przypisania z odejmowaniem
        Iterator& operator-=(std::ptrdiff_t arg)
        {
            return operator+=(-arg);
        }

        // Operator dodawania
        Iterator operator+(std::ptrdiff_t arg)
        {
            auto result = *this;
            result += arg;
            return result;
        }

        // Operator odejmowania
        Iterator operator-(std::ptrdiff_t arg)
        {
            auto result = *this;
            result -= arg;
            return result;
        }

        // Operator odejmowania iteratorów
        std::ptrdiff_t operator-(const Iterator& arg) const
        {
            if (isArray())
                return (*iter.array) - (*arg.iter.array);
            else
                throw std::domain_error("invalid type");
        }

        // Operator indeksowania
        Reference operator[](std::ptrdiff_t arg) const
        {
            if (isArray())
                return *(*iter.array + arg);
            else
                throw std::domain_error("invalid type");
        }

        // Metoda zwracająca klucz obiektu
        const std::string key() const
        {
            if (isObject())
                return (*iter.object)->first;
            else
                throw std::domain_error("invalid type");
        }

        //Metoda zwracająca wartość obiektu
        Reference value() const
        {
            return *(*this);
        }

        // Metoda sprawdza czy iterator wskazuje na obiekt
        const bool isObject() const
        {
            return type == Type::Object;
        }

        // Metoda sprawdza czy iterator wskazuje na listę
        const bool isArray() const
        {
            return type == Type::Array;
        }

        // Metoda sprawdza czy iterator wskazuje na nic
        const bool isNull() const
        {
            return type == Type::Null;
        }

    protected:
        // Unia na iterator bazowy
        union Iter
        {
            Array::iterator* array;
            Object::iterator* object;

            Iter() {}
            Iter(const Array::iterator& arg)
                : array(new Array::iterator(arg)) { }
            Iter(const Object::iterator& arg)
                : object(new Object::iterator(arg)) { }
        };

        // Enumerator określający typ iteratora
        enum class Type
        {
            Array,
            Object,
            Null
        };

        Iter iter;
        Type type;
    };

protected:
    // Metoda rzutuje na obiekt Boolowski
    Boolean get(Boolean*) const;

    // Metoda rzutuje na obiekt łańcucha znaków
    String get(String*) const;

    // Metoda rzutuje na obiekt tablicowy
    Array get(Array*) const;

    // Metoda rzutuje na kontener obiektów
    Object get(Object*) const;

    // Metoda rzutuje obiekt na typ numeryczny
    template <typename T,
        typename std::enable_if<
            std::is_arithmetic<T>::value>::type* = nullptr>
    T get(T*) const
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
        if (arg > (typename std::make_unsigned<T>::type)std::numeric_limits<T>::max())
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
};

#endif // PATR_JSON_HPP
