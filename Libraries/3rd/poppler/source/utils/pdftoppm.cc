//========================================================================
//
// pdftoppm.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2007 Ilmari Heikkinen <ilmari.heikkinen@gmail.com>
// Copyright (C) 2008 Richard Airlie <richard.airlie@maglabs.net>
// Copyright (C) 2009 Michael K. Johnson <a1237@danlj.org>
// Copyright (C) 2009 Shen Liang <shenzhuxi@gmail.com>
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009-2011, 2015, 2018-2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010, 2012, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 Jonathan Liu <net147@gmail.com>
// Copyright (C) 2010 William Bader <williambader@hotmail.com>
// Copyright (C) 2011-2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013, 2015, 2018 Adam Reichold <adamreichold@myopera.com>
// Copyright (C) 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2015 William Bader <williambader@hotmail.com>
// Copyright (C) 2018 Martin Packman <gzlist@googlemail.com>
// Copyright (C) 2019 Yves-Gaël Chény <gitlab@r0b0t.fr>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2019 <corentinf@free.fr>
// Copyright (C) 2019 Kris Jurka <jurka@ejurka.com>
// Copyright (C) 2019 Sébastien Berthier <s.berthier@bee-buzziness.com>
// Copyright (C) 2020 Stéfan van der Walt <sjvdwalt@gmail.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#ifdef _WIN32
#    include <fcntl.h> // for O_BINARY
#    include <io.h> // for setmode
#endif
#include <cstdio>
#include <cmath>
#include "parseargs.h"
#include "goo/gmem.h"
#include "goo/GooString.h"
#include "GlobalParams.h"
#include "Object.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "splash/SplashBitmap.h"
#include "splash/Splash.h"
#include "splash/SplashErrorCodes.h"
#include "SplashOutputDev.h"
#include "Win32Console.h"
#include "numberofcharacters.h"

// Uncomment to build pdftoppm with pthreads
// You may also have to change the buildsystem to
// link pdftoppm to pthread library
// This is here for developer testing not user ready
// #define UTILS_USE_PTHREADS 1

#ifdef UTILS_USE_PTHREADS
#    include <cerrno>
#    include <pthread.h>
#    include <deque>
#endif // UTILS_USE_PTHREADS

#ifdef USE_CMS
#    include <lcms2.h>
#endif

static int firstPage = 1;
static int lastPage = 0;
static bool printOnlyOdd = false;
static bool printOnlyEven = false;
static bool singleFile = false;
static bool scaleDimensionBeforeRotation = false;
static double resolution = 0.0;
static double x_resolution = 150.0;
static double y_resolution = 150.0;
static int scaleTo = 0;
static int x_scaleTo = 0;
static int y_scaleTo = 0;
static int param_x = 0;
static int param_y = 0;
static int param_w = 0;
static int param_h = 0;
static int sz = 0;
static bool hideAnnotations = false;
static bool useCropBox = false;
static bool mono = false;
static bool gray = false;
#ifdef USE_CMS
static GooString displayprofilename;
static GfxLCMSProfilePtr displayprofile;
#endif
static char sep[2] = "-";
static bool forceNum = false;
static bool png = false;
static bool jpeg = false;
static bool jpegcmyk = false;
static bool tiff = false;
static GooString jpegOpt;
static int jpegQuality = -1;
static bool jpegProgressive = false;
static bool jpegOptimize = false;
static bool overprint = false;
static char enableFreeTypeStr[16] = "";
static bool enableFreeType = true;
static char antialiasStr[16] = "";
static char vectorAntialiasStr[16] = "";
static bool fontAntialias = true;
static bool vectorAntialias = true;
static char ownerPassword[33] = "";
static char userPassword[33] = "";
static char TiffCompressionStr[16] = "";
static char thinLineModeStr[8] = "";
static SplashThinLineMode thinLineMode = splashThinLineDefault;
#ifdef UTILS_USE_PTHREADS
static int numberOfJobs = 1;
#endif // UTILS_USE_PTHREADS
static bool quiet = false;
static bool printVersion = false;
static bool printHelp = false;

static const ArgDesc argDesc[] = { { "-f", argInt, &firstPage, 0, "first page to print" },
                                   { "-l", argInt, &lastPage, 0, "last page to print" },
                                   { "-o", argFlag, &printOnlyOdd, 0, "print only odd pages" },
                                   { "-e", argFlag, &printOnlyEven, 0, "print only even pages" },
                                   { "-singlefile", argFlag, &singleFile, 0, "write only the first page and do not add digits" },
                                   { "-scale-dimension-before-rotation", argFlag, &scaleDimensionBeforeRotation, 0, "for rotated pdf, resize dimensions before the rotation" },

                                   { "-r", argFP, &resolution, 0, "resolution, in DPI (default is 150)" },
                                   { "-rx", argFP, &x_resolution, 0, "X resolution, in DPI (default is 150)" },
                                   { "-ry", argFP, &y_resolution, 0, "Y resolution, in DPI (default is 150)" },
                                   { "-scale-to", argInt, &scaleTo, 0, "scales each page to fit within scale-to*scale-to pixel box" },
                                   { "-scale-to-x", argInt, &x_scaleTo, 0, "scales each page horizontally to fit in scale-to-x pixels" },
                                   { "-scale-to-y", argInt, &y_scaleTo, 0, "scales each page vertically to fit in scale-to-y pixels" },

                                   { "-x", argInt, &param_x, 0, "x-coordinate of the crop area top left corner" },
                                   { "-y", argInt, &param_y, 0, "y-coordinate of the crop area top left corner" },
                                   { "-W", argInt, &param_w, 0, "width of crop area in pixels (default is 0)" },
                                   { "-H", argInt, &param_h, 0, "height of crop area in pixels (default is 0)" },
                                   { "-sz", argInt, &sz, 0, "size of crop square in pixels (sets W and H)" },
                                   { "-cropbox", argFlag, &useCropBox, 0, "use the crop box rather than media box" },
                                   { "-hide-annotations", argFlag, &hideAnnotations, 0, "do not show annotations" },

                                   { "-mono", argFlag, &mono, 0, "generate a monochrome PBM file" },
                                   { "-gray", argFlag, &gray, 0, "generate a grayscale PGM file" },
#ifdef USE_CMS
                                   { "-displayprofile", argGooString, &displayprofilename, 0, "ICC color profile to use as the display profile" },
#endif
                                   { "-sep", argString, sep, sizeof(sep), "single character separator between name and page number, default - " },
                                   { "-forcenum", argFlag, &forceNum, 0, "force page number even if there is only one page " },
#ifdef ENABLE_LIBPNG
                                   { "-png", argFlag, &png, 0, "generate a PNG file" },
#endif
#ifdef ENABLE_LIBJPEG
                                   { "-jpeg", argFlag, &jpeg, 0, "generate a JPEG file" },
                                   { "-jpegcmyk", argFlag, &jpegcmyk, 0, "generate a CMYK JPEG file" },
                                   { "-jpegopt", argGooString, &jpegOpt, 0, "jpeg options, with format <opt1>=<val1>[,<optN>=<valN>]*" },
#endif
                                   { "-overprint", argFlag, &overprint, 0, "enable overprint" },
#ifdef ENABLE_LIBTIFF
                                   { "-tiff", argFlag, &tiff, 0, "generate a TIFF file" },
                                   { "-tiffcompression", argString, TiffCompressionStr, sizeof(TiffCompressionStr), "set TIFF compression: none, packbits, jpeg, lzw, deflate" },
#endif
                                   { "-freetype", argString, enableFreeTypeStr, sizeof(enableFreeTypeStr), "enable FreeType font rasterizer: yes, no" },
                                   { "-thinlinemode", argString, thinLineModeStr, sizeof(thinLineModeStr), "set thin line mode: none, solid, shape. Default: none" },

                                   { "-aa", argString, antialiasStr, sizeof(antialiasStr), "enable font anti-aliasing: yes, no" },
                                   { "-aaVector", argString, vectorAntialiasStr, sizeof(vectorAntialiasStr), "enable vector anti-aliasing: yes, no" },

                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },

#ifdef UTILS_USE_PTHREADS
                                   { "-j", argInt, &numberOfJobs, 0, "number of jobs to run concurrently" },
#endif // UTILS_USE_PTHREADS

                                   { "-q", argFlag, &quiet, 0, "don't print any messages or errors" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

static bool needToRotate(int angle)
{
    return (angle == 90) || (angle == 270);
}

static bool parseJpegOptions()
{
    // jpegOpt format is: <opt1>=<val1>,<opt2>=<val2>,...
    const char *nextOpt = jpegOpt.c_str();
    while (nextOpt && *nextOpt) {
        const char *comma = strchr(nextOpt, ',');
        GooString opt;
        if (comma) {
            opt.Set(nextOpt, comma - nextOpt);
            nextOpt = comma + 1;
        } else {
            opt.Set(nextOpt);
            nextOpt = nullptr;
        }
        // here opt is "<optN>=<valN> "
        const char *equal = strchr(opt.c_str(), '=');
        if (!equal) {
            fprintf(stderr, "Unknown jpeg option \"%s\"\n", opt.c_str());
            return false;
        }
        int iequal = equal - opt.c_str();
        GooString value(&opt, iequal + 1, opt.getLength() - iequal - 1);
        opt.del(iequal, opt.getLength() - iequal);
        // here opt is "<optN>" and value is "<valN>"

        if (opt.cmp("quality") == 0) {
            if (!isInt(value.c_str())) {
                fprintf(stderr, "Invalid jpeg quality\n");
                return false;
            }
            jpegQuality = atoi(value.c_str());
            if (jpegQuality < 0 || jpegQuality > 100) {
                fprintf(stderr, "jpeg quality must be between 0 and 100\n");
                return false;
            }
        } else if (opt.cmp("progressive") == 0) {
            jpegProgressive = false;
            if (value.cmp("y") == 0) {
                jpegProgressive = true;
            } else if (value.cmp("n") != 0) {
                fprintf(stderr, "jpeg progressive option must be \"y\" or \"n\"\n");
                return false;
            }
        } else if (opt.cmp("optimize") == 0 || opt.cmp("optimise") == 0) {
            jpegOptimize = false;
            if (value.cmp("y") == 0) {
                jpegOptimize = true;
            } else if (value.cmp("n") != 0) {
                fprintf(stderr, "jpeg optimize option must be \"y\" or \"n\"\n");
                return false;
            }
        } else {
            fprintf(stderr, "Unknown jpeg option \"%s\"\n", opt.c_str());
            return false;
        }
    }
    return true;
}

static auto annotDisplayDecideCbk = [](Annot *annot, void *user_data) { return !hideAnnotations; };

static void savePageSlice(PDFDoc *doc, SplashOutputDev *splashOut, int pg, int x, int y, int w, int h, double pg_w, double pg_h, char *ppmFile)
{
    if (w == 0)
        w = (int)ceil(pg_w);
    if (h == 0)
        h = (int)ceil(pg_h);
    w = (x + w > pg_w ? (int)ceil(pg_w - x) : w);
    h = (y + h > pg_h ? (int)ceil(pg_h - y) : h);
    doc->displayPageSlice(splashOut, pg, x_resolution, y_resolution, 0, !useCropBox, false, false, x, y, w, h, nullptr, nullptr, annotDisplayDecideCbk, nullptr);

    SplashBitmap *bitmap = splashOut->getBitmap();

    SplashBitmap::WriteImgParams params;
    params.jpegQuality = jpegQuality;
    params.jpegProgressive = jpegProgressive;
    params.jpegOptimize = jpegOptimize;
    params.tiffCompression.Set(TiffCompressionStr);

    if (ppmFile != nullptr) {
        SplashError e;

        if (png) {
            e = bitmap->writeImgFile(splashFormatPng, ppmFile, x_resolution, y_resolution);
        } else if (jpeg) {
            e = bitmap->writeImgFile(splashFormatJpeg, ppmFile, x_resolution, y_resolution, &params);
        } else if (jpegcmyk) {
            e = bitmap->writeImgFile(splashFormatJpegCMYK, ppmFile, x_resolution, y_resolution, &params);
        } else if (tiff) {
            e = bitmap->writeImgFile(splashFormatTiff, ppmFile, x_resolution, y_resolution, &params);
        } else {
            e = bitmap->writePNMFile(ppmFile);
        }
        if (e != splashOk) {
            fprintf(stderr, "Could not write image to %s; exiting\n", ppmFile);
            exit(EXIT_FAILURE);
        }
    } else {
#ifdef _WIN32
        setmode(fileno(stdout), O_BINARY);
#endif

        if (png) {
            bitmap->writeImgFile(splashFormatPng, stdout, x_resolution, y_resolution);
        } else if (jpeg) {
            bitmap->writeImgFile(splashFormatJpeg, stdout, x_resolution, y_resolution, &params);
        } else if (tiff) {
            bitmap->writeImgFile(splashFormatTiff, stdout, x_resolution, y_resolution, &params);
        } else {
            bitmap->writePNMFile(stdout);
        }
    }
}

#ifdef UTILS_USE_PTHREADS

struct PageJob
{
    PDFDoc *doc;
    int pg;

    double pg_w, pg_h;
    SplashColor *paperColor;

    char *ppmFile;
};

static std::deque<PageJob> pageJobQueue;
static pthread_mutex_t pageJobMutex = PTHREAD_MUTEX_INITIALIZER;

static void processPageJobs()
{
    while (true) {
        // pop the next job or exit if queue is empty
        pthread_mutex_lock(&pageJobMutex);

        if (pageJobQueue.empty()) {
            pthread_mutex_unlock(&pageJobMutex);
            return;
        }

        PageJob pageJob = pageJobQueue.front();
        pageJobQueue.pop_front();

        pthread_mutex_unlock(&pageJobMutex);

        // process the job
        SplashOutputDev *splashOut = new SplashOutputDev(mono ? splashModeMono1 : gray ? splashModeMono8 : (jpegcmyk || overprint) ? splashModeDeviceN8 : splashModeRGB8, 4, false, *pageJob.paperColor, true, thinLineMode);
        splashOut->setFontAntialias(fontAntialias);
        splashOut->setVectorAntialias(vectorAntialias);
        splashOut->setEnableFreeType(enableFreeType);
#    ifdef USE_CMS
        splashOut->setDisplayProfile(displayprofile);
#    endif
        splashOut->startDoc(pageJob.doc);

        savePageSlice(pageJob.doc, splashOut, pageJob.pg, x, y, w, h, pageJob.pg_w, pageJob.pg_h, pageJob.ppmFile);

        delete splashOut;
        delete[] pageJob.ppmFile;
    }
}

#endif // UTILS_USE_PTHREADS

#if defined(MIKTEX)
int Main(int argc, char** argv)
#else
int main(int argc, char *argv[])
#endif
{
    PDFDoc *doc;
    GooString *fileName = nullptr;
    char *ppmRoot = nullptr;
    char *ppmFile;
    GooString *ownerPW, *userPW;
    SplashColor paperColor;
#ifndef UTILS_USE_PTHREADS
    SplashOutputDev *splashOut;
#else
    pthread_t *jobs;
#endif // UTILS_USE_PTHREADS
    bool ok;
    int exitCode;
    int pg, pg_num_len;
    double pg_w, pg_h;
#ifdef USE_CMS
    cmsColorSpaceSignature displayprofilecolorspace;
#endif

    Win32Console win32Console(&argc, &argv);
    exitCode = 99;

    // parse args
    ok = parseArgs(argDesc, &argc, argv);
    if (mono && gray) {
        ok = false;
    }
    if (resolution != 0.0 && (x_resolution == 150.0 || y_resolution == 150.0)) {
        x_resolution = resolution;
        y_resolution = resolution;
    }
    if (!ok || argc > 3 || printVersion || printHelp) {
        fprintf(stderr, "pdftoppm version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdftoppm", "[PDF-file [PPM-file-prefix]]", argDesc);
        }
        if (printVersion || printHelp)
            exitCode = 0;
        goto err0;
    }
    if (argc > 1)
        fileName = new GooString(argv[1]);
    if (argc == 3)
        ppmRoot = argv[2];

    if (antialiasStr[0]) {
        if (!GlobalParams::parseYesNo2(antialiasStr, &fontAntialias)) {
            fprintf(stderr, "Bad '-aa' value on command line\n");
        }
    }
    if (vectorAntialiasStr[0]) {
        if (!GlobalParams::parseYesNo2(vectorAntialiasStr, &vectorAntialias)) {
            fprintf(stderr, "Bad '-aaVector' value on command line\n");
        }
    }

    if (jpegOpt.getLength() > 0) {
        if (!jpeg)
            fprintf(stderr, "Warning: -jpegopt only valid with jpeg output.\n");
        parseJpegOptions();
    }

    // read config file
    globalParams = std::make_unique<GlobalParams>();
    if (enableFreeTypeStr[0]) {
        if (!GlobalParams::parseYesNo2(enableFreeTypeStr, &enableFreeType)) {
            fprintf(stderr, "Bad '-freetype' value on command line\n");
        }
    }
    if (thinLineModeStr[0]) {
        if (strcmp(thinLineModeStr, "solid") == 0) {
            thinLineMode = splashThinLineSolid;
        } else if (strcmp(thinLineModeStr, "shape") == 0) {
            thinLineMode = splashThinLineShape;
        } else if (strcmp(thinLineModeStr, "none") != 0) {
            fprintf(stderr, "Bad '-thinlinemode' value on command line\n");
        }
    }
    if (quiet) {
        globalParams->setErrQuiet(quiet);
    }

    // open PDF file
    if (ownerPassword[0]) {
        ownerPW = new GooString(ownerPassword);
    } else {
        ownerPW = nullptr;
    }
    if (userPassword[0]) {
        userPW = new GooString(userPassword);
    } else {
        userPW = nullptr;
    }

    if (fileName == nullptr) {
        fileName = new GooString("fd://0");
    }
    if (fileName->cmp("-") == 0) {
        delete fileName;
        fileName = new GooString("fd://0");
    }
    doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);
    delete fileName;

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

    // get page range
    if (firstPage < 1)
        firstPage = 1;
    if (singleFile && lastPage < 1)
        lastPage = firstPage;
    if (lastPage < 1 || lastPage > doc->getNumPages())
        lastPage = doc->getNumPages();
    if (lastPage < firstPage) {
        fprintf(stderr, "Wrong page range given: the first page (%d) can not be after the last page (%d).\n", firstPage, lastPage);
        goto err1;
    }

    // If our page range selection and document size indicate we're only
    // outputting a single page, ensure that even/odd page selection doesn't
    // filter out that single page.
    if (firstPage == lastPage && ((printOnlyEven && firstPage % 2 == 1) || (printOnlyOdd && firstPage % 2 == 0))) {
        fprintf(stderr, "Invalid even/odd page selection, no pages match criteria.\n");
        goto err1;
    }

    if (singleFile && firstPage < lastPage) {
        if (!quiet) {
            fprintf(stderr, "Warning: Single file will write only the first of the %d pages.\n", lastPage + 1 - firstPage);
        }
        lastPage = firstPage;
    }

    // write PPM files
    if (jpegcmyk || overprint) {
        globalParams->setOverprintPreview(true);
        splashClearColor(paperColor);
    } else {
        paperColor[0] = 255;
        paperColor[1] = 255;
        paperColor[2] = 255;
    }

#ifdef USE_CMS
    if (!displayprofilename.toStr().empty()) {
        displayprofile = make_GfxLCMSProfilePtr(cmsOpenProfileFromFile(displayprofilename.c_str(), "r"));
        if (!displayprofile) {
            fprintf(stderr, "Could not open the ICC profile \"%s\".\n", displayprofilename.c_str());
            goto err1;
        }
        if (!cmsIsIntentSupported(displayprofile.get(), INTENT_RELATIVE_COLORIMETRIC, LCMS_USED_AS_OUTPUT) && !cmsIsIntentSupported(displayprofile.get(), INTENT_ABSOLUTE_COLORIMETRIC, LCMS_USED_AS_OUTPUT)
            && !cmsIsIntentSupported(displayprofile.get(), INTENT_SATURATION, LCMS_USED_AS_OUTPUT) && !cmsIsIntentSupported(displayprofile.get(), INTENT_PERCEPTUAL, LCMS_USED_AS_OUTPUT)) {
            fprintf(stderr, "ICC profile \"%s\" is not an output profile.\n", displayprofilename.c_str());
            goto err1;
        }
        displayprofilecolorspace = cmsGetColorSpace(displayprofile.get());
        if (jpegcmyk || overprint) {
            if (displayprofilecolorspace != cmsSigCmykData) {
                fprintf(stderr, "Warning: Supplied ICC profile \"%s\" is not a CMYK profile.\n", displayprofilename.c_str());
            }
        } else if (mono || gray) {
            if (displayprofilecolorspace != cmsSigGrayData) {
                fprintf(stderr, "Warning: Supplied ICC profile \"%s\" is not a monochrome profile.\n", displayprofilename.c_str());
            }
        } else {
            if (displayprofilecolorspace != cmsSigRgbData) {
                fprintf(stderr, "Warning: Supplied ICC profile \"%s\" is not a RGB profile.\n", displayprofilename.c_str());
            }
        }
    }
#endif

#ifndef UTILS_USE_PTHREADS

    splashOut = new SplashOutputDev(mono ? splashModeMono1 : gray ? splashModeMono8 : (jpegcmyk || overprint) ? splashModeDeviceN8 : splashModeRGB8, 4, false, paperColor, true, thinLineMode);

    splashOut->setFontAntialias(fontAntialias);
    splashOut->setVectorAntialias(vectorAntialias);
    splashOut->setEnableFreeType(enableFreeType);
#    ifdef USE_CMS
    splashOut->setDisplayProfile(displayprofile);
#    endif
    splashOut->startDoc(doc);

#endif // UTILS_USE_PTHREADS

    if (sz != 0)
        param_w = param_h = sz;
    pg_num_len = numberOfCharacters(doc->getNumPages());
    for (pg = firstPage; pg <= lastPage; ++pg) {
        if (printOnlyEven && pg % 2 == 1)
            continue;
        if (printOnlyOdd && pg % 2 == 0)
            continue;
        if (useCropBox) {
            pg_w = doc->getPageCropWidth(pg);
            pg_h = doc->getPageCropHeight(pg);
        } else {
            pg_w = doc->getPageMediaWidth(pg);
            pg_h = doc->getPageMediaHeight(pg);
        }

        if (scaleDimensionBeforeRotation && needToRotate(doc->getPageRotate(pg)))
            std::swap(pg_w, pg_h);

        if (scaleTo != 0) {
            resolution = (72.0 * scaleTo) / (pg_w > pg_h ? pg_w : pg_h);
            x_resolution = y_resolution = resolution;
        } else {
            if (x_scaleTo > 0) {
                x_resolution = (72.0 * x_scaleTo) / pg_w;
                if (y_scaleTo == -1)
                    y_resolution = x_resolution;
            }
            if (y_scaleTo > 0) {
                y_resolution = (72.0 * y_scaleTo) / pg_h;
                if (x_scaleTo == -1)
                    x_resolution = y_resolution;
            }
        }
        pg_w = pg_w * (x_resolution / 72.0);
        pg_h = pg_h * (y_resolution / 72.0);

        if (!scaleDimensionBeforeRotation && needToRotate(doc->getPageRotate(pg)))
            std::swap(pg_w, pg_h);

        if (ppmRoot != nullptr) {
            const char *ext = png ? "png" : (jpeg || jpegcmyk) ? "jpg" : tiff ? "tif" : mono ? "pbm" : gray ? "pgm" : "ppm";
            if (singleFile && !forceNum) {
                ppmFile = new char[strlen(ppmRoot) + 1 + strlen(ext) + 1];
                sprintf(ppmFile, "%s.%s", ppmRoot, ext);
            } else {
                ppmFile = new char[strlen(ppmRoot) + 1 + pg_num_len + 1 + strlen(ext) + 1];
                sprintf(ppmFile, "%s%s%0*d.%s", ppmRoot, sep, pg_num_len, pg, ext);
            }
        } else {
            ppmFile = nullptr;
        }
#ifndef UTILS_USE_PTHREADS
        // process job in main thread
        savePageSlice(doc, splashOut, pg, param_x, param_y, param_w, param_h, pg_w, pg_h, ppmFile);

        delete[] ppmFile;
#else

        // queue job for worker threads
        PageJob pageJob = { .doc = doc,
                            .pg = pg,

                            .pg_w = pg_w,
                            .pg_h = pg_h,

                            .paperColor = &paperColor,

                            .ppmFile = ppmFile };

        pageJobQueue.push_back(pageJob);

#endif // UTILS_USE_PTHREADS
    }
#ifndef UTILS_USE_PTHREADS
    delete splashOut;
#else

    // spawn worker threads and wait on them
    jobs = (pthread_t *)malloc(numberOfJobs * sizeof(pthread_t));

    for (int i = 0; i < numberOfJobs; ++i) {
        if (pthread_create(&jobs[i], NULL, (void *(*)(void *))processPageJobs, NULL) != 0) {
            fprintf(stderr, "pthread_create() failed with errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < numberOfJobs; ++i) {
        if (pthread_join(jobs[i], NULL) != 0) {
            fprintf(stderr, "pthread_join() failed with errno: %d\n", errno);
            exit(EXIT_FAILURE);
        }
    }

    free(jobs);

#endif // UTILS_USE_PTHREADS

    exitCode = 0;

    // clean up
err1:
    delete doc;
err0:

    return exitCode;
}
