//========================================================================
//
// UTF.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2016 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Nelson Benítez León <nbenitezl@gmail.com>
// Copyright (C) 2019, 2020 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef UTF_H
#define UTF_H

#include <cstdint>
#include <climits>

#include "goo/GooString.h"
#include "CharTypes.h"

// Convert a UTF-16 string to a UCS-4
//   utf16      - utf16 bytes
//   utf16_len  - number of UTF-16 characters
//   ucs4_out   - if not NULL, allocates and returns UCS-4 string. Free with gfree.
//   returns number of UCS-4 characters
int UTF16toUCS4(const Unicode *utf16, int utf16Len, Unicode **ucs4_out);

// Convert a PDF Text String to UCS-4
//   s          - PDF text string
//   ucs4       - if the number of UCS-4 characters is > 0, allocates and
//                returns UCS-4 string. Free with gfree.
//   returns number of UCS-4 characters
int TextStringToUCS4(const GooString *textStr, Unicode **ucs4);

// check if UCS-4 character is valid
bool UnicodeIsValid(Unicode ucs4);

// is a unicode whitespace character
bool UnicodeIsWhitespace(Unicode ucs4);

// Count number of UTF-16 code units required to convert a UTF-8 string
// (excluding terminating NULL). Each invalid byte is counted as a
// code point since the UTF-8 conversion functions will replace it with
// REPLACEMENT_CHAR.
int utf8CountUtf16CodeUnits(const char *utf8);

// Convert UTF-8 to UTF-16
//  utf8- UTF-8 string to convert. If not null terminated, set maxUtf8 to num
//        bytes to convert
//  utf16 - output buffer to write UTF-16 to. Output will always be null terminated.
//  maxUtf16 - maximum size of output buffer including space for null.
//  maxUtf8 - maximum number of UTF-8 bytes to convert. Conversion stops when
//            either this count is reached or a null is encountered.
// Returns number of UTF-16 code units written (excluding NULL).
int utf8ToUtf16(const char *utf8, uint16_t *utf16, int maxUtf16 = INT_MAX, int maxUtf8 = INT_MAX);

// Allocate utf16 string and convert utf8 into it.
uint16_t *utf8ToUtf16(const char *utf8, int *len = nullptr);

// Count number of UTF-8 bytes required to convert a UTF-16 string to
// UTF-8 (excluding terminating NULL).
int utf16CountUtf8Bytes(const uint16_t *utf16);

// Convert UTF-16 to UTF-8
//  utf16- UTF-16 string to convert. If not null terminated, set maxUtf16 to num
//        code units to convert
//  utf8 - output buffer to write UTF-8 to. Output will always be null terminated.
//  maxUtf8 - maximum size of output buffer including space for null.
//  maxUtf16 - maximum number of UTF-16 code units to convert. Conversion stops when
//            either this count is reached or a null is encountered.
// Returns number of UTF-8 bytes written (excluding NULL).
int utf16ToUtf8(const uint16_t *utf16, char *utf8, int maxUtf8 = INT_MAX, int maxUtf16 = INT_MAX);

// Allocate utf8 string and convert utf16 into it.
char *utf16ToUtf8(const uint16_t *utf16, int *len = nullptr);

// Convert a UCS-4 string to pure ASCII (7bit)
//   in       - UCS-4 string bytes
//   len      - number of UCS-4 characters
//   ucs4_out - if not NULL, allocates and returns UCS-4 string. Free with gfree.
//   out_len  - number of UCS-4 characters in ucs4_out.
//   in_idx   - if not NULL, the int array returned by the out fourth parameter of
//              unicodeNormalizeNFKC() function. Optional, needed for @indices out parameter.
//   indices  - if not NULL, @indices is assigned the location of a newly-allocated array
//              of length @out_len + 1, for each character in the ascii string giving the index
//              of the corresponding character in the text of the line (thanks to this info
//              being passed in @in_idx parameter).
void unicodeToAscii7(const Unicode *in, int len, Unicode **ucs4_out, int *out_len, const int *in_idx, int **indices);

#endif
