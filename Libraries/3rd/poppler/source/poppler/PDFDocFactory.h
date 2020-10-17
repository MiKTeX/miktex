//========================================================================
//
// PDFDocFactory.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2018 Albert Astals Cid <aacid@kde.org>
// Copyright 2019 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#ifndef PDFDOCFACTORY_H
#define PDFDOCFACTORY_H

#include "PDFDoc.h"

class GooString;
class PDFDocBuilder;

//------------------------------------------------------------------------
// PDFDocFactory
//
// PDFDocFactory allows the construction of PDFDocs from different URIs.
//
// By default, it supports local files, 'file://' and 'fd:0' (stdin). When
// compiled with libcurl, it also supports 'http://' and 'https://'.
//
// You can extend the supported URIs by giving a list of PDFDocBuilders to
// the constructor, or by registering a new PDFDocBuilder afterwards.
//------------------------------------------------------------------------

class PDFDocFactory
{

public:
    PDFDocFactory(std::vector<PDFDocBuilder *> *pdfDocBuilders = nullptr);
    ~PDFDocFactory();

    PDFDocFactory(const PDFDocFactory &) = delete;
    PDFDocFactory &operator=(const PDFDocFactory &) = delete;

    // Create a PDFDoc. Returns a PDFDoc. You should check this PDFDoc
    // with PDFDoc::isOk() for failures.
    // The caller is responsible for deleting ownerPassword, userPassWord and guiData.
    PDFDoc *createPDFDoc(const GooString &uri, GooString *ownerPassword = nullptr, GooString *userPassword = nullptr, void *guiDataA = nullptr);

    // Extend supported URIs with the ones from the PDFDocBuilder.
    void registerPDFDocBuilder(PDFDocBuilder *pdfDocBuilder);

private:
    std::vector<PDFDocBuilder *> *builders;
};

#endif /* PDFDOCFACTORY_H */
