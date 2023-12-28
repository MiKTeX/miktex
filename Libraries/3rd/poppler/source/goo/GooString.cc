//========================================================================
//
// GooString.cc
//
// Simple variable-length string type.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2006 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2008-2011, 2016-2018, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Kenji Uno <ku@digitaldolphins.jp>
// Copyright (C) 2012, 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Pino Toscano <pino@kde.org>
// Copyright (C) 2013 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2015 William Bader <williambader@hotmail.com>
// Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2018 Greg Knight <lyngvi@gmail.com>
// Copyright (C) 2019, 2022, 2023 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2023 Even Rouault <even.rouault@mines-paris.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>

#include "gmem.h"
#include "Error.h"
#include "GooString.h"

//------------------------------------------------------------------------

namespace {

union GooStringFormatArg {
    int i;
    unsigned int ui;
    long l;
    unsigned long ul;
    long long ll;
    unsigned long long ull;
    double f;
    char c;
    char *s;
    GooString *gs;
};

enum GooStringFormatType
{
    fmtIntDecimal,
    fmtIntHex,
    fmtIntHexUpper,
    fmtIntOctal,
    fmtIntBinary,
    fmtUIntDecimal,
    fmtUIntHex,
    fmtUIntHexUpper,
    fmtUIntOctal,
    fmtUIntBinary,
    fmtLongDecimal,
    fmtLongHex,
    fmtLongHexUpper,
    fmtLongOctal,
    fmtLongBinary,
    fmtULongDecimal,
    fmtULongHex,
    fmtULongHexUpper,
    fmtULongOctal,
    fmtULongBinary,
    fmtLongLongDecimal,
    fmtLongLongHex,
    fmtLongLongHexUpper,
    fmtLongLongOctal,
    fmtLongLongBinary,
    fmtULongLongDecimal,
    fmtULongLongHex,
    fmtULongLongHexUpper,
    fmtULongLongOctal,
    fmtULongLongBinary,
    fmtDouble,
    fmtDoubleTrimSmallAware,
    fmtDoubleTrim,
    fmtChar,
    fmtString,
    fmtGooString,
    fmtSpace
};

const char *const formatStrings[] = { "d",   "x",   "X",   "o",   "b",   "ud",  "ux",   "uX",   "uo",   "ub",   "ld",   "lx", "lX", "lo", "lb", "uld", "ulx", "ulX", "ulo",
                                      "ulb", "lld", "llx", "llX", "llo", "llb", "ulld", "ullx", "ullX", "ullo", "ullb", "f",  "gs", "g",  "c",  "s",   "t",   "w",   nullptr };

void formatInt(long long x, char *buf, int bufSize, bool zeroFill, int width, int base, const char **p, int *len, bool upperCase = false);

void formatUInt(unsigned long long x, char *buf, int bufSize, bool zeroFill, int width, int base, const char **p, int *len, bool upperCase = false);

void formatDouble(double x, char *buf, int bufSize, int prec, bool trim, const char **p, int *len);

void formatDoubleSmallAware(double x, char *buf, int bufSize, int prec, bool trim, const char **p, int *len);

}

//------------------------------------------------------------------------

std::unique_ptr<GooString> GooString::format(const char *fmt, ...)
{
    auto s = std::make_unique<GooString>();

    va_list argList;
    va_start(argList, fmt);
    s->appendfv(fmt, argList);
    va_end(argList);

    return s;
}

std::unique_ptr<GooString> GooString::formatv(const char *fmt, va_list argList)
{
    auto s = std::make_unique<GooString>();

    s->appendfv(fmt, argList);

    return s;
}

GooString *GooString::appendf(const char *fmt, ...)
{
    va_list argList;
    va_start(argList, fmt);
    appendfv(fmt, argList);
    va_end(argList);

    return this;
}

GooString *GooString::appendfv(const char *fmt, va_list argList)
{
    GooStringFormatArg *args;
    int argsLen, argsSize;
    GooStringFormatArg arg;
    int idx, width, prec;
    bool reverseAlign, zeroFill;
    GooStringFormatType ft;
    char buf[65];
    int len, i;
    const char *p0, *p1;
    const char *str;
    GooStringFormatArg argsBuf[8];

    argsLen = 0;
    argsSize = sizeof(argsBuf) / sizeof(argsBuf[0]);
    args = argsBuf;

    p0 = fmt;
    while (*p0) {
        if (*p0 == '{') {
            ++p0;
            if (*p0 == '{') {
                ++p0;
                append('{');
            } else {

                // parse the format string
                if (!(*p0 >= '0' && *p0 <= '9')) {
                    break;
                }
                idx = *p0 - '0';
                for (++p0; *p0 >= '0' && *p0 <= '9'; ++p0) {
                    idx = 10 * idx + (*p0 - '0');
                }
                if (*p0 != ':') {
                    break;
                }
                ++p0;
                if (*p0 == '-') {
                    reverseAlign = true;
                    ++p0;
                } else {
                    reverseAlign = false;
                }
                width = 0;
                zeroFill = *p0 == '0';
                for (; *p0 >= '0' && *p0 <= '9'; ++p0) {
                    width = 10 * width + (*p0 - '0');
                }
                if (width < 0) {
                    width = 0;
                }
                if (*p0 == '.') {
                    ++p0;
                    prec = 0;
                    for (; *p0 >= '0' && *p0 <= '9'; ++p0) {
                        prec = 10 * prec + (*p0 - '0');
                    }
                } else {
                    prec = 0;
                }
                for (ft = (GooStringFormatType)0; formatStrings[ft]; ft = (GooStringFormatType)(ft + 1)) {
                    if (!strncmp(p0, formatStrings[ft], strlen(formatStrings[ft]))) {
                        break;
                    }
                }
                if (!formatStrings[ft]) {
                    break;
                }
                p0 += strlen(formatStrings[ft]);
                if (*p0 != '}') {
                    break;
                }
                ++p0;

                // fetch the argument
                if (idx > argsLen) {
                    break;
                }
                if (idx == argsLen) {
                    if (argsLen == argsSize) {
                        argsSize *= 2;
                        if (args == argsBuf) {
                            args = (GooStringFormatArg *)gmallocn(argsSize, sizeof(GooStringFormatArg));
                            memcpy(args, argsBuf, argsLen * sizeof(GooStringFormatArg));
                        } else {
                            args = (GooStringFormatArg *)greallocn(args, argsSize, sizeof(GooStringFormatArg));
                        }
                    }
                    switch (ft) {
                    case fmtIntDecimal:
                    case fmtIntHex:
                    case fmtIntHexUpper:
                    case fmtIntOctal:
                    case fmtIntBinary:
                    case fmtSpace:
                        args[argsLen].i = va_arg(argList, int);
                        break;
                    case fmtUIntDecimal:
                    case fmtUIntHex:
                    case fmtUIntHexUpper:
                    case fmtUIntOctal:
                    case fmtUIntBinary:
                        args[argsLen].ui = va_arg(argList, unsigned int);
                        break;
                    case fmtLongDecimal:
                    case fmtLongHex:
                    case fmtLongHexUpper:
                    case fmtLongOctal:
                    case fmtLongBinary:
                        args[argsLen].l = va_arg(argList, long);
                        break;
                    case fmtULongDecimal:
                    case fmtULongHex:
                    case fmtULongHexUpper:
                    case fmtULongOctal:
                    case fmtULongBinary:
                        args[argsLen].ul = va_arg(argList, unsigned long);
                        break;
                    case fmtLongLongDecimal:
                    case fmtLongLongHex:
                    case fmtLongLongHexUpper:
                    case fmtLongLongOctal:
                    case fmtLongLongBinary:
                        args[argsLen].ll = va_arg(argList, long long);
                        break;
                    case fmtULongLongDecimal:
                    case fmtULongLongHex:
                    case fmtULongLongHexUpper:
                    case fmtULongLongOctal:
                    case fmtULongLongBinary:
                        args[argsLen].ull = va_arg(argList, unsigned long long);
                        break;
                    case fmtDouble:
                    case fmtDoubleTrim:
                    case fmtDoubleTrimSmallAware:
                        args[argsLen].f = va_arg(argList, double);
                        break;
                    case fmtChar:
                        args[argsLen].c = (char)va_arg(argList, int);
                        break;
                    case fmtString:
                        args[argsLen].s = va_arg(argList, char *);
                        break;
                    case fmtGooString:
                        args[argsLen].gs = va_arg(argList, GooString *);
                        break;
                    }
                    ++argsLen;
                }

                // format the argument
                arg = args[idx];
                switch (ft) {
                case fmtIntDecimal:
                    formatInt(arg.i, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtIntHex:
                    formatInt(arg.i, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtIntHexUpper:
                    formatInt(arg.i, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtIntOctal:
                    formatInt(arg.i, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtIntBinary:
                    formatInt(arg.i, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtUIntDecimal:
                    formatUInt(arg.ui, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtUIntHex:
                    formatUInt(arg.ui, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtUIntHexUpper:
                    formatUInt(arg.ui, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtUIntOctal:
                    formatUInt(arg.ui, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtUIntBinary:
                    formatUInt(arg.ui, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtLongDecimal:
                    formatInt(arg.l, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtLongHex:
                    formatInt(arg.l, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtLongHexUpper:
                    formatInt(arg.l, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtLongOctal:
                    formatInt(arg.l, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtLongBinary:
                    formatInt(arg.l, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtULongDecimal:
                    formatUInt(arg.ul, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtULongHex:
                    formatUInt(arg.ul, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtULongHexUpper:
                    formatUInt(arg.ul, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtULongOctal:
                    formatUInt(arg.ul, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtULongBinary:
                    formatUInt(arg.ul, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtLongLongDecimal:
                    formatInt(arg.ll, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtLongLongHex:
                    formatInt(arg.ll, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtLongLongHexUpper:
                    formatInt(arg.ll, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtLongLongOctal:
                    formatInt(arg.ll, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtLongLongBinary:
                    formatInt(arg.ll, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtULongLongDecimal:
                    formatUInt(arg.ull, buf, sizeof(buf), zeroFill, width, 10, &str, &len);
                    break;
                case fmtULongLongHex:
                    formatUInt(arg.ull, buf, sizeof(buf), zeroFill, width, 16, &str, &len);
                    break;
                case fmtULongLongHexUpper:
                    formatUInt(arg.ull, buf, sizeof(buf), zeroFill, width, 16, &str, &len, true);
                    break;
                case fmtULongLongOctal:
                    formatUInt(arg.ull, buf, sizeof(buf), zeroFill, width, 8, &str, &len);
                    break;
                case fmtULongLongBinary:
                    formatUInt(arg.ull, buf, sizeof(buf), zeroFill, width, 2, &str, &len);
                    break;
                case fmtDouble:
                    formatDouble(arg.f, buf, sizeof(buf), prec, false, &str, &len);
                    break;
                case fmtDoubleTrim:
                    formatDouble(arg.f, buf, sizeof(buf), prec, true, &str, &len);
                    break;
                case fmtDoubleTrimSmallAware:
                    formatDoubleSmallAware(arg.f, buf, sizeof(buf), prec, true, &str, &len);
                    break;
                case fmtChar:
                    buf[0] = arg.c;
                    str = buf;
                    len = 1;
                    reverseAlign = !reverseAlign;
                    break;
                case fmtString: {
                    str = arg.s;
                    const size_t strlen_str = strlen(str);
                    if (strlen_str > static_cast<size_t>(std::numeric_limits<int>::max())) {
                        error(errSyntaxWarning, 0, "String truncated to INT_MAX bytes");
                        len = std::numeric_limits<int>::max();
                    } else {
                        len = static_cast<int>(strlen_str);
                    }
                    reverseAlign = !reverseAlign;
                    break;
                }
                case fmtGooString:
                    if (arg.gs) {
                        str = arg.gs->c_str();
                        len = arg.gs->getLength();
                    } else {
                        str = "(null)";
                        len = 6;
                    }
                    reverseAlign = !reverseAlign;
                    break;
                case fmtSpace:
                    str = buf;
                    len = 0;
                    width = arg.i;
                    break;
                }

                // append the formatted arg, handling width and alignment
                if (!reverseAlign && len < width) {
                    for (i = len; i < width; ++i) {
                        append(' ');
                    }
                }
                append(str, len);
                if (reverseAlign && len < width) {
                    for (i = len; i < width; ++i) {
                        append(' ');
                    }
                }
            }

        } else if (*p0 == '}') {
            ++p0;
            if (*p0 == '}') {
                ++p0;
            }
            append('}');

        } else {
            for (p1 = p0 + 1; *p1 && *p1 != '{' && *p1 != '}'; ++p1) {
                ;
            }
            append(p0, p1 - p0);
            p0 = p1;
        }
    }

    if (args != argsBuf) {
        gfree(args);
    }

    return this;
}

namespace {

const char lowerCaseDigits[17] = "0123456789abcdef";
const char upperCaseDigits[17] = "0123456789ABCDEF";

void formatInt(long long x, char *buf, int bufSize, bool zeroFill, int width, int base, const char **p, int *len, bool upperCase)
{
    const char *vals = upperCase ? upperCaseDigits : lowerCaseDigits;
    bool neg;
    int start, i, j;
    unsigned long long abs_x;

    i = bufSize;
    if ((neg = x < 0)) {
        abs_x = -x;
    } else {
        abs_x = x;
    }
    start = neg ? 1 : 0;
    if (abs_x == 0) {
        buf[--i] = '0';
    } else {
        while (i > start && abs_x) {
            buf[--i] = vals[abs_x % base];
            abs_x /= base;
        }
    }
    if (zeroFill) {
        for (j = bufSize - i; i > start && j < width - start; ++j) {
            buf[--i] = '0';
        }
    }
    if (neg) {
        buf[--i] = '-';
    }
    *p = buf + i;
    *len = bufSize - i;
}

void formatUInt(unsigned long long x, char *buf, int bufSize, bool zeroFill, int width, int base, const char **p, int *len, bool upperCase)
{
    const char *vals = upperCase ? upperCaseDigits : lowerCaseDigits;
    int i, j;

    i = bufSize;
    if (x == 0) {
        buf[--i] = '0';
    } else {
        while (i > 0 && x) {
            buf[--i] = vals[x % base];
            x /= base;
        }
    }
    if (zeroFill) {
        for (j = bufSize - i; i > 0 && j < width; ++j) {
            buf[--i] = '0';
        }
    }
    *p = buf + i;
    *len = bufSize - i;
}

void formatDouble(double x, char *buf, int bufSize, int prec, bool trim, const char **p, int *len)
{
    bool neg, started;
    double x2;
    int d, i, j;

    if ((neg = x < 0)) {
        x = -x;
    }
    x = floor(x * pow(10.0, prec) + 0.5);
    i = bufSize;
    started = !trim;
    for (j = 0; j < prec && i > 1; ++j) {
        x2 = floor(0.1 * (x + 0.5));
        d = (int)floor(x - 10 * x2 + 0.5);
        if (started || d != 0) {
            buf[--i] = '0' + d;
            started = true;
        }
        x = x2;
    }
    if (i > 1 && started) {
        buf[--i] = '.';
    }
    if (i > 1) {
        do {
            x2 = floor(0.1 * (x + 0.5));
            d = (int)floor(x - 10 * x2 + 0.5);
            buf[--i] = '0' + d;
            x = x2;
        } while (i > 1 && x);
    }
    if (neg) {
        buf[--i] = '-';
    }
    *p = buf + i;
    *len = bufSize - i;
}

void formatDoubleSmallAware(double x, char *buf, int bufSize, int prec, bool trim, const char **p, int *len)
{
    double absX = fabs(x);
    if (absX >= 0.1) {
        formatDouble(x, buf, bufSize, prec, trim, p, len);
    } else {
        while (absX < 0.1 && prec < 16) {
            absX = absX * 10;
            prec++;
        }
        formatDouble(x, buf, bufSize, prec, trim, p, len);
    }
}

}

GooString *GooString::lowerCase()
{
    lowerCase(*this);
    return this;
}

void GooString::lowerCase(std::string &s)
{
    for (auto &c : s) {
        if (std::isupper(c)) {
            c = std::tolower(c);
        }
    }
}

std::string GooString::toLowerCase(const std::string &s)
{
    std::string newString = s;
    lowerCase(newString);
    return s;
}

void GooString::prependUnicodeMarker()
{
    insert(0, "\xFE\xFF", 2);
}

bool GooString::startsWith(const char *prefix) const
{
    return startsWith(toStr(), prefix);
}

bool GooString::endsWith(const char *suffix) const
{
    return endsWith(toStr(), suffix);
}
