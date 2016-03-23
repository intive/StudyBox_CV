#ifndef PATRCV_JSON_H
#define PATRCV_JSON_H

#include <map>
#include <vector>


namespace Json
{
    class Value;

    /* Enum typów, które może przechowywać obiekt Value
     *
     * Int    - wartość numeryczną typu całkowitego
     * Double - wartość numeryczną typu zmiennoprzecinkowego
     * String - ciąg znaków
     * Object - zbiór par key : value, gdzie: key - string, value - dowolny inny typ Json
     * Array  - uporządkowany ciąg wartości rozdzielonych przecinkami
     * Bool   - true lub false
     * Null   - null
     */
    enum class Type
    {
        Int,
        Double,
        String,
        Object,
        Array,
        Bool,
        Null
    };

    
    /* Klasa reprezentująca obiekt Json - zbiór par string : value
     
      Przykład:
      { "k1" : 123, "k2" : "abc",
        "k3" : [ 1, 2, 3 ]
        "k4" : { "k5" : null } }
     */
    class Object
    {
        std::map<std::string, Value> container;

    public:
        // Domyślny konstruktor
        Object() = default;


        // Konstruktor kopiujący
        Object(const Object& o);


        // Operator przypisania
        Object& operator=(const Object& o);


        // Konstruktor umożliwiający wykorzystanie listy inicjalizującej
        Object(const std::initializer_list<std::pair<std::string, Value>>& list);


        // Zwraca string obiektu Json
        std::string serialize();

        // Zapisuje string obiektu Json do pliku
        void serializeToFile(const std::string& filename);


        /* Dodaje parę do obiektu.
           Zwraca obiekt na cześć, którego została wywołana metoda
           pozwalając na łączenie wywołań tej metody, np.
           auto obj = Json::Object().Add("k1", "v1").Add("k2", 2);
         */
        Object& Add(const std::string& key, const Value& value);


        // Dodaje listę par <std::string, Json::Value>
        Object& Add(const std::initializer_list<std::pair<std::string, Value>>& list);


        /* Wrapper wokół std::map<std::string, Json::Value>.begin()
           Zwraca iterator wskazujący na pierwszą parę wewnętrznego kontenera klasy.
           Umożliwia iterację po kontenerze.
         */
        std::map<std::string, Value>::iterator begin() { return container.begin(); }


        /* Wrapper wokół std::map<std::string, Json::Value>.end()
           Zwraca iterator wskazujący na miejsce za ostatnią parą wewnętrznego kontenera klasy.
           Umożliwia iterację po kontenerze.
         */
        std::map<std::string, Value>::iterator end() { return container.end(); }


        /* Wrapper wokół std::map<std::string, Json::Value>.find(key)
           Zwraca iterator wskazujący na: poszukiwaną parę (w przypadku sukcesu)
           lub miejsca za ostatnią parą (niepowodzenie).
         */
        std::map<std::string, Value>::iterator find(const std::string& key) { return container.find(key); }


        /* Wrapper wokół std::map<std::string, Json::Value>.at(key)
           W przypadku istnienia danego klucza zwraca jego wartość.
           W przeciwnym wypadku rzucony jest wyjątek std::out_of_range.
         */
        Value& at(const std::string& key) { return container.at(key); }


        /* Setter dla par<std::string, Json::Value>
           jsonObject["key"] = 123;
         */
        Value& operator[](const std::string& index) { return container[index]; }


        /* Getter dla par<std::string, Json::Value>
           Json::Value val = jsonObject["key"];
         */
        const Value& operator[](const std::string& index) const;


        // Zwraca ilość par w obiekcie
        unsigned long size() { return container.size(); }
    };



    // Reprezentuje tablicę - uporządkowany zbiór wartości oddzielonych przecinkami
    class Array
    {
        std::vector<Value> container;

    public:
        // Domyślny konstruktor
        Array() = default;


        // Konstruktor kopiujący
        Array(const Array& a);


        // Operator przypisania
        Array& operator=(const Array& a);


        // Konstruktor umożliwiający wykorzystanie listy inicjalizującej
        Array(const std::initializer_list<Value>& list);


        /* Zwraca string tablicy Json
           Przykład:
                [ 123, 456, "asd", true, null ]
         */
        std::string serialize();
        void serializeToFile(const std::string& filename);


        /* Pozwala na dodanie elementu typu Json::Value do tablicy.
           Zwraca obiekt na cześć, którego została wywołana metoda
           pozwalając na łączenie wywołań tej metody, np.
                auto arr = Json::Array().Add("v1").Add(2);
         */
        Array& Add(const Value& val);


        /* Pozwala na dodanie listych elementu typu Json::Value do tablicy.
           Przykład użycia:
                JsonArray jsonArr({ 1, true, "abc" });
         */
        Array& Add(const std::initializer_list<Value>& vals);


        /* Wrapper wokół std::vector<Json::Value>.begin()
           Zwraca iterator wskazujący na pierwszy element wewnętrznego kontenera klasy.
           Umożliwia iterację po kontenerze.
         */
        std::vector<Value>::iterator begin() { return container.begin(); }


        /* Wrapper wokół std::vector<Json::Value>.end()
           Zwraca iterator wskazujący na miejsce po ostatnim elemencie wewnętrznego kontenera klasy.
           Umożliwia iterację po kontenerze.
         */
        std::vector<Value>::iterator end() { return container.end(); }


        // Zwraca ilość elementów w tablicy
        unsigned long size() { return container.size(); }
    };

    class Value
    {
        union JsonUnion
        {
            bool         jsonBool;
            int          jsonInt;
            double       jsonDouble;
            Array*       jsonArray;
            Object*      jsonObject;
            std::string* jsonString;

            JsonUnion() { };
            JsonUnion(bool b)               : jsonBool(b) { }
            JsonUnion(int i)                : jsonInt(i) { }
            JsonUnion(double d)             : jsonDouble(d) { }
            JsonUnion(float f)              : jsonDouble(f) { }
            JsonUnion(const Array& a)       : jsonArray(new Array(a)) { }
            JsonUnion(const Object& o)      : jsonObject(new Object(o)) { }
            JsonUnion(const std::string& s) : jsonString(new std::string(s)) { }

            ~JsonUnion() {}
        };

        // Przechowuje typ aktualnie przechowywanego obiektu Json.
        Type type;

        JsonUnion value;        

    public:
        // Konstruktor kopiujący
        Value(const Value& v);

        Value& operator=(const Value& v);

        // Domyślny konstruktor tworzy obiekt przechowujący null.
        Value() = default;

        // Konstruktor tworzy obiekt przechowujący null.
        Value(std::nullptr_t) : type(Type::Null) { }


        // Konstruktor tworzy obiekt przechowujący wartość logiczną: true lub false.
        Value(bool b) : value(b), type(Type::Bool) { }


        // Konstruktor tworzy obiekt przechowujący liczbę całkowitą.
        Value(int i) : value(i), type(Type::Int) { }


        // Konstruktor tworzy obiekt przechowujący liczbę zmiennoprzecinkową.
        Value(double d) : value(d), type(Type::Double) { }


        // Konstruktor tworzy obiekt przechowujący liczbę.
        Value(float f) : value(f), type(Type::Double) { }


        // Konstruktor tworzy obiekt przechowujący tablicę Json.
        Value(const Array& ja) : value(ja), type(Type::Array) { }


        // Konstruktor tworzy obiekt przechowujący obiekt Json.
        Value(const Object& jo) : value(jo), type(Type::Object) { }


        // Konstruktor tworzy obiekt przechowujący ciąg znaków Json.
        Value(const std::string& s) : value(s), type(Type::String) { }


        // Konstruktor tworzy obiekt przechowujący ciąg znaków Json.
        Value(const char* chptr) : value(std::string(chptr)), type(Type::String) { }


        // Konstruktor tworzy obiekt przechowujący ciąg znaków Json.
        Value(char ch) : value(std::string(1, ch)), type(Type::String) { }


        // Destruktor
        ~Value();


        // Zwraca reprezentację przechowywanej wartości w postaci ciągu znaków.
        std::string serialize() const;


        // Zwraca Json::Type aktualnie przechowywanego elementu.
        Type getType() const;


        // Jeśli przechowywany jest obiekt zgodny z żądaniem, zwraca jego wartość. W przeciwnym razie rzuca std::runtime_error.
        bool        getBool() const;
        int         getInt() const;
        double      getDouble() const;
        Array       getArray() const;
        Object      getObject() const;
        std::string getString() const;

        // Jeśli przechowywany jest obiekt zgodny z żądaniem, zwraca jego wartość. W przeciwnym razie zwraca podany obiekt.
        bool        getBool(bool def) const;
        int         getInt(int def) const;
        double      getDouble(double def) const;
        Array       getArray(Array def) const;
        Object      getObject(Object def) const;
        std::string getString(std::string def) const;

    };
}
#endif //PATRCV_JSON_H
