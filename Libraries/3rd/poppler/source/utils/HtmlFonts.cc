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
// Copyright (C) 2007, 2010, 2012, 2018, 2020 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Boris Toloknov <tlknv@yandex.ru>
// Copyright (C) 2008 Tomas Are Haavet <tomasare@gmail.com>
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
// Copyright (C) 2011 Stephen Reichling <sreichling@chegg.com>
// Copyright (C) 2012 Igor Slepchin <igor.slepchin@gmail.com>
// Copyright (C) 2012 Luis Parravicini <lparravi@gmail.com>
// Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
// Copyright (C) 2017 Jason Crain <jason@inspiresomeone.us>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Steven Boswell <ulatekh@yahoo.com>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 Eddie Kohler <ekohler@gmail.com>
// Copyright (C) 2024 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "HtmlFonts.h"
#include "HtmlUtils.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include "GfxFont.h"
#include <cstdio>

namespace {

const char *const defaultFamilyName = "Times";

const char *const styleSuffixes[] = {
    "-Regular", "-Bold", "-BoldOblique", "-BoldItalic", "-Oblique", "-Italic", "-Roman",
};

void removeStyleSuffix(std::string &familyName)
{
    for (const char *const styleSuffix : styleSuffixes) {
        auto pos = familyName.rfind(styleSuffix);
        if (pos != std::string::npos) {
            familyName.resize(pos);
            return;
        }
    }
}

}

#define xoutRound(x) ((int)(x + 0.5))
extern bool xml;
extern bool fontFullName;

HtmlFontColor::HtmlFontColor(GfxRGB rgb, double opacity_)
{
    r = static_cast<int>(rgb.r / 65535.0 * 255.0);
    g = static_cast<int>(rgb.g / 65535.0 * 255.0);
    b = static_cast<int>(rgb.b / 65535.0 * 255.0);
    opacity = static_cast<int>(opacity_ * 255.999);
    if (!(Ok(r) && Ok(b) && Ok(g) && Ok(opacity))) {
        if (!globalParams->getErrQuiet()) {
            fprintf(stderr, "Error : Bad color (%d,%d,%d,%d) reset to (0,0,0,255)\n", r, g, b, opacity);
        }
        r = 0;
        g = 0;
        b = 0;
        opacity = 255;
    }
}

GooString *HtmlFontColor::convtoX(unsigned int xcol) const
{
    GooString *xret = new GooString();
    char tmp;
    unsigned int k;
    k = (xcol / 16);
    if (k < 10) {
        tmp = (char)('0' + k);
    } else {
        tmp = (char)('a' + k - 10);
    }
    xret->append(tmp);
    k = (xcol % 16);
    if (k < 10) {
        tmp = (char)('0' + k);
    } else {
        tmp = (char)('a' + k - 10);
    }
    xret->append(tmp);
    return xret;
}

GooString *HtmlFontColor::toString() const
{
    GooString *tmp = new GooString("#");
    GooString *tmpr = convtoX(r);
    GooString *tmpg = convtoX(g);
    GooString *tmpb = convtoX(b);
    tmp->append(tmpr);
    tmp->append(tmpg);
    tmp->append(tmpb);
    delete tmpr;
    delete tmpg;
    delete tmpb;
    return tmp;
}

HtmlFont::HtmlFont(const GfxFont &font, int _size, GfxRGB rgb, double opacity)
{
    color = HtmlFontColor(rgb, opacity);

    lineSize = -1;

    size = _size;
    italic = false;
    bold = false;
    rotOrSkewed = false;

    if (font.isBold() || font.getWeight() >= GfxFont::W700) {
        bold = true;
    }
    if (font.isItalic()) {
        italic = true;
    }

    if (const std::optional<std::string> &fontname = font.getName()) {
        FontName = new GooString(*fontname);

        GooString fontnameLower(*fontname);
        fontnameLower.lowerCase();

        if (!bold && strstr(fontnameLower.c_str(), "bold")) {
            bold = true;
        }

        if (!italic && (strstr(fontnameLower.c_str(), "italic") || strstr(fontnameLower.c_str(), "oblique"))) {
            italic = true;
        }

        familyName = fontname->c_str();
        removeStyleSuffix(familyName);
    } else {
        FontName = new GooString(defaultFamilyName);
        familyName = defaultFamilyName;
    }

    rotSkewMat[0] = rotSkewMat[1] = rotSkewMat[2] = rotSkewMat[3] = 0;
}

HtmlFont::HtmlFont(const HtmlFont &x)
{
    size = x.size;
    lineSize = x.lineSize;
    italic = x.italic;
    bold = x.bold;
    familyName = x.familyName;
    color = x.color;
    FontName = new GooString(x.FontName);
    rotOrSkewed = x.rotOrSkewed;
    memcpy(rotSkewMat, x.rotSkewMat, sizeof(rotSkewMat));
}

HtmlFont::~HtmlFont()
{
    delete FontName;
}

HtmlFont &HtmlFont::operator=(const HtmlFont &x)
{
    if (this == &x) {
        return *this;
    }
    size = x.size;
    lineSize = x.lineSize;
    italic = x.italic;
    bold = x.bold;
    familyName = x.familyName;
    color = x.color;
    delete FontName;
    FontName = new GooString(x.FontName);
    return *this;
}

/*
  This function is used to compare font uniquely for insertion into
  the list of all encountered fonts
*/
bool HtmlFont::isEqual(const HtmlFont &x) const
{
    return (size == x.size) && (lineSize == x.lineSize) && (FontName->cmp(x.FontName) == 0) && (bold == x.bold) && (italic == x.italic) && (color.isEqual(x.getColor())) && isRotOrSkewed() == x.isRotOrSkewed()
            && (!isRotOrSkewed() || rot_matrices_equal(getRotMat(), x.getRotMat()));
}

/*
  This one is used to decide whether two pieces of text can be joined together
  and therefore we don't care about bold/italics properties
*/
bool HtmlFont::isEqualIgnoreBold(const HtmlFont &x) const
{
    return ((size == x.size) && (familyName == x.familyName) && (color.isEqual(x.getColor())));
}

GooString *HtmlFont::getFontName()
{
    return new GooString(familyName);
}

GooString *HtmlFont::getFullName()
{
    return new GooString(FontName);
}

// this method if plain wrong todo
std::unique_ptr<GooString> HtmlFont::HtmlFilter(const Unicode *u, int uLen)
{
    auto tmp = std::make_unique<GooString>();
    const UnicodeMap *uMap;
    char buf[8];
    int n;

    // get the output encoding
    if (!(uMap = globalParams->getTextEncoding())) {
        return tmp;
    }

    for (int i = 0; i < uLen; ++i) {
        // skip control characters.  W3C disallows them and they cause a warning
        // with PHP.
        if (u[i] <= 31 && u[i] != '\t') {
            continue;
        }

        switch (u[i]) {
        case '"':
            tmp->append("&#34;");
            break;
        case '&':
            tmp->append("&amp;");
            break;
        case '<':
            tmp->append("&lt;");
            break;
        case '>':
            tmp->append("&gt;");
            break;
        case ' ':
        case '\t':
            tmp->append(!xml && (i + 1 >= uLen || !tmp->getLength() || tmp->getChar(tmp->getLength() - 1) == ' ') ? "&#160;" : " ");
            break;
        default: {
            // convert unicode to string
            if ((n = uMap->mapUnicode(u[i], buf, sizeof(buf))) > 0) {
                tmp->append(buf, n);
            }
        }
        }
    }

    return tmp;
}

HtmlFontAccu::HtmlFontAccu() { }

HtmlFontAccu::~HtmlFontAccu() { }

int HtmlFontAccu::AddFont(const HtmlFont &font)
{
    std::vector<HtmlFont>::iterator i;
    for (i = accu.begin(); i != accu.end(); ++i) {
        if (font.isEqual(*i)) {
            return (int)(i - (accu.begin()));
        }
    }

    accu.push_back(font);
    return (accu.size() - 1);
}

// get CSS font definition for font #i
GooString *HtmlFontAccu::CSStyle(int i, int j)
{
    GooString *tmp = new GooString();

    std::vector<HtmlFont>::iterator g = accu.begin();
    g += i;
    HtmlFont font = *g;
    GooString *colorStr = font.getColor().toString();
    GooString *fontName = (fontFullName ? font.getFullName() : font.getFontName());

    if (!xml) {
        tmp->append(".ft");
        tmp->append(std::to_string(j));
        tmp->append(std::to_string(i));
        tmp->append("{font-size:");
        tmp->append(std::to_string(font.getSize()));
        if (font.getLineSize() != -1 && font.getLineSize() != 0) {
            tmp->append("px;line-height:");
            tmp->append(std::to_string(font.getLineSize()));
        }
        tmp->append("px;font-family:");
        tmp->append(fontName); // font.getFontName());
        tmp->append(";color:");
        tmp->append(colorStr);
        if (font.getColor().getOpacity() != 1.0) {
            tmp->append(";opacity:");
            tmp->append(std::to_string(font.getColor().getOpacity()));
        }
        // if there is rotation or skew, include the matrix
        if (font.isRotOrSkewed()) {
            const double *const text_mat = font.getRotMat();
            GooString matrix_str(" matrix(");
            matrix_str.appendf("{0:10.10g}, {1:10.10g}, {2:10.10g}, {3:10.10g}, 0, 0)", text_mat[0], text_mat[1], text_mat[2], text_mat[3]);
            tmp->append(";-moz-transform:");
            tmp->append(&matrix_str);
            tmp->append(";-webkit-transform:");
            tmp->append(&matrix_str);
            tmp->append(";-o-transform:");
            tmp->append(&matrix_str);
            tmp->append(";-ms-transform:");
            tmp->append(&matrix_str);
            // Todo: 75% is a wild guess that seems to work pretty well;
            // We probably need to calculate the real percentage
            // Based on the characteristic baseline and bounding box of current font
            // PDF origin is at baseline
            tmp->append(";-moz-transform-origin: left 75%");
            tmp->append(";-webkit-transform-origin: left 75%");
            tmp->append(";-o-transform-origin: left 75%");
            tmp->append(";-ms-transform-origin: left 75%");
        }
        tmp->append(";}");
    }
    if (xml) {
        tmp->append("<fontspec id=\"");
        tmp->append(std::to_string(i));
        tmp->append("\" size=\"");
        tmp->append(std::to_string(font.getSize()));
        tmp->append("\" family=\"");
        tmp->append(fontName);
        tmp->append("\" color=\"");
        tmp->append(colorStr);
        if (font.getColor().getOpacity() != 1.0) {
            tmp->append("\" opacity=\"");
            tmp->append(std::to_string(font.getColor().getOpacity()));
        }
        tmp->append("\"/>");
    }

    delete fontName;
    delete colorStr;
    return tmp;
}
