//========================================================================
//
// pdftops.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
// Modified for Debian by Hamish Moffatt, 22 May 2002.
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
// Copyright (C) 2007-2008, 2010, 2015, 2017, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009 Till Kamppeter <till.kamppeter@gmail.com>
// Copyright (C) 2009 Sanjoy Mahajan <sanjoy@mit.edu>
// Copyright (C) 2009, 2011, 2012, 2014-2016, 2020 William Bader <williambader@hotmail.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2012 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2014, 2017 Adrian Johnson <ajohnson@redneon.com>
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
#include <cstring>
#include "parseargs.h"
#include "goo/GooString.h"
#include "goo/gmem.h"
#include "GlobalParams.h"
#include "Object.h"
#include "Stream.h"
#include "Array.h"
#include "Dict.h"
#include "XRef.h"
#include "Catalog.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "PSOutputDev.h"
#include "Error.h"
#include "Win32Console.h"

static bool setPSPaperSize(char *size, int &psPaperWidth, int &psPaperHeight)
{
    if (!strcmp(size, "match")) {
        psPaperWidth = psPaperHeight = -1;
    } else if (!strcmp(size, "letter")) {
        psPaperWidth = 612;
        psPaperHeight = 792;
    } else if (!strcmp(size, "legal")) {
        psPaperWidth = 612;
        psPaperHeight = 1008;
    } else if (!strcmp(size, "A4")) {
        psPaperWidth = 595;
        psPaperHeight = 842;
    } else if (!strcmp(size, "A3")) {
        psPaperWidth = 842;
        psPaperHeight = 1190;
    } else {
        return false;
    }
    return true;
}

static int firstPage = 1;
static int lastPage = 0;
static bool level1 = false;
static bool level1Sep = false;
static bool level2 = false;
static bool level2Sep = false;
static bool level3 = false;
static bool level3Sep = false;
static bool origPageSizes = false;
static bool doEPS = false;
static bool doForm = false;
#ifdef OPI_SUPPORT
static bool doOPI = false;
#endif
static int splashResolution = 0;
static bool psBinary = false;
static bool noEmbedT1Fonts = false;
static bool noEmbedTTFonts = false;
static bool noEmbedCIDPSFonts = false;
static bool noEmbedCIDTTFonts = false;
static bool fontPassthrough = false;
static bool optimizeColorSpace = false;
static bool passLevel1CustomColor = false;
static char rasterAntialiasStr[16] = "";
static char forceRasterizeStr[16] = "";
static bool preload = false;
static char paperSize[15] = "";
static int paperWidth = -1;
static int paperHeight = -1;
static bool noCrop = false;
static bool expand = false;
static bool noShrink = false;
static bool noCenter = false;
static bool duplex = false;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static bool quiet = false;
static bool printVersion = false;
static bool printHelp = false;
static bool overprint = false;

static const ArgDesc argDesc[] = { { "-f", argInt, &firstPage, 0, "first page to print" },
                                   { "-l", argInt, &lastPage, 0, "last page to print" },
                                   { "-level1", argFlag, &level1, 0, "generate Level 1 PostScript" },
                                   { "-level1sep", argFlag, &level1Sep, 0, "generate Level 1 separable PostScript" },
                                   { "-level2", argFlag, &level2, 0, "generate Level 2 PostScript" },
                                   { "-level2sep", argFlag, &level2Sep, 0, "generate Level 2 separable PostScript" },
                                   { "-level3", argFlag, &level3, 0, "generate Level 3 PostScript" },
                                   { "-level3sep", argFlag, &level3Sep, 0, "generate Level 3 separable PostScript" },
                                   { "-origpagesizes", argFlag, &origPageSizes, 0, "conserve original page sizes" },
                                   { "-eps", argFlag, &doEPS, 0, "generate Encapsulated PostScript (EPS)" },
                                   { "-form", argFlag, &doForm, 0, "generate a PostScript form" },
#ifdef OPI_SUPPORT
                                   { "-opi", argFlag, &doOPI, 0, "generate OPI comments" },
#endif
                                   { "-r", argInt, &splashResolution, 0, "resolution for rasterization, in DPI (default is 300)" },
                                   { "-binary", argFlag, &psBinary, 0, "write binary data in Level 1 PostScript" },
                                   { "-noembt1", argFlag, &noEmbedT1Fonts, 0, "don't embed Type 1 fonts" },
                                   { "-noembtt", argFlag, &noEmbedTTFonts, 0, "don't embed TrueType fonts" },
                                   { "-noembcidps", argFlag, &noEmbedCIDPSFonts, 0, "don't embed CID PostScript fonts" },
                                   { "-noembcidtt", argFlag, &noEmbedCIDTTFonts, 0, "don't embed CID TrueType fonts" },
                                   { "-passfonts", argFlag, &fontPassthrough, 0, "don't substitute missing fonts" },
                                   { "-aaRaster", argString, rasterAntialiasStr, sizeof(rasterAntialiasStr), "enable anti-aliasing on rasterization: yes, no" },
                                   { "-rasterize", argString, forceRasterizeStr, sizeof(forceRasterizeStr), "control rasterization: always, never, whenneeded" },
                                   { "-optimizecolorspace", argFlag, &optimizeColorSpace, 0, "convert gray RGB images to gray color space" },
                                   { "-passlevel1customcolor", argFlag, &passLevel1CustomColor, 0, "pass custom color in level1sep" },
                                   { "-preload", argFlag, &preload, 0, "preload images and forms" },
                                   { "-paper", argString, paperSize, sizeof(paperSize), "paper size (letter, legal, A4, A3, match)" },
                                   { "-paperw", argInt, &paperWidth, 0, "paper width, in points" },
                                   { "-paperh", argInt, &paperHeight, 0, "paper height, in points" },
                                   { "-nocrop", argFlag, &noCrop, 0, "don't crop pages to CropBox" },
                                   { "-expand", argFlag, &expand, 0, "expand pages smaller than the paper size" },
                                   { "-noshrink", argFlag, &noShrink, 0, "don't shrink pages larger than the paper size" },
                                   { "-nocenter", argFlag, &noCenter, 0, "don't center pages smaller than the paper size" },
                                   { "-duplex", argFlag, &duplex, 0, "enable duplex printing" },
                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },
                                   { "-overprint", argFlag, &overprint, 0, "enable overprint" },
                                   { "-q", argFlag, &quiet, 0, "don't print any messages or errors" },
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
    PDFDoc *doc;
    GooString *fileName;
    GooString *psFileName;
    PSLevel level;
    PSOutMode mode;
    GooString *ownerPW, *userPW;
    PSOutputDev *psOut;
    bool ok;
    int exitCode;
    bool rasterAntialias = false;
    std::vector<int> pages;

    Win32Console win32Console(&argc, &argv);
    exitCode = 99;

    // parse args
    ok = parseArgs(argDesc, &argc, argv);
    if (!ok || argc < 2 || argc > 3 || printVersion || printHelp) {
        fprintf(stderr, "pdftops version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdftops", "<PDF-file> [<PS-file>]", argDesc);
        }
        if (printVersion || printHelp)
            exit(0);
        else
            exit(1);
    }
    if ((level1 ? 1 : 0) + (level1Sep ? 1 : 0) + (level2 ? 1 : 0) + (level2Sep ? 1 : 0) + (level3 ? 1 : 0) + (level3Sep ? 1 : 0) > 1) {
        fprintf(stderr, "Error: use only one of the 'level' options.\n");
        exit(1);
    }
    if ((doEPS ? 1 : 0) + (doForm ? 1 : 0) > 1) {
        fprintf(stderr, "Error: use only one of -eps, and -form\n");
        exit(1);
    }
    if (level1) {
        level = psLevel1;
    } else if (level1Sep) {
        level = psLevel1Sep;
    } else if (level2Sep) {
        level = psLevel2Sep;
    } else if (level3) {
        level = psLevel3;
    } else if (level3Sep) {
        level = psLevel3Sep;
    } else {
        level = psLevel2;
    }
    if (doForm && level < psLevel2) {
        fprintf(stderr, "Error: forms are only available with Level 2 output.\n");
        exit(1);
    }
    mode = doEPS ? psModeEPS : doForm ? psModeForm : psModePS;
    fileName = new GooString(argv[1]);

    // read config file
    globalParams = std::make_unique<GlobalParams>();
    if (origPageSizes) {
        paperWidth = paperHeight = -1;
    }
    if (paperSize[0]) {
        if (origPageSizes) {
            fprintf(stderr, "Error: -origpagesizes and -paper may not be used together.\n");
            exit(1);
        }
        if (!setPSPaperSize(paperSize, paperWidth, paperHeight)) {
            fprintf(stderr, "Invalid paper size\n");
            delete fileName;
            goto err0;
        }
    }
    if (overprint) {
        globalParams->setOverprintPreview(true);
    }
    if (expand) {
        globalParams->setPSExpandSmaller(true);
    }
    if (noShrink) {
        globalParams->setPSShrinkLarger(false);
    }
    if (level1 || level1Sep || level2 || level2Sep || level3 || level3Sep) {
        globalParams->setPSLevel(level);
    }
    if (quiet) {
        globalParams->setErrQuiet(quiet);
    }

    // open PDF file
    if (ownerPassword[0] != '\001') {
        ownerPW = new GooString(ownerPassword);
    } else {
        ownerPW = nullptr;
    }
    if (userPassword[0] != '\001') {
        userPW = new GooString(userPassword);
    } else {
        userPW = nullptr;
    }
    if (fileName->cmp("-") == 0) {
        delete fileName;
        fileName = new GooString("fd://0");
    }

    doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);

    if (userPW) {
        delete userPW;
    }
    if (ownerPW) {
        delete ownerPW;
    }
    if (!doc->isOk()) {
        exitCode = 1;
        goto err1;
    }

#ifdef ENFORCE_PERMISSIONS
    // check for print permission
    if (!doc->okToPrint()) {
        error(errNotAllowed, -1, "Printing this document is not allowed.");
        exitCode = 3;
        goto err1;
    }
#endif

    // construct PostScript file name
    if (argc == 3) {
        psFileName = new GooString(argv[2]);
    } else if (fileName->cmp("fd://0") == 0) {
        error(errCommandLine, -1, "You have to provide an output filename when reading from stdin.");
        goto err1;
    } else {
        const char *p = fileName->c_str() + fileName->getLength() - 4;
        if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
            psFileName = new GooString(fileName->c_str(), fileName->getLength() - 4);
        } else {
            psFileName = fileName->copy();
        }
        psFileName->append(doEPS ? ".eps" : ".ps");
    }

    // get page range
    if (firstPage < 1) {
        firstPage = 1;
    }
    if (lastPage < 1 || lastPage > doc->getNumPages()) {
        lastPage = doc->getNumPages();
    }
    if (lastPage < firstPage) {
        error(errCommandLine, -1, "Wrong page range given: the first page ({0:d}) can not be after the last page ({1:d}).", firstPage, lastPage);
        goto err2;
    }

    // check for multi-page EPS or form
    if ((doEPS || doForm) && firstPage != lastPage) {
        error(errCommandLine, -1, "EPS and form files can only contain one page.");
        goto err2;
    }

    for (int i = firstPage; i <= lastPage; ++i) {
        pages.push_back(i);
    }

    // write PostScript file
    psOut = new PSOutputDev(psFileName->c_str(), doc, nullptr, pages, mode, paperWidth, paperHeight, noCrop, duplex);
    if (noCenter) {
        psOut->setPSCenter(false);
    }

    if (rasterAntialiasStr[0]) {
        if (!GlobalParams::parseYesNo2(rasterAntialiasStr, &rasterAntialias)) {
            fprintf(stderr, "Bad '-aaRaster' value on command line\n");
        }
    }

    if (forceRasterizeStr[0]) {
        PSForceRasterize forceRasterize = psRasterizeWhenNeeded;
        if (strcmp(forceRasterizeStr, "whenneeded") == 0) {
            forceRasterize = psRasterizeWhenNeeded;
        } else if (strcmp(forceRasterizeStr, "always") == 0) {
            forceRasterize = psAlwaysRasterize;
        } else if (strcmp(forceRasterizeStr, "never") == 0) {
            forceRasterize = psNeverRasterize;
        } else {
            fprintf(stderr, "Bad '-rasterize' value on command line\n");
        }
        psOut->setForceRasterize(forceRasterize);
    }

    if (splashResolution > 0) {
        psOut->setRasterResolution(splashResolution);
    }
    psOut->setEmbedType1(!noEmbedT1Fonts);
    psOut->setEmbedTrueType(!noEmbedTTFonts);
    psOut->setEmbedCIDPostScript(!noEmbedCIDPSFonts);
    psOut->setEmbedCIDTrueType(!noEmbedCIDTTFonts);
    psOut->setFontPassthrough(fontPassthrough);
    psOut->setPreloadImagesForms(preload);
    psOut->setOptimizeColorSpace(optimizeColorSpace);
    psOut->setPassLevel1CustomColor(passLevel1CustomColor);
#ifdef OPI_SUPPORT
    psOut->setGenerateOPI(doOPI);
#endif
    psOut->setUseBinary(psBinary);

    psOut->setRasterAntialias(rasterAntialias);
    if (psOut->isOk()) {
        for (int i = firstPage; i <= lastPage; ++i) {
            doc->displayPage(psOut, i, 72, 72, 0, noCrop, !noCrop, true);
        }
    } else {
        delete psOut;
        exitCode = 2;
        goto err2;
    }
    delete psOut;

    exitCode = 0;

    // clean up
err2:
    delete psFileName;
err1:
    delete doc;
    delete fileName;
err0:

    return exitCode;
}
