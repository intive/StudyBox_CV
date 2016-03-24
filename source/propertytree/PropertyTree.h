#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <stdexcept>
#include <typeinfo>
#include <sstream>

/// Type trait sprawdza czy typ jest ³añcuchem znaków
template <typename T>
struct is_string
    : public std::integral_constant<bool,
    std::is_same<      char *, typename std::decay<T>::type>::value ||
    std::is_same<const char *, typename std::decay<T>::type>::value
    > {};
template <>
struct is_string<std::string> : std::true_type {};

/// Type trait sprawdza czy typ jest kontenerem.
/**
 * ¯eby podlega³ klasyfikacji, musi zawieraæ metody begin i end
 * oraz zwraca³ const_iterator zdefiniowany jako typedef.
 */
template <typename T>
struct is_container
{
    template <typename U, typename it_t = typename U::const_iterator >
    struct sfinae
    {
        //typedef typename U::const_iterator it_t;
        template < typename V, typename IT, IT(U::*)() const, IT(U::*)() const >
        struct type_ {};

        typedef type_<U, it_t, static_cast<it_t(U::*)() const>(&U::begin), static_cast<it_t(U::*)() const>(&U::end)> type;
    };

    template <typename U> static char test(typename sfinae<U>::type*);
    template <typename U> static long test(...);

    enum { value = (1 == sizeof test<T>(0)) };
};

/// S³u¿y do konwersji PropertyTree na wartoœci typu T.
template<typename T, typename = void>
struct PropertyTreeInputConverter;

/// S³u¿y do konwersji wartoœci typu T na PropertyTree.
template<typename T, typename = void>
struct PropertyTreeOutputConverter;

/// Klasa opisuj¹ca drzewo parametrów.
/**
 * Drzewo zawiera wektor poddrzew (ma potomków) albo wartoœæ zapisan¹ jako std::string (jest liœciem).
 * Zawiera 3 metody:
 *   find() - zwraca poddrzewo odpowiadaj¹ce podanemu kluczowi.
 *   get<T>() - zwraca wartoœæ drzewa jako okreœlony typ.
 *   put() - wstawia wartoœæ do drzewa.
 *
 * Dostêp do elementów drzewa odbywa siê za pomoc¹ etykiet.
 * Etykieta to ci¹g znaków delimitowany znakiem '.', który oddziela
 * poziomy drzewa.
 * Na przyk³ad:
 *   "foo" powoduje wyszukanie poddrzewa o nazwie "foo".
 *   "foo.bar" powoduje wyszukanie poddrzewa o nazwie "foo" a w nim
 *   poddrzewa o nazwie "bar".
 *   Je¿eli drzewo jest typu Array dostêp do poszczególnych elementów uzyskany jest poprzez
 *   znak '.' na koñcu etykiety, np.: "foo.bar." oznacza element tablicy bar z poddrzewa foo.
 *   Dostêp do poszczególnych elementów tablic uzyskiwany jest za pomoc¹ wartoœci przekazywanych
 *   jako kolejne argumenty do funkcji wyszukuj¹cych.
 * Kod:
 *   tree.find("foo.bar.baz.", 2, 3) // spowoduje zwrócenie 3. elementu tablicy "baz" bêd¹cej 2. elementem
 *   tablicy "bar", która z kolei jest elementem drzewa foo, który nie jest tablic¹, zatem indeksowanie
 *   nie ma mocy.
 */
class PropertyTree
{
public:
    typedef std::string KeyType;
    typedef std::string DataType;
    typedef std::vector<std::pair<KeyType, PropertyTree>> Children;
    typedef Children::iterator Iterator;
    typedef Children::const_iterator ConstIterator;
    
    /// Podstawowe rozró¿nialne typy.
    /**
     * Domyœlnym typem jest Object.
     * Za ustawianie typów odpowiedzialny jest u¿ytkownik lub
     * klasa wykonuj¹ca konwersjê do drzewa.
     */
    enum Type
    {
        Null,
        Boolean,
        Value, // wartoœæ liczbowa.
        String,
        Array,
        Object // wartoœæ domyœlna.
    };

    /// Tworzy nowy obiekt z domyœlnym typem wartoœci.
    PropertyTree() : valueType(Type::Object)
    {
    }

    /// Zwraca wartoœæ liœcia lub pusty ³añcuch je¿eli nie jest liœciem.
    const std::string& string() const
    {
        return data;
    }

    std::string& string()
    {
        return data;
    }

    /// Zwraca typ drzewa.
    Type& type()
    {
        return valueType;
    }

    const Type& type() const
    {
        return valueType;
    }

    /// Sprawdza, czy drzewo zawiera potomków.
    /**
     * @return true, je¿eli jest liœciem, false w przeciwnym wypadku.
     */
    bool empty() const
    {
        if (valueType == Type::Array)
            return false;
        return begin() == end();
    }

    /// Zwraca iterator do potomków.
    Iterator begin()
    {
        return children.begin();
    }

    ConstIterator begin() const
    {
        return children.begin();
    }

    Iterator end()
    {
        return children.end();
    }

    ConstIterator end() const
    {
        return children.end();
    }

    /// Grupa metod odpowiedzialnych za pobranie wartoœci w okreœlonym typie.
    /**
     * Zwracaj¹ wartoœci otwrzymane od obiektu Converter dla danego typu.
     * Domyœlnym typem obiektu Converter jest PropertyTreeInputConverter<T>,
     * który zawiera podstawowe specjalizacje oraz posiada mo¿liwoœci rozbudowy przez u¿ytkownika.
     * Dostêp do zagnie¿d¿onych elementów uzyskaæ mo¿na poprzez wykorzystanie etykiet (p. etykiety w opisie klasy).
     */

    /// Zwraca wartoœæ typu std::string, je¿eli jest liœciem.
    /**
     * Metoda wywo³uje Converter::operator()(const std::string&).
     * Je¿eli drzewo nie jest liœciem (albo nie ma wartoœci), rzuca wyj¹tek std::bad_cast.
     */
    template<typename T = DataType, typename Converter = PropertyTreeInputConverter<T>, typename std::enable_if<std::is_same<T, KeyType>::value>::type* = nullptr>
    T get(Converter&& c = PropertyTreeInputConverter<T>()) const
    {
        if (!data.empty())
        {
            return c(data);
        }
        else
        {
            throw std::bad_cast();
        }
    }

    /// Zwraca wartoœæ typu T, gdzie T nie jest std::string.
    /**
     * Metoda wywo³uje Converter::operator()(const PropertyTree&).
     */
    template<typename T = DataType, typename Converter = PropertyTreeInputConverter<T>, typename std::enable_if<!std::is_same<T, KeyType>::value && !is_string<Converter>::value>::type* = nullptr>
    T get(Converter&& c = PropertyTreeInputConverter<T>()) const
    {
        return c(*this);
    }

    /// Grupa metod odpowiedzialnych za zwrócenie poddrzewa zgodnie z zadanymi parametrami.
    /**
     * Wykorzystanie funkcji jest analogiczne do metod get().
     * Dostêp do zagnie¿d¿onych elementów uzyskaæ mo¿na poprzez wykorzystanie etykiet (p. etykiety w opisie klasy).
     */

    /// Zwraca poddrzewo dla podanej etykiety.
    const PropertyTree& find(const KeyType& label) const
    {
        return find(label, 0);
    }

    /// Zwraca poddrzewo dla podanej etykiety, zgodnie z zadanymi indeksami elementów. 
    template<typename... Indices>
    const PropertyTree& find(const KeyType& label, std::size_t index, Indices&&... indices) const
    {
        return findImpl(label, 0, index, std::forward<Indices>(indices)...);
    }

    PropertyTree& find(const KeyType& label, std::size_t index = 0)
    {
        return const_cast<PropertyTree&>(const_cast<const PropertyTree&>(*this).find(label, index));
    }

    /// Grupa metod odpowiedzialnych za dodanie wartoœci do drzewa.
    /**
    * Dodaj¹ wartoœci otwrzymane od obiektu Converter dla danego typu.
    * Domyœlnym typem obiektu Converter jest PropertyTreeOutputConverter<T>,
    * który zawiera podstawowe specjalizacje oraz posiada mo¿liwoœci rozbudowy przez u¿ytkownika.
    * Converter jest obiektem spe³niaj¹cym std::is_function<U> przyjmuj¹cym wartoœæ T i zwracaj¹cym
    * drzewo PropertyTree.
    */

    /// Dodaje poddrzewo do drzewa.
    /**
     * Operacja odbywa siê bez dodatkowych obiektów konwertuj¹cych.
     */
    PropertyTree& put(const KeyType& key, const PropertyTree& tree)
    {
        children.push_back(std::make_pair(key, tree));
        return *this;
    }

    /// Dodaje wartoœæ typu std::string do drzewa.
    /**
     * Tworzy now¹ parê "opis":"wartoœæ".
     */
    PropertyTree& put(const KeyType& key, const DataType& value)
    {
        PropertyTree tree;
        tree.put(value);
        children.push_back(std::make_pair(key, tree));
        return *this;
    }

    /// Dodaje wartoœæ do drzewa.
    /**
     * Wartoœæ podlega konwersji poprzez obiekt typu Converter.
     */
    template<typename T, typename Converter = PropertyTreeOutputConverter<T>>
    PropertyTree& put(const KeyType& key, const T& value, Converter&& c = PropertyTreeOutputConverter<T>())
    {
        children.push_back(std::make_pair(key, c(value)));
        return *this;
    }

    /// Dodaje poddrzewo do drzewa z pust¹ nazw¹.
    PropertyTree& put(const PropertyTree& tree)
    {
        put("", tree);
        return *this;
    }

    /// Podmienia drzewo na takie wynikaj¹ca z konwersji przez obiekt Converter.
    /**
     * Istotna jest ró¿nica miêdzy put(PropertyTree) a put(T), poniewa¿ to pierwsze dodaje poddrzewo,
     * a drugie podmienia obecne drzewo.
     */
    template<typename T, typename Converter = PropertyTreeOutputConverter<T>, typename std::enable_if<std::is_function<Converter>::value>::type>
    PropertyTree& put(const T& data, Converter&& c = PropertyTreeOutputConverter<T>())
    {
        //Converter c;
        *this = std::move(c(data));
        return *this;
    }

    /// Ustawia wartoœæ drzewa na dany ³añcuch znaków.
    /**
     * Istotna jest ró¿nica miêdzy innymi jednoargumentowyni funkcjami put() ze wzglêdu na to,
     * ¿e ta powoduje podmianê charakterystyczn¹ dla liœci.
     * Ró¿nice te wynikaj¹ z trzymania w jednym wêŸle danych o potomkach i danych w³asnych charakterystycznych
     * dla liœci.
     */
    PropertyTree& put(const std::string& data)
    {
        this->data = data;
        return *this;
    }

    /// Ustawia wartoœæ drzewa na dany ³añcuch znaków.
    /**
     * p. put(std::string).
     * Przeci¹¿enie potrzebne aby unikn¹æ niejednoznacznoœci w kompilacji.
     */
    PropertyTree& put(const char* data)
    {
        this->data = data;
        return *this;
    }

private:

    /// Wrapper do metody wyszukuj¹cej dla ignorowanych indeksów.
    const PropertyTree& findImpl(const KeyType& label) const
    {
        return findImpl(label, 0);
    }

    /// Implementacja metody wyszukuj¹cej.
    /**
     * W przypadku wyjœcia poza zasiêg rzuca wyj¹tek std::out_of_range.
     */
    template<typename... Indices>
    const PropertyTree& findImpl(const KeyType& label, std::size_t index, Indices&&... indices) const
    {
        auto dot = label.find_first_of('.');
        if (dot != KeyType::npos)
        {
            std::string begin(label.begin(), label.begin() + dot);
            std::string end(label.begin() + dot + 1, label.end());
            auto& tree = findImpl(begin, index);
            if (tree.type() == Type::Array)
            {
                end = '.' + end;
                return tree.findImpl(std::move(end), std::forward<Indices>(indices)...);
            }
            if (label.begin() + dot + 1 == label.end())
                return tree;
            return tree.findImpl(std::move(end), index, std::forward<Indices>(indices)...);
        }

        std::size_t currentIndex = 0;

        for (const auto& pair : children)
        {
            if (pair.first == label)
            {
                if (type() == Type::Array && currentIndex++ == index)
                    return pair.second;
                else if (type() != Type::Array)
                    return pair.second;
            }
        }
        throw std::out_of_range("index out of range");
    }

    Type valueType;
    DataType data;
    Children children;
};

/// Type trait do sprawdzenia czy klasa zawiera metodê T::serialize.
template<typename T>
struct is_serializable
{
    typedef char one;
    typedef long two;

    template <typename C> static one test(decltype(&C::serialize));
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

/// Type trait do sprawdzenia czy klasa zawiera metodê T::deserialize.
template<typename T>
struct is_deserializable
{
    typedef char one;
    typedef long two;

    template <typename C> static one test(decltype(&C::deserialize));
    template <typename C> static two test(...);

public:
    enum { value = sizeof(test<T>(0)) == sizeof(char) };
};

/// Ogólna klasa do konwersji liœci drzew na typ T.
/**
 * Do konwersji wykorzystuje std::istringstream.
 * Aby typ by³ konwertowalny przez t¹ klasê (bez specjalizacji),
 * musi przeci¹¿aæ operator >>(std::istream&, T).
 */
template<typename T, typename>
struct PropertyTreeInputConverter
{
    T operator ()(const std::string& data)
    {
        T t;
        std::istringstream ss(data);
        ss.setf(std::ios_base::boolalpha);
        ss >> t;
        if (ss.fail())
            throw std::bad_cast();
        return t;
    }

    T operator ()(const PropertyTree& data)
    {
        return operator()(data.string());
    }
};

/// Specjalizacja wykorzystuj¹ca metodê deserialize() typu T.
/**
 * Klasy w pe³ni specjalizuj¹ce klasê PropertyTreeInputConverter
 * maj¹ pierwszeñstwo przed t¹ specjalizacj¹.
 */
template<typename T>
struct PropertyTreeInputConverter<T, typename std::enable_if<is_deserializable<T>::value>::type>
{
    T operator ()(const PropertyTree& data)
    {
        T t;
        t.deserialize(data);
        return t;
    }
};

/// Specjalizacja dla ³añcuchów znakowych.
template<>
struct PropertyTreeInputConverter<std::string>
{
    std::string operator ()(const PropertyTree& data)
    {
        std::string retval;
        for (auto& val : data)
        {
            retval += val.second.get<std::string>();
        }

        return retval;
    }

    std::string operator ()(const std::string& data)
    {
        return data;
    }
};

/// Ogólna klasa do konwersji typów typ PropertyTree (liœæ).
/**
* Do konwersji wykorzystuje std::ostringstream.
* Aby typ by³ konwertowalny przez t¹ klasê (bez specjalizacji),
* musi przeci¹¿aæ operator <<(std::ostream&, T).
*/
template<typename T, typename>
struct PropertyTreeOutputConverter
{
    PropertyTree operator ()(const T& data)
    {
        std::ostringstream ss;
        ss << data;
        PropertyTree tree;
        tree.put(ss.str());
        tree.type() = PropertyTree::Type::Value;
        return tree;
    }
};

/// Specjalizacja wykorzystuj¹ca metodê serialize() typu T.
/**
* Klasy w pe³ni specjalizuj¹ce klasê PropertyTreeOutputConverter
* maj¹ pierwszeñstwo przed t¹ specjalizacj¹.
*/
template<typename T>
struct PropertyTreeOutputConverter<T, typename std::enable_if<is_serializable<T>::value>::type>
{
    PropertyTree operator ()(const T& data)
    {
        return data.serialize();
    }
};

/// Specjalizacja dla std::string oraz pochodnych.
template<typename T>
struct PropertyTreeOutputConverter<T, typename std::enable_if<is_string<T>::value>::type>
{
    PropertyTree operator ()(const T& data)
    {
        PropertyTree tree;
        tree.type() = PropertyTree::Type::String;
        tree.put(data);
        return tree;
    }
};

/// Specjalizacja dla typu bool.
/**
 * Ustawia typ wartoœci drzewa na PropertyTree::Type::Boolean.
 */
template<>
struct PropertyTreeOutputConverter<bool>
{
    PropertyTree operator ()(bool data)
    {
        std::ostringstream ss;
        ss << std::boolalpha << data;
        PropertyTree tree;
        tree.type() = PropertyTree::Type::Boolean;
        tree.put(ss.str());
        return tree;
    }
};

/// Specjalizacja dla typu nullptr_t.
/**
* Ustawia typ wartoœci drzewa na PropertyTree::Type::Null.
*/
template<typename T>
struct PropertyTreeOutputConverter<T, typename std::enable_if<std::is_null_pointer<T>::value>::type>
{
    PropertyTree operator ()(const T& data)
    {
        PropertyTree tree;
        tree.type() = PropertyTree::Type::Null;
        tree.put("null");
        return tree;
    }
};

/// Specjalizacja dla kontenerów.
/**
 * p. definicja type trait is_container.
 * Konstruuje poddrzewo z wartoœciami kontenera.
 * Ustawia typ na PropertyTree::Type::Array.
 */
template<typename T>
struct PropertyTreeOutputConverter<T, typename std::enable_if<is_container<T>::value>::type>
{
    PropertyTree operator ()(const T& data)
    {
        PropertyTree tree;
        tree.type() = PropertyTree::Type::Array;

        for (auto& val : data)
        {
            tree.put("", val);
        }

        return tree;
    }
};

inline bool operator ==(const PropertyTree& lhs, const PropertyTree& rhs)
{
    if (lhs.empty())
        return lhs.get<std::string>() == rhs.get<std::string>();

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

