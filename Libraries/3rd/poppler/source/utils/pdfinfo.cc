//========================================================================
//
// pdfinfo.cc
//
// Copyright 1998-2003 Glyph & Cog, LLC
// Copyright 2013 Igalia S.L.
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2006 Dom Lachowicz <cinamod@hotmail.com>
// Copyright (C) 2007-2010, 2012, 2016-2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2011 Vittal Aithal <vittal.aithal@cognidox.com>
// Copyright (C) 2012, 2013, 2016-2018 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2013 Adrian Perez de Castro <aperez@igalia.com>
// Copyright (C) 2013 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2018 Evangelos Rigas <erigas@rnd2.org>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2019 Thomas Fischer <fischer@unix-ag.uni-kl.de>
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
#include <ctime>
#include <cmath>
#include <map>
#include "parseargs.h"
#include "printencodings.h"
#include "goo/GooString.h"
#include "goo/gfile.h"
#include "goo/glibc.h"
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
#include "CharTypes.h"
#include "UnicodeMap.h"
#include "UTF.h"
#include "Error.h"
#include "DateInfo.h"
#include "JSInfo.h"
#include "StructTreeRoot.h"
#include "StructElement.h"
#include "Win32Console.h"

static int firstPage = 1;
static int lastPage = 0;
static bool printBoxes = false;
static bool printMetadata = false;
static bool printJS = false;
static bool isoDates = false;
static bool rawDates = false;
static char textEncName[128] = "";
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static bool printVersion = false;
static bool printHelp = false;
static bool printEnc = false;
static bool printStructure = false;
static bool printStructureText = false;
static bool printDests = false;

static const ArgDesc argDesc[] = { { "-f", argInt, &firstPage, 0, "first page to convert" },
                                   { "-l", argInt, &lastPage, 0, "last page to convert" },
                                   { "-box", argFlag, &printBoxes, 0, "print the page bounding boxes" },
                                   { "-meta", argFlag, &printMetadata, 0, "print the document metadata (XML)" },
                                   { "-js", argFlag, &printJS, 0, "print all JavaScript in the PDF" },
                                   { "-struct", argFlag, &printStructure, 0, "print the logical document structure (for tagged files)" },
                                   { "-struct-text", argFlag, &printStructureText, 0, "print text contents along with document structure (for tagged files)" },
                                   { "-isodates", argFlag, &isoDates, 0, "print the dates in ISO-8601 format" },
                                   { "-rawdates", argFlag, &rawDates, 0, "print the undecoded date strings directly from the PDF file" },
                                   { "-dests", argFlag, &printDests, 0, "print all named destinations in the PDF" },
                                   { "-enc", argString, textEncName, sizeof(textEncName), "output text encoding name" },
                                   { "-listenc", argFlag, &printEnc, 0, "list available encodings" },
                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

static void printInfoString(Dict *infoDict, const char *key, const char *text, const UnicodeMap *uMap)
{
    const GooString *s1;
    Unicode *u;
    char buf[8];
    int i, n, len;

    Object obj = infoDict->lookup(key);
    if (obj.isString()) {
        fputs(text, stdout);
        s1 = obj.getString();
        len = TextStringToUCS4(s1, &u);
        for (i = 0; i < len; i++) {
            n = uMap->mapUnicode(u[i], buf, sizeof(buf));
            fwrite(buf, 1, n, stdout);
        }
        gfree(u);
        fputc('\n', stdout);
    }
}

static void printInfoDate(Dict *infoDict, const char *key, const char *text)
{
    const char *s;
    int year, mon, day, hour, min, sec, tz_hour, tz_minute;
    char tz;
    struct tm tmStruct;
    time_t time;
    char buf[256];

    Object obj = infoDict->lookup(key);
    if (obj.isString()) {
        fputs(text, stdout);
        s = obj.getString()->c_str();
        // TODO do something with the timezone info
        if (parseDateString(s, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute)) {
            tmStruct.tm_year = year - 1900;
            tmStruct.tm_mon = mon - 1;
            tmStruct.tm_mday = day;
            tmStruct.tm_hour = hour;
            tmStruct.tm_min = min;
            tmStruct.tm_sec = sec;
            tmStruct.tm_wday = -1;
            tmStruct.tm_yday = -1;
            tmStruct.tm_isdst = -1;
            // compute the tm_wday and tm_yday fields
            time = timegm(&tmStruct);
            if (time != (time_t)-1) {
                int offset = (tz_hour * 60 + tz_minute) * 60;
                if (tz == '-')
                    offset *= -1;
                time -= offset;
                localtime_r(&time, &tmStruct);
                strftime(buf, sizeof(buf), "%c %Z", &tmStruct);
                fputs(buf, stdout);
            } else {
                fputs(s, stdout);
            }
        } else {
            fputs(s, stdout);
        }
        fputc('\n', stdout);
    }
}

static void printISODate(Dict *infoDict, const char *key, const char *text)
{
    const char *s;
    int year, mon, day, hour, min, sec, tz_hour, tz_minute;
    char tz;

    Object obj = infoDict->lookup(key);
    if (obj.isString()) {
        fputs(text, stdout);
        s = obj.getString()->c_str();
        if (parseDateString(s, &year, &mon, &day, &hour, &min, &sec, &tz, &tz_hour, &tz_minute)) {
            fprintf(stdout, "%04d-%02d-%02dT%02d:%02d:%02d", year, mon, day, hour, min, sec);
            if (tz_hour == 0 && tz_minute == 0) {
                fprintf(stdout, "Z");
            } else {
                fprintf(stdout, "%c%02d", tz, tz_hour);
                if (tz_minute)
                    fprintf(stdout, ":%02d", tz_minute);
            }
        } else {
            fputs(s, stdout);
        }
        fputc('\n', stdout);
    }
}

static void printBox(const char *text, const PDFRectangle *box)
{
    printf("%s%8.2f %8.2f %8.2f %8.2f\n", text, box->x1, box->y1, box->x2, box->y2);
}

static void printIndent(unsigned indent)
{
    while (indent--) {
        putchar(' ');
        putchar(' ');
    }
}

static void printAttribute(const Attribute *attribute, unsigned indent)
{
    printIndent(indent);
    printf(" /%s ", attribute->getTypeName());
    if (attribute->getType() == Attribute::UserProperty) {
        GooString *name = attribute->getName();
        printf("(%s) ", name->c_str());
        delete name;
    }
    attribute->getValue()->print(stdout);
    if (attribute->getFormattedValue()) {
        printf(" \"%s\"", attribute->getFormattedValue());
    }
    if (attribute->isHidden()) {
        printf(" [hidden]");
    }
}

static void printStruct(const StructElement *element, unsigned indent)
{
    if (element->isObjectRef()) {
        printIndent(indent);
        printf("Object %i %i\n", element->getObjectRef().num, element->getObjectRef().gen);
        return;
    }

    if (printStructureText && element->isContent()) {
        GooString *text = element->getText(false);
        printIndent(indent);
        if (text) {
            printf("\"%s\"\n", text->c_str());
        } else {
            printf("(No content?)\n");
        }
        delete text;
    }

    if (!element->isContent()) {
        printIndent(indent);
        printf("%s", element->getTypeName());
        if (element->getID()) {
            printf(" <%s>", element->getID()->c_str());
        }
        if (element->getTitle()) {
            printf(" \"%s\"", element->getTitle()->c_str());
        }
        if (element->getRevision() > 0) {
            printf(" r%u", element->getRevision());
        }
        if (element->isInline() || element->isBlock()) {
            printf(" (%s)", element->isInline() ? "inline" : "block");
        }
        if (element->getNumAttributes()) {
            putchar(':');
            for (unsigned i = 0; i < element->getNumAttributes(); i++) {
                putchar('\n');
                printAttribute(element->getAttribute(i), indent + 1);
            }
        }

        putchar('\n');
        for (unsigned i = 0; i < element->getNumChildren(); i++) {
            printStruct(element->getChild(i), indent + 1);
        }
    }
}

struct GooStringCompare
{
    bool operator()(GooString *lhs, GooString *rhs) const { return lhs->cmp(const_cast<GooString *>(rhs)) < 0; }
};

static void printLinkDest(const std::unique_ptr<LinkDest> &dest)
{
    GooString s;

    switch (dest->getKind()) {
    case destXYZ:
        s.append("[ XYZ ");
        if (dest->getChangeLeft()) {
            s.appendf("{0:4.0g} ", dest->getLeft());
        } else {
            s.append("null ");
        }
        if (dest->getChangeTop()) {
            s.appendf("{0:4.0g} ", dest->getTop());
        } else {
            s.append("null ");
        }
        if (dest->getChangeZoom()) {
            s.appendf("{0:4.2f} ", dest->getZoom());
        } else {
            s.append("null ");
        }
        break;
    case destFit:
        s.append("[ Fit ");
        break;
    case destFitH:
        if (dest->getChangeTop()) {
            s.appendf("[ FitH {0:4.0g} ", dest->getTop());
        } else {
            s.append("[ FitH null ");
        }
        break;
    case destFitV:
        if (dest->getChangeLeft()) {
            s.appendf("[ FitV {0:4.0g} ", dest->getLeft());
        } else {
            s.append("[ FitV null ");
        }
        break;
    case destFitR:
        s.appendf("[ FitR {0:4.0g} {1:4.0g} {2:4.0g} {3:4.0g} ", dest->getLeft(), dest->getBottom(), dest->getRight(), dest->getTop());
        break;
    case destFitB:
        s.append("[ FitB ");
        break;
    case destFitBH:
        if (dest->getChangeTop()) {
            s.appendf("[ FitBH {0:4.0g} ", dest->getTop());
        } else {
            s.append("[ FitBH null ");
        }
        break;
    case destFitBV:
        if (dest->getChangeLeft()) {
            s.appendf("[ FitBV {0:4.0g} ", dest->getLeft());
        } else {
            s.append("[ FitBV null ");
        }
        break;
    }

    s.append("                                ");
    s.setChar(26, ']');
    s.setChar(27, '\0');
    printf("%s", s.c_str());
}

static void printDestinations(PDFDoc *doc, const UnicodeMap *uMap)
{
    std::map<Ref, std::map<GooString *, std::unique_ptr<LinkDest>, GooStringCompare>> map;

    int numDests = doc->getCatalog()->numDestNameTree();
    for (int i = 0; i < numDests; i++) {
        GooString *name = new GooString(doc->getCatalog()->getDestNameTreeName(i));
        std::unique_ptr<LinkDest> dest = doc->getCatalog()->getDestNameTreeDest(i);
        if (dest && dest->isPageRef()) {
            Ref pageRef = dest->getPageRef();
            map[pageRef].insert(std::make_pair(name, std::move(dest)));
        } else {
            delete name;
        }
    }

    numDests = doc->getCatalog()->numDests();
    for (int i = 0; i < numDests; i++) {
        GooString *name = new GooString(doc->getCatalog()->getDestsName(i));
        std::unique_ptr<LinkDest> dest = doc->getCatalog()->getDestsDest(i);
        if (dest && dest->isPageRef()) {
            Ref pageRef = dest->getPageRef();
            map[pageRef].insert(std::make_pair(name, std::move(dest)));
        } else {
            delete name;
        }
    }

    printf("Page  Destination                 Name\n");
    for (int i = firstPage; i <= lastPage; i++) {
        Ref *ref = doc->getCatalog()->getPageRef(i);
        if (ref) {
            auto pageDests = map.find(*ref);
            if (pageDests != map.end()) {
                for (auto &it : pageDests->second) {
                    printf("%4d ", i);
                    printLinkDest(it.second);
                    printf(" \"");
                    Unicode *u;
                    char buf[8];
                    const int len = TextStringToUCS4(it.first, &u);
                    for (int j = 0; j < len; j++) {
                        const int n = uMap->mapUnicode(u[j], buf, sizeof(buf));
                        fwrite(buf, 1, n, stdout);
                    }
                    gfree(u);
                    printf("\"\n");
                    delete it.first;
                }
            }
        }
    }
}

static void printPdfSubtype(PDFDoc *doc, const UnicodeMap *uMap)
{
    const Object info = doc->getDocInfo();
    if (info.isDict()) {
        const PDFSubtype pdftype = doc->getPDFSubtype();

        if ((pdftype == subtypeNull) | (pdftype == subtypeNone)) {
            return;
        }

        std::unique_ptr<GooString> part;
        std::unique_ptr<GooString> abbr;
        std::unique_ptr<GooString> standard;
        std::unique_ptr<GooString> typeExp;
        std::unique_ptr<GooString> confExp;

        // Form title from PDFSubtype
        switch (pdftype) {
        case subtypePDFA:
            printInfoString(info.getDict(), "GTS_PDFA1Version", "PDF subtype:    ", uMap);
            typeExp = std::make_unique<GooString>("ISO 19005 - Electronic document file format for long-term preservation (PDF/A)");
            standard = std::make_unique<GooString>("ISO 19005");
            abbr = std::make_unique<GooString>("PDF/A");
            break;
        case subtypePDFE:
            printInfoString(info.getDict(), "GTS_PDFEVersion", "PDF subtype:    ", uMap);
            typeExp = std::make_unique<GooString>("ISO 24517 - Engineering document format using PDF (PDF/E)");
            standard = std::make_unique<GooString>("ISO 24517");
            abbr = std::make_unique<GooString>("PDF/E");
            break;
        case subtypePDFUA:
            printInfoString(info.getDict(), "GTS_PDFUAVersion", "PDF subtype:    ", uMap);
            typeExp = std::make_unique<GooString>("ISO 14289 - Electronic document file format enhancement for accessibility (PDF/UA)");
            standard = std::make_unique<GooString>("ISO 14289");
            abbr = std::make_unique<GooString>("PDF/UA");
            break;
        case subtypePDFVT:
            printInfoString(info.getDict(), "GTS_PDFVTVersion", "PDF subtype:    ", uMap);
            typeExp = std::make_unique<GooString>("ISO 16612 - Electronic document file format for variable data exchange (PDF/VT)");
            standard = std::make_unique<GooString>("ISO 16612");
            abbr = std::make_unique<GooString>("PDF/VT");
            break;
        case subtypePDFX:
            printInfoString(info.getDict(), "GTS_PDFXVersion", "PDF subtype:    ", uMap);
            typeExp = std::make_unique<GooString>("ISO 15930 - Electronic document file format for prepress digital data exchange (PDF/X)");
            standard = std::make_unique<GooString>("ISO 15930");
            abbr = std::make_unique<GooString>("PDF/X");
            break;
        case subtypeNone:
        case subtypeNull:
        default:
            return;
        }

        // Form the abbreviation from PDFSubtypePart and PDFSubtype
        const PDFSubtypePart subpart = doc->getPDFSubtypePart();
        switch (pdftype) {
        case subtypePDFX:
            switch (subpart) {
            case subtypePart1:
                abbr->append("-1:2001");
                break;
            case subtypePart2:
                abbr->append("-2");
                break;
            case subtypePart3:
                abbr->append("-3:2002");
                break;
            case subtypePart4:
                abbr->append("-1:2003");
                break;
            case subtypePart5:
                abbr->append("-2");
                break;
            case subtypePart6:
                abbr->append("-3:2003");
                break;
            case subtypePart7:
                abbr->append("-4");
                break;
            case subtypePart8:
                abbr->append("-5");
                break;
            default:
                break;
            }
            break;
        case subtypeNone:
        case subtypeNull:
            break;
        default:
            abbr->appendf("-{0:d}", subpart);
            break;
        }

        // Form standard from PDFSubtypePart
        switch (subpart) {
        case subtypePartNone:
        case subtypePartNull:
            break;
        default:
            standard->appendf("-{0:d}", subpart);
            break;
        }

        // Form the subtitle from PDFSubtypePart and PDFSubtype
        switch (pdftype) {
        case subtypePDFA:
            switch (subpart) {
            case subtypePart1:
                part = std::make_unique<GooString>("Use of PDF 1.4");
                break;
            case subtypePart2:
                part = std::make_unique<GooString>("Use of ISO 32000-1");
                break;
            case subtypePart3:
                part = std::make_unique<GooString>("Use of ISO 32000-1 with support for embedded files");
                break;
            default:
                break;
            }
            break;
        case subtypePDFE:
            switch (subpart) {
            case subtypePart1:
                part = std::make_unique<GooString>("Use of PDF 1.6");
                break;
            default:
                break;
            }
            break;
        case subtypePDFUA:
            switch (subpart) {
            case subtypePart1:
                part = std::make_unique<GooString>("Use of ISO 32000-1");
                break;
            case subtypePart2:
                part = std::make_unique<GooString>("Use of ISO 32000-2");
                break;
            case subtypePart3:
                part = std::make_unique<GooString>("Use of ISO 32000-1 with support for embedded files");
                break;
            default:
                break;
            }
            break;
        case subtypePDFVT:
            switch (subpart) {
            case subtypePart1:
                part = std::make_unique<GooString>("Using PPML 2.1 and PDF 1.4");
                break;
            case subtypePart2:
                part = std::make_unique<GooString>("Using PDF/X-4 and PDF/X-5 (PDF/VT-1 and PDF/VT-2)");
                break;
            case subtypePart3:
                part = std::make_unique<GooString>("Using PDF/X-6 (PDF/VT-3)");
                break;
            default:
                break;
            }
            break;
        case subtypePDFX:
            switch (subpart) {
            case subtypePart1:
                part = std::make_unique<GooString>("Complete exchange using CMYK data (PDF/X-1 and PDF/X-1a)");
                break;
            case subtypePart3:
                part = std::make_unique<GooString>("Complete exchange suitable for colour-managed workflows (PDF/X-3)");
                break;
            case subtypePart4:
                part = std::make_unique<GooString>("Complete exchange of CMYK and spot colour printing data using PDF 1.4 (PDF/X-1a)");
                break;
            case subtypePart5:
                part = std::make_unique<GooString>("Partial exchange of printing data using PDF 1.4 (PDF/X-2) [Withdrawn]");
                break;
            case subtypePart6:
                part = std::make_unique<GooString>("Complete exchange of printing data suitable for colour-managed workflows using PDF 1.4 (PDF/X-3)");
                break;
            case subtypePart7:
                part = std::make_unique<GooString>("Complete exchange of printing data (PDF/X-4) and partial exchange of printing data with external profile reference (PDF/X-4p) using PDF 1.6");
                break;
            case subtypePart8:
                part = std::make_unique<GooString>("Partial exchange of printing data using PDF 1.6 (PDF/X-5)");
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }

        // Form Conformance explanation from PDFSubtypeConformance
        switch (doc->getPDFSubtypeConformance()) {
        case subtypeConfA:
            confExp = std::make_unique<GooString>("Level A, Accessible");
            break;
        case subtypeConfB:
            confExp = std::make_unique<GooString>("Level B, Basic");
            break;
        case subtypeConfG:
            confExp = std::make_unique<GooString>("Level G, External graphical content");
            break;
        case subtypeConfN:
            confExp = std::make_unique<GooString>("Level N, External ICC profile");
            break;
        case subtypeConfP:
            confExp = std::make_unique<GooString>("Level P, Embedded ICC profile");
            break;
        case subtypeConfPG:
            confExp = std::make_unique<GooString>("Level PG, Embedded ICC profile and external graphical content");
            break;
        case subtypeConfU:
            confExp = std::make_unique<GooString>("Level U, Unicode support");
            break;
        case subtypeConfNone:
        case subtypeConfNull:
        default:
            confExp.reset();
            break;
        }

        printf("    Title:         %s\n", typeExp->c_str());
        printf("    Abbreviation:  %s\n", abbr->c_str());
        if (part.get())
            printf("    Subtitle:      Part %d: %s\n", subpart, part->c_str());
        else
            printf("    Subtitle:      Part %d\n", subpart);
        printf("    Standard:      %s-%d\n", typeExp->toStr().substr(0, 9).c_str(), subpart);
        if (confExp.get())
            printf("    Conformance:   %s\n", confExp->c_str());
    }
}

static void printInfo(PDFDoc *doc, const UnicodeMap *uMap, long long filesize, bool multiPage)
{
    Page *page;
    char buf[256];
    double w, h, wISO, hISO, isoThreshold;
    int pg, i;
    int r;

    // print doc info
    Object info = doc->getDocInfo();
    if (info.isDict()) {
        printInfoString(info.getDict(), "Title", "Title:          ", uMap);
        printInfoString(info.getDict(), "Subject", "Subject:        ", uMap);
        printInfoString(info.getDict(), "Keywords", "Keywords:       ", uMap);
        printInfoString(info.getDict(), "Author", "Author:         ", uMap);
        printInfoString(info.getDict(), "Creator", "Creator:        ", uMap);
        printInfoString(info.getDict(), "Producer", "Producer:       ", uMap);
        if (isoDates) {
            printISODate(info.getDict(), "CreationDate", "CreationDate:   ");
            printISODate(info.getDict(), "ModDate", "ModDate:        ");
        } else if (rawDates) {
            printInfoString(info.getDict(), "CreationDate", "CreationDate:   ", uMap);
            printInfoString(info.getDict(), "ModDate", "ModDate:        ", uMap);
        } else {
            printInfoDate(info.getDict(), "CreationDate", "CreationDate:   ");
            printInfoDate(info.getDict(), "ModDate", "ModDate:        ");
        }
    }

    // print tagging info
    printf("Tagged:         %s\n", (doc->getCatalog()->getMarkInfo() & Catalog::markInfoMarked) ? "yes" : "no");
    printf("UserProperties: %s\n", (doc->getCatalog()->getMarkInfo() & Catalog::markInfoUserProperties) ? "yes" : "no");
    printf("Suspects:       %s\n", (doc->getCatalog()->getMarkInfo() & Catalog::markInfoSuspects) ? "yes" : "no");

    // print form info
    switch (doc->getCatalog()->getFormType()) {
    case Catalog::NoForm:
        printf("Form:           none\n");
        break;
    case Catalog::AcroForm:
        printf("Form:           AcroForm\n");
        break;
    case Catalog::XfaForm:
        printf("Form:           XFA\n");
        break;
    }

    // print javascript info
    {
        JSInfo jsInfo(doc, firstPage - 1);
        jsInfo.scanJS(lastPage - firstPage + 1);
        printf("JavaScript:     %s\n", jsInfo.containsJS() ? "yes" : "no");
    }

    // print page count
    printf("Pages:          %d\n", doc->getNumPages());

    // print encryption info
    printf("Encrypted:      ");
    if (doc->isEncrypted()) {
        unsigned char *fileKey;
        CryptAlgorithm encAlgorithm;
        int keyLength;
        doc->getXRef()->getEncryptionParameters(&fileKey, &encAlgorithm, &keyLength);

        const char *encAlgorithmName = "unknown";
        switch (encAlgorithm) {
        case cryptRC4:
            encAlgorithmName = "RC4";
            break;
        case cryptAES:
            encAlgorithmName = "AES";
            break;
        case cryptAES256:
            encAlgorithmName = "AES-256";
            break;
        case cryptNone:
            break;
        }

        printf("yes (print:%s copy:%s change:%s addNotes:%s algorithm:%s)\n", doc->okToPrint(true) ? "yes" : "no", doc->okToCopy(true) ? "yes" : "no", doc->okToChange(true) ? "yes" : "no", doc->okToAddNotes(true) ? "yes" : "no",
               encAlgorithmName);
    } else {
        printf("no\n");
    }

    // print page size
    for (pg = firstPage; pg <= lastPage; ++pg) {
        w = doc->getPageCropWidth(pg);
        h = doc->getPageCropHeight(pg);
        if (multiPage) {
            printf("Page %4d size: %g x %g pts", pg, w, h);
        } else {
            printf("Page size:      %g x %g pts", w, h);
        }
        if ((fabs(w - 612) < 1 && fabs(h - 792) < 1) || (fabs(w - 792) < 1 && fabs(h - 612) < 1)) {
            printf(" (letter)");
        } else {
            hISO = sqrt(sqrt(2.0)) * 7200 / 2.54;
            wISO = hISO / sqrt(2.0);
            isoThreshold = hISO * 0.003; ///< allow for 0.3% error when guessing conformance to ISO 216, A series
            for (i = 0; i <= 6; ++i) {
                if ((fabs(w - wISO) < isoThreshold && fabs(h - hISO) < isoThreshold) || (fabs(w - hISO) < isoThreshold && fabs(h - wISO) < isoThreshold)) {
                    printf(" (A%d)", i);
                    break;
                }
                hISO = wISO;
                wISO /= sqrt(2.0);
                isoThreshold /= sqrt(2.0);
            }
        }
        printf("\n");
        r = doc->getPageRotate(pg);
        if (multiPage) {
            printf("Page %4d rot:  %d\n", pg, r);
        } else {
            printf("Page rot:       %d\n", r);
        }
    }

    // print the boxes
    if (printBoxes) {
        if (multiPage) {
            for (pg = firstPage; pg <= lastPage; ++pg) {
                page = doc->getPage(pg);
                if (!page) {
                    error(errSyntaxError, -1, "Failed to print boxes for page {0:d}", pg);
                    continue;
                }
                sprintf(buf, "Page %4d MediaBox: ", pg);
                printBox(buf, page->getMediaBox());
                sprintf(buf, "Page %4d CropBox:  ", pg);
                printBox(buf, page->getCropBox());
                sprintf(buf, "Page %4d BleedBox: ", pg);
                printBox(buf, page->getBleedBox());
                sprintf(buf, "Page %4d TrimBox:  ", pg);
                printBox(buf, page->getTrimBox());
                sprintf(buf, "Page %4d ArtBox:   ", pg);
                printBox(buf, page->getArtBox());
            }
        } else {
            page = doc->getPage(firstPage);
            if (!page) {
                error(errSyntaxError, -1, "Failed to print boxes for page {0:d}", firstPage);
            } else {
                printBox("MediaBox:       ", page->getMediaBox());
                printBox("CropBox:        ", page->getCropBox());
                printBox("BleedBox:       ", page->getBleedBox());
                printBox("TrimBox:        ", page->getTrimBox());
                printBox("ArtBox:         ", page->getArtBox());
            }
        }
    }

    // print file size
    printf("File size:      %lld bytes\n", filesize);

    // print linearization info
    printf("Optimized:      %s\n", doc->isLinearized() ? "yes" : "no");

    // print PDF version
    printf("PDF version:    %d.%d\n", doc->getPDFMajorVersion(), doc->getPDFMinorVersion());

    printPdfSubtype(doc, uMap);
}

#if defined(MIKTEX)
int Main(int argc, char** argv)
#else
int main(int argc, char *argv[])
#endif
{
    PDFDoc *doc;
    GooString *fileName;
    GooString *ownerPW, *userPW;
    const UnicodeMap *uMap;
    FILE *f;
    bool ok;
    int exitCode;
    bool multiPage;

    exitCode = 99;

    // parse args
    Win32Console win32console(&argc, &argv);
    ok = parseArgs(argDesc, &argc, argv);
    if (!ok || (argc != 2 && !printEnc) || printVersion || printHelp) {
        fprintf(stderr, "pdfinfo version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdfinfo", "<PDF-file>", argDesc);
        }
        if (printVersion || printHelp)
            exitCode = 0;
        goto err0;
    }

    if (printStructureText)
        printStructure = true;

    // read config file
    globalParams = std::make_unique<GlobalParams>();

    if (printEnc) {
        printEncodings();
        exitCode = 0;
        goto err0;
    }

    fileName = new GooString(argv[1]);

    if (textEncName[0]) {
        globalParams->setTextEncoding(textEncName);
    }

    // get mapping to output encoding
    if (!(uMap = globalParams->getTextEncoding())) {
        error(errCommandLine, -1, "Couldn't get text encoding");
        delete fileName;
        goto err1;
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
        goto err2;
    }

    // get page range
    if (firstPage < 1) {
        firstPage = 1;
    }
    if (lastPage == 0) {
        multiPage = false;
    } else {
        multiPage = true;
    }
    if (lastPage < 1 || lastPage > doc->getNumPages()) {
        lastPage = doc->getNumPages();
    }
    if (lastPage < firstPage) {
        error(errCommandLine, -1, "Wrong page range given: the first page ({0:d}) can not be after the last page ({1:d}).", firstPage, lastPage);
        goto err2;
    }

    if (printMetadata) {
        // print the metadata
        const GooString *metadata = doc->readMetadata();
        if (metadata) {
            fputs(metadata->c_str(), stdout);
            fputc('\n', stdout);
            delete metadata;
        }
    } else if (printJS) {
        // print javascript
        JSInfo jsInfo(doc, firstPage - 1);
        jsInfo.scanJS(lastPage - firstPage + 1, stdout, uMap);
    } else if (printStructure || printStructureText) {
        // print structure
        const StructTreeRoot *structTree = doc->getCatalog()->getStructTreeRoot();
        if (structTree) {
            for (unsigned i = 0; i < structTree->getNumChildren(); i++) {
                printStruct(structTree->getChild(i), 0);
            }
        }
    } else if (printDests) {
        printDestinations(doc, uMap);
    } else {
        // print info
        long long filesize = 0;

        f = fopen(fileName->c_str(), "rb");
        if (f) {
            Gfseek(f, 0, SEEK_END);
            filesize = Gftell(f);
            fclose(f);
        }

        if (multiPage == false)
            lastPage = 1;

        printInfo(doc, uMap, filesize, multiPage);
    }
    exitCode = 0;

    // clean up
err2:
    delete doc;
    delete fileName;
err1:
err0:

    return exitCode;
}
