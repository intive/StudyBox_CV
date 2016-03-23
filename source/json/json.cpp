#include <iterator>
#include <sstream>
#include <fstream>

#include "json.h"


namespace Json
{
    //////////////////////////////////// Json Object

    std::string Object::serialize()
    {
        std::ostringstream oss;
        oss << "{";

        auto last = container.empty() ? container.end() : std::prev(container.end());
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            oss << "\"" << it->first << "\":" << it->second.serialize();
            if (it != last)
                oss << ",";
        }

        oss << "}";
        return oss.str();
    }

    void Object::serializeToFile(const std::string & filename)
    {
        if (filename.empty())
            throw std::runtime_error("Json::Object::serializeToFile - filename is empty");
        
        std::ofstream file(filename);
        if (!file.good())
            throw std::runtime_error("Json::Object::serializeToFile - couldn't open file");

        file << this->serialize();
    }


    Object& Object::Add(const std::string& key, const Value& value)
    {
        container[key] = value;
        return *this;
    }


    const Value& Object::operator[](const std::string& index) const
    {
        std::map<std::string, Value>::const_iterator it = container.find(index);
        return it->second;
    }


    Object::Object(const Object& o)
    {
        container.insert(o.container.begin(), o.container.end());
    }

    Object& Object::operator=(const Object&  o)
    {
        container.insert(o.container.begin(), o.container.end());
        return *this;
    }

    Object::Object(const std::initializer_list<std::pair<std::string, Value>>& list)
    {
        for (auto& p : list)
            container[p.first] = p.second;
    }


    Object& Object::Add(const std::initializer_list<std::pair<std::string, Value>>& list)
    {
        for (auto& p : list)
            container[p.first] = p.second;

        return *this;
    }


    //////////////////////////////////// Json Array

    std::string Array::serialize()
    {
        std::ostringstream oss;
        oss << "[";

        auto last = container.empty() ? container.end() : prev(container.end());
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            oss << it->serialize();
            if (it != last)
                oss << ",";
        }

        oss << "]";
        return oss.str();
    }

    void Array::serializeToFile(const std::string& filename)
    {
        if (filename.empty())
            throw std::runtime_error("Json::Object::serializeToFile - filename is empty");

        std::ofstream file(filename);
        if (!file.good())
            throw std::runtime_error("Json::Object::serializeToFile - couldn't open given file");

        file << this->serialize();
    }


    Array& Array::Add(const Value& val) {
        container.push_back(val);
        return *this;
    }


    Array& Array::Add(const std::initializer_list<Value>& vals) {
        for (auto& jval : vals)
            container.push_back(jval);

        return *this;
    }


    Array::Array(const Array&  a)
    {
        container = std::vector<Value>(a.container);
    }

    Array& Array::operator=(const Array&  a)
    {
        container = std::vector<Value>(a.container);
        return *this;
    }

    Array::Array(const std::initializer_list<Value>& list)
    {
        for (auto& x : list)
            container.push_back(x);
    }


    //////////////////////////////////// Json Value

    Value::Value(const Value&  v)
    {
        this->type = v.type;
        switch (v.type)
        {
        case Type::Double:
            value = JsonUnion(v.value.jsonDouble);
            break;

        case Type::Int:
            value = JsonUnion(v.value.jsonInt);
            break;

        case Type::Bool:
            value = JsonUnion(v.value.jsonBool);
            break;

        case Type::String:
            value = JsonUnion(v.getString());
            break;        

        case Type::Array:
            value = JsonUnion(v.getArray());
            break;

        case Type::Object:
            value = JsonUnion(v.getObject());
            break;

        default:
            break;
        }
    }

    Value& Value::operator=(const Value&  v)
    {
        this->type = v.type;
        switch (v.type)
        {
        case Type::Double:
            value = JsonUnion(v.value.jsonDouble);
            break;

        case Type::Int:
            value = JsonUnion(v.value.jsonInt);
            break;

        case Type::Bool:
            value = JsonUnion(v.value.jsonBool);
            break;

        case Type::String:
            value = JsonUnion(v.getString());
            break;

        case Type::Array:
            value = JsonUnion(v.getArray());
            break;

        case Type::Object:
            value = JsonUnion(v.getObject());
            break;

        default:
            break;
        }

        return *this;
    }


    Value::~Value()
    {
        switch (type)
        {
        case Type::Array:
            delete value.jsonArray;
            break;

        case Type::Object:
            delete value.jsonObject;
            break;

        case Type::String:
            delete value.jsonString;
            break;

        default:
            break;
        }
    }

    std::string Value::serialize() const
    {
        std::ostringstream oss;

        switch (type)
        {
        case Type::Double:
            oss << value.jsonDouble;
            break;

        case Type::Int:
            oss << value.jsonInt;
            break;

        case Type::String:
            oss << "\"" << *value.jsonString << "\"";
            break;

        case Type::Bool:
            oss << std::boolalpha << value.jsonBool;
            break;

        case Type::Null:
            oss << "null";
            break;

        case Type::Array:
            oss << value.jsonArray->serialize();
            break;

        case Type::Object:
            oss << value.jsonObject->serialize();
            break;
        }

        return oss.str();
    }

    Type Value::getType() const
    {
        return type;
    }

    bool Value::getBool() const
    {
        if (type == Type::Bool)
            return value.jsonBool;

        throw std::runtime_error("Underlying value is not bool type.");
    }

    int Value::getInt() const
    {
        if (type == Type::Int)
            return value.jsonInt;

        throw std::runtime_error("Underlying value is not integer type.");
    }

    double Value::getDouble() const
    {
        if (type == Type::Double)
            return value.jsonDouble;

        throw std::runtime_error("Underlying value is not floating-point type.");
    }

    Array Value::getArray() const
    {
        if (type == Type::Array)
            return *value.jsonArray;

        throw std::runtime_error("Underlying value is not Json::Array type.");
    }

    Object Value::getObject() const
    {
        if (type == Type::Object)
            return *value.jsonObject;

        throw std::runtime_error("Underlying value is not Json::Object type.");
    }

    std::string Value::getString() const
    {
        if (type == Type::String)
            return *value.jsonString;;

        throw std::runtime_error("Underlying value is not Json::String type.");
    }

    bool Value::getBool(bool def) const
    {
        if (type == Type::Bool)
            return value.jsonBool;

        return def;
    }

    int Value::getInt(int def) const
    {
        if (type == Type::Int)
            return value.jsonInt;

        return def;
    }

    double Value::getDouble(double def) const
    {
        if (type == Type::Double)
            return value.jsonDouble;

        return def;
    }

    Array Value::getArray(Array def) const
    {
        if (type == Type::Array)
            return *value.jsonArray;

        return def;
    }

    Object Value::getObject(Object def) const
    {
        if (type == Type::Object)
            return *value.jsonObject;

        return def;
    }

    std::string Value::getString(std::string def) const
    {
        if (type == Type::String)
            return *value.jsonString;;

        return def;
    }
}
