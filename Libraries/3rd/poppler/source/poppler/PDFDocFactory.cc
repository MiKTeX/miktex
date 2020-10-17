//========================================================================
//
// PDFDocFactory.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010 Albert Astals Cid <aacid@kde.org>
// Copyright 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#include <config.h>

#include "PDFDocFactory.h"

#include "goo/GooString.h"
#include "PDFDoc.h"
#include "LocalPDFDocBuilder.h"
#include "StdinPDFDocBuilder.h"
#ifdef ENABLE_LIBCURL
#    include "CurlPDFDocBuilder.h"
#endif
#include "ErrorCodes.h"

//------------------------------------------------------------------------
// PDFDocFactory
//------------------------------------------------------------------------

PDFDocFactory::PDFDocFactory(std::vector<PDFDocBuilder *> *pdfDocBuilders)
{
    if (pdfDocBuilders) {
        builders = pdfDocBuilders;
    } else {
        builders = new std::vector<PDFDocBuilder *>();
    }
    builders->push_back(new LocalPDFDocBuilder());
    builders->push_back(new StdinPDFDocBuilder());
#ifdef ENABLE_LIBCURL
    builders->push_back(new CurlPDFDocBuilder());
#endif
}

PDFDocFactory::~PDFDocFactory()
{
    if (builders) {
        for (auto entry : *builders) {
            delete entry;
        }
        delete builders;
    }
}

PDFDoc *PDFDocFactory::createPDFDoc(const GooString &uri, GooString *ownerPassword, GooString *userPassword, void *guiDataA)
{
    for (int i = builders->size() - 1; i >= 0; i--) {
        PDFDocBuilder *builder = (*builders)[i];
        if (builder->supports(uri)) {
            return builder->buildPDFDoc(uri, ownerPassword, userPassword, guiDataA);
        }
    }

    error(errInternal, -1, "Cannot handle URI '{0:t}'.", &uri);
    GooString *fileName = uri.copy();
    return PDFDoc::ErrorPDFDoc(errOpenFile, fileName);
}

void PDFDocFactory::registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder)
{
    builders->push_back(pdfDocBuilder);
}
