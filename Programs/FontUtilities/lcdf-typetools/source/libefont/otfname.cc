// -*- related-file-name: "../include/efont/otfname.hh" -*-

/* otfname.{cc,hh} -- OpenType name table
 *
 * Copyright (c) 2003-2019 Eddie Kohler
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
#include <efont/otfname.hh>
#include <lcdf/error.hh>
#include <lcdf/straccum.hh>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#define USHORT_AT(d)            (Data::u16_aligned(d))

namespace Efont { namespace OpenType {

static const uint16_t mac_roman_encoding[] = {
                                        // 0x80-0x8F
    0x00C4, 0x00C5, 0x00C7, 0x00C9,     // Adieresis Aring Ccedilla Eacute
    0x00D1, 0x00D6, 0x00DC, 0x00E1,     // Ntilde Odieresis Udieresis aacute
    0x00E0, 0x00E2, 0x00E4, 0x00E3,     // agrave acircumflex adieresis atilde
    0x00E5, 0x00E7, 0x00E9, 0x00E8,     // aring ccedilla eacute egrave
                                        // 0x90-0x9F
    0x00EA, 0x00EB, 0x00ED, 0x00EC,     // ecircumflex edieresis iacute igrave
    0x00EE, 0x00EF, 0x00F1, 0x00F3,     // icircumflex idieresis ntilde oacute
    0x00F2, 0x00F4, 0x00F6, 0x00F5,     // ograve ocircumflex odieresis otilde
    0x00FA, 0x00F9, 0x00FB, 0x00FC,     // uacute ugrave ucircumflex udieresis
                                        // 0xA0-0xAF
    0x2020, 0x00B0, 0x00A2, 0x00A3,     // dagger degree cent sterling
    0x00A7, 0x2022, 0x00B6, 0x00DF,     // section bullet paragraph germandbls
    0x00AE, 0x00A9, 0x2122, 0x00B4,     // registered copyright trademark acute
    0x00A8, 0x2260, 0x00C6, 0x00D8,     // dieresis notequal AE Oslash
                                        // 0xB0-0xBF
    0x221E, 0x00B1, 0x2264, 0x2265,     // infinity plusminus lessequal greaterequal
    0x00A5, 0x00B5, 0x2202, 0x2211,     // yen mu partialdiff summation
    0x220F, 0x03C0, 0x222B, 0x00AA,     // product pi integral ordfeminine
    0x00BA, 0x03A9, 0x00E6, 0x00F8,     // ordmasculine Omegagreek ae oslash
                                        // 0xC0-0xCF
    0x00BF, 0x00A1, 0x00AC, 0x221A,     // questiondown exclamdown logicalnot radical
    0x0192, 0x2248, 0x2206, 0x00AB,     // florin approxequal increment guillemotleft
    0x00BB, 0x2026, 0x00A0, 0x00C0,     // guillemotright ellipsis nbspace Agrave
    0x00C3, 0x00D5, 0x0152, 0x0153,     // Atilde Otilde OE oe
                                        // 0xD0-0xDF
    0x2013, 0x2014, 0x201C, 0x201D,     // endash emdash quotedblleft quotedblright
    0x2018, 0x2019, 0x00F7, 0x25CA,     // quoteleft quoteright divide lozenge
    0x00FF, 0x0178, 0x2044, 0x20AC,     // ydieresis Ydieresis fraction Euro
    0x2039, 0x203A, 0xFB01, 0xFB02,     // guilsinglleft guilsinglright fi fl
                                        // 0xE0-0xEF
    0x2021, 0x00B7, 0x201A, 0x201E,     // daggerdbl middot quotesinglbase quotedblbase
    0x2030, 0x00C2, 0x00CA, 0x00C1,     // perthousand Acircumflex Ecircumflex Aacute
    0x00CB, 0x00C8, 0x00CD, 0x00CE,     // Edieresis Egrave Iacute Icircumflex
    0x00CF, 0x00CC, 0x00D3, 0x00D4,     // Idieresis Igrave Oacute Ocircumflex
                                        // 0xF0-0xFF
    0xF8FF, 0x00D2, 0x00DA, 0x00DB,     // apple Ograve Uacute Ucircumflex
    0x00D9, 0x0131, 0x02C6, 0x02DC,     // Ugrave dotlessi circumflex tilde
    0x00AF, 0x02D8, 0x02D9, 0x02DA,     // macron breve dotaccent ring
    0x00B8, 0x02DD, 0x02DB, 0x02C7      // cedilla hungarumlaut ogonek caron
};


Name::Name(const String &s, ErrorHandler *errh)
    : _str(s) {
    _str.align(2);
    _error = parse_header(errh ? errh : ErrorHandler::silent_handler());
}

int
Name::parse_header(ErrorHandler *errh)
{
    // HEADER FORMAT:
    // USHORT   version
    // USHORT   numTables
    int len = _str.length();
    const uint8_t *data = _str.udata();
    if (len == 0)
        return errh->error("font has no %<name%> table"), -EFAULT;
    if (HEADER_SIZE > len)
        return errh->error("%<name%> table too small"), -EFAULT;
    if (!(data[0] == '\000' && data[1] == '\000'))
        return errh->error("bad %<name%> version number"), -ERANGE;
    int count = USHORT_AT(data + 2);
    if (HEADER_SIZE + count*NAMEREC_SIZE > len)
        return errh->error("%<name%> table too small"), -EFAULT;
    return 0;
}

String
Name::name(const_iterator i) const
{
    if (i < end()) {
        int stringOffset = USHORT_AT(_str.udata() + 4);
        int length = USHORT_AT(reinterpret_cast<const uint8_t *>(i) + 8);
        int offset = USHORT_AT(reinterpret_cast<const uint8_t *>(i) + 10);
        if (stringOffset + offset + length <= _str.length())
            return _str.substring(stringOffset + offset, length);
    }
    return String();
}

String
Name::utf8_name(const_iterator i) const
{
    // This code can handle Microsoft Unicode BMP and Mac Roman encodings,
    // but that's it
    if (!(i < end()))
        return String();
    int stringOffset = USHORT_AT(_str.udata() + 4);
    int length = USHORT_AT(reinterpret_cast<const uint8_t *>(i) + 8);
    int offset = USHORT_AT(reinterpret_cast<const uint8_t *>(i) + 10);
    if (stringOffset + offset + length > _str.length())
        return String();
    const unsigned char *begins = _str.udata() + stringOffset + offset;
    const unsigned char *ends = begins + length;
    if (platform(*i) == P_MICROSOFT && encoding(*i) == E_MS_UNICODE_BMP) {
        StringAccum sa;
        for (const unsigned char *s = begins; s + 1 < ends; s += 2)
            sa.append_utf8(Data::u16(s));
        return sa.take_string();
    } else if (platform(*i) == P_MACINTOSH && encoding(*i) == E_MAC_ROMAN) {
        StringAccum sa;
        for (const unsigned char *s = begins; s < ends; s++)
            if (*s >= 0x80) {
                sa.append(begins, s);
                sa.append_utf8(mac_roman_encoding[*s & 0x7F]);
                begins = s + 1;
            }
        if (!sa)
            return _str.substring(begins, ends);
        else {
            sa.append(begins, ends);
            return sa.take_string();
        }
    } else
        return _str.substring(begins, ends);
}

String
Name::english_name(int nameid) const
{
    const_iterator end = this->end();
    const_iterator it = std::find_if(begin(), end, PlatformPred(nameid, P_MICROSOFT, E_MS_UNICODE_BMP, L_MS_ENGLISH_AMERICAN));
    if (it == end)
        it = std::find_if(begin(), end, PlatformPred(nameid, P_MACINTOSH, E_MAC_ROMAN, 0));
    return utf8_name(it);
}

bool
Name::version_chaincontext_reverse_backtrack() const
{
    String vstr = name(std::find_if(begin(), end(), PlatformPred(N_VERSION, 1, 0, 0)));
    const char *v = vstr.begin(), *endv = vstr.end();
    if (v + 20 <= endv) {
        if (v[0] != 'O' || v[1] != 'T' || v[2] != 'F' || v[3] == ';')
            goto try_core;
        for (v += 4; v < endv && *v != ';'; v++)
            /* do nothing */;
        if (v + 3 >= endv || v[1] != 'P' || v[2] != 'S' || v[3] == ';')
            goto try_core;
        for (v += 4; v < endv && *v != ';'; v++)
            /* do nothing */;
        if (v + 11 >= endv || memcmp(v + 1, "Core 1.0.", 9) != 0
            || (v[10] != '2' && v[10] != '3')
            || (v[11] < '0' || v[11] > '9'))
            goto try_core;
        return true;
    }
 try_core:
    v = vstr.begin();
    if (v + 16 > endv
        || v[0] != 'C' || v[1] != 'o' || v[2] != 'r' || v[3] != 'e')
        return false;
    for (v += 4; v < endv && *v != ';'; v++)
        /* do nothing */;
    if (v + 12 > endv || memcmp(v, ";makeotf.lib", 12) != 0)
        return false;
    return true;
}

} // namespace Efont::OpenType
} // namespace Efont
