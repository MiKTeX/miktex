//========================================================================
//
// PDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2018 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef PDFDOCBUILDER_H
#define PDFDOCBUILDER_H

#include "PDFDoc.h"
class GooString;

//------------------------------------------------------------------------
// PDFDocBuilder
//
// PDFDocBuilder is an abstract class that specifies the interface for
// constructing PDFDocs.
//------------------------------------------------------------------------

class PDFDocBuilder
{

public:
    PDFDocBuilder() = default;
    virtual ~PDFDocBuilder() = default;

    PDFDocBuilder(const PDFDocBuilder &) = delete;
    PDFDocBuilder &operator=(const PDFDocBuilder &) = delete;

    // Builds a new PDFDoc. Returns a PDFDoc. You should check this PDFDoc
    // with PDFDoc::isOk() for failures.
    // The caller is responsible for deleting ownerPassword, userPassWord and guiData.
    virtual PDFDoc *buildPDFDoc(const GooString &uri, GooString *ownerPassword = nullptr, GooString *userPassword = nullptr, void *guiDataA = nullptr) = 0;

    // Returns true if the builder supports building a PDFDoc from the URI.
    virtual bool supports(const GooString &uri) = 0;
};

#endif /* PDFDOCBUILDER_H */
