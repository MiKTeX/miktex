/**
 * @file Tokenizer.cpp
 * @author Christian Schenk
 * @brief Tokenizer class
 *
 * @copyright Copyright © 1996-2024 Christian Schenk
 *
 * This file is part of the MiKTeX Util Library.
 *
 * The MiKTeX Util Library is licensed under GNU General Public License version
 * 2 or any later version.
 */

#include <bitset>

#define A7C88F5FBE5C45EB970B3796F331CD89
#include "miktex/Util/config.h"

#if defined(MIKTEX_UTIL_SHARED)
#define MIKTEXUTILEXPORT MIKTEXDLLEXPORT
#else
#define MIKTEXUTILEXPORT
#endif

#include "miktex/Util/CharBuffer.h"
#include "miktex/Util/Tokenizer.h"

#include "internal.h"

using namespace std;

using namespace MiKTeX::Util;

class Tokenizer::impl
{

public:

    CharBuffer<char> buf;
    const char* current = nullptr;
    bitset<256> delims;
    char* next = nullptr;
};

Tokenizer::Tokenizer(const string& s, const string& delims) :
    pimpl(new impl{})
{
    pimpl->buf = s;
    pimpl->next = pimpl->buf.GetData();
    SetDelimiters(delims);
    this->operator++();
}

void Tokenizer::SetDelimiters(const string& delims)
{
    pimpl->delims.reset();
    for (const char& ch : delims)
    {
        pimpl->delims.set(static_cast<unsigned char>(ch));
    }
}

Tokenizer::operator bool() const
{
    return pimpl->current != nullptr && *pimpl->current != 0;
}

string Tokenizer::operator*() const
{
    if (pimpl->current == nullptr)
    {
        // TODO: throw
    }
    return pimpl->current;
}

Tokenizer& Tokenizer::operator++()
{
    // TODO: MIKTEX_ASSERT(pimpl->next != nullptr);
    pimpl->current = pimpl->next;
    while (pimpl->delims[static_cast<unsigned char>(*pimpl->current)] && *pimpl->current != 0)
    {
        ++pimpl->current;
    }
    for (pimpl->next = const_cast<char*>(pimpl->current); *pimpl->next != 0; ++pimpl->next)
    {
        if (pimpl->delims[static_cast<unsigned char>(*pimpl->next)])
        {
            *pimpl->next = 0;
            ++pimpl->next;
            break;
        }
    }
    return *this;
}

Tokenizer::~Tokenizer() noexcept
{
}
