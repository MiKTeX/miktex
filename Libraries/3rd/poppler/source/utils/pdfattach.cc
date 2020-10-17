//========================================================================
//
// pdfattach.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2019, 2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include "gbasename.h"
#include "parseargs.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Error.h"
#include "ErrorCodes.h"
#include "Win32Console.h"

static bool doReplace = false;
static bool printVersion = false;
static bool printHelp = false;

static const ArgDesc argDesc[] = { { "-replace", argFlag, &doReplace, 0, "replace embedded file with same name (if it exists)" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

static bool fileExists(const char *filePath)
{
    FILE *f = openFile(filePath, "r");
    if (f != nullptr) {
        fclose(f);
        return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    Win32Console win32Console(&argc, &argv);

    // parse args
    const bool ok = parseArgs(argDesc, &argc, argv);
    if (!ok || argc != 4 || printVersion || printHelp) {
        fprintf(stderr, "pdfattach version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdfattach", "<input-PDF-file> <file-to-attach> <output-PDF-file>", argDesc);
        }
        return 99;
    }
    const GooString pdfFileName(argv[1]);
    const GooString attachFilePath(argv[2]);

    // init GlobalParams
    globalParams = std::make_unique<GlobalParams>();

    // open PDF file
    std::unique_ptr<PDFDoc> doc(PDFDocFactory().createPDFDoc(pdfFileName, nullptr, nullptr));

    if (!doc->isOk()) {
        fprintf(stderr, "Couldn't open %s\n", pdfFileName.c_str());
        return 1;
    }

    std::unique_ptr<GooFile> attachFile(GooFile::open(&attachFilePath));
    if (!attachFile) {
        fprintf(stderr, "Couldn't open %s\n", attachFilePath.c_str());
        return 2;
    }

    if (fileExists(argv[3])) {
        fprintf(stderr, "File %s already exists.\n", argv[3]);
        return 3;
    }

    const std::string attachFileName = gbasename(attachFilePath.c_str());

    if (!doReplace && doc->getCatalog()->hasEmbeddedFile(attachFileName)) {
        fprintf(stderr, "There is already an embedded file named %s.\n", attachFileName.c_str());
        return 4;
    }

    doc->getCatalog()->addEmbeddedFile(attachFile.get(), attachFileName);

    const GooString outputPdfFilePath(argv[3]);
    const int saveResult = doc->saveAs(&outputPdfFilePath);
    if (saveResult != errNone) {
        fprintf(stderr, "Couldn't save the file properly.\n");
        return 5;
    }

    return 0;
}
