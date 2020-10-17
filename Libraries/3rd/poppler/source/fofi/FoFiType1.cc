//========================================================================
//
// FoFiType1.cc
//
// Copyright 1999-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005, 2008, 2010, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2010 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2014 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2017 Jean Ghali <jghali@libertysurf.fr>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstdlib>
#include <cstring>
#include <climits>
#include "goo/glibc.h"
#include "goo/gmem.h"
#include "goo/GooLikely.h"
#include "FoFiEncodings.h"
#include "FoFiType1.h"
#include "poppler/Error.h"

//------------------------------------------------------------------------
// FoFiType1
//------------------------------------------------------------------------

FoFiType1 *FoFiType1::make(const char *fileA, int lenA)
{
    return new FoFiType1(fileA, lenA, false);
}

FoFiType1 *FoFiType1::load(const char *fileName)
{
    char *fileA;
    int lenA;

    if (!(fileA = FoFiBase::readFile(fileName, &lenA))) {
        return nullptr;
    }
    return new FoFiType1(fileA, lenA, true);
}

FoFiType1::FoFiType1(const char *fileA, int lenA, bool freeFileDataA) : FoFiBase(fileA, lenA, freeFileDataA)
{
    name = nullptr;
    encoding = nullptr;
    fontMatrix[0] = 0.001;
    fontMatrix[1] = 0;
    fontMatrix[2] = 0;
    fontMatrix[3] = 0.001;
    fontMatrix[4] = 0;
    fontMatrix[5] = 0;
    parsed = false;
    undoPFB();
}

FoFiType1::~FoFiType1()
{
    int i;

    if (name) {
        gfree(name);
    }
    if (encoding && encoding != fofiType1StandardEncoding) {
        for (i = 0; i < 256; ++i) {
            gfree(encoding[i]);
        }
        gfree(encoding);
    }
}

const char *FoFiType1::getName()
{
    if (!parsed) {
        parse();
    }
    return name;
}

char **FoFiType1::getEncoding()
{
    if (!parsed) {
        parse();
    }
    return encoding;
}

void FoFiType1::getFontMatrix(double *mat)
{
    int i;

    if (!parsed) {
        parse();
    }
    for (i = 0; i < 6; ++i) {
        mat[i] = fontMatrix[i];
    }
}

void FoFiType1::writeEncoded(const char **newEncoding, FoFiOutputFunc outputFunc, void *outputStream) const
{
    char buf[512];
    char *line, *line2, *p;
    int i;

    // copy everything up to the encoding
    for (line = (char *)file; line && strncmp(line, "/Encoding", 9); line = getNextLine(line))
        ;
    if (!line) {
        // no encoding - just copy the whole font file
        (*outputFunc)(outputStream, (char *)file, len);
        return;
    }
    (*outputFunc)(outputStream, (char *)file, line - (char *)file);

    // write the new encoding
    (*outputFunc)(outputStream, "/Encoding 256 array\n", 20);
    (*outputFunc)(outputStream, "0 1 255 {1 index exch /.notdef put} for\n", 40);
    for (i = 0; i < 256; ++i) {
        if (newEncoding[i]) {
            sprintf(buf, "dup %d /%s put\n", i, newEncoding[i]);
            (*outputFunc)(outputStream, buf, strlen(buf));
        }
    }
    (*outputFunc)(outputStream, "readonly def\n", 13);

    // find the end of the encoding data
    //~ this ought to parse PostScript tokens
    if (!strncmp(line, "/Encoding StandardEncoding def", 30)) {
        line = getNextLine(line);
    } else {
        // skip "/Encoding" + one whitespace char,
        // then look for 'def' preceded by PostScript whitespace
        p = line + 10;
        line = nullptr;
        for (; p < (char *)file + len; ++p) {
            if ((*p == ' ' || *p == '\t' || *p == '\x0a' || *p == '\x0d' || *p == '\x0c' || *p == '\0') && p + 4 <= (char *)file + len && !strncmp(p + 1, "def", 3)) {
                line = p + 4;
                break;
            }
        }
    }

    // some fonts have two /Encoding entries in their dictionary, so we
    // check for a second one here
    if (line) {
        for (line2 = line, i = 0; i < 20 && line2 && strncmp(line2, "/Encoding", 9); line2 = getNextLine(line2), ++i)
            ;
        if (i < 20 && line2) {
            (*outputFunc)(outputStream, line, line2 - line);
            if (!strncmp(line2, "/Encoding StandardEncoding def", 30)) {
                line = getNextLine(line2);
            } else {
                // skip "/Encoding" + one whitespace char,
                // then look for 'def' preceded by PostScript whitespace
                p = line2 + 10;
                line = nullptr;
                for (; p < (char *)file + len; ++p) {
                    if ((*p == ' ' || *p == '\t' || *p == '\x0a' || *p == '\x0d' || *p == '\x0c' || *p == '\0') && p + 4 <= (char *)file + len && !strncmp(p + 1, "def", 3)) {
                        line = p + 4;
                        break;
                    }
                }
            }
        }

        // copy everything after the encoding
        if (line) {
            (*outputFunc)(outputStream, line, ((char *)file + len) - line);
        }
    }
}

char *FoFiType1::getNextLine(char *line) const
{
    while (line < (char *)file + len && *line != '\x0a' && *line != '\x0d') {
        ++line;
    }
    if (line < (char *)file + len && *line == '\x0d') {
        ++line;
    }
    if (line < (char *)file + len && *line == '\x0a') {
        ++line;
    }
    if (line >= (char *)file + len) {
        return nullptr;
    }
    return line;
}

void FoFiType1::parse()
{
    char *line, *line1, *firstLine, *p, *p2;
    char buf[256];
    char c;
    int n, code, base, i, j;
    char *tokptr;
    bool gotMatrix, continueLine;

    gotMatrix = false;
    for (i = 1, line = (char *)file; i <= 100 && line && (!name || !encoding); ++i) {

        // get font name
        if (!name && (line + 9 <= (char *)file + len) && !strncmp(line, "/FontName", 9)) {
            const auto availableFile = (char *)file + len - line;
            const int lineLen = availableFile < 255 ? availableFile : 255;
            strncpy(buf, line, lineLen);
            buf[lineLen] = '\0';
            if ((p = strchr(buf + 9, '/')) && (p = strtok_r(p + 1, " \t\n\r", &tokptr))) {
                name = copyString(p);
            }
            line = getNextLine(line);

            // get encoding
        } else if (!encoding && (line + 30 <= (char *)file + len) && !strncmp(line, "/Encoding StandardEncoding def", 30)) {
            encoding = (char **)fofiType1StandardEncoding;
        } else if (!encoding && (line + 19 <= (char *)file + len) && !strncmp(line, "/Encoding 256 array", 19)) {
            encoding = (char **)gmallocn(256, sizeof(char *));
            for (j = 0; j < 256; ++j) {
                encoding[j] = nullptr;
            }
            continueLine = false;
            for (j = 0, line = getNextLine(line); j < 300 && line && (line1 = getNextLine(line)); ++j, line = line1) {
                if ((n = (int)(line1 - line)) > 255) {
                    error(errSyntaxWarning, -1, "FoFiType1::parse a line has more than 255 characters, we don't support this");
                    n = 255;
                }
                if (continueLine) {
                    continueLine = false;
                    if ((line1 - firstLine) + 1 > (int)sizeof(buf))
                        break;
                    p = firstLine;
                    p2 = buf;
                    while (p < line1) {
                        if (*p == '\n' || *p == '\r') {
                            *p2++ = ' ';
                            p++;
                        } else {
                            *p2++ = *p++;
                        }
                    }
                    *p2 = '\0';
                } else {
                    firstLine = line;
                    strncpy(buf, line, n);
                    buf[n] = '\0';
                }
                for (p = buf; *p == ' ' || *p == '\t'; ++p)
                    ;
                if (!strncmp(p, "dup", 3)) {
                    while (true) {
                        p += 3;
                        for (; *p == ' ' || *p == '\t'; ++p)
                            ;
                        code = 0;
                        if (*p == '8' && p[1] == '#') {
                            base = 8;
                            p += 2;
                        } else if (*p >= '0' && *p <= '9') {
                            base = 10;
                        } else if (*p == '\n' || *p == '\r') {
                            continueLine = true;
                            break;
                        } else {
                            break;
                        }
                        for (; *p >= '0' && *p < '0' + base && code < INT_MAX / (base + (*p - '0')); ++p) {
                            code = code * base + (*p - '0');
                        }
                        for (; *p == ' ' || *p == '\t'; ++p)
                            ;
                        if (*p == '\n' || *p == '\r') {
                            continueLine = true;
                            break;
                        } else if (*p != '/') {
                            break;
                        }
                        ++p;
                        for (p2 = p; *p2 && *p2 != ' ' && *p2 != '\t'; ++p2)
                            ;
                        if (code >= 0 && code < 256) {
                            c = *p2;
                            *p2 = '\0';
                            gfree(encoding[code]);
                            encoding[code] = copyString(p);
                            *p2 = c;
                        }
                        for (p = p2; *p == ' ' || *p == '\t'; ++p)
                            ;
                        if (*p == '\n' || *p == '\r') {
                            continueLine = true;
                            break;
                        }
                        if (strncmp(p, "put", 3)) {
                            break;
                        }
                        for (p += 3; *p == ' ' || *p == '\t'; ++p)
                            ;
                        if (strncmp(p, "dup", 3)) {
                            break;
                        }
                    }
                } else {
                    if (strtok_r(buf, " \t", &tokptr) && (p = strtok_r(nullptr, " \t\n\r", &tokptr)) && !strcmp(p, "def")) {
                        break;
                    }
                }
            }
            //~ check for getinterval/putinterval junk

        } else if (!gotMatrix && (line + 11 <= (char *)file + len) && !strncmp(line, "/FontMatrix", 11)) {
            const auto availableFile = (char *)file + len - (line + 11);
            const int bufLen = availableFile < 255 ? availableFile : 255;
            strncpy(buf, line + 11, bufLen);
            buf[bufLen] = '\0';
            if ((p = strchr(buf, '['))) {
                ++p;
                if ((p2 = strchr(p, ']'))) {
                    *p2 = '\0';
                    for (j = 0; j < 6; ++j) {
                        if ((p = strtok(j == 0 ? p : nullptr, " \t\n\r"))) {
                            fontMatrix[j] = atof(p);
                        } else {
                            break;
                        }
                    }
                }
            }
            gotMatrix = true;

        } else {
            line = getNextLine(line);
        }
    }

    parsed = true;
}

// Undo the PFB encoding, i.e., remove the PFB headers.
void FoFiType1::undoPFB()
{
    bool ok;
    unsigned char *file2;
    int pos1, pos2, type;
    unsigned int segLen;

    ok = true;
    if (getU8(0, &ok) != 0x80 || !ok) {
        return;
    }
    file2 = (unsigned char *)gmalloc(len);
    pos1 = pos2 = 0;
    while (getU8(pos1, &ok) == 0x80 && ok) {
        type = getU8(pos1 + 1, &ok);
        if (type < 1 || type > 2 || !ok) {
            break;
        }
        segLen = getU32LE(pos1 + 2, &ok);
        pos1 += 6;
        if (!ok || !checkRegion(pos1, segLen)) {
            break;
        }
        memcpy(file2 + pos2, file + pos1, segLen);
        pos1 += segLen;
        pos2 += segLen;
    }
    if (freeFileData) {
        gfree((char *)file);
    }
    file = file2;
    freeFileData = true;
    len = pos2;
}
