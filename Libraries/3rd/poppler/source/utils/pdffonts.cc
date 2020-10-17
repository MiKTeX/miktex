//========================================================================
//
// pdffonts.cc
//
// Copyright 2001-2007 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Dominic Lachowicz <cinamod@hotmail.com>
// Copyright (C) 2007-2008, 2010, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <memory>
#include <string>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "FontInfo.h"
#include "Win32Console.h"

static const char *fontTypeNames[] = { "unknown", "Type 1", "Type 1C", "Type 1C (OT)", "Type 3", "TrueType", "TrueType (OT)", "CID Type 0", "CID Type 0C", "CID Type 0C (OT)", "CID TrueType", "CID TrueType (OT)" };

static int firstPage = 1;
static int lastPage = 0;
static bool showSubst = false;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static bool printVersion = false;
static bool printHelp = false;

static const ArgDesc argDesc[] = { { "-f", argInt, &firstPage, 0, "first page to examine" },
                                   { "-l", argInt, &lastPage, 0, "last page to examine" },
                                   { "-subst", argFlag, &showSubst, 0, "show font substitutions" },
                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

#if defined(MIKTEX)
int Main(int argc, char** argv)
#else
int main(int argc, char *argv[])
#endif

{
    std::unique_ptr<GooString> ownerPW, userPW;
    bool ok;

    Win32Console win32Console(&argc, &argv);

    // parse args
    ok = parseArgs(argDesc, &argc, argv);
    if (!ok || argc != 2 || printVersion || printHelp) {
        fprintf(stderr, "pdffonts version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdffonts", "<PDF-file>", argDesc);
        }
        if (printVersion || printHelp)
            return 0;
        return 99;
    }

    std::string fileName(argv[1]);
    if (fileName == "-") {
        fileName = "fd://0";
    }

    // read config file
    globalParams = std::make_unique<GlobalParams>();

    // open PDF file
    if (ownerPassword[0] != '\001') {
        ownerPW = std::make_unique<GooString>(ownerPassword);
    }
    if (userPassword[0] != '\001') {
        userPW = std::make_unique<GooString>(userPassword);
    }

    auto doc = std::unique_ptr<PDFDoc>(PDFDocFactory().createPDFDoc(GooString(fileName), ownerPW.get(), userPW.get()));

    if (!doc->isOk()) {
        return 1;
    }

    // get page range
    if (firstPage < 1) {
        firstPage = 1;
    }
    if (lastPage < 1 || lastPage > doc->getNumPages()) {
        lastPage = doc->getNumPages();
    }
    if (lastPage < firstPage) {
        fprintf(stderr, "Wrong page range given: the first page (%d) can not be after the last page (%d).\n", firstPage, lastPage);
        return 99;
    }

    // get the fonts
    {
        FontInfoScanner scanner(doc.get(), firstPage - 1);
        const std::vector<FontInfo *> fonts = scanner.scan(lastPage - firstPage + 1);

        if (showSubst) {
            // print the font substitutions
            printf("name                                 object ID substitute font                      substitute font file\n");
            printf("------------------------------------ --------- ------------------------------------ ------------------------------------\n");
            for (const FontInfo *font : fonts) {
                if (font->getFile()) {
                    printf("%-36s", font->getName() ? font->getName()->c_str() : "[none]");
                    const Ref fontRef = font->getRef();
                    if (fontRef.gen >= 100000) {
                        printf(" [none]");
                    } else {
                        printf(" %6d %2d", fontRef.num, fontRef.gen);
                    }
                    printf(" %-36s %s\n", font->getSubstituteName() ? font->getSubstituteName()->c_str() : "[none]", font->getFile()->c_str());
                }
                delete font;
            }
        } else {
            // print the font info
            printf("name                                 type              encoding         emb sub uni object ID\n");
            printf("------------------------------------ ----------------- ---------------- --- --- --- ---------\n");
            for (const FontInfo *font : fonts) {
                printf("%-36s %-17s %-16s %-3s %-3s %-3s", font->getName() ? font->getName()->c_str() : "[none]", fontTypeNames[font->getType()], font->getEncoding()->c_str(), font->getEmbedded() ? "yes" : "no",
                       font->getSubset() ? "yes" : "no", font->getToUnicode() ? "yes" : "no");
                const Ref fontRef = font->getRef();
                if (fontRef.gen >= 100000) {
                    printf(" [none]\n");
                } else {
                    printf(" %6d %2d\n", fontRef.num, fontRef.gen);
                }
                delete font;
            }
        }
    }

    return 0;
}
