//========================================================================
//
// CharCodeToUnicode.cc
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006, 2008-2010, 2012, 2018-2022, 2024 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2007 Julien Rebetez <julienr@svn.gnome.org>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2008 Michael Vrable <mvrable@cs.ucsd.edu>
// Copyright (C) 2008 Vasile Gaburici <gaburici@cs.umd.edu>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2014 Jiri Slaby <jirislaby@gmail.com>
// Copyright (C) 2015 Marek Kasik <mkasik@redhat.com>
// Copyright (C) 2017 Jean Ghali <jghali@libertysurf.fr>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 <corentinf@free.fr>
// Copyright (C) 2024 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#include <cstring>
#include <functional>
#include "goo/glibc.h"
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "goo/GooLikely.h"
#include "goo/GooString.h"
#include "Error.h"
#include "GlobalParams.h"
#include "PSTokenizer.h"
#include "CharCodeToUnicode.h"
#include "UTF.h"

//------------------------------------------------------------------------

//------------------------------------------------------------------------

static int getCharFromString(void *data)
{
    unsigned char *p;
    int c;

    p = *(unsigned char **)data;
    if (*p) {
        c = *p++;
        *(unsigned char **)data = p;
    } else {
        c = EOF;
    }
    return c;
}

static int getCharFromFile(void *data)
{
    return fgetc((FILE *)data);
}

//------------------------------------------------------------------------

static const int hexCharVals[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 1x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 2x
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  -1, -1, -1, -1, -1, -1, // 3x
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 4x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 5x
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 6x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 7x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 8x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 9x
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // Ax
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // Bx
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // Cx
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // Dx
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // Ex
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 // Fx
};

// Parse a <len>-byte hex string <s> into *<val>.  Returns false on
// error.
static bool parseHex(const char *s, int len, unsigned int *val)
{
    int i, x, v = 0;

    for (i = 0; i < len; ++i) {
        x = hexCharVals[s[i] & 0xff];
        if (x < 0) {
            *val = 0;
            return false;
        }
        v = (v << 4) + x;
    }
    *val = v;
    return true;
}

//------------------------------------------------------------------------

CharCodeToUnicode *CharCodeToUnicode::makeIdentityMapping()
{
    CharCodeToUnicode *ctu = new CharCodeToUnicode();
    ctu->isIdentity = true;
    ctu->map.resize(1, 0);
    return ctu;
}
CharCodeToUnicode *CharCodeToUnicode::parseCIDToUnicode(const char *fileName, const GooString *collection)
{
    FILE *f;
    CharCode size;
    char buf[64];
    Unicode u;

    if (!(f = openFile(fileName, "r"))) {
        error(errIO, -1, "Couldn't open cidToUnicode file '{0:s}'", fileName);
        return nullptr;
    }

    size = 32768;
    std::vector<Unicode> mapA;
    mapA.resize(size, 0);
    CharCode mapLenA = 0;

    while (getLine(buf, sizeof(buf), f)) {
        if (mapLenA == size) {
            size *= 2;
            mapA.resize(size);
        }
        if (sscanf(buf, "%x", &u) == 1) {
            mapA[mapLenA] = u;
        } else {
            error(errSyntaxWarning, -1, "Bad line ({0:d}) in cidToUnicode file '{1:s}'", (int)(mapLenA + 1), fileName);
            mapA[mapLenA] = 0;
        }
        ++mapLenA;
    }
    fclose(f);
    mapA.resize(mapLenA);

    return new CharCodeToUnicode(collection->toStr(), std::move(mapA), {});
}

CharCodeToUnicode *CharCodeToUnicode::make8BitToUnicode(Unicode *toUnicode)
{
    std::vector<Unicode> data(toUnicode, toUnicode + 256);
    return new CharCodeToUnicode({}, std::move(data), {});
}

CharCodeToUnicode *CharCodeToUnicode::parseCMap(const GooString *buf, int nBits)
{
    CharCodeToUnicode *ctu;

    ctu = new CharCodeToUnicode(std::optional<std::string>());
    const char *p = buf->c_str();
    if (!ctu->parseCMap1(&getCharFromString, &p, nBits)) {
        delete ctu;
        return nullptr;
    }
    return ctu;
}

CharCodeToUnicode *CharCodeToUnicode::parseCMapFromFile(const GooString *fileName, int nBits)
{
    CharCodeToUnicode *ctu;
    FILE *f;

    ctu = new CharCodeToUnicode(std::optional<std::string>());
    if ((f = globalParams->findToUnicodeFile(fileName))) {
        if (!ctu->parseCMap1(&getCharFromFile, f, nBits)) {
            delete ctu;
            fclose(f);
            return nullptr;
        }
    } else {
        error(errSyntaxError, -1, "Couldn't find ToUnicode CMap file for '{0:t}'", fileName);
    }
    return ctu;
}

void CharCodeToUnicode::mergeCMap(const GooString *buf, int nBits)
{
    const char *p = buf->c_str();
    parseCMap1(&getCharFromString, &p, nBits);
}

bool CharCodeToUnicode::parseCMap1(int (*getCharFunc)(void *), void *data, int nBits)
{
    PSTokenizer *pst;
    char tok1[256], tok2[256], tok3[256];
    int n1, n2, n3;
    CharCode i;
    CharCode maxCode, code1, code2;
    GooString *name;
    FILE *f;

    bool ok = false;
    maxCode = (nBits == 8) ? 0xff : (nBits == 16) ? 0xffff : 0xffffffff;
    pst = new PSTokenizer(getCharFunc, data);
    pst->getToken(tok1, sizeof(tok1), &n1);
    while (pst->getToken(tok2, sizeof(tok2), &n2)) {
        if (!strcmp(tok2, "usecmap")) {
            if (tok1[0] == '/') {
                name = new GooString(tok1 + 1);
                if ((f = globalParams->findToUnicodeFile(name))) {
                    if (parseCMap1(&getCharFromFile, f, nBits)) {
                        ok = true;
                    }
                    fclose(f);
                } else {
                    error(errSyntaxError, -1, "Couldn't find ToUnicode CMap file for '{0:t}'", name);
                }
                delete name;
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "beginbfchar")) {
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endbfchar")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endbfchar")) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfchar block in ToUnicode CMap");
                    break;
                }
                if (!(tok1[0] == '<' && tok1[n1 - 1] == '>' && tok2[0] == '<' && tok2[n2 - 1] == '>')) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfchar block in ToUnicode CMap");
                    continue;
                }
                tok1[n1 - 1] = tok2[n2 - 1] = '\0';
                if (!parseHex(tok1 + 1, n1 - 2, &code1)) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfchar block in ToUnicode CMap");
                    continue;
                }
                if (code1 > maxCode) {
                    error(errSyntaxWarning, -1, "Invalid entry in bfchar block in ToUnicode CMap");
                }
                addMapping(code1, tok2 + 1, n2 - 2, 0);
                ok = true;
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "beginbfrange")) {
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endbfrange")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endbfrange") || !pst->getToken(tok3, sizeof(tok3), &n3) || !strcmp(tok3, "endbfrange")) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfrange block in ToUnicode CMap");
                    break;
                }
                if (!(tok1[0] == '<' && tok1[n1 - 1] == '>' && tok2[0] == '<' && tok2[n2 - 1] == '>')) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfrange block in ToUnicode CMap");
                    continue;
                }
                tok1[n1 - 1] = tok2[n2 - 1] = '\0';
                if (!parseHex(tok1 + 1, n1 - 2, &code1) || !parseHex(tok2 + 1, n2 - 2, &code2)) {
                    error(errSyntaxWarning, -1, "Illegal entry in bfrange block in ToUnicode CMap");
                    continue;
                }
                if (code1 > maxCode || code2 > maxCode) {
                    error(errSyntaxWarning, -1, "Invalid entry in bfrange block in ToUnicode CMap");
                    if (code1 > maxCode) {
                        code1 = maxCode;
                    }
                    if (code2 > maxCode) {
                        code2 = maxCode;
                    }
                }
                if (!strcmp(tok3, "[")) {
                    i = 0;
                    while (pst->getToken(tok1, sizeof(tok1), &n1) && code1 + i <= code2) {
                        if (!strcmp(tok1, "]")) {
                            break;
                        }
                        if (tok1[0] == '<' && tok1[n1 - 1] == '>') {
                            tok1[n1 - 1] = '\0';
                            addMapping(code1 + i, tok1 + 1, n1 - 2, 0);
                            ok = true;
                        } else {
                            error(errSyntaxWarning, -1, "Illegal entry in bfrange block in ToUnicode CMap");
                        }
                        ++i;
                    }
                } else if (tok3[0] == '<' && tok3[n3 - 1] == '>') {
                    tok3[n3 - 1] = '\0';
                    for (i = 0; code1 <= code2; ++code1, ++i) {
                        addMapping(code1, tok3 + 1, n3 - 2, i);
                        ok = true;
                    }

                } else {
                    error(errSyntaxWarning, -1, "Illegal entry in bfrange block in ToUnicode CMap");
                }
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "begincidchar")) {
            // the begincidchar operator is not allowed in ToUnicode CMaps,
            // but some buggy PDF generators incorrectly use
            // code-to-CID-type CMaps here
            error(errSyntaxWarning, -1, "Invalid 'begincidchar' operator in ToUnicode CMap");
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endcidchar")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endcidchar")) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidchar block in ToUnicode CMap");
                    break;
                }
                if (!(tok1[0] == '<' && tok1[n1 - 1] == '>')) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidchar block in ToUnicode CMap");
                    continue;
                }
                tok1[n1 - 1] = '\0';
                if (!parseHex(tok1 + 1, n1 - 2, &code1)) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidchar block in ToUnicode CMap");
                    continue;
                }
                if (code1 > maxCode) {
                    error(errSyntaxWarning, -1, "Invalid entry in cidchar block in ToUnicode CMap");
                }
                addMappingInt(code1, atoi(tok2));
                ok = true;
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "begincidrange")) {
            // the begincidrange operator is not allowed in ToUnicode CMaps,
            // but some buggy PDF generators incorrectly use
            // code-to-CID-type CMaps here
            error(errSyntaxWarning, -1, "Invalid 'begincidrange' operator in ToUnicode CMap");
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endcidrange")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endcidrange") || !pst->getToken(tok3, sizeof(tok3), &n3) || !strcmp(tok3, "endcidrange")) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidrange block in ToUnicode CMap");
                    break;
                }
                if (!(tok1[0] == '<' && tok1[n1 - 1] == '>' && tok2[0] == '<' && tok2[n2 - 1] == '>')) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidrange block in ToUnicode CMap");
                    continue;
                }
                tok1[n1 - 1] = tok2[n2 - 1] = '\0';
                if (!parseHex(tok1 + 1, n1 - 2, &code1) || !parseHex(tok2 + 1, n2 - 2, &code2)) {
                    error(errSyntaxWarning, -1, "Illegal entry in cidrange block in ToUnicode CMap");
                    continue;
                }
                if (code1 > maxCode || code2 > maxCode) {
                    error(errSyntaxWarning, -1, "Invalid entry in cidrange block in ToUnicode CMap");
                    if (code2 > maxCode) {
                        code2 = maxCode;
                    }
                }
                for (i = atoi(tok3); code1 <= code2; ++code1, ++i) {
                    addMappingInt(code1, i);
                    ok = true;
                }
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else {
            strcpy(tok1, tok2);
        }
    }
    delete pst;
    return ok;
}

void CharCodeToUnicode::addMapping(CharCode code, char *uStr, int n, int offset)
{
    Unicode u;
    int j;

    if (code > 0xffffff) {
        // This is an arbitrary limit to avoid integer overflow issues.
        // (I've seen CMaps with mappings for <ffffffff>.)
        return;
    }
    if (code >= map.size()) {
        size_t oldLen = map.size();
        auto newLen = oldLen ? 2 * oldLen : 256;
        if (code >= newLen) {
            newLen = (code + 256) & ~255;
        }
        if (unlikely(code >= newLen)) {
            error(errSyntaxWarning, -1, "Illegal code value in CharCodeToUnicode::addMapping");
            return;
        } else {
            map.resize(newLen, 0);
        }
    }
    if (n <= 4) {
        if (!parseHex(uStr, n, &u)) {
            error(errSyntaxWarning, -1, "Illegal entry in ToUnicode CMap");
            return;
        }
        map[code] = u + offset;
        if (!UnicodeIsValid(map[code])) {
            map[code] = 0xfffd;
        }
    } else {
        map[code] = 0;
        int utf16Len = n / 4;
        std::vector<Unicode> utf16(utf16Len);
        utf16.resize(utf16Len);
        for (j = 0; j < utf16Len; ++j) {
            if (!parseHex(uStr + j * 4, 4, &utf16[j])) {
                error(errSyntaxWarning, -1, "Illegal entry in ToUnicode CMap");
                return;
            }
        }
        utf16[utf16Len - 1] += offset;
        sMap.push_back({ code, UTF16toUCS4(utf16.data(), utf16.size()) });
    }
}

void CharCodeToUnicode::addMappingInt(CharCode code, Unicode u)
{
    if (code > 0xffffff) {
        // This is an arbitrary limit to avoid integer overflow issues.
        // (I've seen CMaps with mappings for <ffffffff>.)
        return;
    }
    if (code >= map.size()) {
        size_t oldLen = map.size();
        size_t newLen = oldLen ? 2 * oldLen : 256;
        if (code >= newLen) {
            newLen = (code + 256) & ~255;
        }
        map.resize(newLen, 0);
    }
    map[code] = u;
}

CharCodeToUnicode::CharCodeToUnicode()
{
    refCnt = 1;
    isIdentity = false;
}

CharCodeToUnicode::CharCodeToUnicode(const std::optional<std::string> &tagA) : tag(tagA)
{
    map.resize(256, 0);
    refCnt = 1;
    isIdentity = false;
}
CharCodeToUnicode::CharCodeToUnicode(const std::optional<std::string> &tagA, std::vector<Unicode> &&mapA, std::vector<CharCodeToUnicodeString> &&sMapA) : tag(tagA)
{
    map = std::move(mapA);
    sMap = std::move(sMapA);
    refCnt = 1;
    isIdentity = false;
}

void CharCodeToUnicode::incRefCnt()
{
    ++refCnt;
}

void CharCodeToUnicode::decRefCnt()
{
    if (--refCnt == 0) {
        delete this;
    }
}

bool CharCodeToUnicode::match(const GooString *tagA)
{
    return tag && tag == tagA->toStr();
}

void CharCodeToUnicode::setMapping(CharCode c, Unicode *u, int len)
{
    size_t i;
    int j;

    if (map.empty() || isIdentity) {
        return;
    }
    if (len == 1) {
        map[c] = u[0];
    } else {
        std::optional<std::reference_wrapper<CharCodeToUnicodeString>> element;
        for (i = 0; i < sMap.size(); ++i) {
            if (sMap[i].c == c) {
                sMap[i].u.clear();
                element = std::ref(sMap[i]);
                break;
            }
        }
        if (!element) {
            sMap.emplace_back();
            element = std::ref(sMap.back());
        }
        map[c] = 0;
        element->get().c = c;
        element->get().u.reserve(len);
        for (j = 0; j < len; ++j) {
            if (UnicodeIsValid(u[j])) {
                element->get().u.push_back(u[j]);
            } else {
                element->get().u.push_back(0xfffd);
            }
        }
    }
}

int CharCodeToUnicode::mapToUnicode(CharCode c, Unicode const **u) const
{
    if (isIdentity) {
        auto that = const_cast<CharCodeToUnicode *>(this);
        that->map[0] = (Unicode)c;
        *u = map.data();
        return 1;
    }
    if (c >= map.size()) {
        return 0;
    }
    if (map[c]) {
        *u = &map[c];
        return 1;
    }
    for (auto i = sMap.size(); i > 0; --i) { // in reverse so CMap takes precedence
        if (sMap[i - 1].c == c) {
            *u = sMap[i - 1].u.data();
            return sMap[i - 1].u.size();
        }
    }
    return 0;
}

int CharCodeToUnicode::mapToCharCode(const Unicode *u, CharCode *c, int usize) const
{
    // look for charcode in map
    if (usize == 1 || (usize > 1 && !(*u & ~0xff))) {
        if (isIdentity) {
            *c = (CharCode)*u;
            return 1;
        }
        for (CharCode i = 0; i < map.size(); i++) {
            if (map[i] == *u) {
                *c = i;
                return 1;
            }
        }
        *c = 'x';
    } else {
        size_t j;
        // for each entry in the sMap
        for (const auto &element : sMap) {
            // if the entry's unicode length isn't the same are usize, the strings
            // are obviously different
            if (element.u.size() != size_t(usize)) {
                continue;
            }
            // compare the string char by char
            for (j = 0; j < element.u.size(); j++) {
                if (element.u[j] != u[j]) {
                    break;
                }
            }

            // we have the same strings
            if (j == element.u.size()) {
                *c = element.c;
                return 1;
            }
        }
    }
    return 0;
}

//------------------------------------------------------------------------

CharCodeToUnicodeCache::CharCodeToUnicodeCache(int sizeA)
{
    int i;

    size = sizeA;
    cache = (CharCodeToUnicode **)gmallocn(size, sizeof(CharCodeToUnicode *));
    for (i = 0; i < size; ++i) {
        cache[i] = nullptr;
    }
}

CharCodeToUnicodeCache::~CharCodeToUnicodeCache()
{
    int i;

    for (i = 0; i < size; ++i) {
        if (cache[i]) {
            cache[i]->decRefCnt();
        }
    }
    gfree(cache);
}

CharCodeToUnicode *CharCodeToUnicodeCache::getCharCodeToUnicode(const GooString *tag)
{
    CharCodeToUnicode *ctu;
    int i, j;

    if (cache[0] && cache[0]->match(tag)) {
        cache[0]->incRefCnt();
        return cache[0];
    }
    for (i = 1; i < size; ++i) {
        if (cache[i] && cache[i]->match(tag)) {
            ctu = cache[i];
            for (j = i; j >= 1; --j) {
                cache[j] = cache[j - 1];
            }
            cache[0] = ctu;
            ctu->incRefCnt();
            return ctu;
        }
    }
    return nullptr;
}

void CharCodeToUnicodeCache::add(CharCodeToUnicode *ctu)
{
    int i;

    if (cache[size - 1]) {
        cache[size - 1]->decRefCnt();
    }
    for (i = size - 1; i >= 1; --i) {
        cache[i] = cache[i - 1];
    }
    cache[0] = ctu;
    ctu->incRefCnt();
}
