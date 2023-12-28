//========================================================================
//
// CurlPDFDocBuilder.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2010 Hib Eris <hib@hiberis.nl>
// Copyright 2010, 2017, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2021 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#include <config.h>

#include "CurlPDFDocBuilder.h"

#include "CachedFile.h"
#include "CurlCachedFile.h"
#include "ErrorCodes.h"

//------------------------------------------------------------------------
// CurlPDFDocBuilder
//------------------------------------------------------------------------

std::unique_ptr<PDFDoc> CurlPDFDocBuilder::buildPDFDoc(const GooString &uri, const std::optional<GooString> &ownerPassword, const std::optional<GooString> &userPassword, void *guiDataA)
{
    CachedFile *cachedFile = new CachedFile(new CurlCachedFileLoader(uri.toStr()));

    if (cachedFile->getLength() == ((unsigned int)-1)) {
        cachedFile->decRefCnt();
        return PDFDoc::ErrorPDFDoc(errOpenFile, std::unique_ptr<GooString>(uri.copy()));
    }

    BaseStream *str = new CachedFileStream(cachedFile, 0, false, cachedFile->getLength(), Object(objNull));

    return std::make_unique<PDFDoc>(str, ownerPassword, userPassword, guiDataA);
}

bool CurlPDFDocBuilder::supports(const GooString &uri)
{
    if (uri.cmpN("http://", 7) == 0 || uri.cmpN("https://", 8) == 0) {
        return true;
    } else {
        return false;
    }
}
