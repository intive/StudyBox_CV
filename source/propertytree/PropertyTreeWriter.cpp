#include "PropertyTreeWriter.h"
#include "PropertyTree.h"
#include "../json/json.h"

namespace {

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
                value = '"' + value + '"';
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

Json::Object ToJsonObjectImpl(const PropertyTree& tree);
Json::Array ToJsonArrayImpl(const PropertyTree& tree)
{
    Json::Array array;
    for (auto& subtree : tree)
    {
        switch (subtree.second.type())
        {
        case PropertyTree::Value: array.Add(subtree.second.get<double>()); break;
        case PropertyTree::Boolean: array.Add(subtree.second.get<bool>()); break;
        case PropertyTree::Null: array.Add(nullptr); break;
        case PropertyTree::String: array.Add(subtree.second.get<std::string>()); break;
        case PropertyTree::Array: array.Add(ToJsonArrayImpl(subtree.second)); break;
        case PropertyTree::Object: array.Add(ToJsonObjectImpl(subtree.second)); break;
        }
    }

    return array;
}

Json::Object ToJsonObjectImpl(const PropertyTree& tree)
{
    Json::Object object;
    for (auto& subtree : tree)
    {
        switch (subtree.second.type())
        {
        case PropertyTree::Value: object.Add(subtree.first, subtree.second.get<double>()); break;
        case PropertyTree::Boolean: object.Add(subtree.first, subtree.second.get<bool>()); break;
        case PropertyTree::Null: object.Add(subtree.first, nullptr); break;
        case PropertyTree::String: object.Add(subtree.first, subtree.second.get<std::string>()); break;
        case PropertyTree::Array: object.Add(subtree.first, ToJsonArrayImpl(subtree.second)); break;
        case PropertyTree::Object: object.Add(subtree.first, ToJsonObjectImpl(subtree.second)); break;
        }
    }

    return object;
}

} // namespace

Json::Object ToJsonObject(const PropertyTree & tree)
{
    if (tree.type() == PropertyTree::Object)
        return ToJsonObjectImpl(tree);
    throw std::range_error("tree does not contain valid json");
}
