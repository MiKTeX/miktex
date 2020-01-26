// -*- related-file-name: "../include/lcdf/straccum.hh" -*-
/*
 * straccum.{cc,hh} -- build up strings with operator<<
 * Eddie Kohler
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2001-2019 Eddie Kohler
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <lcdf/straccum.hh>
#include <lcdf/vector.hh>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

/** @class StringAccum
 * @brief Efficiently build up Strings from pieces.
 *
 * Like the String class, StringAccum represents a string of characters.
 * However, unlike a String, a StringAccum is inherently mutable, and
 * efficiently supports building up a large string from many smaller pieces.
 *
 * StringAccum objects support operator<<() operations for most fundamental
 * data types.  A StringAccum is generally built up by operator<<(), and then
 * turned into a String by the take_string() method.  Extracting the String
 * from a StringAccum does no memory allocation or copying; the StringAccum's
 * memory is donated to the String.
 *
 * <h3>Out-of-memory StringAccums</h3>
 *
 * When there is not enough memory to add requested characters to a
 * StringAccum object, the object becomes a special "out-of-memory"
 * StringAccum.  Out-of-memory objects are contagious: the result of any
 * concatenation operation involving an out-of-memory StringAccum is another
 * out-of-memory StringAccum.  Appending an out-of-memory String with "sa <<
 * s" makes "sa" out-of-memory.  (However, other usually-equivalent calls,
 * such as "sa.append(s.begin(), s.end())", <em>do not</em> make "sa"
 * out-of-memory.)  Calling take_string() on an out-of-memory StringAccum
 * returns an out-of-memory String.
 *
 * Out-of-memory StringAccum objects have length zero.
 */


void
StringAccum::assign_out_of_memory()
{
    assert(_cap >= 0);
    if (_cap > 0)
        delete[] (_s - MEMO_SPACE);
    _s = reinterpret_cast<unsigned char *>(const_cast<char *>(String::out_of_memory_data()));
    _cap = -1;
    _len = 0;
}

char *
StringAccum::grow(int want)
{
    // can't append to out-of-memory strings
    if (_cap < 0) {
        errno = ENOMEM;
        return 0;
    }

    int ncap = (_cap ? (_cap + MEMO_SPACE) * 2 : 128) - MEMO_SPACE;
    while (ncap <= want)
        ncap = (ncap + MEMO_SPACE) * 2 - MEMO_SPACE;

    unsigned char *n = new unsigned char[ncap + MEMO_SPACE];
    if (!n) {
        assign_out_of_memory();
        errno = ENOMEM;
        return 0;
    }
    n += MEMO_SPACE;

    if (_s) {
        memcpy(n, _s, _len);
        delete[] (_s - MEMO_SPACE);
    }
    _s = n;
    _cap = ncap;
    return reinterpret_cast<char *>(_s + _len);
}

int
StringAccum::resize(int len)
{
    assert(len >= 0);
    if (len > _cap && !grow(len))
        return -ENOMEM;
    else {
        _len = len;
        return 0;
    }
}

char *
StringAccum::hard_extend(int nadjust, int nreserve)
{
    char *x = grow(_len + nadjust + nreserve);
    if (x)
        _len += nadjust;
    return x;
}

const char *
StringAccum::c_str()
{
    if (_len < _cap || grow(_len))
        _s[_len] = '\0';
    return reinterpret_cast<char *>(_s);
}

void
StringAccum::append_fill(int c, int len)
{
    if (char *s = extend(len))
        memset(s, c, len);
}

void
StringAccum::append_utf8_hard(unsigned ch)
{
    if (ch < 0x80)
        append((unsigned char) ch);
    else if (ch < 0x800) {
        append((unsigned char) (0xC0 + ((ch >> 6) & 0x1F)));
        append((unsigned char) (0x80 + (ch & 0x3F)));
    } else if (ch < 0x10000) {
        append((unsigned char) (0xE0 + ((ch >> 12) & 0x0F)));
        append((unsigned char) (0x80 + ((ch >> 6) & 0x3F)));
        append((unsigned char) (0x80 + (ch & 0x3F)));
    } else if (ch < 0x110000) {
        append((unsigned char) (0xF0 + ((ch >> 18) & 0x07)));
        append((unsigned char) (0x80 + ((ch >> 12) & 0x3F)));
        append((unsigned char) (0x80 + ((ch >> 6) & 0x3F)));
        append((unsigned char) (0x80 + (ch & 0x3F)));
    } else
        append((unsigned char) '?');
}

void
StringAccum::hard_append(const char *s, int len)
{
    // We must be careful about calls like "sa.append(sa.begin(), sa.end())";
    // a naive implementation might use sa's data after freeing it.
    const char *my_s = reinterpret_cast<char *>(_s);

    if (len <= 0) {
        // do nothing
    } else if (_len + len <= _cap) {
    success:
        memcpy(_s + _len, s, len);
        _len += len;
    } else if (s < my_s || s >= my_s + _cap) {
        if (grow(_len + len))
            goto success;
    } else {
        unsigned char *old_s = _s;
        int old_len = _len;

        _s = 0;
        _len = 0;
        _cap = 0;

        if (char *new_s = extend(old_len + len)) {
            memcpy(new_s, old_s, old_len);
            memcpy(new_s + old_len, s, len);
        }

        delete[] (old_s - MEMO_SPACE);
    }
}

void
StringAccum::append(const char *s)
{
    hard_append(s, strlen(s));
}

String
StringAccum::take_string()
{
    int len = length();
    int cap = _cap;
    char *str = reinterpret_cast<char *>(_s);
    if (len > 0) {
        _s = 0;
        _len = _cap = 0;
        return String::make_claim(str, len, cap);
    } else if (!out_of_memory())
        return String();
    else {
        clear();
        return String::make_out_of_memory();
    }
}

void
StringAccum::swap(StringAccum &o)
{
    unsigned char *os = o._s;
    int olen = o._len, ocap = o._cap;
    o._s = _s;
    o._len = _len, o._cap = _cap;
    _s = os;
    _len = olen, _cap = ocap;
}

/** @relates StringAccum
    @brief Append decimal representation of @a i to @a sa.
    @return @a sa */
StringAccum &
operator<<(StringAccum &sa, long i)
{
    if (char *x = sa.reserve(24)) {
        int len = sprintf(x, "%ld", i);
        sa.adjust_length(len);
    }
    return sa;
}

/** @relates StringAccum
    @brief Append decimal representation of @a u to @a sa.
    @return @a sa */
StringAccum &
operator<<(StringAccum &sa, unsigned long u)
{
    if (char *x = sa.reserve(24)) {
        int len = sprintf(x, "%lu", u);
        sa.adjust_length(len);
    }
    return sa;
}

StringAccum &
operator<<(StringAccum &sa, double d)
{
    if (char *x = sa.reserve(256)) {
        int len = sprintf(x, "%.12g", d);
        sa.adjust_length(len);
    }
    return sa;
}

StringAccum &
StringAccum::snprintf(int n, const char *format, ...)
{
    va_list val;
    va_start(val, format);
    if (char *x = reserve(n + 1)) {
#if HAVE_VSNPRINTF
        int len = vsnprintf(x, n + 1, format, val);
#else
        int len = vsprintf(x, format, val);
        assert(len <= n);
#endif
        adjust_length(len);
    }
    va_end(val);
    return *this;
}

void
StringAccum::append_break_lines(const String& text, int linelen, const String &leftmargin)
{
    if (text.length() == 0)
        return;
    const char* line = text.begin();
    const char* ends = text.end();
    linelen -= leftmargin.length();
    for (const char* s = line; s < ends; s++) {
        const char* start = s;
        while (s < ends && isspace((unsigned char) *s))
            s++;
        const char* word = s;
        while (s < ends && !isspace((unsigned char) *s))
            s++;
        if (s - line > linelen && start > line) {
            *this << leftmargin;
            append(line, start - line);
            *this << '\n';
            line = word;
        }
    }
    if (line < text.end()) {
        *this << leftmargin;
        append(line, text.end() - line);
        *this << '\n';
    }
}
