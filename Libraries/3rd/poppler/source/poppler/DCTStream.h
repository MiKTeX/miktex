//========================================================================
//
// DCTStream.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2005 Jeff Muizelaar <jeff@infidigm.net>
// Copyright 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright 2005-2007, 2009-2011, 2017, 2019, 2021 Albert Astals Cid <aacid@kde.org>
// Copyright 2010 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright 2011 Daiki Ueno <ueno@unixuser.org>
// Copyright 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright 2020 Lluís Batlle i Rossell <viric@viric.name>
//
//========================================================================

#ifndef DCTSTREAM_H
#define DCTSTREAM_H

#include "poppler-config.h"
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <csetjmp>
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#include <cstring>
#include <cctype>
#include "goo/gmem.h"
#include "goo/gfile.h"
#include "Error.h"
#include "Object.h"
#include "Decrypt.h"
#include "Stream.h"

extern "C" {
#include <jpeglib.h>
#include <jerror.h>
}

struct str_src_mgr
{
    struct jpeg_source_mgr pub;
    JOCTET buffer;
    Stream *str;
    int index;
};

struct str_error_mgr
{
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
    int width;
    int height;
};

class DCTStream : public FilterStream
{
public:
    DCTStream(Stream *strA, int colorXformA, Dict *dict, int recursion);
    ~DCTStream() override;
    StreamKind getKind() const override { return strDCT; }
    void reset() override;
    int getChar() override;
    int lookChar() override;
    GooString *getPSFilter(int psLevel, const char *indent) override;
    bool isBinary(bool last = true) const override;

private:
    void init();

    bool hasGetChars() override { return true; }
    bool readLine();
    int getChars(int nChars, unsigned char *buffer) override;

    int colorXform;
    JSAMPLE *current;
    JSAMPLE *limit;
    struct jpeg_decompress_struct cinfo;
    struct str_error_mgr err;
    struct str_src_mgr src;
    JSAMPARRAY row_buffer;
};

#endif
