//========================================================================
//
// GlobalParams.cc
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
// Copyright (C) 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005, 2007-2010, 2012, 2015, 2017-2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Jonathan Blandford <jrb@redhat.com>
// Copyright (C) 2006, 2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2006 Takashi Iwai <tiwai@suse.de>
// Copyright (C) 2006 Ed Catmur <ed@catmur.co.uk>
// Copyright (C) 2007 Krzysztof Kowalczyk <kkowalczyk@gmail.com>
// Copyright (C) 2007, 2009 Jonathan Kew <jonathan_kew@sil.org>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2009, 2011, 2012, 2015 William Bader <williambader@hotmail.com>
// Copyright (C) 2009 Kovid Goyal <kovid@kovidgoyal.net>
// Copyright (C) 2010, 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Patrick Spendrin <ps_ml@gmx.de>
// Copyright (C) 2010 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2011 Pino Toscano <pino@kde.org>
// Copyright (C) 2011 Koji Otani <sho@bbr.jp>
// Copyright (C) 2012 Yi Yang <ahyangyi@gmail.com>
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2012 Peter Breitenlohner <peb@mppmu.mpg.de>
// Copyright (C) 2013, 2014 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2017 Christoph Cullmann <cullmann@kde.org>
// Copyright (C) 2017 Jean Ghali <jghali@libertysurf.fr>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018, 2020 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 Kai Pastor <dg0yt@darc.de>
// Copyright (C) 2021, 2022 Stefan Löffler <st.loeffler@gmail.com>
// Copyright (C) 2021 sunderme <sunderme@gmx.de>
// Copyright (C) 2022 Even Rouault <even.rouault@spatialys.com>
// Copyright (C) 2022 Claes Nästén <pekdon@gmail.com>
// Copyright (C) 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
// Copyright (C) 2023 Shivodit Gill <shivodit.gill@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <algorithm>
#include <cstring>
#include <cstdio>
#include <cctype>
#ifdef _WIN32
#    include <shlobj.h>
#    include <mbstring.h>
#endif
#ifdef ANDROID
#    include <android/font.h>
#    include <android/font_matcher.h>
#    include <android/system_fonts.h>
#endif
#include "goo/glibc.h"
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "goo/gdir.h"
#include "Error.h"
#include "NameToCharCode.h"
#include "CharCodeToUnicode.h"
#include "UnicodeMap.h"
#include "CMap.h"
#include "FontEncodingTables.h"
#include "GlobalParams.h"
#include "GfxFont.h"

#ifdef WITH_FONTCONFIGURATION_FONTCONFIG
#    include <fontconfig/fontconfig.h>
#endif

#ifndef _MSC_VER
#    include <strings.h>
#endif

#ifndef FC_WEIGHT_BOOK
#    define FC_WEIGHT_BOOK 75
#endif

#include "NameToUnicodeTable.h"
#include "UnicodeMapTables.h"
#include "UnicodeMapFuncs.h"

#include "fofi/FoFiTrueType.h"
#include "fofi/FoFiIdentifier.h"

//------------------------------------------------------------------------

#define cidToUnicodeCacheSize 4
#define unicodeToUnicodeCacheSize 4

//------------------------------------------------------------------------

std::unique_ptr<GlobalParams> globalParams;

#if defined(ENABLE_RELOCATABLE) && defined(_WIN32)

/* search for data relative to where we are installed */

static HMODULE hmodule;

extern "C" {
/* Provide declaration to squelch -Wmissing-declarations warning */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        hmodule = hinstDLL;
        break;
    }

    return TRUE;
}
}

static std::string get_poppler_localdir(const std::string &suffix)
{
    const std::string binSuffix("\\bin");
    std::string retval(MAX_PATH, '\0');

    if (!GetModuleFileNameA(hmodule, retval.data(), retval.size())) {
        return POPPLER_DATADIR;
    }

    const std::string::size_type p = retval.rfind('\\');
    if (p != std::string::npos) {
        retval.erase(p);
        if (retval.size() > binSuffix.size() && stricmp(retval.substr(p - binSuffix.size()).c_str(), binSuffix.c_str()) == 0) {
            retval.erase(p - binSuffix.size());
        }
    }
    retval += suffix;
    retval.shrink_to_fit();
    return retval;
}

static const char *get_poppler_datadir(void)
{
    static std::string retval;
    static bool beenhere = false;

    if (!beenhere) {
        retval = get_poppler_localdir("\\share\\poppler");
        beenhere = true;
    }

    return retval.c_str();
}

#    undef POPPLER_DATADIR
#    define POPPLER_DATADIR get_poppler_datadir()

static const char *get_poppler_fontsdir(void)
{
    static std::string retval;
    static bool beenhere = false;

    if (!beenhere) {
        retval = get_poppler_localdir("\\share\\fonts");
        beenhere = true;
    }

    return retval.c_str();
}
#    undef POPPLER_FONTSDIR
#    define POPPLER_FONTSDIR get_poppler_fontsdir()

#else
#    define POPPLER_FONTSDIR nullptr
#endif

//------------------------------------------------------------------------
// SysFontInfo
//------------------------------------------------------------------------

class SysFontInfo
{
public:
    GooString *name;
    bool bold;
    bool italic;
    bool oblique;
    bool fixedWidth;
    GooString *path;
    SysFontType type;
    int fontNum; // for TrueType collections
    GooString *substituteName;

    SysFontInfo(GooString *nameA, bool boldA, bool italicA, bool obliqueA, bool fixedWidthA, GooString *pathA, SysFontType typeA, int fontNumA, GooString *substituteNameA);
    ~SysFontInfo();
    SysFontInfo(const SysFontInfo &) = delete;
    SysFontInfo &operator=(const SysFontInfo &) = delete;
    bool match(const SysFontInfo *fi) const;
    bool match(const GooString *nameA, bool boldA, bool italicA, bool obliqueA, bool fixedWidthA) const;
    bool match(const GooString *nameA, bool boldA, bool italicA) const;
};

SysFontInfo::SysFontInfo(GooString *nameA, bool boldA, bool italicA, bool obliqueA, bool fixedWidthA, GooString *pathA, SysFontType typeA, int fontNumA, GooString *substituteNameA)
{
    name = nameA;
    bold = boldA;
    italic = italicA;
    oblique = obliqueA;
    fixedWidth = fixedWidthA;
    path = pathA;
    type = typeA;
    fontNum = fontNumA;
    substituteName = substituteNameA;
}

SysFontInfo::~SysFontInfo()
{
    delete name;
    delete path;
    delete substituteName;
}

bool SysFontInfo::match(const SysFontInfo *fi) const
{
    return !strcasecmp(name->c_str(), fi->name->c_str()) && bold == fi->bold && italic == fi->italic && oblique == fi->oblique && fixedWidth == fi->fixedWidth;
}

bool SysFontInfo::match(const GooString *nameA, bool boldA, bool italicA, bool obliqueA, bool fixedWidthA) const
{
    return !strcasecmp(name->c_str(), nameA->c_str()) && bold == boldA && italic == italicA && oblique == obliqueA && fixedWidth == fixedWidthA;
}

bool SysFontInfo::match(const GooString *nameA, bool boldA, bool italicA) const
{
    return !strcasecmp(name->c_str(), nameA->c_str()) && bold == boldA && italic == italicA;
}

//------------------------------------------------------------------------
// SysFontList
//------------------------------------------------------------------------

class SysFontList
{
public:
    SysFontList();
    ~SysFontList();
    SysFontList(const SysFontList &) = delete;
    SysFontList &operator=(const SysFontList &) = delete;
    const SysFontInfo *find(const std::string &name, bool isFixedWidth, bool exact, const std::vector<std::string> &filesToIgnore = {});

    const std::vector<SysFontInfo *> &getFonts() const { return fonts; }

#ifdef _WIN32
    void scanWindowsFonts(const std::string &winFontDir);
#endif
#ifdef WITH_FONTCONFIGURATION_FONTCONFIG
    void addFcFont(SysFontInfo *si) { fonts.push_back(si); }
#endif
private:
#ifdef _WIN32
    SysFontInfo *makeWindowsFont(const char *name, int fontNum, const char *path);
#endif

    std::vector<SysFontInfo *> fonts;
};

SysFontList::SysFontList() { }

SysFontList::~SysFontList()
{
    for (auto entry : fonts) {
        delete entry;
    }
}

const SysFontInfo *SysFontList::find(const std::string &name, bool fixedWidth, bool exact, const std::vector<std::string> &filesToIgnore)
{
    GooString *name2;
    bool bold, italic, oblique;
    int n;

    name2 = new GooString(name);

    // remove space, comma, dash chars
    {
        int i = 0;
        while (i < name2->getLength()) {
            const char c = name2->getChar(i);
            if (c == ' ' || c == ',' || c == '-') {
                name2->del(i);
            } else {
                ++i;
            }
        }
        n = name2->getLength();
    }

    // remove trailing "MT" (Foo-MT, Foo-BoldMT, etc.)
    if (n > 2 && !strcmp(name2->c_str() + n - 2, "MT")) {
        name2->del(n - 2, 2);
        n -= 2;
    }

    // look for "Regular"
    if (n > 7 && !strcmp(name2->c_str() + n - 7, "Regular")) {
        name2->del(n - 7, 7);
        n -= 7;
    }

    // look for "Italic"
    if (n > 6 && !strcmp(name2->c_str() + n - 6, "Italic")) {
        name2->del(n - 6, 6);
        italic = true;
        n -= 6;
    } else {
        italic = false;
    }

    // look for "Oblique"
    if (n > 6 && !strcmp(name2->c_str() + n - 7, "Oblique")) {
        name2->del(n - 7, 7);
        oblique = true;
        n -= 6;
    } else {
        oblique = false;
    }

    // look for "Bold"
    if (n > 4 && !strcmp(name2->c_str() + n - 4, "Bold")) {
        name2->del(n - 4, 4);
        bold = true;
        n -= 4;
    } else {
        bold = false;
    }

    // remove trailing "MT" (FooMT-Bold, etc.)
    if (n > 2 && !strcmp(name2->c_str() + n - 2, "MT")) {
        name2->del(n - 2, 2);
        n -= 2;
    }

    // remove trailing "PS"
    if (n > 2 && !strcmp(name2->c_str() + n - 2, "PS")) {
        name2->del(n - 2, 2);
        n -= 2;
    }

    // remove trailing "IdentityH"
    if (n > 9 && !strcmp(name2->c_str() + n - 9, "IdentityH")) {
        name2->del(n - 9, 9);
        n -= 9;
    }

    // search for the font
    const SysFontInfo *fi = nullptr;
    for (const SysFontInfo *f : fonts) {
        fi = f;
        if (fi->match(name2, bold, italic, oblique, fixedWidth)) {
            if (std::find(filesToIgnore.begin(), filesToIgnore.end(), fi->path->toStr()) == filesToIgnore.end()) {
                break;
            }
        }
        fi = nullptr;
    }
    if (!fi && !exact && bold) {
        // try ignoring the bold flag
        for (const SysFontInfo *f : fonts) {
            fi = f;
            if (fi->match(name2, false, italic)) {
                if (std::find(filesToIgnore.begin(), filesToIgnore.end(), fi->path->toStr()) == filesToIgnore.end()) {
                    break;
                }
            }
            fi = nullptr;
        }
    }
    if (!fi && !exact && (bold || italic)) {
        // try ignoring the bold and italic flags
        for (const SysFontInfo *f : fonts) {
            fi = f;
            if (fi->match(name2, false, false)) {
                if (std::find(filesToIgnore.begin(), filesToIgnore.end(), fi->path->toStr()) == filesToIgnore.end()) {
                    break;
                }
            }
            fi = nullptr;
        }
    }

    delete name2;
    return fi;
}

#define globalParamsLocker() const std::scoped_lock locker(mutex)
#define unicodeMapCacheLocker() const std::scoped_lock locker(unicodeMapCacheMutex)
#define cMapCacheLocker() const std::scoped_lock locker(cMapCacheMutex)

//------------------------------------------------------------------------
// parsing
//------------------------------------------------------------------------

GlobalParams::GlobalParams(const char *customPopplerDataDir) : popplerDataDir(customPopplerDataDir)
{
    // scan the encoding in reverse because we want the lowest-numbered
    // index for each char name ('space' is encoded twice)
    macRomanReverseMap = new NameToCharCode();
    for (int i = 255; i >= 0; --i) {
        if (macRomanEncoding[i]) {
            macRomanReverseMap->add(macRomanEncoding[i], (CharCode)i);
        }
    }

    nameToUnicodeZapfDingbats = new NameToCharCode();
    nameToUnicodeText = new NameToCharCode();
    sysFonts = new SysFontList();
    textEncoding = new GooString("UTF-8");
    printCommands = false;
    profileCommands = false;
    errQuiet = false;

    cidToUnicodeCache = new CharCodeToUnicodeCache(cidToUnicodeCacheSize);
    unicodeToUnicodeCache = new CharCodeToUnicodeCache(unicodeToUnicodeCacheSize);
    unicodeMapCache = new UnicodeMapCache();
    cMapCache = new CMapCache();

    utf8Map = nullptr;

    baseFontsInitialized = false;

    // set up the initial nameToUnicode tables
    for (int i = 0; nameToUnicodeZapfDingbatsTab[i].name; ++i) {
        nameToUnicodeZapfDingbats->add(nameToUnicodeZapfDingbatsTab[i].name, nameToUnicodeZapfDingbatsTab[i].u);
    }

    for (int i = 0; nameToUnicodeTextTab[i].name; ++i) {
        nameToUnicodeText->add(nameToUnicodeTextTab[i].name, nameToUnicodeTextTab[i].u);
    }

    // set up the residentUnicodeMaps table
    residentUnicodeMaps.reserve(6);
    UnicodeMap map = { "Latin1", false, latin1UnicodeMapRanges, latin1UnicodeMapLen };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));
    map = { "ASCII7", false, ascii7UnicodeMapRanges, ascii7UnicodeMapLen };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));
    map = { "Symbol", false, symbolUnicodeMapRanges, symbolUnicodeMapLen };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));
    map = { "ZapfDingbats", false, zapfDingbatsUnicodeMapRanges, zapfDingbatsUnicodeMapLen };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));
    map = { "UTF-8", true, &mapUTF8 };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));
    map = { "UTF-16", true, &mapUTF16 };
    residentUnicodeMaps.emplace(map.getEncodingName(), std::move(map));

    scanEncodingDirs();
}

void GlobalParams::scanEncodingDirs()
{
    GDir *dir;
    std::unique_ptr<GDirEntry> entry;
    const char *dataRoot = popplerDataDir ? popplerDataDir : POPPLER_DATADIR;

    // allocate buffer large enough to append "/nameToUnicode"
    size_t bufSize = strlen(dataRoot) + strlen("/nameToUnicode") + 1;
    char *dataPathBuffer = new char[bufSize];

    snprintf(dataPathBuffer, bufSize, "%s/nameToUnicode", dataRoot);
    dir = new GDir(dataPathBuffer, true);
    while (entry = dir->getNextEntry(), entry != nullptr) {
        if (!entry->isDir()) {
            parseNameToUnicode(entry->getFullPath());
        }
    }
    delete dir;

    snprintf(dataPathBuffer, bufSize, "%s/cidToUnicode", dataRoot);
    dir = new GDir(dataPathBuffer, false);
    while (entry = dir->getNextEntry(), entry != nullptr) {
        addCIDToUnicode(entry->getName(), entry->getFullPath());
    }
    delete dir;

    snprintf(dataPathBuffer, bufSize, "%s/unicodeMap", dataRoot);
    dir = new GDir(dataPathBuffer, false);
    while (entry = dir->getNextEntry(), entry != nullptr) {
        addUnicodeMap(entry->getName(), entry->getFullPath());
    }
    delete dir;

    snprintf(dataPathBuffer, bufSize, "%s/cMap", dataRoot);
    dir = new GDir(dataPathBuffer, false);
    while (entry = dir->getNextEntry(), entry != nullptr) {
        addCMapDir(entry->getName(), entry->getFullPath());
        toUnicodeDirs.push_back(entry->getFullPath()->copy());
    }
    delete dir;

    delete[] dataPathBuffer;
}

void GlobalParams::parseNameToUnicode(const GooString *name)
{
    char *tok1, *tok2;
    FILE *f;
    char buf[256];
    int line;
    Unicode u;
    char *tokptr;

    if (!(f = openFile(name->c_str(), "r"))) {
        error(errIO, -1, "Couldn't open 'nameToUnicode' file '{0:t}'", name);
        return;
    }
    line = 1;
    while (getLine(buf, sizeof(buf), f)) {
        tok1 = strtok_r(buf, " \t\r\n", &tokptr);
        tok2 = strtok_r(nullptr, " \t\r\n", &tokptr);
        if (tok1 && tok2) {
            sscanf(tok1, "%x", &u);
            nameToUnicodeText->add(tok2, u);
        } else {
            error(errConfig, -1, "Bad line in 'nameToUnicode' file ({0:t}:{1:d})", name, line);
        }
        ++line;
    }
    fclose(f);
}

void GlobalParams::addCIDToUnicode(const GooString *collection, const GooString *fileName)
{
    cidToUnicodes[collection->toStr()] = fileName->toStr();
}

void GlobalParams::addUnicodeMap(const GooString *encodingName, const GooString *fileName)
{
    unicodeMaps[encodingName->toStr()] = fileName->toStr();
}

void GlobalParams::addCMapDir(const GooString *collection, const GooString *dir)
{
    cMapDirs.emplace(collection->toStr(), dir->toStr());
}

bool GlobalParams::parseYesNo2(const char *token, bool *flag)
{
    if (!strcmp(token, "yes")) {
        *flag = true;
    } else if (!strcmp(token, "no")) {
        *flag = false;
    } else {
        return false;
    }
    return true;
}

GlobalParams::~GlobalParams()
{
    delete macRomanReverseMap;

    delete nameToUnicodeZapfDingbats;
    delete nameToUnicodeText;
    for (auto entry : toUnicodeDirs) {
        delete entry;
    }
    delete sysFonts;
    delete textEncoding;

    delete cidToUnicodeCache;
    delete unicodeToUnicodeCache;
    delete unicodeMapCache;
    delete cMapCache;
}

//------------------------------------------------------------------------
// accessors
//------------------------------------------------------------------------

CharCode GlobalParams::getMacRomanCharCode(const char *charName)
{
    // no need to lock - macRomanReverseMap is constant
    return macRomanReverseMap->lookup(charName);
}

Unicode GlobalParams::mapNameToUnicodeAll(const char *charName)
{
    // no need to lock - nameToUnicodeZapfDingbats and nameToUnicodeText are constant
    Unicode u = nameToUnicodeZapfDingbats->lookup(charName);
    if (!u) {
        u = nameToUnicodeText->lookup(charName);
    }
    return u;
}

Unicode GlobalParams::mapNameToUnicodeText(const char *charName)
{
    // no need to lock - nameToUnicodeText is constant
    return nameToUnicodeText->lookup(charName);
}

UnicodeMap *GlobalParams::getResidentUnicodeMap(const std::string &encodingName)
{
    UnicodeMap *map = nullptr;

    globalParamsLocker();
    const auto unicodeMap = residentUnicodeMaps.find(encodingName);
    if (unicodeMap != residentUnicodeMaps.end()) {
        map = &unicodeMap->second;
    }

    return map;
}

FILE *GlobalParams::getUnicodeMapFile(const std::string &encodingName)
{
    FILE *file = nullptr;

    globalParamsLocker();
    const auto unicodeMap = unicodeMaps.find(encodingName);
    if (unicodeMap != unicodeMaps.end()) {
        file = openFile(unicodeMap->second.c_str(), "r");
    }

    return file;
}

FILE *GlobalParams::findCMapFile(const GooString *collection, const GooString *cMapName)
{
    FILE *file = nullptr;

    globalParamsLocker();
    const auto collectionCMapDirs = cMapDirs.equal_range(collection->toStr());
    for (auto cMapDir = collectionCMapDirs.first; cMapDir != collectionCMapDirs.second; ++cMapDir) {
        auto *const path = new GooString(cMapDir->second);
        appendToPath(path, cMapName->c_str());
        file = openFile(path->c_str(), "r");
        delete path;
        if (file) {
            break;
        }
    }

    return file;
}

FILE *GlobalParams::findToUnicodeFile(const GooString *name)
{
    GooString *fileName;
    FILE *f;

    globalParamsLocker();
    for (const GooString *dir : toUnicodeDirs) {
        fileName = appendToPath(dir->copy(), name->c_str());
        f = openFile(fileName->c_str(), "r");
        delete fileName;
        if (f) {
            return f;
        }
    }
    return nullptr;
}

#ifdef WITH_FONTCONFIGURATION_FONTCONFIG
static bool findModifier(const std::string &name, const size_t modStart, const char *modifier, size_t &start)
{
    if (modStart == std::string::npos) {
        return false;
    }

    size_t match = name.find(modifier, modStart);
    if (match == std::string::npos) {
        return false;
    } else {
        if (start == std::string::npos || match < start) {
            start = match;
        }
        return true;
    }
}

static const char *getFontLang(const GfxFont *font)
{
    const char *lang;

    // find the language we want the font to support
    if (font->isCIDFont()) {
        const GooString *collection = ((GfxCIDFont *)font)->getCollection();
        if (collection) {
            if (strcmp(collection->c_str(), "Adobe-GB1") == 0) {
                lang = "zh-cn"; // Simplified Chinese
            } else if (strcmp(collection->c_str(), "Adobe-CNS1") == 0) {
                lang = "zh-tw"; // Traditional Chinese
            } else if (strcmp(collection->c_str(), "Adobe-Japan1") == 0) {
                lang = "ja"; // Japanese
            } else if (strcmp(collection->c_str(), "Adobe-Japan2") == 0) {
                lang = "ja"; // Japanese
            } else if (strcmp(collection->c_str(), "Adobe-Korea1") == 0) {
                lang = "ko"; // Korean
            } else if (strcmp(collection->c_str(), "Adobe-UCS") == 0) {
                lang = "xx";
            } else if (strcmp(collection->c_str(), "Adobe-Identity") == 0) {
                lang = "xx";
            } else {
                error(errUnimplemented, -1, "Unknown CID font collection: {0:t}. If this is expected to be a valid PDF document, please report to poppler bugtracker.", collection);
                lang = "xx";
            }
        } else {
            lang = "xx";
        }
    } else {
        lang = "xx";
    }
    return lang;
}

static FcPattern *buildFcPattern(const GfxFont *font, const GooString *base14Name)
{
    int weight = -1, slant = -1, width = -1, spacing = -1;
    FcPattern *p;

    // this is all heuristics will be overwritten if font had proper info
    std::string fontName;
    if (base14Name == nullptr) {
        fontName = font->getNameWithoutSubsetTag();
    } else {
        fontName = base14Name->toStr();
    }

    size_t modStart = fontName.find(',');
    if (modStart == std::string::npos) {
        modStart = fontName.find('-');
    }

    // remove the - from the names, for some reason, Fontconfig does not
    // understand "MS-Mincho" but does with "MS Mincho"
    std::replace(fontName.begin(), fontName.end(), '-', ' ');

    size_t start = std::string::npos;
    findModifier(fontName, modStart, "Regular", start);
    findModifier(fontName, modStart, "Roman", start);

    if (findModifier(fontName, modStart, "Oblique", start)) {
        slant = FC_SLANT_OBLIQUE;
    }
    if (findModifier(fontName, modStart, "Italic", start)) {
        slant = FC_SLANT_ITALIC;
    }
    if (findModifier(fontName, modStart, "Bold", start)) {
        weight = FC_WEIGHT_BOLD;
    }
    if (findModifier(fontName, modStart, "Light", start)) {
        weight = FC_WEIGHT_LIGHT;
    }
    if (findModifier(fontName, modStart, "Medium", start)) {
        weight = FC_WEIGHT_MEDIUM;
    }
    if (findModifier(fontName, modStart, "Condensed", start)) {
        width = FC_WIDTH_CONDENSED;
    }

    std::string family;
    if (start == std::string::npos) {
        family = fontName;
    } else {
        // There have been "modifiers" in the name, crop them to obtain
        // the family name
        family = fontName.substr(0, modStart);
    }

    // use font flags
    if (font->isFixedWidth()) {
        spacing = FC_MONO;
    }
    if (font->isBold()) {
        weight = FC_WEIGHT_BOLD;
    }
    if (font->isItalic()) {
        slant = FC_SLANT_ITALIC;
    }

    // if the FontDescriptor specified a family name use it
    if (font->getFamily()) {
        family = font->getFamily()->toStr();
    }

    // if the FontDescriptor specified a weight use it
    switch (font->getWeight()) {
    case GfxFont::W100:
        weight = FC_WEIGHT_EXTRALIGHT;
        break;
    case GfxFont::W200:
        weight = FC_WEIGHT_LIGHT;
        break;
    case GfxFont::W300:
        weight = FC_WEIGHT_BOOK;
        break;
    case GfxFont::W400:
        weight = FC_WEIGHT_NORMAL;
        break;
    case GfxFont::W500:
        weight = FC_WEIGHT_MEDIUM;
        break;
    case GfxFont::W600:
        weight = FC_WEIGHT_DEMIBOLD;
        break;
    case GfxFont::W700:
        weight = FC_WEIGHT_BOLD;
        break;
    case GfxFont::W800:
        weight = FC_WEIGHT_EXTRABOLD;
        break;
    case GfxFont::W900:
        weight = FC_WEIGHT_BLACK;
        break;
    default:
        break;
    }

    // if the FontDescriptor specified a width use it
    switch (font->getStretch()) {
    case GfxFont::UltraCondensed:
        width = FC_WIDTH_ULTRACONDENSED;
        break;
    case GfxFont::ExtraCondensed:
        width = FC_WIDTH_EXTRACONDENSED;
        break;
    case GfxFont::Condensed:
        width = FC_WIDTH_CONDENSED;
        break;
    case GfxFont::SemiCondensed:
        width = FC_WIDTH_SEMICONDENSED;
        break;
    case GfxFont::Normal:
        width = FC_WIDTH_NORMAL;
        break;
    case GfxFont::SemiExpanded:
        width = FC_WIDTH_SEMIEXPANDED;
        break;
    case GfxFont::Expanded:
        width = FC_WIDTH_EXPANDED;
        break;
    case GfxFont::ExtraExpanded:
        width = FC_WIDTH_EXTRAEXPANDED;
        break;
    case GfxFont::UltraExpanded:
        width = FC_WIDTH_ULTRAEXPANDED;
        break;
    default:
        break;
    }

    const char *lang = getFontLang(font);

    p = FcPatternBuild(nullptr, FC_FAMILY, FcTypeString, family.c_str(), FC_LANG, FcTypeString, lang, NULL);
    if (slant != -1) {
        FcPatternAddInteger(p, FC_SLANT, slant);
    }
    if (weight != -1) {
        FcPatternAddInteger(p, FC_WEIGHT, weight);
    }
    if (width != -1) {
        FcPatternAddInteger(p, FC_WIDTH, width);
    }
    if (spacing != -1) {
        FcPatternAddInteger(p, FC_SPACING, spacing);
    }

    return p;
}
#endif

GooString *GlobalParams::findFontFile(const std::string &fontName)
{
    GooString *path = nullptr;

    setupBaseFonts(POPPLER_FONTSDIR);
    globalParamsLocker();
    const auto fontFile = fontFiles.find(fontName);
    if (fontFile != fontFiles.end()) {
        path = new GooString(fontFile->second);
    }

    return path;
}

#if defined(WITH_FONTCONFIGURATION_FONTCONFIG) || defined(WITH_FONTCONFIGURATION_WIN32)

static bool supportedFontForEmbedding(Unicode uChar, const char *filepath, int faceIndex)
{
    if (!GooString::endsWith(filepath, ".ttf") && !GooString::endsWith(filepath, ".ttc") && !GooString::endsWith(filepath, ".otf")) {
        // for now we only support ttf, ttc, otf fonts
        return false;
    }

    const FoFiIdentifierType fontFoFiType = FoFiIdentifier::identifyFile(filepath);
    if (fontFoFiType != fofiIdTrueType && fontFoFiType != fofiIdTrueTypeCollection && fontFoFiType != fofiIdOpenTypeCFF8Bit && fontFoFiType != fofiIdOpenTypeCFFCID) {
        // for now we only support ttf, ttc, otf fonts
        return false;
    }

    const std::unique_ptr<FoFiTrueType> fft = FoFiTrueType::load(filepath, faceIndex);
    if (!fft) {
        error(errIO, -1, "Form::addFontToDefaultResources. Failed to FoFiTrueType::load {0:s}", filepath);
        return false;
    }

    // Look for the Unicode BMP cmaps, which are 0/3 or 3/1
    int unicodeBMPCMap = fft->findCmap(0, 3);
    if (unicodeBMPCMap < 0) {
        unicodeBMPCMap = fft->findCmap(3, 1);
    }
    if (unicodeBMPCMap < 0) {
        // for now we only support files with unicode bmp cmaps
        return false;
    }

    const int glyph = fft->mapCodeToGID(unicodeBMPCMap, uChar);
    return glyph > 0;
}

#endif

/* if you can't or don't want to use Fontconfig, you need to implement
   this function for your platform. For Windows, it's in GlobalParamsWin.cc
*/
#ifdef WITH_FONTCONFIGURATION_FONTCONFIG
// not needed for fontconfig
void GlobalParams::setupBaseFonts(const char *) { }

GooString *GlobalParams::findBase14FontFile(const GooString *base14Name, const GfxFont *font, GooString *substituteFontName)
{
    SysFontType type;
    int fontNum;

    return findSystemFontFile(font, &type, &fontNum, substituteFontName, base14Name);
}

GooString *GlobalParams::findSystemFontFile(const GfxFont *font, SysFontType *type, int *fontNum, GooString *substituteFontName, const GooString *base14Name)
{
    const SysFontInfo *fi = nullptr;
    FcPattern *p = nullptr;
    GooString *path = nullptr;
    const std::optional<std::string> &fontName = font->getName();
    GooString substituteName;
    if (!fontName) {
        return nullptr;
    }

    globalParamsLocker();

    if ((fi = sysFonts->find(*fontName, font->isFixedWidth(), true))) {
        path = fi->path->copy();
        *type = fi->type;
        *fontNum = fi->fontNum;
        substituteName.Set(fi->substituteName->c_str());
    } else {
        FcChar8 *s;
        char *ext;
        FcResult res;
        FcFontSet *set;
        int i;
        FcLangSet *lb = nullptr;
        p = buildFcPattern(font, base14Name);

        if (!p) {
            goto fin;
        }
        FcConfigSubstitute(nullptr, p, FcMatchPattern);
        FcDefaultSubstitute(p);
        set = FcFontSort(nullptr, p, FcFalse, nullptr, &res);
        if (!set) {
            goto fin;
        }

        // find the language we want the font to support
        const char *lang = getFontLang(font);
        if (strcmp(lang, "xx") != 0) {
            lb = FcLangSetCreate();
            FcLangSetAdd(lb, (FcChar8 *)lang);
        }

        /*
          scan twice.
          first: fonts support the language
          second: all fonts (fall back)
        */
        while (fi == nullptr) {
            for (i = 0; i < set->nfont; ++i) {
                res = FcPatternGetString(set->fonts[i], FC_FILE, 0, &s);
                if (res != FcResultMatch || !s) {
                    continue;
                }
                if (lb != nullptr) {
                    FcLangSet *l;
                    res = FcPatternGetLangSet(set->fonts[i], FC_LANG, 0, &l);
                    if (res != FcResultMatch || !FcLangSetContains(l, lb)) {
                        continue;
                    }
                }
                FcChar8 *s2;
                res = FcPatternGetString(set->fonts[i], FC_FULLNAME, 0, &s2);
                if (res == FcResultMatch && s2) {
                    substituteName.Set((char *)s2);
                } else {
                    // fontconfig does not extract fullname for some fonts
                    // create the fullname from family and style
                    res = FcPatternGetString(set->fonts[i], FC_FAMILY, 0, &s2);
                    if (res == FcResultMatch && s2) {
                        substituteName.Set((char *)s2);
                        res = FcPatternGetString(set->fonts[i], FC_STYLE, 0, &s2);
                        if (res == FcResultMatch && s2) {
                            GooString *style = new GooString((char *)s2);
                            if (style->cmp("Regular") != 0) {
                                substituteName.append(" ");
                                substituteName.append(style);
                            }
                            delete style;
                        }
                    }
                }
                ext = strrchr((char *)s, '.');
                if (!ext) {
                    continue;
                }
                if (!strncasecmp(ext, ".ttf", 4) || !strncasecmp(ext, ".ttc", 4) || !strncasecmp(ext, ".otf", 4)) {
                    int weight, slant;
                    bool bold = font->isBold();
                    bool italic = font->isItalic();
                    bool oblique = false;
                    FcPatternGetInteger(set->fonts[i], FC_WEIGHT, 0, &weight);
                    FcPatternGetInteger(set->fonts[i], FC_SLANT, 0, &slant);
                    if (weight == FC_WEIGHT_DEMIBOLD || weight == FC_WEIGHT_BOLD || weight == FC_WEIGHT_EXTRABOLD || weight == FC_WEIGHT_BLACK) {
                        bold = true;
                    }
                    if (slant == FC_SLANT_ITALIC) {
                        italic = true;
                    }
                    if (slant == FC_SLANT_OBLIQUE) {
                        oblique = true;
                    }
                    *fontNum = 0;
                    *type = (!strncasecmp(ext, ".ttc", 4)) ? sysFontTTC : sysFontTTF;
                    FcPatternGetInteger(set->fonts[i], FC_INDEX, 0, fontNum);
                    SysFontInfo *sfi = new SysFontInfo(new GooString(*fontName), bold, italic, oblique, font->isFixedWidth(), new GooString((char *)s), *type, *fontNum, substituteName.copy());
                    sysFonts->addFcFont(sfi);
                    fi = sfi;
                    path = new GooString((char *)s);
                } else if (!strncasecmp(ext, ".pfa", 4) || !strncasecmp(ext, ".pfb", 4)) {
                    int weight, slant;
                    bool bold = font->isBold();
                    bool italic = font->isItalic();
                    bool oblique = false;
                    FcPatternGetInteger(set->fonts[i], FC_WEIGHT, 0, &weight);
                    FcPatternGetInteger(set->fonts[i], FC_SLANT, 0, &slant);
                    if (weight == FC_WEIGHT_DEMIBOLD || weight == FC_WEIGHT_BOLD || weight == FC_WEIGHT_EXTRABOLD || weight == FC_WEIGHT_BLACK) {
                        bold = true;
                    }
                    if (slant == FC_SLANT_ITALIC) {
                        italic = true;
                    }
                    if (slant == FC_SLANT_OBLIQUE) {
                        oblique = true;
                    }
                    *fontNum = 0;
                    *type = (!strncasecmp(ext, ".pfa", 4)) ? sysFontPFA : sysFontPFB;
                    FcPatternGetInteger(set->fonts[i], FC_INDEX, 0, fontNum);
                    SysFontInfo *sfi = new SysFontInfo(new GooString(*fontName), bold, italic, oblique, font->isFixedWidth(), new GooString((char *)s), *type, *fontNum, substituteName.copy());
                    sysFonts->addFcFont(sfi);
                    fi = sfi;
                    path = new GooString((char *)s);
                } else {
                    continue;
                }
                break;
            }
            if (lb != nullptr) {
                FcLangSetDestroy(lb);
                lb = nullptr;
            } else {
                /* scan all fonts of the list */
                break;
            }
        }
        FcFontSetDestroy(set);
    }
    if (path == nullptr && (fi = sysFonts->find(*fontName, font->isFixedWidth(), false))) {
        path = fi->path->copy();
        *type = fi->type;
        *fontNum = fi->fontNum;
    }
    if (substituteFontName) {
        substituteFontName->Set(substituteName.c_str());
    }
fin:
    if (p) {
        FcPatternDestroy(p);
    }

    return path;
}

FamilyStyleFontSearchResult GlobalParams::findSystemFontFileForFamilyAndStyle(const std::string &fontFamily, const std::string &fontStyle, const std::vector<std::string> &filesToIgnore)
{
    FcPattern *p = FcPatternBuild(nullptr, FC_FAMILY, FcTypeString, fontFamily.c_str(), FC_STYLE, FcTypeString, fontStyle.c_str(), nullptr);
    FcConfigSubstitute(nullptr, p, FcMatchPattern);
    FcDefaultSubstitute(p);
    if (p) {
        const std::unique_ptr<FcPattern, void (*)(FcPattern *)> pDeleter(p, [](FcPattern *pattern) { FcPatternDestroy(pattern); });
        FcResult res;
        FcFontSet *fontSet = FcFontSort(nullptr, p, FcFalse, nullptr, &res);
        if (fontSet) {
            const std::unique_ptr<FcFontSet, void (*)(FcFontSet *)> fontSetDeleter(fontSet, [](FcFontSet *fSet) { FcFontSetDestroy(fSet); });
            if (res == FcResultMatch) {
                for (int i = 0; i < fontSet->nfont; i++) {
                    FcChar8 *fcFilePath = nullptr;
                    int faceIndex = 0;
                    FcPatternGetString(fontSet->fonts[i], FC_FILE, 0, &fcFilePath);
                    FcPatternGetInteger(fontSet->fonts[i], FC_INDEX, 0, &faceIndex);

                    const std::string sFilePath = reinterpret_cast<char *>(fcFilePath);
                    if (std::find(filesToIgnore.begin(), filesToIgnore.end(), sFilePath) == filesToIgnore.end()) {
                        return FamilyStyleFontSearchResult(sFilePath, faceIndex);
                    }
                }
            }
        }
    }

    error(errIO, -1, "Couldn't find font file for {0:s} {1:s}", fontFamily.c_str(), fontStyle.c_str());
    return {};
}

UCharFontSearchResult GlobalParams::findSystemFontFileForUChar(Unicode uChar, const GfxFont &fontToEmulate)
{
    FcPattern *pattern = buildFcPattern(&fontToEmulate, nullptr);

    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult result = FcResultMatch;
    FcFontSet *fontSet = FcFontSort(nullptr, pattern, FcFalse, nullptr, &result);
    FcPatternDestroy(pattern);

    if (fontSet) {
        const std::unique_ptr<FcFontSet, void (*)(FcFontSet *)> fontSetDeleter(fontSet, [](FcFontSet *fSet) { FcFontSetDestroy(fSet); });
        for (int i = 0; i < fontSet->nfont; i++) {
            FcChar8 *fcFilePath = nullptr;
            int faceIndex = 0;
            FcChar8 *fcFamily = nullptr;
            FcChar8 *fcStyle = nullptr;
            FcPatternGetString(fontSet->fonts[i], FC_FILE, 0, &fcFilePath);
            FcPatternGetInteger(fontSet->fonts[i], FC_INDEX, 0, &faceIndex);
            FcPatternGetString(fontSet->fonts[i], FC_FAMILY, 0, &fcFamily);
            FcPatternGetString(fontSet->fonts[i], FC_STYLE, 0, &fcStyle);
            if (!fcFilePath || !fcFamily || !fcStyle) {
                continue;
            }

            const char *filepath = reinterpret_cast<char *>(fcFilePath);

            if (supportedFontForEmbedding(uChar, filepath, faceIndex)) {
                return UCharFontSearchResult(filepath, faceIndex, reinterpret_cast<char *>(fcFamily), reinterpret_cast<char *>(fcStyle));
            }
        }
    }

    return {};
}
#elif defined(WITH_FONTCONFIGURATION_ANDROID)
// Uses the font file mapping created by GlobalParams::setupBaseFonts
// to return the path to a base-14 font file
GooString *GlobalParams::findBase14FontFile(const GooString *base14Name, const GfxFont *font, GooString *substituteFontName)
{
    return findFontFile(base14Name->toStr());
}

#    if __ANDROID_API__ >= 29

// This struct is used by the AFontMatcher unique_ptr for destroying the
// AFontMatcher object
struct AFontMatcherDestroyer
{
    void operator()(AFontMatcher *fontmatcher) { AFontMatcher_destroy(fontmatcher); }
};

// This struct is used by the AFontMatcher unique_ptr for destroying the
// AFont object
struct AFontDestroyer
{
    void operator()(AFont *afont) { AFont_close(afont); }
};

#    endif

GooString *GlobalParams::findSystemFontFile(const GfxFont *font, SysFontType *type, int *fontNum, GooString *substituteFontName, const GooString *base14Name)
{
    GooString *path = nullptr;
    const std::optional<std::string> &fontName = font->getName();

    if (!fontName) {
        return nullptr;
    }

    globalParamsLocker();

#    if __ANDROID_API__ >= 29
    // If font is not found in the default base-14 fonts,
    // use Android-NDK's AFontMatcher API instead.
    // Documentation for AFontMatcher API can be found at:
    // https://developer.android.com/ndk/reference/group/font
    std::string genericFontFamily = "serif";

    if (!font->isSerif()) {
        genericFontFamily = "sans-serif";
    } else if (font->isFixedWidth()) {
        genericFontFamily = "monospace";
    }

    std::unique_ptr<AFontMatcher, AFontMatcherDestroyer> fontmatcher { AFontMatcher_create() };

    // Set font weight and italics for the font.
    AFontMatcher_setStyle(fontmatcher.get(), font->getWeight() * 100, font->isItalic());

    // Get font match and the font file's path
    std::unique_ptr<AFont, AFontDestroyer> afont { AFontMatcher_match(fontmatcher.get(), genericFontFamily.c_str(), (uint16_t *)u"A", 1, nullptr) };
    path = new GooString(AFont_getFontFilePath(afont.get()));

    // Set the type of font. Fonts returned by AFontMatcher are of
    // four possible types - ttf, otf, ttc, otc.
    if (path->endsWith(".ttf") || path->endsWith(".otf")) {
        *type = sysFontTTF;
    } else if (path->endsWith(".ttc") || path->endsWith(".otc")) {
        *type = sysFontTTC;
    }
#    else
#        pragma message("Compiling without AFontMatcher API due to Android API version being lower than 29.")
#    endif

    return path;
}

static struct
{
    const char *name;
    const char *otFileName;
} displayFontTab[] = { { "Courier", "NimbusMonoPS-Regular.otf" },
                       { "Courier-Bold", "NimbusMonoPS-Bold.otf" },
                       { "Courier-BoldOblique", "NimbusMonoPS-BoldItalic.otf" },
                       { "Courier-Oblique", "NimbusMonoPS-Italic.otf" },
                       { "Helvetica", "NimbusSans-Regular.otf" },
                       { "Helvetica-Bold", "NimbusSans-Bold.otf" },
                       { "Helvetica-BoldOblique", "NimbusSans-BoldItalic.otf" },
                       { "Helvetica-Oblique", "NimbusSans-Italic.otf" },
                       { "Symbol", "StandardSymbolsPS.otf" },
                       { "Times-Bold", "NimbusRoman-Bold.otf" },
                       { "Times-BoldItalic", "NimbusRoman-BoldItalic.otf" },
                       { "Times-Italic", "NimbusRoman-Italic.otf" },
                       { "Times-Roman", "NimbusRoman-Regular.otf" },
                       { "ZapfDingbats", "D050000L.otf" },
                       { nullptr, nullptr } };

// The path to the font directory. Set by GlobalParams::setFontDir()
static std::string displayFontDir;

// This method creates a mapping from base-14 font names to their
// paths on the file system. On Android, it searches within the
// directory set by GlobalParams::setFontDir().
void GlobalParams::setupBaseFonts(const char *dir)
{
    FILE *f;
    int i;

    for (i = 0; displayFontTab[i].name; ++i) {
        if (fontFiles.count(displayFontTab[i].name) > 0) {
            continue;
        }

        std::unique_ptr<GooString> fontName = std::make_unique<GooString>(displayFontTab[i].name);
        std::unique_ptr<GooString> fileName;
        if (dir) {
            fileName.reset(appendToPath(new GooString(dir), displayFontTab[i].otFileName));
            if ((f = openFile(fileName->c_str(), "rb"))) {
                fclose(f);
            } else {
                fileName.reset();
            }
        }
        if (!displayFontDir.empty()) {
            fileName.reset(appendToPath(new GooString(displayFontDir), displayFontTab[i].otFileName));
            if ((f = openFile(fileName->c_str(), "rb"))) {
                fclose(f);
            } else {
                fileName.reset();
            }
        }
        if (!fileName) {
            error(errConfig, -1, "No display font for '{0:s}'", displayFontTab[i].name);
            continue;
        }
        addFontFile(fontName->toStr(), fileName->toStr());
    }
}

FamilyStyleFontSearchResult GlobalParams::findSystemFontFileForFamilyAndStyle(const std::string &fontFamily, const std::string &fontStyle, const std::vector<std::string> &filesToIgnore)
{
    error(errUnimplemented, -1, "GlobalParams::findSystemFontFileForFamilyAndStyle not implemented for this platform");
    return {};
}

UCharFontSearchResult GlobalParams::findSystemFontFileForUChar(Unicode uChar, const GfxFont &fontToEmulate)
{
    error(errUnimplemented, -1, "GlobalParams::findSystemFontFileForUChar not implemented for this platform");
    return {};
}

#elif defined(WITH_FONTCONFIGURATION_WIN32)
#    include "GlobalParamsWin.cc"

GooString *GlobalParams::findBase14FontFile(const GooString *base14Name, const GfxFont *font, GooString * /*substituteFontName*/)
{
    return findFontFile(base14Name->toStr());
}

#else

FamilyStyleFontSearchResult GlobalParams::findSystemFontFileForFamilyAndStyle(const std::string &fontFamily, const std::string &fontStyle, const std::vector<std::string> &filesToIgnore)
{
    error(errUnimplemented, -1, "GlobalParams::findSystemFontFileForFamilyAndStyle not implemented for this platform");
    return {};
}

UCharFontSearchResult GlobalParams::findSystemFontFileForUChar(Unicode uChar, const GfxFont &fontToEmulate)
{
    error(errUnimplemented, -1, "GlobalParams::findSystemFontFileForUChar not implemented for this platform");
    return {};
}

GooString *GlobalParams::findBase14FontFile(const GooString *base14Name, const GfxFont *font, GooString * /*substituteFontName*/)
{
    return findFontFile(base14Name->toStr());
}

static struct
{
    const char *name;
    const char *t1FileName;
    const char *ttFileName;
} displayFontTab[] = { { "Courier", "n022003l.pfb", "cour.ttf" },
                       { "Courier-Bold", "n022004l.pfb", "courbd.ttf" },
                       { "Courier-BoldOblique", "n022024l.pfb", "courbi.ttf" },
                       { "Courier-Oblique", "n022023l.pfb", "couri.ttf" },
                       { "Helvetica", "n019003l.pfb", "arial.ttf" },
                       { "Helvetica-Bold", "n019004l.pfb", "arialbd.ttf" },
                       { "Helvetica-BoldOblique", "n019024l.pfb", "arialbi.ttf" },
                       { "Helvetica-Oblique", "n019023l.pfb", "ariali.ttf" },
                       { "Symbol", "s050000l.pfb", nullptr },
                       { "Times-Bold", "n021004l.pfb", "timesbd.ttf" },
                       { "Times-BoldItalic", "n021024l.pfb", "timesbi.ttf" },
                       { "Times-Italic", "n021023l.pfb", "timesi.ttf" },
                       { "Times-Roman", "n021003l.pfb", "times.ttf" },
                       { "ZapfDingbats", "d050000l.pfb", nullptr },
                       { nullptr, nullptr, nullptr } };

static const char *displayFontDirs[] = { "/usr/share/ghostscript/fonts", "/usr/local/share/ghostscript/fonts", "/usr/share/fonts/default/Type1", "/usr/share/fonts/default/ghostscript", "/usr/share/fonts/type1/gsfonts", nullptr };

void GlobalParams::setupBaseFonts(const char *dir)
{
    FILE *f;
    int i, j;

    for (i = 0; displayFontTab[i].name; ++i) {
        if (fontFiles.count(displayFontTab[i].name) > 0) {
            continue;
        }
        std::unique_ptr<GooString> fontName = std::make_unique<GooString>(displayFontTab[i].name);
        std::unique_ptr<GooString> fileName;
        if (dir) {
            fileName.reset(appendToPath(new GooString(dir), displayFontTab[i].t1FileName));
            if ((f = openFile(fileName->c_str(), "rb"))) {
                fclose(f);
            } else {
                fileName.reset();
            }
        }
        for (j = 0; !fileName && displayFontDirs[j]; ++j) {
            fileName.reset(appendToPath(new GooString(displayFontDirs[j]), displayFontTab[i].t1FileName));
            if ((f = openFile(fileName->c_str(), "rb"))) {
                fclose(f);
            } else {
                fileName.reset();
            }
        }
        if (!fileName) {
            error(errConfig, -1, "No display font for '{0:s}'", displayFontTab[i].name);
            continue;
        }
        addFontFile(fontName->toStr(), fileName->toStr());
    }
}

GooString *GlobalParams::findSystemFontFile(const GfxFont *font, SysFontType *type, int *fontNum, GooString * /*substituteFontName*/, const GooString * /*base14Name*/)
{
    const SysFontInfo *fi;
    GooString *path;

    const std::optional<std::string> &fontName = font->getName();
    if (!fontName) {
        return nullptr;
    }

    path = nullptr;
    globalParamsLocker();
    if ((fi = sysFonts->find(*fontName, font->isFixedWidth(), false))) {
        path = fi->path->copy();
        *type = fi->type;
        *fontNum = fi->fontNum;
    }

    return path;
}
#endif

std::string GlobalParams::getTextEncodingName() const
{
    globalParamsLocker();
    return textEncoding->toStr();
}

const UnicodeMap *GlobalParams::getUtf8Map()
{
    if (!utf8Map) {
        utf8Map = globalParams->getUnicodeMap("UTF-8");
    }

    return utf8Map;
}

bool GlobalParams::getPrintCommands()
{
    globalParamsLocker();
    return printCommands;
}

bool GlobalParams::getProfileCommands()
{
    globalParamsLocker();
    return profileCommands;
}

bool GlobalParams::getErrQuiet()
{
    // no locking -- this function may get called from inside a locked
    // section
    return errQuiet;
}

CharCodeToUnicode *GlobalParams::getCIDToUnicode(const GooString *collection)
{
    CharCodeToUnicode *ctu;

    globalParamsLocker();
    if (!(ctu = cidToUnicodeCache->getCharCodeToUnicode(collection))) {
        const auto cidToUnicode = cidToUnicodes.find(collection->toStr());
        if (cidToUnicode != cidToUnicodes.end()) {
            if ((ctu = CharCodeToUnicode::parseCIDToUnicode(cidToUnicode->second.c_str(), collection))) {
                cidToUnicodeCache->add(ctu);
            }
        }
    }

    return ctu;
}

const UnicodeMap *GlobalParams::getUnicodeMap(const std::string &encodingName)
{
    const UnicodeMap *map;

    if (!(map = getResidentUnicodeMap(encodingName))) {
        unicodeMapCacheLocker();
        map = unicodeMapCache->getUnicodeMap(encodingName);
    }

    return map;
}

std::shared_ptr<CMap> GlobalParams::getCMap(const GooString *collection, const GooString *cMapName)
{
    cMapCacheLocker();
    return cMapCache->getCMap(collection, cMapName);
}

const UnicodeMap *GlobalParams::getTextEncoding()
{
    return getUnicodeMap(textEncoding->toStr());
}

std::vector<std::string> GlobalParams::getEncodingNames()
{
    std::vector<std::string> result;
    result.reserve(residentUnicodeMaps.size() + unicodeMaps.size());
    for (const auto &unicodeMap : residentUnicodeMaps) {
        result.push_back(unicodeMap.first);
    }
    for (const auto &unicodeMap : unicodeMaps) {
        result.push_back(unicodeMap.first);
    }
    return result;
}

//------------------------------------------------------------------------
// functions to set parameters
//------------------------------------------------------------------------

void GlobalParams::addFontFile(const std::string &fontName, const std::string &path)
{
    globalParamsLocker();
    fontFiles[fontName] = path;
}

void GlobalParams::setTextEncoding(const char *encodingName)
{
    globalParamsLocker();
    delete textEncoding;
    textEncoding = new GooString(encodingName);
}

void GlobalParams::setPrintCommands(bool printCommandsA)
{
    globalParamsLocker();
    printCommands = printCommandsA;
}

void GlobalParams::setProfileCommands(bool profileCommandsA)
{
    globalParamsLocker();
    profileCommands = profileCommandsA;
}

void GlobalParams::setErrQuiet(bool errQuietA)
{
    globalParamsLocker();
    errQuiet = errQuietA;
}

#ifdef ANDROID
void GlobalParams::setFontDir(const std::string &fontDir)
{
    displayFontDir = fontDir;
}
#endif

GlobalParamsIniter::GlobalParamsIniter(ErrorCallback errorCallback)
{
    const std::scoped_lock lock { mutex };

    if (count == 0) {
        globalParams = std::make_unique<GlobalParams>(!customDataDir.empty() ? customDataDir.c_str() : nullptr);

        setErrorCallback(errorCallback);
    }

    count++;
}

GlobalParamsIniter::~GlobalParamsIniter()
{
    const std::scoped_lock lock { mutex };

    --count;

    if (count == 0) {
        globalParams.reset();
    }
}

bool GlobalParamsIniter::setCustomDataDir(const std::string &dir)
{
    const std::scoped_lock lock { mutex };

    if (count == 0) {
        customDataDir = dir;
        return true;
    }

    return false;
}

std::mutex GlobalParamsIniter::mutex;
int GlobalParamsIniter::count = 0;
std::string GlobalParamsIniter::customDataDir;
