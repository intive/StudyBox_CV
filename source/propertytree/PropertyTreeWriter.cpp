#include "PropertyTreeWriter.h"
#include "PropertyTree.h"
#include "../json/Json.hpp"

#include <cstdint>

namespace {

std::string Unescape(const std::string& str)
{
    std::string newstr;
    for (auto it = str.begin(); it != str.end(); ++it)
    {
        char current = *it;
        switch (current)
        {
        case '\\': newstr.append("\\\\"); break;
        case '"':  newstr.append("\\\""); break;
        case '/':  newstr.append("\\/"); break;
        case '\b':  newstr.append("\\\b"); break;
        case '\f':  newstr.append("\\\f"); break;
        case '\n':  newstr.append("\\\n"); break;
        case '\r':  newstr.append("\\\r"); break;
        case '\t':  newstr.append("\\\t"); break;
        default:
            newstr.push_back(current);
        }
    }

    return newstr;
}

void WriteJsonImpl(const PropertyTree& tree, std::ostream& stream, bool whitespace, int indent = 1)
{
    std::string symbol;
    if (tree.type() == PropertyTree::Type::Array)
        symbol = "[]";
    else
        symbol = "{}";
    stream << symbol[0];

    if (whitespace && tree.begin() != tree.end())
        stream << std::endl;

    for (const auto& pair : tree)
    {
        for (int i = 0; whitespace && i < indent; ++i)
            stream << "  ";
        if (!pair.first.empty())
        {
            stream << '"' << pair.first << "\":";
            if (whitespace)
                stream << ' ';
        }

        if (pair.second.empty())
        {
            auto value = pair.second.get<std::string>();
            if (pair.second.type() == PropertyTree::Type::String)
                value = '"' + Unescape(value) + '"';
            if (!value.empty())
                stream << value;
        }
        else
        {
            WriteJsonImpl(pair.second, stream, whitespace, indent + 1);
        }
        if (&pair != &*--tree.end())
            stream << ',';
        if (whitespace)
            stream << std::endl;
    }
    if (tree.begin() != tree.end())
        for (int i = 0; whitespace && i < indent - 1; ++i)
            stream << "  ";
    stream << symbol[1];
    if (whitespace && indent <= 1)
        stream << std::endl;
}

} // namespace

void WriteJson(const PropertyTree& tree, std::ostream& stream, bool whitespace)
{
    try
    {
        WriteJsonImpl(tree, stream, whitespace);
    }
    catch (const std::out_of_range&)
    {
        throw std::range_error("tree does not contain valid json");
    }
}

namespace {

template<typename F, typename... Args>
void DeduceNumericType(const PropertyTree& tree, F&& func, Args&&... args)
{
    auto str = tree.get<std::string>();
    if (str.find_first_of('.') != std::string::npos || str.find_first_of('e') != std::string::npos || str.find_first_of('E') != std::string::npos)
    {
        func(tree.get<double>(), std::forward<Args>(args)...);//array.Add(tree.get<double>());
        return;
    }
    else if (str[0] == '-')
    {
        func(tree.get<int64_t>(), std::forward<Args>(args)...);
        return;
    }
    else
    {
        func(tree.get<uint64_t>(), std::forward<Args>(args)...);
        return;
    }
}

struct AddToJson
{
    template<typename Value, typename JsonObject, typename Arg>
    void operator ()(Value&& value, JsonObject&& object, Arg&& arg)
    {
        object[std::forward<Arg>(arg)] = std::forward<Value>(value);
    }

    template<typename Value, typename JsonObject>
    void operator ()(Value&& value, JsonObject&& object)
    {
        object.push_back(std::forward<Value>(value));
    }
};

template<typename ToObject, typename ToArray, typename Func, typename JsonObject, typename... Args>
void Deduce(const PropertyTree& tree, ToObject&& toObject, ToArray&& toArray, Func&& func, JsonObject&& object, Args&&... args)
{
    switch (tree.type())
    {
    case PropertyTree::Value: DeduceNumericType(tree, func, std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    case PropertyTree::Boolean: func(tree.get<bool>(), std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    case PropertyTree::Null: func(nullptr, std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    case PropertyTree::String: func(Unescape(tree.get<std::string>()), std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    case PropertyTree::Array: func(toArray(tree), std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    case PropertyTree::Object: func(toObject(tree), std::forward<JsonObject>(object), std::forward<Args>(args)...); break;
    }
}

std::vector<Json> ToJsonArrayImpl(const PropertyTree& tree);
Json ToJsonObjectImpl(const PropertyTree& tree)
{
    Json object;
    AddToJson func;
    for (auto& subtree : tree)
    {
        Deduce(subtree.second, ToJsonObjectImpl, ToJsonArrayImpl, func, object, subtree.first);
    }
    return object;
}

std::vector<Json> ToJsonArrayImpl(const PropertyTree& tree)
{
    std::vector<Json> array;
    AddToJson func;
    for (auto& subtree : tree)
    {
        Deduce(subtree.second, ToJsonObjectImpl, ToJsonArrayImpl, func, array);
    }

    return array;
}

} // namespace

Json ToJsonObject(const PropertyTree & tree)
{
    if (tree.type() == PropertyTree::Object)
        return ToJsonObjectImpl(tree);
    throw std::range_error("tree does not contain valid json");
}
