// -*- related-file-name: "../include/efont/t1rw.hh" -*-

/* t1rw.{cc,hh} -- Type 1 font reading and writing
 *
 * Copyright (c) 1998-2019 Eddie Kohler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version. This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#if defined(MIKTEX)
#include <unistd.h>
#endif
#include <efont/t1rw.hh>
#include <efont/t1cs.hh>
#include <lcdf/straccum.hh>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
namespace Efont {

unsigned char Type1Reader::xvalue_store[257];
unsigned char *Type1Reader::xvalue = &Type1Reader::xvalue_store[1];

void
Type1Reader::static_initialize()
{
    if (xvalue['A'])
        return;
    // rely on static data being initialized to 0
    xvalue['0'] = 0;  xvalue['1'] = 1;  xvalue['2'] = 2;  xvalue['3'] = 3;
    xvalue['4'] = 4;  xvalue['5'] = 5;  xvalue['6'] = 6;  xvalue['7'] = 7;
    xvalue['8'] = 8;  xvalue['9'] = 9;
    xvalue['A'] = 10; xvalue['B'] = 11; xvalue['C'] = 12; xvalue['D'] = 13;
    xvalue['E'] = 14; xvalue['F'] = 15;
    xvalue['a'] = 10; xvalue['b'] = 11; xvalue['c'] = 12; xvalue['d'] = 13;
    xvalue['e'] = 14; xvalue['f'] = 15;
}


Type1Reader::Type1Reader()
    : _data(new unsigned char[DATA_SIZE]), _len(0), _pos(0),
      _ungot(-1), _eexec(false)
{
    static_initialize();
}

Type1Reader::~Type1Reader()
{
    delete[] _data;
}


void
Type1Reader::set_charstring_definer(PermString x)
{
    _charstring_definer = x;
}

void
Type1Reader::switch_eexec(bool on, unsigned char *data, int len)
{
    if (on) {
        if (_pos < len + 3) {
            unsigned char *new_data = new unsigned char[len + 3 + DATA_SIZE];
            assert(_len <= DATA_SIZE);
            memcpy(new_data + len + 3, _data + _pos, _len - _pos);
            _len = len + 3 + _len - _pos;
            _pos = len + 3;
            delete[] _data;
            _data = new_data;
        }
        int original_pos = _pos;
        // don't forget _ungot!!
        if (_ungot >= 0)
            _data[--_pos] = _ungot, _ungot = -1;
        if (_crlf == 0 || _crlf == 2)
            _data[--_pos] = '\n';
        if (_crlf == 1 || _crlf == 2)
            _data[--_pos] = '\r';
        memcpy(_data + _pos - len, data, len);
        _pos -= len;
        start_eexec(original_pos - _pos);
    }
    _eexec = on;
}


int
Type1Reader::more_data()
{
    _pos = 0;
    _len = more_data(_data, DATA_SIZE);
    if (_len < 0)
        return -1;
    else
        return _data[_pos++];
}


inline int
Type1Reader::get_base()
{
    if (_pos >= _len)
        return more_data();
    else
        return _data[_pos++];
}


inline int
Type1Reader::eexec(int c)
{
    unsigned char answer = (unsigned char)(c ^ (_r >> 8));
    _r = (((unsigned char)c + _r) * (uint32_t) t1C1 + t1C2) & 0xFFFF;
    return answer;
}


int
Type1Reader::ascii_eexec_get()
{
    int d1 = get_base();
    while (isspace(d1))
        d1 = get_base();

    int d2 = get_base();
    while (isspace(d2))
        d2 = get_base();
    if (d2 < 0)
        return -1;

    return eexec((xvalue[d1] << 4) | (xvalue[d2]));
}


inline int
Type1Reader::get()
{
    if (!_eexec)
        return get_base();

    else if (_binary_eexec) {
        int c = get_base();
        return c < 0 ? c : eexec(c);

    } else
        return ascii_eexec_get();
}


void
Type1Reader::start_eexec(int initial_ascii)
{
    /* God damn this _ungot thing!! It makes sense not to check it on every
       char fetch, since it can only be set at the end of next_line; therefore,
       only the first get() into a user-visible function might need to check
       _ungot. The problem is I forgot start_eexec() was such a function! */
    int c = _ungot < 0 ? get_base() : _ungot;
    initial_ascii--;
    _ungot = -1;

    /* Strictly speaking, I should look for whitespace even in binary sections
       of PFB fonts; but it turns out some PFBs would be unreadable with that
       pedantic rule. */
    while (isspace(c) && (initial_ascii >= 0 || !preserve_whitespace()))
        c = get_base(), initial_ascii--;

    /* Differentiate between ASCII eexec and binary eexec. */
    int rand_bytes[4];
    _binary_eexec = false;
    for (int i = 0; i < 4; i++) {
        if (i)
            c = get_base();
        rand_bytes[i] = c;
        if (!isxdigit(c))
            _binary_eexec = true;
    }

    _r = t1R_ee;
    if (_binary_eexec)
        for (int i = 0; i < 4; i++)
            eexec(rand_bytes[i]);
    else {
        for (int i = 0; i < 2; i++) {
            c = xvalue[rand_bytes[i*2]] * 16 + xvalue[rand_bytes[i*2+1]];
            eexec(c);
        }
        ascii_eexec_get();
        ascii_eexec_get();
    }
}


bool
Type1Reader::test_charstring(StringAccum &str)
{
    /* PERFORMANCE NOTE: This function is definitely a bottleneck. Making it
       more efficient cut time by about 40%. */

    if (!_charstring_definer)
        return false;
    if (_charstring_len >= 0)
        return str.length() <= _charstring_start + _charstring_len;

    str.append('\0');           // protect against running off end of string
    char *s = str.data();
    char *start;
    while (*s == ' ')
        s++;

    if (s[0] == '/')
        s++;
    else if (s[0] == 'd' && s[1] == 'u' && s[2] == 'p' && isspace((unsigned char) s[3])) {
        s += 4;
        // 17.Jan.2000 -- some fonts have extra space here.
        while (isspace((unsigned char) *s))
            s++;
    } else
        goto not_charstring;

    // Force one literal space rather than isspace().
    // Why? Consistency: we force 1 literal space around _charstring_definer.
    while (*s != ' ' && *s)
        s++;
    if (*s++ != ' ' || !isdigit((unsigned char) *s))
        goto not_charstring;
    start = s;
    s++;
    while (*s != ' ' && *s)
        s++;
    if (strncmp(s, _charstring_definer.c_str(), _charstring_definer.length()) != 0)
        goto not_charstring;

    _charstring_len = strtol(start, 0, 10);
    _charstring_start = (s - str.data()) + _charstring_definer.length();
    str.pop_back();
    return str.length() <= _charstring_start + _charstring_len;

  not_charstring:
    str.pop_back();
    return false;
}


/* PERFORMANCE NOTE: All kinds of speedup tricks tried with eexec -- buffering
   (doing eexec processing at more_data time), etc., and all this stuff -- and
   found to have essentially zero effect for PFB fonts. Now, it might be
   effective for PFAs; but who cares? I hate PFAs. */

bool
Type1Reader::next_line(StringAccum &s)
{
    if (_len < 0)
        return false;

    // Can't be a charstring if incoming accumulator has nonzero length.
    _charstring_start = 0;
    _charstring_len = (s.length() > 0 ? 0 : -1);

    int first_char = _ungot < 0 ? get() : _ungot;
    _ungot = -1;

    for (int c = first_char; c >= 0; c = get())
        switch (c) {

          case '\n':
            if (test_charstring(s))
                goto normal;
            else {
                _crlf = 0;
                goto done;
            }

          case '\r':
            // check for \r\n (counts as only one line ending)
            if (test_charstring(s))
                goto normal;
            c = get();
            if (c != '\n' || preserve_whitespace())
                _ungot = c, _crlf = 1;
            else
                _crlf = 2;
            goto done;

          normal:
          default:
            s.append((char)c);
            break;

        }

  done:
    return true;
}


int
Type1Reader::get_data(unsigned char *data, int len)
{
    if (_len < 0)
        return -1;
    if (len <= 0)
        return 0;

    int pos = 0;
    if (_ungot >= 0) {
        *data++ = _ungot;
        pos++;
        _ungot = -1;
    }

    for (; pos < len; pos++) {
        int c = get();
        if (c < 0)
            break;
        *data++ = c;
    }

    return pos;
}


/*****
 * Type1PFAReader
 **/

Type1PFAReader::Type1PFAReader(FILE *f)
    : _f(f)
{
}

int
Type1PFAReader::more_data(unsigned char *data, int len)
{
    size_t size = fread(data, 1, len, _f);
    return size ? (int)size : -1;
}


/*****
 * Type1PFBReader
 **/

Type1PFBReader::Type1PFBReader(FILE *f)
    : _f(f), _binary(false), _left(0)
{
}

int
Type1PFBReader::more_data(unsigned char *data, int len)
{
    while (_left == 0) {
        int c = getc(_f);
        if (c != 128)
            return -1;

        c = getc(_f);
        if (c == 3 || c < 1 || c > 3)
            return -1;
        _binary = (c == 2);

        _left = getc(_f);
        _left |= getc(_f) << 8;
        _left |= getc(_f) << 16;
        _left |= getc(_f) << 24;
    }

    if (_left < 0)
        return -1;

    if (len > _left)
        len = _left;
    _left -= len;

    return fread(data, 1, len, _f);
}

bool
Type1PFBReader::preserve_whitespace() const
{
    return _binary;
}


/*****
 * Type1SubsetReader
 **/

Type1SubsetReader::Type1SubsetReader(Type1Reader *r, int left)
    : _reader(r), _left(left)
{
}

int
Type1SubsetReader::more_data(unsigned char *data, int len)
{
    if (_left <= 0)
        return -1;
    if (len > _left)
        len = _left;
    int how_much = _reader->get_data(data, len);
    if (how_much > 0)
        _left -= how_much;
    return how_much;
}

bool
Type1SubsetReader::preserve_whitespace() const
{
    return _reader->preserve_whitespace();
}


/*****
 * Writer
 **/

Type1Writer::Type1Writer()
    : _buf(new unsigned char[BufSize]), _pos(0),
      _eexec(false), _eexec_start(-1), _eexec_end(-1), _lenIV(4)
{
}


Type1Writer::~Type1Writer()
{
    assert(!_pos);
    delete[] _buf;
}


inline unsigned char
Type1Writer::eexec(int p)
{
    unsigned char c = ((unsigned char)p ^ (_r >> 8)) & 0xFF;
    _r = ((c + _r) * (uint32_t) t1C1 + t1C2) & 0xFFFF;
    return c;
}


void
Type1Writer::flush()
{
    local_flush();
}


/* PERFORMANCE NOTE: Doing eexec processing during flush -- which streamlines
   code for the print() methods -- seems to save some time (4-5%). It also
   makes write performance more consistent. But I have mixed feelings. */

void
Type1Writer::local_flush()
{
    if (_eexec_start >= 0 && _eexec_end < 0)
        _eexec_end = _pos;
    for (int p = _eexec_start; p < _eexec_end; p++)
        _buf[p] = eexec(_buf[p]);
    print0(_buf, _pos);
    _pos = 0;
    _eexec_start = _eexec ? 0 : -1;
    _eexec_end = -1;
}


void
Type1Writer::print(const char *s, int n)
{
    while (n > 0) {
        if (_pos >= BufSize)
            local_flush();
        int copy = BufSize - _pos;
        if (copy > n) copy = n;
        memcpy(_buf + _pos, s, copy);
        _pos += copy;
        s += copy;
        n -= copy;
    }
}


Type1Writer &
Type1Writer::operator<<(int x)
{
    char str[128];
    sprintf(str, "%d", x);
    print(str, strlen(str));
    return *this;
}


Type1Writer &
Type1Writer::operator<<(double x)
{
    char str[256];
    sprintf(str, "%g", x);
    print(str, strlen(str));
    return *this;
}


void
Type1Writer::switch_eexec(bool on)
{
    _eexec = on;
    if (_eexec) {
        _eexec_start = _pos;
        _r = t1R_ee;
        print("SUCK", 4);
    } else
        _eexec_end = _pos;
}


/*****
 * Type1PFAWriter
 **/

Type1PFAWriter::Type1PFAWriter(FILE *f)
    : _f(f), _hex_line(0)
{
}

Type1PFAWriter::~Type1PFAWriter()
{
    flush();
}


void
Type1PFAWriter::switch_eexec(bool on)
{
    flush();
    _hex_line = 0;
    Type1Writer::switch_eexec(on);
}

void
Type1PFAWriter::print0(const unsigned char *c, int l)
{
    if (eexecing()) {
        const char *hex = "0123456789ABCDEF";
        for (; l; c++, l--) {
            putc(hex[*c / 16], _f);
            putc(hex[*c % 16], _f);
            if (++_hex_line == 39) {
                putc('\n', _f);
                _hex_line = 0;
            }
        }
    } else {
        ssize_t result = fwrite(c, 1, l, _f);
        (void) result;
    }
}


/*****
 * Type1PFBWriter
 **/

Type1PFBWriter::Type1PFBWriter(FILE *f)
    : _f(f), _binary(false)
{
}

Type1PFBWriter::~Type1PFBWriter()
{
    flush();
    fputc(128, _f);
    fputc(3, _f);
}

void
Type1PFBWriter::flush()
{
    Type1Writer::flush();
    if (_save.length()) {
        fputc(128, _f);
        fputc(_binary ? 2 : 1, _f);
        long l = _save.length();
        fputc(l & 0xFF, _f);
        fputc((l >> 8) & 0xFF, _f);
        fputc((l >> 16) & 0xFF, _f);
        fputc((l >> 24) & 0xFF, _f);
        ssize_t result = fwrite(_save.data(), 1, _save.length(), _f);
        (void) result;
        _save.clear();
    }
}

void
Type1PFBWriter::switch_eexec(bool on)
{
    flush();
    Type1Writer::switch_eexec(on);
    _binary = on;
}

void
Type1PFBWriter::print0(const unsigned char *c, int l)
{
    char *m = _save.extend(l);
    memcpy(m, c, l);
}

}
