#pragma once

#include "macro_map.h"

#include <cassert>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>
#include <functional>
#include <map>
#include <algorithm>

#include "optionalVersion.h"

struct AbsolutePath;

enum class SerializeFormat
{
    Json,
    MessagePack
};

// A tag type that can be used to write `null` to json.
struct JsonNull
{
    void swap(JsonNull& arg) noexcept;
};

class Reader
{
public:
    virtual ~Reader()
    {
    }
    virtual SerializeFormat Format() const = 0;

    virtual bool IsBool() = 0;
    virtual bool IsNull() = 0;
    virtual bool IsArray() = 0;
    virtual bool IsInt() = 0;
    virtual bool IsInt64() = 0;
    virtual bool IsUint64() = 0;
    virtual bool IsDouble() = 0;
    virtual bool IsNumber() = 0;
    virtual bool IsString() = 0;

    virtual void GetNull() = 0;
    virtual bool GetBool() = 0;
    virtual int GetInt() = 0;
    virtual uint32_t GetUint32() = 0;
    virtual int64_t GetInt64() = 0;
    virtual uint64_t GetUint64() = 0;
    virtual double GetDouble() = 0;
    virtual std::string GetString() = 0;

    virtual bool HasMember(char const* x) = 0;
    virtual std::unique_ptr<Reader> operator[](char const* x) = 0;
    virtual void IterMap(std::function<void(char const*, Reader&)> fn) = 0;
    virtual void IterArray(std::function<void(Reader&)> fn) = 0;
    virtual void DoMember(char const* name, std::function<void(Reader&)> fn) = 0;
    virtual std::string ToString() const = 0;
};

class Writer
{
public:
    virtual ~Writer()
    {
    }
    virtual SerializeFormat Format() const = 0;

    virtual void Null() = 0;
    virtual void Bool(bool x) = 0;
    virtual void Int(int x) = 0;
    virtual void Uint32(uint32_t x) = 0;
    virtual void Int64(int64_t x) = 0;
    virtual void Uint64(uint64_t x) = 0;
    virtual void Double(double x) = 0;
    virtual void String(char const* x) = 0;
    virtual void String(char const* x, size_t len) = 0;
    virtual void StartArray(size_t) = 0;
    virtual void EndArray() = 0;
    virtual void StartObject() = 0;
    virtual void EndObject() = 0;
    virtual void Key(char const* name) = 0;
};

struct optionals_mandatory_tag
{
};

#define REFLECT_MEMBER_START() ReflectMemberStart(visitor, value)
#define REFLECT_MEMBER_END() ReflectMemberEnd(visitor, value);
#define REFLECT_MEMBER_END1(value) ReflectMemberEnd(visitor, value);
#define REFLECT_MEMBER(name) ReflectMember(visitor, #name, value.name)
#define REFLECT_MEMBER_OPTIONALS(name) ReflectMember(visitor, #name, value.name, optionals_mandatory_tag {})
#define REFLECT_MEMBER2(name, value) ReflectMember(visitor, name, value)

#define MAKE_REFLECT_TYPE_PROXY(type_name) MAKE_REFLECT_TYPE_PROXY2(type_name, std::underlying_type<type_name>::type)
#define MAKE_REFLECT_TYPE_PROXY2(type, as_type) \
    inline void Reflect(Reader& visitor, type& value) \
    { \
        as_type value0; \
        ::Reflect(visitor, value0); \
        value = static_cast<type>(value0); \
    } \
    inline void Reflect(Writer& visitor, type& value) \
    { \
        auto value0 = static_cast<as_type>(value); \
        ::Reflect(visitor, value0); \
    }

#define _MAPPABLE_REFLECT_MEMBER(name) REFLECT_MEMBER(name);
#define _MAPPABLE_REFLECT_MEMBER_OPTIONALS(name) REFLECT_MEMBER_OPTIONALS(name);

#define MAKE_REFLECT_EMPTY_STRUCT(type, ...) \
    template<typename TVisitor> \
    void Reflect(TVisitor& visitor, type& value) \
    { \
        REFLECT_MEMBER_START(); \
        REFLECT_MEMBER_END(); \
    }

#define MAKE_REFLECT_STRUCT(type, ...) \
    template<typename TVisitor> \
    void Reflect(TVisitor& visitor, type& value) \
    { \
        REFLECT_MEMBER_START(); \
        MACRO_MAP(_MAPPABLE_REFLECT_MEMBER, __VA_ARGS__) \
        REFLECT_MEMBER_END(); \
    }

#define _MAPPABLE_SWAP_MEMBER(name) std::swap(name, arg.name);

#define MAKE_SWAP_METHOD(type, ...) \
    void swap(type& arg) noexcept \
    { \
        MACRO_MAP(_MAPPABLE_SWAP_MEMBER, __VA_ARGS__) \
    }

#define MAKE_REFLECT_STRUCT_OPTIONALS_MANDATORY(type, ...) \
    template<typename TVisitor> \
    void Reflect(TVisitor& visitor, type& value) \
    { \
        REFLECT_MEMBER_START(); \
        MACRO_MAP(_MAPPABLE_REFLECT_MEMBER_OPTIONALS, __VA_ARGS__) \
        REFLECT_MEMBER_END(); \
    }

// clang-format off
// Config has many fields, we need to support at least its number of fields.
#define NUM_VA_ARGS_IMPL(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,N,...) N
#define NUM_VA_ARGS(...) NUM_VA_ARGS_IMPL(__VA_ARGS__,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)
// clang-format on

#define _MAPPABLE_REFLECT_ARRAY(name) Reflect(visitor, value.name);

// Reflects the struct so it is serialized as an array instead of an object.
// This currently only supports writers.
#define MAKE_REFLECT_STRUCT_WRITER_AS_ARRAY(type, ...) \
    inline void Reflect(Writer& visitor, type& value) \
    { \
        visitor.StartArray(NUM_VA_ARGS(__VA_ARGS__)); \
        MACRO_MAP(_MAPPABLE_REFLECT_ARRAY, __VA_ARGS__) \
        visitor.EndArray(); \
    }

//// Elementary types

void Reflect(Reader& visitor, uint8_t& value);
void Reflect(Writer& visitor, uint8_t& value);

void Reflect(Reader& visitor, short& value);
void Reflect(Writer& visitor, short& value);

void Reflect(Reader& visitor, unsigned short& value);
void Reflect(Writer& visitor, unsigned short& value);

void Reflect(Reader& visitor, int& value);
void Reflect(Writer& visitor, int& value);

void Reflect(Reader& visitor, unsigned& value);
void Reflect(Writer& visitor, unsigned& value);

void Reflect(Reader& visitor, long& value);
void Reflect(Writer& visitor, long& value);

void Reflect(Reader& visitor, unsigned long& value);
void Reflect(Writer& visitor, unsigned long& value);

void Reflect(Reader& visitor, long long& value);
void Reflect(Writer& visitor, long long& value);

void Reflect(Reader& visitor, unsigned long long& value);
void Reflect(Writer& visitor, unsigned long long& value);

void Reflect(Reader& visitor, double& value);
void Reflect(Writer& visitor, double& value);

void Reflect(Reader& visitor, bool& value);
void Reflect(Writer& visitor, bool& value);

void Reflect(Reader& visitor, std::string& value);
void Reflect(Writer& visitor, std::string& value);

void Reflect(Reader& visitor, JsonNull& value);
void Reflect(Writer& visitor, JsonNull& value);

void Reflect(Reader& visitor, SerializeFormat& value);
void Reflect(Writer& visitor, SerializeFormat& value);

//// Type constructors
template<typename T>
void Reflect(Reader& visitor, optional<T>& value)
{
    if (visitor.IsNull())
    {
        visitor.GetNull();
        return;
    }
    T real_value;
    Reflect(visitor, real_value);
    value = std::move(real_value);
}
template<typename T>
void Reflect(Writer& visitor, optional<T>& value)
{
    if (value)
    {
        Reflect(visitor, *value);
    }
    else
    {
        visitor.Null();
    }
}

template<typename T>
void ReflectMember(Writer& visitor, char const* name, optional<T>& value)
{
    // For TypeScript optional property key?: value in the spec,
    // We omit both key and value if value is std::nullopt (null) for JsonWriter
    // to reduce output. But keep it for other serialization formats.
    if (value || visitor.Format() != SerializeFormat::Json)
    {
        visitor.Key(name);
        Reflect(visitor, value);
    }
}

template<typename T>
void ReflectMember(Writer& visitor, char const* name, T& value, optionals_mandatory_tag)
{
    visitor.Key(name);
    Reflect(visitor, value);
}
template<typename T>
void ReflectMember(Reader& visitor, char const* name, T& value, optionals_mandatory_tag)
{
    Reflect(visitor, value);
}

template<class T>
void Reflect(Reader& visitor, std::map<std::string, T>& value)
{
    visitor.IterMap(
        [&](char const* name, Reader& entry)
        {
            T entry_value;
            Reflect(entry, entry_value);
            value[name] = (std::move(entry_value));
        }
    );
}
template<class _Ty>
void Reflect(Writer& visitor, std::map<std::string, _Ty>& value)
{
    REFLECT_MEMBER_START();
    for (auto& it : value)
    {
        visitor.Key(it.first.c_str());
        Reflect(visitor, it.second);
    }
    REFLECT_MEMBER_END();
}

// std::vector
template<typename T>
void Reflect(Reader& visitor, std::vector<T>& values)
{
    visitor.IterArray(
        [&](Reader& entry)
        {
            T entry_value;
            Reflect(entry, entry_value);
            values.push_back(std::move(entry_value));
        }
    );
}

template<typename T>
void Reflect(Writer& visitor, std::vector<T>& values)
{
    visitor.StartArray(values.size());
    for (auto& value : values)
    {
        Reflect(visitor, value);
    }
    visitor.EndArray();
}

// ReflectMember

inline void DefaultReflectMemberStart(Writer& visitor)
{
    visitor.StartObject();
}
inline void DefaultReflectMemberStart(Reader&)
{
}

template<typename T>
bool ReflectMemberStart(Reader&, T&)
{
    return false;
}
template<typename T>
bool ReflectMemberStart(Writer& visitor, T&)
{
    visitor.StartObject();
    return true;
}

template<typename T>
void ReflectMemberEnd(Reader&, T&)
{
}
template<typename T>
void ReflectMemberEnd(Writer& visitor, T&)
{
    visitor.EndObject();
}

template<typename T>
void ReflectMember(Reader& visitor, char const* name, T& value)
{
    visitor.DoMember(name, [&](Reader& child) { Reflect(child, value); });
}
template<typename T>
void ReflectMember(Writer& visitor, char const* name, T& value)
{
    visitor.Key(name);
    Reflect(visitor, value);
}

template<class _Ty1, class _Ty2>
void Reflect(Writer& visitor, std::pair<optional<_Ty1>, optional<_Ty2>>& value)
{
    if (value.first)
    {
        Reflect(visitor, value.first);
    }
    else
    {
        Reflect(visitor, value.second);
    }
}
template<class _Ty2>
void Reflect(Reader& visitor, std::pair<optional<bool>, optional<_Ty2>>& value)
{
    if (visitor.IsBool())
    {
        Reflect(visitor, value.first);
        return;
    }

    Reflect(visitor, value.second);
}
template<class _Ty2>
void Reflect(Reader& visitor, std::pair<optional<std::string>, optional<_Ty2>>& value)
{
    if (visitor.IsString())
    {
        Reflect(visitor, value.first);
        return;
    }

    Reflect(visitor, value.second);
}

template<class _Ty1, class _Ty2>
void Reflect(Reader& visitor, std::pair<optional<_Ty1>, optional<_Ty2>>& value)
{
    try
    {
        Reflect(visitor, value.second);
    }
    catch (...)
    {
        Reflect(visitor, value.first);
    }
}
