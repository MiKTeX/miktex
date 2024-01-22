/**
 * @file miktex/Util/CharBuffer.h
 * @author Christian Schenk
 * @brief CharBuffer class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#pragma once

#include <miktex/Util/config.h>

#include <cstddef>
#include <cwchar>

#include <string>

#include "StringUtil.h"
#include "inliners.h"

MIKTEX_UTIL_BEGIN_NAMESPACE;

/// Instances of this class store characters.
template<typename CharType, int BUFSIZE = 512> class CharBuffer
{

public:

    CharBuffer() = default;

    CharBuffer(const CharBuffer& other)
    {
        Set(other);
    }

    CharBuffer(CharBuffer&& other) noexcept
    {
        if (other.buffer == other.smallBuffer)
        {
            memcpy(this->smallBuffer, other.smallBuffer, BUFSIZE);
            this->buffer = this->smallBuffer;
        }
        else
        {
            this->buffer = other.buffer;
        }
        this->capacity = other.capacity;
        other.buffer = other.smallBuffer;
        other.capacity = BUFSIZE;
        other.Clear();
    }

    CharBuffer& operator=(const CharBuffer& other)
    {
        Set(other);
        return *this;
    }

    CharBuffer& operator=(CharBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Reset();
            if (other.buffer == other.smallBuffer)
            {
                memcpy(this->smallBuffer, other.smallBuffer, BUFSIZE);
                this->buffer = this->smallBuffer;
            }
            else
            {
                this->buffer = other.buffer;
            }
            this->capacity = other.capacity;
            other.buffer = other.smallBuffer;
            other.capacity = BUFSIZE;
            other.Clear();
        }
        return *this;
    }

    virtual ~CharBuffer() noexcept
    {
        try
        {
            Reset();
        }
        catch (const std::exception&)
        {
        }
    }

    explicit CharBuffer(const char* lpsz)
    {
        Set(lpsz);
    }

    explicit CharBuffer(const char16_t* lpsz)
    {
        Set(lpsz);
    }

    explicit CharBuffer(const wchar_t* lpsz)
    {
        Set(lpsz);
    }

    explicit CharBuffer(const std::basic_string<char>& other)
    {
        Set(other);
    }

    explicit CharBuffer(const std::basic_string<char16_t>& other)
    {
        Set(other);
    }

    explicit CharBuffer(const std::basic_string<wchar_t>& other)
    {
        Set(other);
    }

    explicit CharBuffer(std::size_t n)
    {
        Clear();
        Reserve(n);
    }

    void Set(const CharBuffer& other)
    {
        if (this != &other)
        {
            Reserve(other.capacity);
            memcpy(this->buffer, other.buffer, other.capacity * sizeof(CharType));
        }
    }

    template<typename OtherCharType> void Set(const OtherCharType* lpsz)
    {
        // TODO: MIKTEX_ASSERT_STRING_OR_NIL(lpsz);
        if (lpsz == nullptr)
        {
            Reset();
        }
        else
        {
            std::size_t requiredSize;
            if (sizeof(CharType) < sizeof(OtherCharType))
            {
                requiredSize = StrLen(lpsz) * 4 + 1; // worst case: wchar_t to UTF-8
            }
            else
            {
                requiredSize = StrLen(lpsz) + 1;
            }
            Reserve(requiredSize);
            StringUtil::CopyCeeString(buffer, GetCapacity(), lpsz);
        }
    }

    template<typename OtherCharType> void Set(const std::basic_string<OtherCharType>& s)
    {
        Set(s.c_str());
    }

    void Append(const std::basic_string<CharType>& s)
    {
        Reserve(GetLength() + s.length() + 1);
        StringUtil::AppendCeeString(buffer, GetCapacity(), s.c_str());
    }

    void Append(const CharType* lpsz)
    {
        // TODO: MIKTEX_ASSERT_STRING(lpsz);
        Reserve(GetLength() + StrLen(lpsz) + 1);
        StringUtil::AppendCeeString(buffer, GetCapacity(), lpsz);
    }

    void Append(const CharType* s, std::size_t len)
    {
        std::size_t idx = GetLength();
        Reserve(idx + len + 1);
        for (; len > 0; --len, ++idx, ++s)
        {
            buffer[idx] = *s;
        }
        buffer[idx] = 0;
    }

    void Append(CharType ch)
    {
        std::size_t len = GetLength();
        Reserve(len + 2);
        buffer[len] = ch;
        buffer[len + 1] = 0;
    }

    void Clear()
    {
        buffer[0] = 0;
    }

    /// Tests if the character sequence is empty.
    /// @return Returns true if the character sequence is empty.
    bool Empty() const
    {
        return buffer[0] == 0;
    }

    void Reset()
    {
        if (buffer != smallBuffer)
        {
            delete[] buffer;
            buffer = smallBuffer;
            capacity = BUFSIZE;
        }
        Clear();
    }

    void Reserve(std::size_t newSize)
    {
        if (newSize > BUFSIZE && newSize > capacity)
        {
            CharType* newBuffer = new CharType[newSize];
            memcpy(newBuffer, buffer, capacity * sizeof(CharType));
            if (buffer != smallBuffer)
            {
                delete[] buffer;
            }
            buffer = newBuffer;
            capacity = newSize;
        }
    }

    /// Converts this CharBuffer object into a string object.
    /// @return Returns a string object.
    std::basic_string<CharType> ToString() const
    {
        return std::basic_string<CharType>(buffer);
    }

    const CharType* GetData() const
    {
        return buffer;
    }

    CharType* GetData()
    {
        return buffer;
    }

    std::size_t GetLength() const
    {
        std::size_t idx;
        for (idx = 0; idx < capacity && buffer[idx] != 0; ++idx)
        {
            ;
        }
        return idx;
    }

    std::size_t GetCapacity() const
    {
        return capacity;
    }

    const CharType& operator[](std::size_t idx) const
    {
        // TODO: MIKTEX_ASSERT(idx < GetCapacity());
        return buffer[idx];
    }

    CharType& operator[](std::size_t idx)
    {
        // TODO: MIKTEX_ASSERT(idx < GetCapacity());
        return buffer[idx];
    }

    template<typename OtherCharType> CharBuffer& operator=(const OtherCharType* lpsz)
    {
        Set(lpsz);
        return *this;
    }

    template<typename OtherCharType> CharBuffer& operator=(const std::basic_string<OtherCharType>& s)
    {
        Set(s);
        return *this;
    }

    CharBuffer& operator+=(const CharType* lpsz)
    {
        Append(lpsz);
        return *this;
    }

    CharBuffer& operator+=(const std::basic_string<CharType>& s)
    {
        Append(s);
        return *this;
    }

    CharBuffer& operator+=(CharType ch)
    {
        Append(ch);
        return *this;
    }

private:

    CharType* buffer = smallBuffer;
    std::size_t capacity = BUFSIZE;
    CharType smallBuffer[BUFSIZE] = { 0 };
};

MIKTEX_UTIL_END_NAMESPACE;
