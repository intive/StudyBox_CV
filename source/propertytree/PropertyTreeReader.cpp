#include "PropertyTreeReader.h"
#include "PropertyTree.h"

namespace {

/// Sprawdza czy znak jest znakiem białym w JSON.
bool IsWhitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

/// Sprawdza czy znak jest znakiem kontrolnym (występującym po '\').
/**
 * Nie zawiera przypadku \u, który sygnalizuje wartość Unicode.
 * W chwili obecnej ta funkcjonalność nie jest wspierana.
 */
char IsControl(char c)
{
    switch (c)
    {
    case '\\': return '\\';
    case '"': return '"';
    case '/': return '/';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    default:
        return '\0';
    }
}

/// Sprawdza czy znak jest znakiem interpunkcyjnym (rozdzielającym składnię JSON).
bool IsPunctuation(char c)
{
    switch (c)
    {
    case ',':
    case '}':
    case ']':
        return true;
    default:
        return false;
    }
}

/// Czyta i usuwa ze strumienia znaki do napotkania znaku nie-białego.
/**
 * Wartość zwracana nie jest usunięta ze strumienia.
 */
char TrimWhitespace(std::istream& stream)
{
    char c = stream.peek();

    if (!IsWhitespace(c))
        return c;

    do
    {
        //stream.read(&c, 1);
        c = stream.peek();
    } while (IsWhitespace(c) && stream.read(&c, 1));
    return c;
}

class JsonReader
{
public:
    enum Result
    {
        Indeterminate,
        Bad,
        Good
    };

protected:
    JsonReader() = default;
    ~JsonReader() = default;
};

/// Parser wartości JSON typu string.
class JsonStringReader : public JsonReader
{
public:
    JsonStringReader() : state(State::Initial) {}

    Result consume(char c, std::string& buffer)
    {
        switch (state)
        {
        case State::Initial:
            if (c == '"')
            {
                state = State::QuotationOpen;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::QuotationOpen:
            if (c == '"')
            {
                state = State::QuotationClosed;
                return Result::Indeterminate;
            }
            if (c == '\\')
            {
                state = State::ControlCharacter;
                return Result::Indeterminate;
            }
            buffer += c;
            return Result::Indeterminate;
        case State::ControlCharacter:
            if ((c = IsControl(c)) == '\0')
            {
                return Result::Bad;
            }
            buffer += c;
            state = State::QuotationOpen;
            return Result::Indeterminate;
        case State::QuotationClosed:
            return Result::Good;
        default:
            return Result::Bad;
        }

    }

    void reset()
    {
        state = State::Initial;
    }

private:
    enum State
    {
        Initial,
        QuotationOpen,
        ControlCharacter,
        QuotationClosed,
    } state;
};

/// Dokonuje odczytu znaku ','.
/**
 * @return false jeżeli znak nie został odnaleziony.
 */
bool ReadJsonComma(std::istream& stream)
{
    if (TrimWhitespace(stream) == ',')
    {
        char c;
        stream.read(&c, 1);
        return true;
    }
    return false;
}

/// Dokonuje odczytu znaku ':'.
/**
* @return false jeżeli znak nie został odnaleziony.
*/
bool ReadJsonColon(std::istream& stream)
{
    if (TrimWhitespace(stream) == ':')
    {
        char c;
        stream.read(&c, 1);
        return true;
    }
    return false;
}

/// Parser liczby JSON.
class JsonNumberReader : public JsonReader
{
public:
    JsonNumberReader() : state(State::Initial) {}

    Result consume(char c, std::string& buffer)
    {
        switch (state)
        {
        case State::Initial:
            if (c == '-')
            {
                buffer += c;
                return Result::Indeterminate;
            }
        case State::DigitOrZero:
            if (c > '0' && c <= '9')
            {
                state = State::DigitInteger;
            }
            else if (c == '0')
            {
                state = State::Zero;
            }
            else
            {
                return Result::Bad;
            }
            buffer += c;
            return Result::Indeterminate;
        case State::DigitExponent:
        case State::DigitFractional:
        case State::DigitInteger:
            if (c >= '0' && c <= '9')
            {
                buffer += c;
                return Result::Indeterminate;
            }
            // fall-through
        case State::Zero:
            if (c == '.' && state < State::DigitFractional)
            {
                state = State::DigitFractional;
            }
            else if ((c == 'e' || c == 'E') && state < State::Exponent)
            {
                state = State::Exponent;
            }
            else if (IsWhitespace(c) || IsPunctuation(c))
            {
                return Result::Good;
            }
            else
            {
                return Result::Bad;
            }
            buffer += c;
            return Result::Indeterminate;

        case State::Exponent:
            if ((c == '+' || c == '-') || (c >= '0' && c <= '9'))
            {
                buffer += c;
                state = State::DigitExponent;
                return Result::Indeterminate;
            }
            return Result::Bad;
        default:
            return Result::Bad;
        }
    }

    void reset()
    {
        state = State::Initial;
    }

private:
    enum State
    {
        Initial,
        DigitOrZero,
        DigitInteger,
        Zero,
        DigitFractional,
        Exponent,
        DigitExponent
    } state;
};

/// Parser dla wartości true.
class JsonTrueReader : public JsonReader
{
public:
    JsonTrueReader() : state(State::Initial) {}

    Result consume(char c, std::string& buffer)
    {
        switch (state)
        {
        case State::Initial:
            if (c == 't')
            {
                state = State::T;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::T:
            if (c == 'r')
            {
                state = State::R;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::R:
            if (c == 'u')
            {
                state = State::U;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::U:
            if (c == 'e')
            {
                state = State::E;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::E:
            if (IsWhitespace(c) || IsPunctuation(c))
            {
                return Result::Good;
            }
            return Result::Bad;
        default:
            return Result::Bad;
        }
    }

private:
    enum State
    {
        Initial,
        T,
        R,
        U,
        E,
    } state;
};

/// Parser dla wartości false.
class JsonFalseReader : public JsonReader
{
public:
    JsonFalseReader() : state(State::Initial) {}

    Result consume(char c, std::string& buffer)
    {
        switch (state)
        {
        case State::Initial:
            if (c == 'f')
            {
                state = State::F;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::F:
            if (c == 'a')
            {
                state = State::A;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::A:
            if (c == 'l')
            {
                state = State::L;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::L:
            if (c == 's')
            {
                state = State::S;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::S:
            if (c == 'e')
            {
                state = State::E;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::E:
            if (IsWhitespace(c) || IsPunctuation(c))
            {
                return Result::Good;
            }
            return Result::Bad;
        default:
            return Result::Bad;
        }
    }

private:
    enum State
    {
        Initial,
        F,
        A,
        L,
        S,
        E,
    } state;
};

/// Parser dla wartości null.
class JsonNullReader : public JsonReader
{
public:
    JsonNullReader() : state(State::Initial) {}

    Result consume(char c, std::string& buffer)
    {
        switch (state)
        {
        case State::Initial:
            if (c == 'n')
            {
                state = State::N;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::N:
            if (c == 'u')
            {
                state = State::U;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::U:
            if (c == 'l')
            {
                state = State::FirstL;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::FirstL:
            if (c == 'l')
            {
                state = State::SecondL;
                buffer += c;
                return Result::Indeterminate;
            }
            return Result::Bad;
        case State::SecondL:
            if (IsWhitespace(c) || IsPunctuation(c))
            {
                return Result::Good;
            }
            return Result::Bad;
        default:
            return Result::Bad;
        }
    }

private:
    enum State
    {
        Initial,
        N,
        U,
        FirstL,
        SecondL,
    } state;
};

/// Dokonuje odczytu wartości JSON string.
bool ReadJsonString(std::istream& stream, PropertyTree& tree, std::string& label)
{
    std::string buffer;
    JsonStringReader reader;
    char c = TrimWhitespace(stream);

    for (;;)
    {
        c = stream.peek();
        auto result = reader.consume(c, buffer);
        if (result == JsonStringReader::Result::Bad)
            return false;
        else if (result == JsonStringReader::Result::Good)
        {
            label = buffer;
            return true;
        }
        stream.read(&c, 1);
    }
}

/// Dokonuje odczytu wartości JSON number.
bool ReadJsonNumber(std::istream& stream, PropertyTree& tree, const std::string& label)
{
    std::string buffer;
    JsonNumberReader reader;
    for (;;)
    {
        char c = stream.peek();
        auto result = reader.consume(c, buffer);
        if (result == JsonNumberReader::Result::Bad)
            return false;
        else if (result == JsonNumberReader::Result::Good)
        {
            tree.put(label, buffer);
            return true;
        }
        stream.read(&c, 1);
    }
}

bool ReadJsonObject(std::istream& stream, PropertyTree& tree, const std::string& label);
bool ReadJsonArray(std::istream& stream, PropertyTree& tree, const std::string& label);

/// Dokonuje odczytu JSON value, tj. Number, Boolean, Null, String, Array, Object.
bool ReadJsonValue(std::istream& stream, PropertyTree& tree, const std::string& label)
{
    char c = stream.peek();
    if (IsWhitespace(c))
    {
        c = TrimWhitespace(stream);
    }
    if (c == '{') return ReadJsonObject(stream, tree, label);
    else if (c == '[') return ReadJsonArray(stream, tree, label);
    else
    {
        std::tuple<JsonStringReader, std::string, bool> jsonString;
        std::tuple<JsonNumberReader, std::string, bool> jsonNumber;
        std::tuple<JsonTrueReader, std::string, bool> jsonTrue;
        std::tuple<JsonFalseReader, std::string, bool> jsonFalse;
        std::tuple<JsonNullReader, std::string, bool> jsonNull;
        for (int i = 0;; ++i)
        {
            if (i != 0)
                c = stream.peek();

            if (!std::get<2>(jsonString))
            {
                auto result = std::get<0>(jsonString).consume(c, std::get<1>(jsonString));
                if (result == JsonNumberReader::Result::Bad)
                {
                    std::get<2>(jsonString) = true;
                }
                else if (result == JsonNumberReader::Result::Good)
                {
                    tree.type() = PropertyTree::Type::String;
                    tree.put(std::get<1>(jsonString));
                    return true;
                }
            }

            if (!std::get<2>(jsonNumber))
            {
                auto result = std::get<0>(jsonNumber).consume(c, std::get<1>(jsonNumber));
                if (result == JsonNumberReader::Result::Bad)
                {
                    std::get<2>(jsonNumber) = true;
                }
                else if (result == JsonNumberReader::Result::Good)
                {
                    tree.type() = PropertyTree::Type::Value;
                    tree.put(std::get<1>(jsonNumber));
                    return true;
                }
            }

            if (!std::get<2>(jsonTrue))
            {
                auto result = std::get<0>(jsonTrue).consume(c, std::get<1>(jsonTrue));
                if (result == JsonNumberReader::Result::Bad)
                {
                    std::get<2>(jsonTrue) = true;
                }
                else if (result == JsonNumberReader::Result::Good)
                {
                    tree.type() = PropertyTree::Type::Boolean;
                    tree.put(std::get<1>(jsonTrue));
                    return true;
                }
            }

            if (!std::get<2>(jsonFalse))
            {
                auto result = std::get<0>(jsonFalse).consume(c, std::get<1>(jsonFalse));
                if (result == JsonNumberReader::Result::Bad)
                {
                    std::get<2>(jsonFalse) = true;
                }
                else if (result == JsonNumberReader::Result::Good)
                {
                    tree.type() = PropertyTree::Type::Boolean;
                    tree.put(std::get<1>(jsonFalse));
                    return true;
                }
            }

            if (!std::get<2>(jsonNull))
            {
                auto result = std::get<0>(jsonNull).consume(c, std::get<1>(jsonNull));
                if (result == JsonNumberReader::Result::Bad)
                {
                    std::get<2>(jsonNull) = true;
                }
                else if (result == JsonNumberReader::Result::Good)
                {
                    tree.type() = PropertyTree::Type::Null;
                    tree.put(std::get<1>(jsonNull));
                    return true;
                }
            }

            if (std::get<2>(jsonNull) && std::get<2>(jsonTrue) && std::get<2>(jsonFalse) && std::get<2>(jsonNumber) && std::get<2>(jsonString))
                return false;

            stream.read(&c, 1);
        }
    }
    return false;
}

/// Dokonuje odczytu JSON pair, tj. String + ':' + Value.
bool ReadJsonPair(std::istream& stream, std::pair<std::string, PropertyTree>& tree)
{
    return ReadJsonString(stream, tree.second, tree.first) && ReadJsonColon(stream) && ReadJsonValue(stream, tree.second, tree.first);
}

/// Dokonuje odczytu JSON object.
bool ReadJsonObject(std::istream& stream, PropertyTree& tree, const std::string& label)
{
    PropertyTree& object = tree;
    object.type() = PropertyTree::Type::Object;
    char c;
    stream.read(&c, 1);
    c = stream.peek();
    if (IsWhitespace(c))
    {
        stream.read(&c, 1);
        c = TrimWhitespace(stream);
    }
    if (c == '}')
    {
        stream.read(&c, 1);
        return true;
    }
    std::pair<std::string, PropertyTree> pair;
    bool comma = true;
    while (comma)
    {
        if (!ReadJsonPair(stream, pair))
            return false;
        comma = ReadJsonComma(stream);
        if (comma)
        {
            object.put(pair.first, pair.second);
            pair = std::pair<std::string, PropertyTree>();
        }
    }
    c = TrimWhitespace(stream);
    if (c != '}')
        return false;

    stream.read(&c, 1);

    object.put(pair.first, pair.second);
    return true;
}

bool ReadJsonImpl(std::istream& stream, PropertyTree& tree, const std::string& term = "", bool readValue = false);

/// Dokonuje odczytu JSON array.
bool ReadJsonArray(std::istream& stream, PropertyTree& tree, const std::string& label)
{
    PropertyTree& array = tree;
    array.type() = PropertyTree::Type::Array;
    char c;
    stream.read(&c, 1);
    c = stream.peek();
    if (IsWhitespace(c))
    {
        stream.read(&c, 1);
        c = TrimWhitespace(stream);
    }
    if (c == ']')
    {
        stream.read(&c, 1);
        return true;
    }
    bool comma = true;
    PropertyTree value;
    while (comma)
    {
        if (!ReadJsonImpl(stream, value, ",]", true))
            return false;
        comma = ReadJsonComma(stream);
        if (comma)
        {
            array.put("", value);
            value = PropertyTree();
        }
    }
    c = TrimWhitespace(stream);
    if (c != ']')
        return false;

    stream.read(&c, 1);

    array.put("", value);
    return true;
}

/// Dokonuje odczytu wartości dla korzenia i JSON array.
bool ReadJsonImpl(std::istream& stream, PropertyTree& tree, const std::string& term, bool readValue)
{
    char c = stream.peek();
    if (IsWhitespace(c))
        c = TrimWhitespace(stream);
    while (c != -1)
    {
        if (c == '{')
        {
            if (!ReadJsonObject(stream, tree, ""))
                return false;
        }
        else if (c == '[')
        {
            if (!ReadJsonArray(stream, tree, ""))
                return false;
        }
        else if (readValue)
        {
            if (!ReadJsonValue(stream, tree, ""))
                return false;
        }
        else if (!IsWhitespace(c))
        {
            return false;
        }

        c = TrimWhitespace(stream);

        if (term.find(c) != std::string::npos)
            break;
    }

    return true;
}

}

void ReadJson(PropertyTree& tree, std::istream& stream)
{
    auto res = stream.rdbuf()->in_avail();
    if (!stream.rdbuf()->in_avail() || !ReadJsonImpl(stream, tree))
        throw std::range_error("stream does not contain valid json");
}
