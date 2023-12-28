//========================================================================
//
// PDFDocFactory.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2017 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright 2019, 2021 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2021 Christian Persch <chpe@src.gnome.org>
//
//========================================================================

#include <config.h>

#include "PDFDocFactory.h"

#include "goo/GooString.h"
#include "PDFDoc.h"
#include "LocalPDFDocBuilder.h"
#include "FDPDFDocBuilder.h"
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
    builders->push_back(new FileDescriptorPDFDocBuilder());
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

std::unique_ptr<PDFDoc> PDFDocFactory::createPDFDoc(const GooString &uri, const std::optional<GooString> &ownerPassword, const std::optional<GooString> &userPassword, void *guiDataA)
{
    for (int i = builders->size() - 1; i >= 0; i--) {
        PDFDocBuilder *builder = (*builders)[i];
        if (builder->supports(uri)) {
            return builder->buildPDFDoc(uri, ownerPassword, userPassword, guiDataA);
        }
    }

    error(errInternal, -1, "Cannot handle URI '{0:t}'.", &uri);
    return PDFDoc::ErrorPDFDoc(errOpenFile, std::unique_ptr<GooString>(uri.copy()));
}

void PDFDocFactory::registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder)
{
    builders->push_back(pdfDocBuilder);
}
