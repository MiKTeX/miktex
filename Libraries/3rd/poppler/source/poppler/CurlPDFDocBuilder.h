//========================================================================
//
// CurlPDFDocBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2018 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#ifndef CURLPDFDOCBUILDER_H
#define CURLPDFDOCBUILDER_H

#include "PDFDocBuilder.h"

//------------------------------------------------------------------------
// CurlPDFDocBuilder
//
// The CurlPDFDocBuilder implements a PDFDocBuilder for 'http(s)://'.
//------------------------------------------------------------------------

class CurlPDFDocBuilder : public PDFDocBuilder
{

public:
    PDFDoc *buildPDFDoc(const GooString &uri, GooString *ownerPassword = nullptr, GooString *userPassword = nullptr, void *guiDataA = nullptr) override;
    bool supports(const GooString &uri) override;
};

#endif /* CURLPDFDOCBUILDER_H */
