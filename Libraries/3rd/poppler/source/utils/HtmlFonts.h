//========================================================================
//
// This file comes from pdftohtml project
// http://pdftohtml.sourceforge.net
//
// Copyright from:
// Gueorgui Ovtcharov
// Rainer Dorsch <http://www.ra.informatik.uni-stuttgart.de/~rainer/>
// Mikhail Kruk <meshko@cs.brandeis.edu>
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
// Copyright (C) 2010, 2012, 2017, 2018, 2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2011 Steven Murdoch <Steven.Murdoch@cl.cam.ac.uk>
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
// Copyright (C) 2012 Igor Slepchin <igor.slepchin@gmail.com>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2020 Eddie Kohler <ekohler@gmail.com>
// Copyright (C) 2022 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef _HTML_FONTS_H
#define _HTML_FONTS_H
#include "goo/GooString.h"
#include "GfxState.h"
#include "CharTypes.h"
#include <vector>

class HtmlFontColor
{
private:
    unsigned int r;
    unsigned int g;
    unsigned int b;
    unsigned int opacity;
    bool Ok(unsigned int xcol) { return xcol <= 255; }
    GooString *convtoX(unsigned int xcol) const;

public:
    HtmlFontColor() : r(0), g(0), b(0), opacity(255) { }
    HtmlFontColor(GfxRGB rgb, double opacity);
    HtmlFontColor(const HtmlFontColor &x)
    {
        r = x.r;
        g = x.g;
        b = x.b;
        opacity = x.opacity;
    }
    HtmlFontColor &operator=(const HtmlFontColor &x)
    {
        r = x.r;
        g = x.g;
        b = x.b;
        opacity = x.opacity;
        return *this;
    }
    ~HtmlFontColor() {};
    GooString *toString() const;
    double getOpacity() const { return opacity / 255.0; }
    bool isEqual(const HtmlFontColor &col) const { return ((r == col.r) && (g == col.g) && (b == col.b) && (opacity == col.opacity)); }
};

class HtmlFont
{
private:
    int size;
    int lineSize;
    bool italic;
    bool bold;
    bool rotOrSkewed;
    std::string familyName;
    GooString *FontName;
    HtmlFontColor color;
    double rotSkewMat[4]; // only four values needed for rotation and skew
public:
    HtmlFont(const GfxFont &font, int _size, GfxRGB rgb, double opacity);
    HtmlFont(const HtmlFont &x);
    HtmlFont &operator=(const HtmlFont &x);
    HtmlFontColor getColor() const { return color; }
    ~HtmlFont();
    GooString *getFullName();
    bool isItalic() const { return italic; }
    bool isBold() const { return bold; }
    bool isRotOrSkewed() const { return rotOrSkewed; }
    int getSize() const { return size; }
    int getLineSize() const { return lineSize; }
    void setLineSize(int _lineSize) { lineSize = _lineSize; }
    void setRotMat(const double *const mat)
    {
        rotOrSkewed = true;
        memcpy(rotSkewMat, mat, sizeof(rotSkewMat));
    }
    const double *getRotMat() const { return rotSkewMat; }
    GooString *getFontName();
    static GooString *HtmlFilter(const Unicode *u, int uLen); // char* s);
    bool isEqual(const HtmlFont &x) const;
    bool isEqualIgnoreBold(const HtmlFont &x) const;
    void print() const { printf("font: %s (%s) %d %s%s\n", FontName->c_str(), familyName.c_str(), size, bold ? "bold " : "", italic ? "italic " : ""); };
};

class HtmlFontAccu
{
private:
    std::vector<HtmlFont> accu;

public:
    HtmlFontAccu();
    ~HtmlFontAccu();
    HtmlFontAccu(const HtmlFontAccu &) = delete;
    HtmlFontAccu &operator=(const HtmlFontAccu &) = delete;
    int AddFont(const HtmlFont &font);
    const HtmlFont *Get(int i) const { return &accu[i]; }
    GooString *CSStyle(int i, int j = 0);
    int size() const { return accu.size(); }
};
#endif
