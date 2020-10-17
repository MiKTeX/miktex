//========================================================================
//
// FoFiType1.h
//
// Copyright 1999-2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2018 Albert Astals Cid <aacid@kde.org>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef FOFITYPE1_H
#define FOFITYPE1_H

#include "FoFiBase.h"

//------------------------------------------------------------------------
// FoFiType1
//------------------------------------------------------------------------

class FoFiType1 : public FoFiBase
{
public:
    // Create a FoFiType1 object from a memory buffer.
    static FoFiType1 *make(const char *fileA, int lenA);

    // Create a FoFiType1 object from a file on disk.
    static FoFiType1 *load(const char *fileName);

    ~FoFiType1() override;

    // Return the font name.
    const char *getName();

    // Return the encoding, as an array of 256 names (any of which may
    // be NULL).
    char **getEncoding();

    // Return the font matrix as an array of six numbers.
    void getFontMatrix(double *mat);

    // Write a version of the Type 1 font file with a new encoding.
    void writeEncoded(const char **newEncoding, FoFiOutputFunc outputFunc, void *outputStream) const;

private:
    FoFiType1(const char *fileA, int lenA, bool freeFileDataA);

    char *getNextLine(char *line) const;
    void parse();
    void undoPFB();

    char *name;
    char **encoding;
    double fontMatrix[6];
    bool parsed;
};

#endif
