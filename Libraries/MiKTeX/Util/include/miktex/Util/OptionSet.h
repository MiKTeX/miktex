/**
 * @file miktex/Util/OptionSet.h
 * @author Christian Schenk
 * @brief OptionSet class
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

#include <bitset>
#include <initializer_list>

MIKTEX_UTIL_BEGIN_NAMESPACE;

template<typename EnumClass_, int Size_ = 32> class OptionSet
{

public:

    OptionSet() = default;
    OptionSet(const OptionSet& other) = default;
    OptionSet& operator=(const OptionSet& other) = default;
    OptionSet(OptionSet&& other) = default;
    OptionSet& operator=(OptionSet&& other) = default;
    ~OptionSet() = default;

    OptionSet(const std::initializer_list<EnumClass_>& options)
    {
        for (const EnumClass_& opt : options)
        {
            bits[(std::size_t)opt] = true;
        }
    }

    OptionSet& operator+=(EnumClass_ opt)
    {
        bits[(std::size_t)opt] = true;
        return *this;
    }

    OptionSet& operator+=(const OptionSet<EnumClass_>& other)
    {
        this->bits |= other.bits;
        return *this;
    }

    OptionSet operator+(const EnumClass_& other) const
    {
        OptionSet<EnumClass_> result(*this);
        return result += other;
    }

    OptionSet& operator-=(EnumClass_ opt)
    {
        bits[(std::size_t)opt] = false;
        return *this;
    }

    OptionSet operator-(const EnumClass_& other) const
    {
        OptionSet<EnumClass_> result(*this);
        return result -= other;
    }

    bool operator[](EnumClass_ opt) const
    {
        return bits[(std::size_t)opt];
    }

    bool operator==(const OptionSet<EnumClass_>& other) const
    {
        return bits == other.bits;
    }

    bool operator!=(const OptionSet<EnumClass_>& other) const
    {
        return bits != other.bits;
    }

    void Set()
    {
        bits.set();
    }

    void Reset()
    {
        bits.reset();
    }

private:

    std::bitset<Size_> bits;
};

MIKTEX_UTIL_END_NAMESPACE;
