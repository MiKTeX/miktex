//========================================================================
//
// pdfdetach.cc
//
// Copyright 2010 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2013 Yury G. Kudryashov <urkud.urkud@gmail.com>
// Copyright (C) 2014, 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018, 2020, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2021 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 <r.coeffier@bee-buzziness.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <cstdio>
#include "goo/gmem.h"
#include "parseargs.h"
#include "Annot.h"
#include "GlobalParams.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "FileSpec.h"
#include "CharTypes.h"
#include "Catalog.h"
#include "UnicodeMap.h"
#include "PDFDocEncoding.h"
#include "Error.h"
#include "Win32Console.h"

static bool doList = false;
static int saveNum = 0;
static char saveFile[128] = "";
static bool saveAll = false;
static char savePath[1024] = "";
static char textEncName[128] = "";
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static bool printVersion = false;
static bool printHelp = false;

static const ArgDesc argDesc[] = { { "-list", argFlag, &doList, 0, "list all embedded files" },
                                   { "-save", argInt, &saveNum, 0, "save the specified embedded file (file number)" },
                                   { "-savefile", argString, &saveFile, sizeof(saveFile), "save the specified embedded file (file name)" },
                                   { "-saveall", argFlag, &saveAll, 0, "save all embedded files" },
                                   { "-o", argString, savePath, sizeof(savePath), "file name for the saved embedded file" },
                                   { "-enc", argString, textEncName, sizeof(textEncName), "output text encoding name" },
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
    std::unique_ptr<PDFDoc> doc;
    GooString *fileName;
    const UnicodeMap *uMap;
    std::optional<GooString> ownerPW, userPW;
    char uBuf[8];
    char path[1024];
    char *p;
    bool ok;
    bool hasSaveFile;
    std::vector<std::unique_ptr<FileSpec>> embeddedFiles;
    int nFiles, nPages, n, i, j;
    Page *page;
    Annots *annots;
    const GooString *s1;
    Unicode u;
    bool isUnicode;

    Win32Console win32Console(&argc, &argv);

    // parse args
    ok = parseArgs(argDesc, &argc, argv);
    hasSaveFile = strlen(saveFile) > 0;
    if ((doList ? 1 : 0) + ((saveNum != 0) ? 1 : 0) + ((hasSaveFile != 0) ? 1 : 0) + (saveAll ? 1 : 0) != 1) {
        ok = false;
    }
    if (!ok || argc != 2 || printVersion || printHelp) {
        fprintf(stderr, "pdfdetach version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdfdetach", "<PDF-file>", argDesc);
        }
        return 99;
    }
    fileName = new GooString(argv[1]);

    // read config file
    globalParams = std::make_unique<GlobalParams>();
    if (textEncName[0]) {
        globalParams->setTextEncoding(textEncName);
    }

    // get mapping to output encoding
    if (!(uMap = globalParams->getTextEncoding())) {
        error(errConfig, -1, "Couldn't get text encoding");
        delete fileName;
        return 99;
    }

    // open PDF file
    if (ownerPassword[0] != '\001') {
        ownerPW = GooString(ownerPassword);
    }
    if (userPassword[0] != '\001') {
        userPW = GooString(userPassword);
    }

    doc = PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW);

    if (!doc->isOk()) {
        return 1;
    }

    for (i = 0; i < doc->getCatalog()->numEmbeddedFiles(); ++i) {
        embeddedFiles.push_back(doc->getCatalog()->embeddedFile(i));
    }

    nPages = doc->getCatalog()->getNumPages();
    for (i = 0; i < nPages; ++i) {
        page = doc->getCatalog()->getPage(i + 1);
        if (!page) {
            continue;
        }
        annots = page->getAnnots();
        if (!annots) {
            break;
        }

        for (Annot *annot : annots->getAnnots()) {
            if (annot->getType() != Annot::typeFileAttachment) {
                continue;
            }
            embeddedFiles.push_back(std::make_unique<FileSpec>(static_cast<AnnotFileAttachment *>(annot)->getFile()));
        }
    }

    nFiles = embeddedFiles.size();

    // list embedded files
    if (doList) {
        printf("%d embedded files\n", nFiles);
        for (i = 0; i < nFiles; ++i) {
            const std::unique_ptr<FileSpec> &fileSpec = embeddedFiles[i];
            printf("%d: ", i + 1);
            s1 = fileSpec->getFileName();
            if (!s1) {
                return 3;
            }
            if (s1->hasUnicodeMarker()) {
                isUnicode = true;
                j = 2;
            } else {
                isUnicode = false;
                j = 0;
            }
            while (j < s1->getLength()) {
                if (isUnicode) {
                    u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j + 1) & 0xff);
                    j += 2;
                } else {
                    u = pdfDocEncoding[s1->getChar(j) & 0xff];
                    ++j;
                }
                n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
                fwrite(uBuf, 1, n, stdout);
            }
            fputc('\n', stdout);
        }

        // save all embedded files
    } else if (saveAll) {
        for (i = 0; i < nFiles; ++i) {
            const std::unique_ptr<FileSpec> &fileSpec = embeddedFiles[i];
            if (savePath[0]) {
                n = strlen(savePath);
                if (n > (int)sizeof(path) - 2) {
                    n = sizeof(path) - 2;
                }
                memcpy(path, savePath, n);
                path[n] = '/';
                p = path + n + 1;
            } else {
                p = path;
            }
            s1 = fileSpec->getFileName();
            if (!s1) {
                return 3;
            }
            if (s1->hasUnicodeMarker()) {
                isUnicode = true;
                j = 2;
            } else {
                isUnicode = false;
                j = 0;
            }
            while (j < s1->getLength()) {
                if (isUnicode) {
                    u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j + 1) & 0xff);
                    j += 2;
                } else {
                    u = pdfDocEncoding[s1->getChar(j) & 0xff];
                    ++j;
                }
                n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
                if (p + n >= path + sizeof(path)) {
                    break;
                }
                memcpy(p, uBuf, n);
                p += n;
            }
            *p = '\0';

            auto *embFile = fileSpec->getEmbeddedFile();
            if (!embFile || !embFile->isOk()) {
                return 3;
            }
            if (!embFile->save(path)) {
                error(errIO, -1, "Error saving embedded file as '{0:s}'", p);
                return 2;
            }
        }

        // save an embedded file
    } else {
        if (hasSaveFile) {
            for (i = 0; i < nFiles; ++i) {
                const std::unique_ptr<FileSpec> &fileSpec = embeddedFiles[i];
                s1 = fileSpec->getFileName();
                if (strcmp(s1->c_str(), saveFile) == 0) {
                    saveNum = i + 1;
                    break;
                }
            }
        }
        if (saveNum < 1 || saveNum > nFiles) {
            error(errCommandLine, -1, hasSaveFile ? "Invalid file name" : "Invalid file number");
            return 99;
        }

        const std::unique_ptr<FileSpec> &fileSpec = embeddedFiles[saveNum - 1];
        if (savePath[0]) {
            p = savePath;
        } else {
            p = path;
            s1 = fileSpec->getFileName();
            if (!s1) {
                return 3;
            }
            if (s1->hasUnicodeMarker()) {
                isUnicode = true;
                j = 2;
            } else {
                isUnicode = false;
                j = 0;
            }
            while (j < s1->getLength()) {
                if (isUnicode) {
                    u = ((s1->getChar(j) & 0xff) << 8) | (s1->getChar(j + 1) & 0xff);
                    j += 2;
                } else {
                    u = pdfDocEncoding[s1->getChar(j) & 0xff];
                    ++j;
                }
                n = uMap->mapUnicode(u, uBuf, sizeof(uBuf));
                if (p + n >= path + sizeof(path)) {
                    break;
                }
                memcpy(p, uBuf, n);
                p += n;
            }
            *p = '\0';
            p = path;
        }

        auto *embFile = fileSpec->getEmbeddedFile();
        if (!embFile || !embFile->isOk()) {
            return 3;
        }
        if (!embFile->save(p)) {
            error(errIO, -1, "Error saving embedded file as '{0:s}'", p);
            return 2;
        }
    }

    return 0;
}
