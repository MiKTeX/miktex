//========================================================================
//
// CMap.cc
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
// Copyright (C) 2008 Koji Otani <sho@bbr.jp>
// Copyright (C) 2008, 2009, 2017-2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2013 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 LE GARREC Vincent <legarrec.vincent@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "goo/GooString.h"
#include "Error.h"
#include "GlobalParams.h"
#include "PSTokenizer.h"
#include "CMap.h"
#include "Object.h"

//------------------------------------------------------------------------

struct CMapVectorEntry
{
    bool isVector;
    union {
        CMapVectorEntry *vector;
        CID cid;
    };
};

//------------------------------------------------------------------------

static int getCharFromFile(void *data)
{
    return fgetc((FILE *)data);
}

static int getCharFromStream(void *data)
{
    return ((Stream *)data)->getChar();
}

//------------------------------------------------------------------------

std::shared_ptr<CMap> CMap::parse(CMapCache *cache, const GooString *collectionA, Object *obj)
{
    std::shared_ptr<CMap> cMap;
    GooString *cMapNameA;

    if (obj->isName()) {
        cMapNameA = new GooString(obj->getName());
        if (!(cMap = globalParams->getCMap(collectionA, cMapNameA))) {
            error(errSyntaxError, -1, "Unknown CMap '{0:t}' for character collection '{1:t}'", cMapNameA, collectionA);
        }
        delete cMapNameA;
    } else if (obj->isStream()) {
        if (!(cMap = CMap::parse(nullptr, collectionA, obj->getStream()))) {
            error(errSyntaxError, -1, "Invalid CMap in Type 0 font");
        }
    } else {
        error(errSyntaxError, -1, "Invalid Encoding in Type 0 font");
        return {};
    }
    return cMap;
}

std::shared_ptr<CMap> CMap::parse(CMapCache *cache, const GooString *collectionA, const GooString *cMapNameA)
{
    FILE *f;

    if (!(f = globalParams->findCMapFile(collectionA, cMapNameA))) {

        // Check for an identity CMap.
        if (!cMapNameA->cmp("Identity") || !cMapNameA->cmp("Identity-H")) {
            return std::shared_ptr<CMap>(new CMap(collectionA->copy(), cMapNameA->copy(), 0));
        }
        if (!cMapNameA->cmp("Identity-V")) {
            return std::shared_ptr<CMap>(new CMap(collectionA->copy(), cMapNameA->copy(), 1));
        }

        error(errSyntaxError, -1, "Couldn't find '{0:t}' CMap file for '{1:t}' collection", cMapNameA, collectionA);
        return {};
    }

    auto cMap = std::shared_ptr<CMap>(new CMap(collectionA->copy(), cMapNameA->copy()));
    cMap->parse2(cache, &getCharFromFile, f);

    fclose(f);

    return cMap;
}

std::shared_ptr<CMap> CMap::parse(CMapCache *cache, const GooString *collectionA, Stream *str)
{
    auto cMap = std::shared_ptr<CMap>(new CMap(collectionA->copy(), nullptr));
    Object obj1 = str->getDict()->lookup("UseCMap");
    if (!obj1.isNull()) {
        cMap->useCMap(cache, &obj1);
    }

    str->reset();
    cMap->parse2(cache, &getCharFromStream, str);
    str->close();
    return cMap;
}

void CMap::parse2(CMapCache *cache, int (*getCharFunc)(void *), void *data)
{
    PSTokenizer *pst;
    char tok1[256], tok2[256], tok3[256];
    int n1, n2, n3;
    unsigned int start = 0, end = 0, code;

    pst = new PSTokenizer(getCharFunc, data);
    pst->getToken(tok1, sizeof(tok1), &n1);
    while (pst->getToken(tok2, sizeof(tok2), &n2)) {
        if (!strcmp(tok2, "usecmap")) {
            if (tok1[0] == '/') {
                useCMap(cache, tok1 + 1);
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok1, "/WMode")) {
            wMode = atoi(tok2);
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "begincidchar")) {
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endcidchar")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endcidchar")) {
                    error(errSyntaxError, -1, "Illegal entry in cidchar block in CMap");
                    break;
                }
                if (!(tok1[0] == '<' && tok1[n1 - 1] == '>' && n1 >= 4 && (n1 & 1) == 0)) {
                    error(errSyntaxError, -1, "Illegal entry in cidchar block in CMap");
                    continue;
                }
                tok1[n1 - 1] = '\0';
                if (sscanf(tok1 + 1, "%x", &code) != 1) {
                    error(errSyntaxError, -1, "Illegal entry in cidchar block in CMap");
                    continue;
                }
                n1 = (n1 - 2) / 2;
                addCIDs(code, code, n1, (CID)atoi(tok2));
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else if (!strcmp(tok2, "begincidrange")) {
            while (pst->getToken(tok1, sizeof(tok1), &n1)) {
                if (!strcmp(tok1, "endcidrange")) {
                    break;
                }
                if (!pst->getToken(tok2, sizeof(tok2), &n2) || !strcmp(tok2, "endcidrange") || !pst->getToken(tok3, sizeof(tok3), &n3) || !strcmp(tok3, "endcidrange")) {
                    error(errSyntaxError, -1, "Illegal entry in cidrange block in CMap");
                    break;
                }
                if (tok1[0] == '<' && tok2[0] == '<' && n1 == n2 && n1 >= 4 && (n1 & 1) == 0) {
                    tok1[n1 - 1] = tok2[n1 - 1] = '\0';
                    sscanf(tok1 + 1, "%x", &start);
                    sscanf(tok2 + 1, "%x", &end);
                    n1 = (n1 - 2) / 2;
                    addCIDs(start, end, n1, (CID)atoi(tok3));
                }
            }
            pst->getToken(tok1, sizeof(tok1), &n1);
        } else {
            strcpy(tok1, tok2);
        }
    }
    delete pst;
}

CMap::CMap(GooString *collectionA, GooString *cMapNameA)
{
    int i;

    collection = collectionA;
    cMapName = cMapNameA;
    isIdent = false;
    wMode = 0;
    vector = (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
    for (i = 0; i < 256; ++i) {
        vector[i].isVector = false;
        vector[i].cid = 0;
    }
}

CMap::CMap(GooString *collectionA, GooString *cMapNameA, int wModeA)
{
    collection = collectionA;
    cMapName = cMapNameA;
    isIdent = true;
    wMode = wModeA;
    vector = nullptr;
}

void CMap::useCMap(CMapCache *cache, const char *useName)
{
    GooString *useNameStr;
    std::shared_ptr<CMap> subCMap;

    useNameStr = new GooString(useName);
    // if cache is non-NULL, we already have a lock, and we can use
    // CMapCache::getCMap() directly; otherwise, we need to use
    // GlobalParams::getCMap() in order to acqure the lock need to use
    // GlobalParams::getCMap
    if (cache) {
        subCMap = cache->getCMap(collection, useNameStr);
    } else {
        subCMap = globalParams->getCMap(collection, useNameStr);
    }
    delete useNameStr;
    if (!subCMap) {
        return;
    }
    isIdent = subCMap->isIdent;
    if (subCMap->vector) {
        copyVector(vector, subCMap->vector);
    }
}

void CMap::useCMap(CMapCache *cache, Object *obj)
{
    std::shared_ptr<CMap> subCMap = CMap::parse(cache, collection, obj);
    if (!subCMap) {
        return;
    }
    isIdent = subCMap->isIdent;
    if (subCMap->vector) {
        copyVector(vector, subCMap->vector);
    }
}

void CMap::copyVector(CMapVectorEntry *dest, CMapVectorEntry *src)
{
    int i, j;

    for (i = 0; i < 256; ++i) {
        if (src[i].isVector) {
            if (!dest[i].isVector) {
                dest[i].isVector = true;
                dest[i].vector = (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
                for (j = 0; j < 256; ++j) {
                    dest[i].vector[j].isVector = false;
                    dest[i].vector[j].cid = 0;
                }
            }
            copyVector(dest[i].vector, src[i].vector);
        } else {
            if (dest[i].isVector) {
                error(errSyntaxError, -1, "Collision in usecmap");
            } else {
                dest[i].cid = src[i].cid;
            }
        }
    }
}

void CMap::addCIDs(unsigned int start, unsigned int end, unsigned int nBytes, CID firstCID)
{
    if (nBytes > 4) {
        error(errSyntaxError, -1, "Illegal entry in cidchar block in CMap");
        return;
    }

    const unsigned int start1 = start & 0xffffff00;
    const unsigned int end1 = end & 0xffffff00;
    for (unsigned int i = start1; i <= end1; i += 0x100) {
        CMapVectorEntry *vec = vector;
        for (unsigned int j = nBytes - 1; j >= 1; --j) {
            const int byte = (i >> (8 * j)) & 0xff;
            if (!vec[byte].isVector) {
                vec[byte].isVector = true;
                vec[byte].vector = (CMapVectorEntry *)gmallocn(256, sizeof(CMapVectorEntry));
                for (unsigned int k = 0; k < 256; ++k) {
                    vec[byte].vector[k].isVector = false;
                    vec[byte].vector[k].cid = 0;
                }
            }
            vec = vec[byte].vector;
        }
        const int byte0 = (i < start) ? (start & 0xff) : 0;
        const int byte1 = (i + 0xff > end) ? (end & 0xff) : 0xff;
        for (int byte = byte0; byte <= byte1; ++byte) {
            if (vec[byte].isVector) {
                error(errSyntaxError, -1, "Invalid CID ({0:ux} [{1:ud} bytes]) in CMap", i, nBytes);
            } else {
                vec[byte].cid = firstCID + ((i + byte) - start);
            }
        }
    }
}

CMap::~CMap()
{
    delete collection;
    delete cMapName;
    if (vector) {
        freeCMapVector(vector);
    }
}

void CMap::freeCMapVector(CMapVectorEntry *vec)
{
    int i;

    for (i = 0; i < 256; ++i) {
        if (vec[i].isVector) {
            freeCMapVector(vec[i].vector);
        }
    }
    gfree(vec);
}

bool CMap::match(const GooString *collectionA, const GooString *cMapNameA)
{
    return !collection->cmp(collectionA) && !cMapName->cmp(cMapNameA);
}

CID CMap::getCID(const char *s, int len, CharCode *c, int *nUsed)
{
    CMapVectorEntry *vec;
    CharCode cc;
    int n, i;

    vec = vector;
    cc = 0;
    n = 0;
    while (vec && n < len) {
        i = s[n++] & 0xff;
        cc = (cc << 8) | i;
        if (!vec[i].isVector) {
            *c = cc;
            *nUsed = n;
            return vec[i].cid;
        }
        vec = vec[i].vector;
    }
    if (isIdent && len >= 2) {
        // identity CMap
        *nUsed = 2;
        *c = cc = ((s[0] & 0xff) << 8) + (s[1] & 0xff);
        return cc;
    }
    *nUsed = 1;
    *c = s[0] & 0xff;
    return 0;
}

void CMap::setReverseMapVector(unsigned int startCode, CMapVectorEntry *vec, unsigned int *rmap, unsigned int rmapSize, unsigned int ncand)
{
    int i;

    if (vec == nullptr) {
        return;
    }
    for (i = 0; i < 256; i++) {
        if (vec[i].isVector) {
            setReverseMapVector((startCode + i) << 8, vec[i].vector, rmap, rmapSize, ncand);
        } else {
            unsigned int cid = vec[i].cid;

            if (cid < rmapSize) {
                unsigned int cand;

                for (cand = 0; cand < ncand; cand++) {
                    unsigned int code = startCode + i;
                    unsigned int idx = cid * ncand + cand;
                    if (rmap[idx] == 0) {
                        rmap[idx] = code;
                        break;
                    } else if (rmap[idx] == code) {
                        break;
                    }
                }
            }
        }
    }
}

void CMap::setReverseMap(unsigned int *rmap, unsigned int rmapSize, unsigned int ncand)
{
    setReverseMapVector(0, vector, rmap, rmapSize, ncand);
}

//------------------------------------------------------------------------

CMapCache::CMapCache() { }

std::shared_ptr<CMap> CMapCache::getCMap(const GooString *collection, const GooString *cMapName)
{
    int i, j;

    if (cache[0] && cache[0]->match(collection, cMapName)) {
        return cache[0];
    }
    for (i = 1; i < cMapCacheSize; ++i) {
        if (cache[i] && cache[i]->match(collection, cMapName)) {
            std::shared_ptr<CMap> cmap = cache[i];
            for (j = i; j >= 1; --j) {
                cache[j] = cache[j - 1];
            }
            cache[0] = cmap;
            return cmap;
        }
    }
    std::shared_ptr<CMap> cmap = CMap::parse(this, collection, cMapName);
    if (cmap) {
        for (j = cMapCacheSize - 1; j >= 1; --j) {
            cache[j] = cache[j - 1];
        }
        cache[0] = cmap;
        return cmap;
    }
    return {};
}
