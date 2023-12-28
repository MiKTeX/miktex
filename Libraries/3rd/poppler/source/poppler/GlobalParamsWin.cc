/* Written by Krzysztof Kowalczyk (http://blog.kowalczyk.info)
   but mostly based on xpdf code.

   // Copyright (C) 2010, 2012 Hib Eris <hib@hiberis.nl>
   // Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
   // Copyright (C) 2012 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
   // Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
   // Copyright (C) 2012 Mark Brand <mabrand@mabrand.nl>
   // Copyright (C) 2013, 2018, 2019 Adam Reichold <adamreichold@myopera.com>
   // Copyright (C) 2013 Dmytro Morgun <lztoad@gmail.com>
   // Copyright (C) 2017 Christoph Cullmann <cullmann@kde.org>
   // Copyright (C) 2017, 2018, 2020-2023 Albert Astals Cid <aacid@kde.org>
   // Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
   // Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
   // Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
   // Copyright (C) 2021 Stefan Löffler <st.loeffler@gmail.com>
   // Copyright (C) 2021 sunderme <sunderme@gmx.de>

TODO: instead of a fixed mapping defined in displayFontTab, it could
scan the whole fonts directory, parse TTF files and build font
description for all fonts available in Windows. That's how MuPDF works.
*/

#ifndef PACKAGE_NAME
#    include <config.h>
#endif

#include <windows.h>

#include <shlobj.h>
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cassert>

#include "goo/gmem.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "Error.h"
#include "NameToCharCode.h"
#include "CharCodeToUnicode.h"
#include "UnicodeMap.h"
#include "CMap.h"
#include "FontEncodingTables.h"
#include "GlobalParams.h"
#include "GfxFont.h"
#include <sys/stat.h>
#include "Object.h"
#include "Stream.h"
#include "Lexer.h"
#include "Parser.h"

#define DEFAULT_SUBSTITUTE_FONT "Helvetica"
#define DEFAULT_CID_FONT_AC1_MSWIN "MingLiU" /* Adobe-CNS1 for Taiwan, HongKong */
#define DEFAULT_CID_FONT_AG1_MSWIN "SimSun" /* Adobe-GB1 for PRC, Singapore */
#define DEFAULT_CID_FONT_AJ1_MSWIN "MS-Mincho" /* Adobe-Japan1 */
#define DEFAULT_CID_FONT_AJ2_MSWIN "MS-Mincho" /* Adobe-Japan2 (legacy) */
#define DEFAULT_CID_FONT_AK1_MSWIN "Batang" /* Adobe-Korea1 */
#define DEFAULT_CID_FONT_MSWIN "ArialUnicode" /* Unknown */

static const struct
{
    const char *name;
    const std::vector<std::string> fileNames;
    bool warnIfMissing;
} displayFontTab[] = { { "Courier", { "n022003l.pfb", "cour.ttf" }, true },
                       { "Courier-Bold", { "n022004l.pfb", "courbd.ttf" }, true },
                       { "Courier-BoldOblique", { "n022024l.pfb", "courbi.ttf" }, true },
                       { "Courier-Oblique", { "n022023l.pfb", "couri.ttf" }, true },
                       { "Helvetica", { "n019003l.pfb", "arial.ttf" }, true },
                       { "Helvetica-Bold", { "n019004l.pfb", "arialbd.ttf" }, true },
                       { "Helvetica-BoldOblique", { "n019024l.pfb", "arialbi.ttf" }, true },
                       { "Helvetica-Oblique", { "n019023l.pfb", "ariali.ttf" }, true },
                       { "Symbol", { "s050000l.pfb", "StandardSymbolsPS.otf", "StandardSymbolsPS.ttf" }, true },
                       { "Times-Bold", { "n021004l.pfb", "timesbd.ttf" }, true },
                       { "Times-BoldItalic", { "n021024l.pfb", "timesbi.ttf" }, true },
                       { "Times-Italic", { "n021023l.pfb", "timesi.ttf" }, true },
                       { "Times-Roman", { "n021003l.pfb", "times.ttf" }, true },
                       // TODO: not sure if "wingding.ttf" is right
                       { "ZapfDingbats", { "d050000l.pfb", "wingding.ttf" }, true },

                       // those seem to be frequently accessed by PDF files and I kind of guess
                       // which font file do the refer to
                       { "Palatino", { "pala.ttf" }, true },
                       { "Palatino-Roman", { "pala.ttf" }, true },
                       { "Palatino-Bold", { "palab.ttf" }, true },
                       { "Palatino-Italic", { "palai.ttf" }, true },
                       { "Palatino,Italic", { "palai.ttf" }, true },
                       { "Palatino-BoldItalic", { "palabi.ttf" }, true },

                       { "ArialBlack", { "arialbd.ttf" }, true },

                       { "ArialNarrow", { "arialn.ttf" }, true },
                       { "ArialNarrow,Bold", { "arialnb.ttf" }, true },
                       { "ArialNarrow,Italic", { "arialni.ttf" }, true },
                       { "ArialNarrow,BoldItalic", { "arialnbi.ttf" }, true },
                       { "ArialNarrow-Bold", { "arialnb.ttf" }, true },
                       { "ArialNarrow-Italic", { "arialni.ttf" }, true },
                       { "ArialNarrow-BoldItalic", { "arialnbi.ttf" }, true },

                       { "HelveticaNarrow", { "arialn.ttf" }, true },
                       { "HelveticaNarrow,Bold", { "arialnb.ttf" }, true },
                       { "HelveticaNarrow,Italic", { "arialni.ttf" }, true },
                       { "HelveticaNarrow,BoldItalic", { "arialnbi.ttf" }, true },
                       { "HelveticaNarrow-Bold", { "arialnb.ttf" }, true },
                       { "HelveticaNarrow-Italic", { "arialni.ttf" }, true },
                       { "HelveticaNarrow-BoldItalic", { "arialnbi.ttf" }, true },

                       { "BookAntiqua", { "bkant.ttf" }, true },
                       { "BookAntiqua,Bold", { "bkant.ttf" }, true },
                       { "BookAntiqua,Italic", { "bkant.ttf" }, true },
                       { "BookAntiqua,BoldItalic", { "bkant.ttf" }, true },
                       { "BookAntiqua-Bold", { "bkant.ttf" }, true },
                       { "BookAntiqua-Italic", { "bkant.ttf" }, true },
                       { "BookAntiqua-BoldItalic", { "bkant.ttf" }, true },

                       { "Verdana", { "verdana.ttf" }, true },
                       { "Verdana,Bold", { "verdanab.ttf" }, true },
                       { "Verdana,Italic", { "verdanai.ttf" }, true },
                       { "Verdana,BoldItalic", { "verdanaz.ttf" }, true },
                       { "Verdana-Bold", { "verdanab.ttf" }, true },
                       { "Verdana-Italic", { "verdanai.ttf" }, true },
                       { "Verdana-BoldItalic", { "verdanaz.ttf" }, true },

                       { "Tahoma", { "tahoma.ttf" }, true },
                       { "Tahoma,Bold", { "tahomabd.ttf" }, true },
                       { "Tahoma,Italic", { "tahoma.ttf" }, true },
                       { "Tahoma,BoldItalic", { "tahomabd.ttf" }, true },
                       { "Tahoma-Bold", { "tahomabd.ttf" }, true },
                       { "Tahoma-Italic", { "tahoma.ttf" }, true },
                       { "Tahoma-BoldItalic", { "tahomabd.ttf" }, true },

                       { "CCRIKH+Verdana", { "verdana.ttf" }, true },
                       { "CCRIKH+Verdana,Bold", { "verdanab.ttf" }, true },
                       { "CCRIKH+Verdana,Italic", { "verdanai.ttf" }, true },
                       { "CCRIKH+Verdana,BoldItalic", { "verdanaz.ttf" }, true },
                       { "CCRIKH+Verdana-Bold", { "verdanab.ttf" }, true },
                       { "CCRIKH+Verdana-Italic", { "verdanai.ttf" }, true },
                       { "CCRIKH+Verdana-BoldItalic", { "verdanaz.ttf" }, true },

                       { "Georgia", { "georgia.ttf" }, true },
                       { "Georgia,Bold", { "georgiab.ttf" }, true },
                       { "Georgia,Italic", { "georgiai.ttf" }, true },
                       { "Georgia,BoldItalic", { "georgiaz.ttf" }, true },
                       { "Georgia-Bold", { "georgiab.ttf" }, true },
                       { "Georgia-Italic", { "georgiai.ttf" }, true },
                       { "Georgia-BoldItalic", { "georgiaz.ttf" }, true },

                       // fallback for Adobe CID fonts:
                       { "MingLiU", { "mingliu.ttf" }, false },
                       { "SimSun", { "simsun.ttf" }, false },
                       { "MS-Mincho", { "msmincho.ttf" }, false },
                       { "Batang", { "batang.ttf" }, false },
                       { "ArialUnicode", { "arialuni.ttf" }, true },
                       {} };

static std::string GetWindowsFontDir()
{
    char winFontDir[MAX_PATH];
    winFontDir[0] = '\0';

    if (SHGetFolderPathA(nullptr, CSIDL_FONTS, nullptr, SHGFP_TYPE_CURRENT, winFontDir) == S_OK) {
        return winFontDir;
    }

    // return the windows directory + fonts
    GetWindowsDirectoryA(winFontDir, MAX_PATH);
    if (winFontDir[0]) {
        return std::string(winFontDir) + "\\fonts";
    }

    return {};
}

static bool FileExists(const char *path)
{
    FILE *f = openFile(path, "rb");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

void SysFontList::scanWindowsFonts(const std::string &winFontDir)
{
    OSVERSIONINFO version;
    const char *path;
    DWORD idx, valNameLen, dataLen, type;
    HKEY regKey;
    char valName[1024], data[1024];
    int n, fontNum;
    char *p0, *p1;
    GooString *fontPath;

    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);
    if (version.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        path = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\";
    } else {
        path = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Fonts\\";
    }
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &regKey) == ERROR_SUCCESS) {
        idx = 0;
        while (1) {
            valNameLen = sizeof(valName) - 1;
            dataLen = sizeof(data) - 1;
            if (RegEnumValueA(regKey, idx, valName, &valNameLen, nullptr, &type, (LPBYTE)data, &dataLen) != ERROR_SUCCESS) {
                break;
            }
            if (type == REG_SZ && valNameLen > 0 && valNameLen < sizeof(valName) && dataLen > 0 && dataLen < sizeof(data)) {
                valName[valNameLen] = '\0';
                data[dataLen] = '\0';
                n = strlen(data);
                if (!strcasecmp(data + n - 4, ".ttf") || !strcasecmp(data + n - 4, ".ttc") || !strcasecmp(data + n - 4, ".otf")) {
                    fontPath = new GooString(data);
                    if (!(dataLen >= 3 && data[1] == ':' && data[2] == '\\')) {
                        fontPath->insert(0, '\\');
                        fontPath->insert(0, winFontDir);
                        fontPath->append('\0');
                    }
                    p0 = valName;
                    fontNum = 0;
                    while (*p0) {
                        p1 = strstr(p0, " & ");
                        if (p1) {
                            *p1 = '\0';
                            p1 = p1 + 3;
                        } else {
                            p1 = p0 + strlen(p0);
                        }
                        fonts.push_back(makeWindowsFont(p0, fontNum, fontPath->c_str()));
                        p0 = p1;
                        ++fontNum;
                    }
                    delete fontPath;
                }
            }
            ++idx;
        }
        RegCloseKey(regKey);
    }
}

SysFontInfo *SysFontList::makeWindowsFont(const char *name, int fontNum, const char *path)
{
    int n;
    bool bold, italic, oblique, fixedWidth;
    GooString *s;
    char c;
    int i;
    SysFontType type;
    GooString substituteName;

    n = strlen(name);
    bold = italic = oblique = fixedWidth = false;

    // remove trailing ' (TrueType)'
    if (n > 11 && !strncmp(name + n - 11, " (TrueType)", 11)) {
        n -= 11;
    }

    // remove trailing ' (OpenType)'
    if (n > 11 && !strncmp(name + n - 11, " (OpenType)", 11)) {
        n -= 11;
    }

    // remove trailing ' Italic'
    if (n > 7 && !strncmp(name + n - 7, " Italic", 7)) {
        n -= 7;
        italic = true;
    }

    // remove trailing ' Oblique'
    if (n > 7 && !strncmp(name + n - 8, " Oblique", 8)) {
        n -= 8;
        oblique = true;
    }

    // remove trailing ' Bold'
    if (n > 5 && !strncmp(name + n - 5, " Bold", 5)) {
        n -= 5;
        bold = true;
    }

    // remove trailing ' Regular'
    if (n > 5 && !strncmp(name + n - 8, " Regular", 8)) {
        n -= 8;
    }

    // the familyname cannot indicate whether a font is fixedWidth or not.
    // some well-known fixedWidth typeface family names or keyword are checked.
    if (strstr(name, "Courier") || strstr(name, "Fixed") || (strstr(name, "Mono") && !strstr(name, "Monotype")) || strstr(name, "Typewriter"))
        fixedWidth = true;
    else
        fixedWidth = false;

    //----- normalize the font name
    s = new GooString(name, n);
    i = 0;
    while (i < s->getLength()) {
        c = s->getChar(i);
        if (c == ' ' || c == ',' || c == '-') {
            s->del(i);
        } else {
            ++i;
        }
    }

    if (!strcasecmp(path + strlen(path) - 4, ".ttc")) {
        type = sysFontTTC;
    } else {
        type = sysFontTTF;
    }

    return new SysFontInfo(s, bold, italic, oblique, fixedWidth, new GooString(path), type, fontNum, substituteName.copy());
}

static GooString *replaceSuffix(GooString *path, const char *suffixA, const char *suffixB)
{
    int suffLenA = strlen(suffixA);
    int suffLenB = strlen(suffixB);
    int baseLenA = path->getLength() - suffLenA;
    int baseLenB = path->getLength() - suffLenB;

    if (!strcasecmp(path->c_str() + baseLenA, suffixA)) {
        path->del(baseLenA, suffLenA)->append(suffixB);
    } else if (!strcasecmp(path->c_str() + baseLenB, suffixB)) {
        path->del(baseLenB, suffLenB)->append(suffixA);
    }

    return path;
}

void GlobalParams::setupBaseFonts(const char *dir)
{
    if (baseFontsInitialized)
        return;
    baseFontsInitialized = true;

    const std::string winFontDir = GetWindowsFontDir();

    std::vector<std::string> fontDirs;
    if (dir) {
        fontDirs.emplace_back(dir);
    }
    if (!winFontDir.empty()) {
        fontDirs.emplace_back(winFontDir);
    }

    for (int i = 0; displayFontTab[i].name; ++i) {
        if (fontFiles.count(displayFontTab[i].name) > 0)
            continue;

        const GooString fontName = GooString(displayFontTab[i].name);

        bool fontFound = false;
        for (const std::string &fontDir : fontDirs) {
            for (const std::string &fileName : displayFontTab[i].fileNames) {
                const std::unique_ptr<GooString> fontPath(appendToPath(new GooString(fontDir), fileName.c_str()));
                if (FileExists(fontPath->c_str()) || FileExists(replaceSuffix(fontPath.get(), ".pfb", ".pfa")->c_str()) || FileExists(replaceSuffix(fontPath.get(), ".ttc", ".ttf")->c_str())) {
                    addFontFile(fontName.toStr(), fontPath->toStr());
                    fontFound = true;
                    break;
                }
            }

            if (fontFound) {
                break;
            }
        }

        if (!fontFound && displayFontTab[i].warnIfMissing) {
            error(errSyntaxError, -1, "No display font for '{0:s}'", displayFontTab[i].name);
        }
    }
    if (!winFontDir.empty()) {
        sysFonts->scanWindowsFonts(winFontDir);
    }

    const char *dataRoot = popplerDataDir ? popplerDataDir : POPPLER_DATADIR;
    const std::string fileName = std::string(dataRoot).append("/cidfmap");

    // try to open file
    const std::unique_ptr<GooFile> file = GooFile::open(fileName);

    if (file) {
        Parser *parser;
        parser = new Parser(nullptr, new FileStream(file.get(), 0, false, file->size(), Object(objNull)), true);
        Object obj1 = parser->getObj();
        while (!obj1.isEOF()) {
            Object obj2 = parser->getObj();
            if (obj1.isName()) {
                // Substitutions
                if (obj2.isDict()) {
                    Object obj3 = obj2.getDict()->lookup("Path");
                    if (obj3.isString())
                        addFontFile(GooString(obj1.getName()).toStr(), obj3.getString()->toStr());
                    // Aliases
                } else if (obj2.isName()) {
                    substFiles.emplace(obj1.getName(), obj2.getName());
                }
            }
            obj1 = parser->getObj();
            // skip trailing ';'
            while (obj1.isCmd(";")) {
                obj1 = parser->getObj();
            }
        }
        delete parser;
    }
}

static const char *findSubstituteName(const GfxFont *font, const std::unordered_map<std::string, std::string> &fontFiles, const std::unordered_map<std::string, std::string> &substFiles, const char *origName)
{
    assert(origName);
    if (!origName)
        return nullptr;
    GooString *name2 = new GooString(origName);
    int n = strlen(origName);
    // remove trailing "-Identity-H"
    if (n > 11 && !strcmp(name2->c_str() + n - 11, "-Identity-H")) {
        name2->del(n - 11, 11);
        n -= 11;
    }
    // remove trailing "-Identity-V"
    if (n > 11 && !strcmp(name2->c_str() + n - 11, "-Identity-V")) {
        name2->del(n - 11, 11);
        n -= 11;
    }
    const auto substFile = substFiles.find(name2->c_str());
    if (substFile != substFiles.end()) {
        delete name2;
        return substFile->second.c_str();
    }

    /* TODO: try to at least guess bold/italic/bolditalic from the name */
    delete name2;
    if (font->isCIDFont()) {
        const GooString *collection = ((GfxCIDFont *)font)->getCollection();

        const char *name3 = nullptr;
        if (!collection->cmp("Adobe-CNS1"))
            name3 = DEFAULT_CID_FONT_AC1_MSWIN;
        else if (!collection->cmp("Adobe-GB1"))
            name3 = DEFAULT_CID_FONT_AG1_MSWIN;
        else if (!collection->cmp("Adobe-Japan1"))
            name3 = DEFAULT_CID_FONT_AJ1_MSWIN;
        else if (!collection->cmp("Adobe-Japan2"))
            name3 = DEFAULT_CID_FONT_AJ2_MSWIN;
        else if (!collection->cmp("Adobe-Korea1"))
            name3 = DEFAULT_CID_FONT_AK1_MSWIN;

        if (name3 && fontFiles.count(name3) != 0)
            return name3;

        if (fontFiles.count(DEFAULT_CID_FONT_MSWIN) != 0)
            return DEFAULT_CID_FONT_MSWIN;
    }
    return DEFAULT_SUBSTITUTE_FONT;
}

/* Windows implementation of external font matching code */
GooString *GlobalParams::findSystemFontFile(const GfxFont *font, SysFontType *type, int *fontNum, GooString *substituteFontName, const GooString *base14Name)
{
    const SysFontInfo *fi;
    GooString *path = nullptr;
    const std::optional<std::string> &fontName = font->getName();
    if (!fontName)
        return nullptr;
    const std::scoped_lock locker(mutex);
    setupBaseFonts(POPPLER_FONTSDIR);

    // TODO: base14Name should be changed?
    // In the system using FontConfig, findSystemFontFile() uses
    // base14Name only for the creation of query pattern.

    if ((fi = sysFonts->find(*fontName, false, false))) {
        path = fi->path->copy();
        *type = fi->type;
        *fontNum = fi->fontNum;
        if (substituteFontName)
            substituteFontName->Set(fi->substituteName->c_str());
    } else {
        GooString *substFontName = new GooString(findSubstituteName(font, fontFiles, substFiles, fontName->c_str()));
        error(errSyntaxError, -1, "Couldn't find a font for '{0:s}', subst is '{1:t}'", fontName->c_str(), substFontName);
        const auto fontFile = fontFiles.find(substFontName->toStr());
        if (fontFile != fontFiles.end()) {
            path = new GooString(fontFile->second.c_str());
            if (substituteFontName)
                substituteFontName->Set(path->c_str());
            if (!strcasecmp(path->c_str() + path->getLength() - 4, ".ttc")) {
                *type = sysFontTTC;
            } else {
                *type = sysFontTTF;
            }
            *fontNum = 0;
        }
    }

    return path;
}

FamilyStyleFontSearchResult GlobalParams::findSystemFontFileForFamilyAndStyle(const std::string &fontFamily, const std::string &fontStyle, const std::vector<std::string> &filesToIgnore)
{
    const std::scoped_lock locker(mutex);
    setupBaseFonts(POPPLER_FONTSDIR);

    const std::string familyAndStyle = fontFamily + " " + fontStyle;

    const SysFontInfo *fi = sysFonts->find(familyAndStyle, false, false, filesToIgnore);
    if (fi) {
        return FamilyStyleFontSearchResult(fi->path->toStr(), fi->fontNum);
    }

    return {};
}

UCharFontSearchResult GlobalParams::findSystemFontFileForUChar(Unicode uChar, const GfxFont &fontToEmulate)
{
    const std::scoped_lock locker(mutex);
    setupBaseFonts(POPPLER_FONTSDIR);

    const std::vector<SysFontInfo *> &fonts = sysFonts->getFonts();
    for (SysFontInfo *f : fonts) {
        // This is not super great given that it ignores fontToEmulate, but will do for now
        if (supportedFontForEmbedding(uChar, f->path->c_str(), f->fontNum)) {
            std::string style;
            if (f->italic) {
                style = "Italic";
            }
            if (f->oblique) {
                if (!style.empty()) {
                    style += " ";
                }
                style += "Oblique";
            }
            if (f->bold) {
                if (!style.empty()) {
                    style += " ";
                }
                style += "Bold";
            }
            return UCharFontSearchResult(f->path->toStr(), f->fontNum, f->name->toStr(), style);
        }
    }

    return {};
}
