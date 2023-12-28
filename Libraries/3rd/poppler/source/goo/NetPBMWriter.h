//========================================================================
//
// NetPBMWriter.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright (C) 2009 Stefan Thomas <thomas@eload24.com>
// Copyright (C) 2009, 2011, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2010, 2013 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Brian Cameron <brian.cameron@oracle.com>
// Copyright (C) 2011 Thomas Freitag <Thomas.Freitag@alfa.de>
//
//========================================================================

#ifndef NETPBMWRITER_H
#define NETPBMWRITER_H

#include "poppler-config.h"
#include "poppler_private_export.h"

#include "ImgWriter.h"

// Writer for the NetPBM formats (PBM and PPM)
// This format is documented at:
//   http://netpbm.sourceforge.net/doc/pbm.html
//   http://netpbm.sourceforge.net/doc/ppm.html

class POPPLER_PRIVATE_EXPORT NetPBMWriter : public ImgWriter
{
public:
    /* RGB        - 3 bytes/pixel
     * MONOCHROME - 8 pixels/byte
     */
    enum Format
    {
        RGB,
        MONOCHROME
    };

    explicit NetPBMWriter(Format formatA = RGB);
    ~NetPBMWriter() override = default;

    bool init(FILE *f, int width, int height, double /*hDPI*/, double /*vDPI*/) override;

    bool writePointers(unsigned char **rowPointers, int rowCount) override;
    bool writeRow(unsigned char **row) override;

    bool close() override;

private:
    FILE *file;
    Format format;
    int width;
};

#endif
