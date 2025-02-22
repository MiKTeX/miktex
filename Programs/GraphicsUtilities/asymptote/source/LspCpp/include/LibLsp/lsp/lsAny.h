#pragma once

#include "LibLsp/JsonRpc/serializer.h"
#include <string>
#include "LibLsp/JsonRpc/message.h"
namespace lsp
{
struct Any
{
    //! Type of JSON value
    enum Type
    {
        kUnKnown = -1,
        kNullType = 0, //!< null
        kFalseType = 1, //!< false
        kTrueType = 2, //!< true
        kObjectType = 3, //!< object
        kArrayType = 4, //!< array
        kStringType = 5, //!< string
        kNumberType = 6 //!< number
    };

    template<typename T>
    bool Get(T& value);

    template<typename T>
    void Set(T& value);

    int GuessType();
    int GetType();

    void Set(std::unique_ptr<LspMessage> value);

    void SetJsonString(std::string&& _data, Type _type);

    void SetJsonString(std::string const& _data, Type _type);

    std::string const& Data() const
    {
        return data;
    }

    void swap(Any& arg) noexcept;

    /*
                 *Example for GetFromMap
                        struct A{
                                std::string  visitor;
                                bool   verbose;
                        }
                        REFLECT_MAP_TO_STRUCT(A,visitor,verbose)

                        std::string data = "{\"visitor\":\"default\",\"verbose\":\"true\"};
                        lsp:Any any;
                        any.SetJsonString(data, static_cast<lsp::Any::Type>(-1));
                        A a_object;
                        any.GetFromMap(a_object);
                */
    template<typename T>
    bool GetFromMap(T& value);

    template<typename T>
    bool GetForMapHelper(T& value);
    bool GetForMapHelper(std::string& value);
    bool GetForMapHelper(optional<std::string>& value);

private:
    std::unique_ptr<Reader> GetReader();
    std::unique_ptr<Writer> GetWriter() const;
    void SetData(std::unique_ptr<Writer>&);

    std::string data;
    int jsonType = kUnKnown;
};

}; // namespace lsp

extern void Reflect(Reader& visitor, lsp::Any& value);
extern void Reflect(Writer& visitor, lsp::Any& value);

template<typename T>
void ReflectMember(std::map<std::string, lsp::Any>& visitor, char const* name, T& value)
{

    auto it = visitor.find(name);
    if (it != visitor.end())
    {
        it->second.GetForMapHelper(value);
    }
}
template<typename T>
void ReflectMember(std::map<std::string, std::string>& visitor, char const* name, T& value)
{

    auto it = visitor.find(name);
    if (it != visitor.end())
    {
        lsp::Any any;
        any.SetJsonString(it->second, static_cast<lsp::Any::Type>(-1));
        any.Get(value);
    }
}

#define REFLECT_MAP_TO_STRUCT(type, ...) \
    template<typename TVisitor> \
    void ReflectMap(TVisitor& visitor, type& value) \
    { \
        MACRO_MAP(_MAPPABLE_REFLECT_MEMBER, __VA_ARGS__) \
    }

namespace lsp
{
template<typename T>
bool Any::Get(T& value)
{
    auto const visitor = GetReader();
    Reflect(*visitor, value);
    return true;
}

template<typename T>
void Any::Set(T& value)
{
    auto visitor = GetWriter();
    Reflect(*visitor, value);
    SetData(visitor);
}

template<typename T>
bool Any::GetFromMap(T& value)
{
    auto const visitor = GetReader();
    std::map<std::string, lsp::Any> _temp;
    Reflect(*visitor, _temp);
    ReflectMap(_temp, value);
    return true;
}

template<typename T>
bool Any::GetForMapHelper(T& value)
{
    jsonType = GetType();
    if (jsonType == kStringType)
    {
        auto copy = data;
        copy.erase(copy.find_last_not_of('"') + 1);
        copy.erase(0, copy.find_first_not_of('"'));
        lsp::Any any;
        any.SetJsonString(copy, kUnKnown);
        any.Get(value);
    }
    else
    {
        Get(value);
    }
    return true;
}
} // namespace lsp
