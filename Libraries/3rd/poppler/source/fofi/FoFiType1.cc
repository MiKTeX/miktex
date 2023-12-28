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
// Copyright (C) 2005, 2008, 2010, 2018, 2021-2023 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2005 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2010 Jakub Wilk <jwilk@jwilk.net>
// Copyright (C) 2014 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2017 Jean Ghali <jghali@libertysurf.fr>
// Copyright (C) 2022 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <charconv>
#include <optional>

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

FoFiType1 *FoFiType1::make(const unsigned char *fileA, int lenA)
{
    return new FoFiType1(fileA, lenA, false);
}

FoFiType1::FoFiType1(const unsigned char *fileA, int lenA, bool freeFileDataA) : FoFiBase(fileA, lenA, freeFileDataA)
{
    encoding = nullptr;
    parsed = false;
    undoPFB();
}

FoFiType1::~FoFiType1()
{
    if (encoding && encoding != fofiType1StandardEncoding) {
        for (int i = 0; i < 256; ++i) {
            gfree(encoding[i]);
        }
        gfree(encoding);
    }
}

std::string FoFiType1::getName()
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

void FoFiType1::writeEncoded(const char **newEncoding, FoFiOutputFunc outputFunc, void *outputStream) const
{
    char buf[512];
    char *line, *line2, *p;
    int i;

    // copy everything up to the encoding
    for (line = (char *)file; line && strncmp(line, "/Encoding", 9); line = getNextLine(line)) {
        ;
    }
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
        for (line2 = line, i = 0; i < 20 && line2 && strncmp(line2, "/Encoding", 9); line2 = getNextLine(line2), ++i) {
            ;
        }
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

static const char tokenSeparators[] = " \t\n\r";

class FoFiType1Tokenizer
{
public:
    explicit FoFiType1Tokenizer(std::string_view &&stringViewA) : stringView(stringViewA) { }

    std::optional<std::string_view> getToken()
    {
        const auto length = stringView.length();
        if (currentPos >= length) {
            return {};
        }

        std::string_view::size_type pos = stringView.find_first_of(tokenSeparators, currentPos);
        while (pos == currentPos) {
            // skip multiple contiguous separators
            ++currentPos;
            pos = stringView.find_first_of(tokenSeparators, currentPos);
        }
        if (pos == std::string_view::npos) {
            std::string_view token = stringView.substr(currentPos, length - currentPos);
            currentPos = length;
            return token;
        }

        std::string_view token = stringView.substr(currentPos, pos - currentPos);

        currentPos = pos + 1;

        return token;
    }

private:
    std::string_view::size_type currentPos = 0;
    const std::string_view stringView;
};

void FoFiType1::parse()
{
    FoFiType1Tokenizer tokenizer(std::string_view(reinterpret_cast<const char *>(file), len));
    while (name.empty() || !encoding) {
        const std::optional<std::string_view> token = tokenizer.getToken();

        if (!token) {
            break;
        }

        if (name.empty() && token == "/FontName") {
            const std::optional<std::string_view> fontNameToken = tokenizer.getToken();
            if (!fontNameToken) {
                break;
            }

            // Skip the /
            name = fontNameToken->substr(1);

        } else if (!encoding && token == "/Encoding") {
            const std::optional<std::string_view> token2 = tokenizer.getToken();
            if (!token2) {
                break;
            }

            const std::optional<std::string_view> token3 = tokenizer.getToken();
            if (!token3) {
                break;
            }

            if (token2 == "StandardEncoding" && token3 == "def") {
                encoding = (char **)fofiType1StandardEncoding;
            } else if (token2 == "256" && token3 == "array") {
                encoding = (char **)gmallocn(256, sizeof(char *));
                for (int j = 0; j < 256; ++j) {
                    encoding[j] = nullptr;
                }

                while (true) {
                    const std::optional<std::string_view> encodingToken = tokenizer.getToken();
                    if (!encodingToken) {
                        break;
                    }

                    if (encodingToken == "dup") {
                        std::optional<std::string_view> codeToken = tokenizer.getToken();
                        if (!codeToken) {
                            break;
                        }

                        std::optional<std::string_view> nameToken;
                        // Sometimes font data has code and name together without spacing i.e. 33/exclam
                        // if that happens don't call getToken again and just split codeToken in 2
                        const auto slashPositionInCodeToken = codeToken->find('/');
                        if (slashPositionInCodeToken != std::string_view::npos) {
                            nameToken = codeToken->substr(slashPositionInCodeToken, codeToken->length() - slashPositionInCodeToken);
                            codeToken = codeToken->substr(0, slashPositionInCodeToken);
                        } else {
                            nameToken = tokenizer.getToken();
                        }

                        if (!nameToken) {
                            break;
                        }

                        int code = 0;
                        if (codeToken->length() > 2 && codeToken->at(0) == '8' && codeToken->at(1) == '#') {
                            std::from_chars(codeToken->data() + 2, codeToken->data() + codeToken->length(), code, 8);
                        } else {
                            std::from_chars(codeToken->data(), codeToken->data() + codeToken->length(), code);
                        }

                        if (code >= 0 && code < 256 && nameToken->length() > 1) {
                            gfree(encoding[code]);
                            encoding[code] = copyString(nameToken->data() + 1, nameToken->length() - 1);
                        }

                    } else if (encodingToken == "def") {
                        break;
                    }
                }
            }
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
