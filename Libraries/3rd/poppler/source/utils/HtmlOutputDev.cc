//========================================================================
//
// HtmlOutputDev.cc
//
// Copyright 1997-2002 Glyph & Cog, LLC
//
// Changed 1999-2000 by G.Ovtcharov
//
// Changed 2002 by Mikhail Kruk
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005-2013, 2016-2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008 Kjartan Maraas <kmaraas@gnome.org>
// Copyright (C) 2008 Boris Toloknov <tlknv@yandex.ru>
// Copyright (C) 2008 Haruyuki Kawabe <Haruyuki.Kawabe@unisys.co.jp>
// Copyright (C) 2008 Tomas Are Haavet <tomasare@gmail.com>
// Copyright (C) 2009 Warren Toomey <wkt@tuhs.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Reece Dunn <msclrhd@gmail.com>
// Copyright (C) 2010, 2012, 2013, 2022 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2010 OSSD CDAC Mumbai by Leena Chourey (leenac@cdacmumbai.in) and Onkar Potdar (onkar@cdacmumbai.in)
// Copyright (C) 2011 Joshua Richardson <jric@chegg.com>
// Copyright (C) 2011 Stephen Reichling <sreichling@chegg.com>
// Copyright (C) 2011, 2012 Igor Slepchin <igor.slepchin@gmail.com>
// Copyright (C) 2012 Ihar Filipau <thephilips@gmail.com>
// Copyright (C) 2012 Gerald Schmidt <solahcin@gmail.com>
// Copyright (C) 2012 Pino Toscano <pino@kde.org>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Julien Nabet <serval2412@yahoo.fr>
// Copyright (C) 2013 Johannes Brandstätter <jbrandstaetter@gmail.com>
// Copyright (C) 2014 Fabio D'Urso <fabiodurso@hotmail.it>
// Copyright (C) 2016 Vincent Le Garrec <legarrec.vincent@gmail.com>
// Copyright (C) 2017 Caolán McNamara <caolanm@redhat.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Thibaut Brard <thibaut.brard@gmail.com>
// Copyright (C) 2018-2020 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019, 2020, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2020 Eddie Kohler <ekohler@gmail.com>
// Copyright (C) 2021 Christopher Hasse <hasse.christopher@gmail.com>
// Copyright (C) 2022 Brian Rosenfield <brosenfi@yahoo.com>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include "config.h"
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstddef>
#include <cctype>
#include <cmath>
#include <iostream>
#include "goo/GooString.h"
#include "goo/gbasename.h"
#include "goo/gbase64.h"
#include "goo/gbasename.h"
#include "UnicodeMap.h"
#include "goo/gmem.h"
#include "Error.h"
#include "GfxState.h"
#include "Page.h"
#include "Annot.h"
#include "PNGWriter.h"
#include "GlobalParams.h"
#include "HtmlOutputDev.h"
#include "HtmlFonts.h"
#include "HtmlUtils.h"
#include "InMemoryFile.h"
#include "Outline.h"
#include "PDFDoc.h"

#define DEBUG __FILE__ << ": " << __LINE__ << ": DEBUG: "

class HtmlImage
{
public:
    HtmlImage(std::unique_ptr<GooString> &&_fName, GfxState *state) : fName(std::move(_fName))
    {
        state->transform(0, 0, &xMin, &yMax);
        state->transform(1, 1, &xMax, &yMin);
    }
    ~HtmlImage() = default;
    HtmlImage(const HtmlImage &) = delete;
    HtmlImage &operator=(const HtmlImage &) = delete;

    double xMin, xMax; // image x coordinates
    double yMin, yMax; // image y coordinates
    std::unique_ptr<GooString> fName; // image file name
};

// returns true if x is closer to y than x is to z
static inline bool IS_CLOSER(double x, double y, double z)
{
    return std::fabs((x) - (y)) < std::fabs((x) - (z));
}

extern bool complexMode;
extern bool singleHtml;
extern bool dataUrls;
extern bool ignore;
extern bool printCommands;
extern bool printHtml;
extern bool noframes;
extern bool stout;
extern bool xml;
extern bool noRoundedCoordinates;
extern bool showHidden;
extern bool noMerge;

extern double wordBreakThreshold;

static bool debug = false;

#if 0
static GooString* Dirname(GooString* str){
  
  char *p=str->c_str();
  int len=str->getLength();
  for (int i=len-1;i>=0;i--)
    if (*(p+i)==SLASH) 
      return new GooString(p,i+1);
  return new GooString();
}
#endif

static std::unique_ptr<GooString> print_matrix(const double *mat)
{
    return GooString::format("[{0:g} {1:g} {2:g} {3:g} {4:g} {5:g}]", *mat, mat[1], mat[2], mat[3], mat[4], mat[5]);
}

static std::unique_ptr<GooString> print_uni_str(const Unicode *u, const unsigned uLen)
{
    if (!uLen) {
        return std::make_unique<GooString>("");
    }
    std::unique_ptr<GooString> gstr_buff0 = GooString::format("{0:c}", (*u < 0x7F ? *u & 0xFF : '?'));
    for (unsigned i = 1; i < uLen; i++) {
        if (u[i] < 0x7F) {
            gstr_buff0->append(u[i] < 0x7F ? static_cast<char>(u[i]) & 0xFF : '?');
        }
    }

    return gstr_buff0;
}

//------------------------------------------------------------------------
// HtmlString
//------------------------------------------------------------------------

HtmlString::HtmlString(GfxState *state, double fontSize, HtmlFontAccu *_fonts) : fonts(_fonts)
{
    double x, y;

    state->transform(state->getCurX(), state->getCurY(), &x, &y);
    if (std::shared_ptr<const GfxFont> font = state->getFont()) {
        double ascent = font->getAscent();
        double descent = font->getDescent();
        if (ascent > 1.05) {
            // printf( "ascent=%.15g is too high, descent=%.15g\n", ascent, descent );
            ascent = 1.05;
        }
        if (descent < -0.4) {
            // printf( "descent %.15g is too low, ascent=%.15g\n", descent, ascent );
            descent = -0.4;
        }
        yMin = y - ascent * fontSize;
        yMax = y - descent * fontSize;
        GfxRGB rgb;
        state->getFillRGB(&rgb);
        HtmlFont hfont = HtmlFont(*font, std::lround(fontSize), rgb, state->getFillOpacity());
        if (isMatRotOrSkew(state->getTextMat())) {
            double normalizedMatrix[4];
            memcpy(normalizedMatrix, state->getTextMat(), sizeof(normalizedMatrix));
            // browser rotates the opposite way
            // so flip the sign of the angle -> sin() components change sign
            if (debug) {
                std::cerr << DEBUG << "before transform: " << print_matrix(normalizedMatrix)->c_str() << std::endl;
            }
            normalizedMatrix[1] *= -1;
            normalizedMatrix[2] *= -1;
            if (debug) {
                std::cerr << DEBUG << "after reflecting angle: " << print_matrix(normalizedMatrix)->c_str() << std::endl;
            }
            normalizeRotMat(normalizedMatrix);
            if (debug) {
                std::cerr << DEBUG << "after norm: " << print_matrix(normalizedMatrix)->c_str() << std::endl;
            }
            hfont.setRotMat(normalizedMatrix);
        }
        fontpos = fonts->AddFont(hfont);
    } else {
        // this means that the PDF file draws text without a current font,
        // which should never happen
        yMin = y - 0.95 * fontSize;
        yMax = y + 0.35 * fontSize;
        fontpos = 0;
    }
    if (yMin == yMax) {
        // this is a sanity check for a case that shouldn't happen -- but
        // if it does happen, we want to avoid dividing by zero later
        yMin = y;
        yMax = y + 1;
    }
    col = 0;
    text = nullptr;
    xRight = nullptr;
    link = nullptr;
    len = size = 0;
    yxNext = nullptr;
    xyNext = nullptr;
    htext = new GooString();
    dir = textDirUnknown;
}

HtmlString::~HtmlString()
{
    gfree(text);
    delete htext;
    gfree(xRight);
}

void HtmlString::addChar(GfxState *state, double x, double y, double dx, double dy, Unicode u)
{
    if (dir == textDirUnknown) {
        // dir = UnicodeMap::getDirection(u);
        dir = textDirLeftRight;
    }

    if (len == size) {
        size += 16;
        text = (Unicode *)grealloc(text, size * sizeof(Unicode));
        xRight = (double *)grealloc(xRight, size * sizeof(double));
    }
    text[len] = u;
    if (len == 0) {
        xMin = x;
    }
    xMax = xRight[len] = x + dx;
    // printf("added char: %f %f xright = %f\n", x, dx, x+dx);
    ++len;
}

void HtmlString::endString()
{
    if (dir == textDirRightLeft && len > 1) {
        // printf("will reverse!\n");
        for (int i = 0; i < len / 2; i++) {
            Unicode ch = text[i];
            text[i] = text[len - i - 1];
            text[len - i - 1] = ch;
        }
    }
}

//------------------------------------------------------------------------
// HtmlPage
//------------------------------------------------------------------------

HtmlPage::HtmlPage(bool rawOrderA)
{
    rawOrder = rawOrderA;
    curStr = nullptr;
    yxStrings = nullptr;
    xyStrings = nullptr;
    yxCur1 = yxCur2 = nullptr;
    fonts = new HtmlFontAccu();
    links = new HtmlLinks();
    pageWidth = 0;
    pageHeight = 0;
    fontsPageMarker = 0;
    DocName = nullptr;
    firstPage = -1;
}

HtmlPage::~HtmlPage()
{
    clear();
    delete DocName;
    delete fonts;
    delete links;
    for (auto entry : imgList) {
        delete entry;
    }
}

void HtmlPage::updateFont(GfxState *state)
{
    const char *name;
    int code;
    double dimLength;

    // adjust the font size
    fontSize = state->getTransformedFontSize();
    const GfxFont *const font = state->getFont().get();
    if (font && font->getType() == fontType3) {
        // Grab the font size from the font bounding box if possible - remember to
        // scale from the glyph coordinate system.
        const double *fontBBox = font->getFontBBox();
        const double *fontMat = font->getFontMatrix();
        dimLength = (fontBBox[3] - fontBBox[1]) * fontMat[3];
        if (dimLength > 0) {
            fontSize *= dimLength;
        } else {
            // This is a hack which makes it possible to deal with some Type 3
            // fonts.  The problem is that it's impossible to know what the
            // base coordinate system used in the font is without actually
            // rendering the font.  This code tries to guess by looking at the
            // width of the character 'm' (which breaks if the font is a
            // subset that doesn't contain 'm').
            for (code = 0; code < 256; ++code) {
                if ((name = ((Gfx8BitFont *)font)->getCharName(code)) && name[0] == 'm' && name[1] == '\0') {
                    break;
                }
            }
            if (code < 256) {
                dimLength = ((Gfx8BitFont *)font)->getWidth(code);
                if (dimLength != 0) {
                    // 600 is a generic average 'm' width -- yes, this is a hack
                    fontSize *= dimLength / 0.6;
                }
            }
            if (fontMat[0] != 0) {
                fontSize *= fabs(fontMat[3] / fontMat[0]);
            }
        }
    }
}

void HtmlPage::beginString(GfxState *state, const GooString *s)
{
    curStr = new HtmlString(state, fontSize, fonts);
}

void HtmlPage::conv()
{
    for (HtmlString *tmp = yxStrings; tmp; tmp = tmp->yxNext) {
        delete tmp->htext;
        tmp->htext = HtmlFont::HtmlFilter(tmp->text, tmp->len);

        size_t linkIndex = 0;
        if (links->inLink(tmp->xMin, tmp->yMin, tmp->xMax, tmp->yMax, linkIndex)) {
            tmp->link = links->getLink(linkIndex);
        }
    }
}

void HtmlPage::addChar(GfxState *state, double x, double y, double dx, double dy, double ox, double oy, const Unicode *u, int uLen)
{
    double x1, y1, w1, h1, dx2, dy2;
    int n, i;
    state->transform(x, y, &x1, &y1);
    n = curStr->len;

    // check that new character is in the same direction as current string
    // and is not too far away from it before adding
    // if ((UnicodeMap::getDirection(u[0]) != curStr->dir) ||
    // XXX
    if (debug) {
        const double *text_mat = state->getTextMat();
        // rotation is (cos q, sin q, -sin q, cos q, 0, 0)
        // sin q is zero iff there is no rotation, or 180 deg. rotation;
        // for 180 rotation, cos q will be negative
        if (text_mat[0] < 0 || !is_within(text_mat[1], .1, 0)) {
            std::cerr << DEBUG << "rotation matrix for \"" << print_uni_str(u, uLen)->c_str() << '"' << std::endl;
            std::cerr << "text " << print_matrix(state->getTextMat())->c_str();
        }
    }
    if (n > 0 && // don't start a new string, unless there is already a string
                 // TODO: the following line assumes that text is flowing left to
                 // right, which will not necessarily be the case, e.g. if rotated;
                 // It assesses whether or not two characters are close enough to
                 // be part of the same string
        fabs(x1 - curStr->xRight[n - 1]) > wordBreakThreshold * (curStr->yMax - curStr->yMin) &&
        // rotation is (cos q, sin q, -sin q, cos q, 0, 0)
        // sin q is zero iff there is no rotation, or 180 deg. rotation;
        // for 180 rotation, cos q will be negative
        !rot_matrices_equal(curStr->getFont().getRotMat(), state->getTextMat())) {
        endString();
        beginString(state, nullptr);
    }
    state->textTransformDelta(state->getCharSpace() * state->getHorizScaling(), 0, &dx2, &dy2);
    dx -= dx2;
    dy -= dy2;
    state->transformDelta(dx, dy, &w1, &h1);
    if (uLen != 0) {
        w1 /= uLen;
        h1 /= uLen;
    }
    for (i = 0; i < uLen; ++i) {
        curStr->addChar(state, x1 + i * w1, y1 + i * h1, w1, h1, u[i]);
    }
}

void HtmlPage::endString()
{
    HtmlString *p1, *p2;
    double h, y1, y2;

    // throw away zero-length strings -- they don't have valid xMin/xMax
    // values, and they're useless anyway
    if (curStr->len == 0) {
        delete curStr;
        curStr = nullptr;
        return;
    }

    curStr->endString();

#if 0 //~tmp
  if (curStr->yMax - curStr->yMin > 20) {
    delete curStr;
    curStr = NULL;
    return;
  }
#endif

    // insert string in y-major list
    h = curStr->yMax - curStr->yMin;
    y1 = curStr->yMin + 0.5 * h;
    y2 = curStr->yMin + 0.8 * h;
    if (rawOrder) {
        p1 = yxCur1;
        p2 = nullptr;
    } else if ((!yxCur1 || (y1 >= yxCur1->yMin && (y2 >= yxCur1->yMax || curStr->xMax >= yxCur1->xMin))) && (!yxCur2 || (y1 < yxCur2->yMin || (y2 < yxCur2->yMax && curStr->xMax < yxCur2->xMin)))) {
        p1 = yxCur1;
        p2 = yxCur2;
    } else {
        for (p1 = nullptr, p2 = yxStrings; p2; p1 = p2, p2 = p2->yxNext) {
            if (y1 < p2->yMin || (y2 < p2->yMax && curStr->xMax < p2->xMin)) {
                break;
            }
        }
        yxCur2 = p2;
    }
    yxCur1 = curStr;
    if (p1) {
        p1->yxNext = curStr;
    } else {
        yxStrings = curStr;
    }
    curStr->yxNext = p2;
    curStr = nullptr;
}

static const char *strrstr(const char *s, const char *ss)
{
    const char *p = strstr(s, ss);
    for (const char *pp = p; pp != nullptr; pp = strstr(p + 1, ss)) {
        p = pp;
    }
    return p;
}

static void CloseTags(GooString *htext, bool &finish_a, bool &finish_italic, bool &finish_bold)
{
    const char *last_italic = finish_italic && (finish_bold || finish_a) ? strrstr(htext->c_str(), "<i>") : nullptr;
    const char *last_bold = finish_bold && (finish_italic || finish_a) ? strrstr(htext->c_str(), "<b>") : nullptr;
    const char *last_a = finish_a && (finish_italic || finish_bold) ? strrstr(htext->c_str(), "<a ") : nullptr;
    if (finish_a && (finish_italic || finish_bold) && last_a > (last_italic > last_bold ? last_italic : last_bold)) {
        htext->append("</a>", 4);
        finish_a = false;
    }
    if (finish_italic && finish_bold && last_italic > last_bold) {
        htext->append("</i>", 4);
        finish_italic = false;
    }
    if (finish_bold) {
        htext->append("</b>", 4);
    }
    if (finish_italic) {
        htext->append("</i>", 4);
    }
    if (finish_a) {
        htext->append("</a>");
    }
}

// Strings are lines of text;
// This function aims to combine strings into lines and paragraphs if !noMerge
// It may also strip out duplicate strings (if they are on top of each other); sometimes they are to create a font effect
void HtmlPage::coalesce()
{
    HtmlString *str1, *str2;
    double space, horSpace, vertSpace, vertOverlap;
    bool addSpace, addLineBreak;
    int n, i;
    double curX, curY;

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%f..%f  y=%f..%f  size=%2d '",
	   str1->xMin, str1->xMax, str1->yMin, str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    for (i = 0; i < str1->len; ++i) {
      fputc(str1->text[i] & 0xff, stdout);
    }
    printf("'\n");
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
    str1 = yxStrings;

    if (!str1) {
        return;
    }

    //----- discard duplicated text (fake boldface, drop shadows)
    if (!complexMode) { /* if not in complex mode get rid of duplicate strings */
        HtmlString *str3;
        bool found;
        while (str1) {
            double size = str1->yMax - str1->yMin;
            double xLimit = str1->xMin + size;
            found = false;
            for (str2 = str1, str3 = str1->yxNext; str3 && str3->xMin < xLimit; str2 = str3, str3 = str2->yxNext) {
                if (str3->len == str1->len && !memcmp(str3->text, str1->text, str1->len * sizeof(Unicode)) && fabs(str3->yMin - str1->yMin) < size * 0.2 && fabs(str3->yMax - str1->yMax) < size * 0.2
                    && fabs(str3->xMax - str1->xMax) < size * 0.1) {
                    found = true;
                    // printf("found duplicate!\n");
                    break;
                }
            }
            if (found) {
                str2->xyNext = str3->xyNext;
                str2->yxNext = str3->yxNext;
                delete str3;
            } else {
                str1 = str1->yxNext;
            }
        }
    } /*- !complexMode */

    str1 = yxStrings;

    const HtmlFont *hfont1 = getFont(str1);
    if (hfont1->isBold()) {
        str1->htext->insert(0, "<b>", 3);
    }
    if (hfont1->isItalic()) {
        str1->htext->insert(0, "<i>", 3);
    }
    if (str1->getLink() != nullptr) {
        GooString *ls = str1->getLink()->getLinkStart();
        str1->htext->insert(0, ls);
        delete ls;
    }
    curX = str1->xMin;
    curY = str1->yMin;

    while (str1 && (str2 = str1->yxNext)) {
        const HtmlFont *hfont2 = getFont(str2);
        space = str1->yMax - str1->yMin; // the height of the font's bounding box
        horSpace = str2->xMin - str1->xMax;
        // if strings line up on left-hand side AND they are on subsequent lines, we need a line break
        addLineBreak = !noMerge && (fabs(str1->xMin - str2->xMin) < 0.4) && IS_CLOSER(str2->yMax, str1->yMax + space, str1->yMax);
        vertSpace = str2->yMin - str1->yMax;

        // printf("coalesce %d %d %f? ", str1->dir, str2->dir, d);

        if (str2->yMin >= str1->yMin && str2->yMin <= str1->yMax) {
            vertOverlap = str1->yMax - str2->yMin;
        } else if (str2->yMax >= str1->yMin && str2->yMax <= str1->yMax) {
            vertOverlap = str2->yMax - str1->yMin;
        } else {
            vertOverlap = 0;
        }

        // Combine strings if:
        //  They appear to be the same font (complex mode only) && going in the same direction AND at least one of the following:
        //  1.  They appear to be part of the same line of text
        //  2.  They appear to be subsequent lines of a paragraph
        //  We assume (1) or (2) above, respectively, based on:
        //  (1)  strings overlap vertically AND
        //       horizontal space between end of str1 and start of str2 is consistent with a single space or less;
        //       when rawOrder, the strings have to overlap vertically by at least 50%
        //  (2)  Strings flow down the page, but the space between them is not too great, and they are lined up on the left
        if (((((rawOrder && vertOverlap > 0.5 * space) || (!rawOrder && str2->yMin < str1->yMax)) && (horSpace > -0.5 * space && horSpace < space)) || (vertSpace >= 0 && vertSpace < 0.5 * space && addLineBreak))
            && (!complexMode || (hfont1->isEqualIgnoreBold(*hfont2))) && // in complex mode fonts must be the same, in other modes fonts do not metter
            str1->dir == str2->dir // text direction the same
        ) {
            //      printf("yes\n");
            n = str1->len + str2->len;
            if ((addSpace = horSpace > wordBreakThreshold * space)) {
                ++n;
            }
            if (addLineBreak) {
                ++n;
            }

            str1->size = (n + 15) & ~15;
            str1->text = (Unicode *)grealloc(str1->text, str1->size * sizeof(Unicode));
            str1->xRight = (double *)grealloc(str1->xRight, str1->size * sizeof(double));
            if (addSpace) {
                str1->text[str1->len] = 0x20;
                str1->htext->append(xml ? " " : "&#160;");
                str1->xRight[str1->len] = str2->xMin;
                ++str1->len;
            }
            if (addLineBreak) {
                str1->text[str1->len] = '\n';
                str1->htext->append("<br/>");
                str1->xRight[str1->len] = str2->xMin;
                ++str1->len;
                str1->yMin = str2->yMin;
                str1->yMax = str2->yMax;
                str1->xMax = str2->xMax;
                int fontLineSize = hfont1->getLineSize();
                int curLineSize = (int)(vertSpace + space);
                if (curLineSize != fontLineSize) {
                    HtmlFont *newfnt = new HtmlFont(*hfont1);
                    newfnt->setLineSize(curLineSize);
                    str1->fontpos = fonts->AddFont(*newfnt);
                    delete newfnt;
                    hfont1 = getFont(str1);
                    // we have to reget hfont2 because it's location could have
                    // changed on resize
                    hfont2 = getFont(str2);
                }
            }
            for (i = 0; i < str2->len; ++i) {
                str1->text[str1->len] = str2->text[i];
                str1->xRight[str1->len] = str2->xRight[i];
                ++str1->len;
            }

            /* fix <i>, <b> if str1 and str2 differ and handle switch of links */
            const HtmlLink *hlink1 = str1->getLink();
            const HtmlLink *hlink2 = str2->getLink();
            bool switch_links = !hlink1 || !hlink2 || !hlink1->isEqualDest(*hlink2);
            bool finish_a = switch_links && hlink1 != nullptr;
            bool finish_italic = hfont1->isItalic() && (!hfont2->isItalic() || finish_a);
            bool finish_bold = hfont1->isBold() && (!hfont2->isBold() || finish_a || finish_italic);
            CloseTags(str1->htext, finish_a, finish_italic, finish_bold);
            if (switch_links && hlink2 != nullptr) {
                GooString *ls = hlink2->getLinkStart();
                str1->htext->append(ls);
                delete ls;
            }
            if ((!hfont1->isItalic() || finish_italic) && hfont2->isItalic()) {
                str1->htext->append("<i>", 3);
            }
            if ((!hfont1->isBold() || finish_bold) && hfont2->isBold()) {
                str1->htext->append("<b>", 3);
            }

            str1->htext->append(str2->htext);
            // str1 now contains href for link of str2 (if it is defined)
            str1->link = str2->link;
            hfont1 = hfont2;
            if (str2->xMax > str1->xMax) {
                str1->xMax = str2->xMax;
            }
            if (str2->yMax > str1->yMax) {
                str1->yMax = str2->yMax;
            }
            str1->yxNext = str2->yxNext;
            delete str2;
        } else { // keep strings separate
            //      printf("no\n");
            bool finish_a = str1->getLink() != nullptr;
            bool finish_bold = hfont1->isBold();
            bool finish_italic = hfont1->isItalic();
            CloseTags(str1->htext, finish_a, finish_italic, finish_bold);

            str1->xMin = curX;
            str1->yMin = curY;
            str1 = str2;
            curX = str1->xMin;
            curY = str1->yMin;
            hfont1 = hfont2;
            if (hfont1->isBold()) {
                str1->htext->insert(0, "<b>", 3);
            }
            if (hfont1->isItalic()) {
                str1->htext->insert(0, "<i>", 3);
            }
            if (str1->getLink() != nullptr) {
                GooString *ls = str1->getLink()->getLinkStart();
                str1->htext->insert(0, ls);
                delete ls;
            }
        }
    }
    str1->xMin = curX;
    str1->yMin = curY;

    bool finish_bold = hfont1->isBold();
    bool finish_italic = hfont1->isItalic();
    bool finish_a = str1->getLink() != nullptr;
    CloseTags(str1->htext, finish_a, finish_italic, finish_bold);

#if 0 //~ for debugging
  for (str1 = yxStrings; str1; str1 = str1->yxNext) {
    printf("x=%3d..%3d  y=%3d..%3d  size=%2d ",
	   (int)str1->xMin, (int)str1->xMax, (int)str1->yMin, (int)str1->yMax,
	   (int)(str1->yMax - str1->yMin));
    printf("'%s'\n", str1->htext->c_str());  
  }
  printf("\n------------------------------------------------------------\n\n");
#endif
}

void HtmlPage::dumpAsXML(FILE *f, int page)
{
    fprintf(f, "<page number=\"%d\" position=\"absolute\"", page);
    fprintf(f, " top=\"0\" left=\"0\" height=\"%d\" width=\"%d\">\n", pageHeight, pageWidth);

    for (int i = fontsPageMarker; i < fonts->size(); i++) {
        GooString *fontCSStyle = fonts->CSStyle(i);
        fprintf(f, "\t%s\n", fontCSStyle->c_str());
        delete fontCSStyle;
    }

    for (auto ptr : imgList) {
        auto img = static_cast<HtmlImage *>(ptr);
        if (!noRoundedCoordinates) {
            fprintf(f, "<image top=\"%d\" left=\"%d\" ", xoutRound(img->yMin), xoutRound(img->xMin));
            fprintf(f, "width=\"%d\" height=\"%d\" ", xoutRound(img->xMax - img->xMin), xoutRound(img->yMax - img->yMin));
        } else {
            fprintf(f, "<image top=\"%f\" left=\"%f\" ", img->yMin, img->xMin);
            fprintf(f, "width=\"%f\" height=\"%f\" ", img->xMax - img->xMin, img->yMax - img->yMin);
        }
        fprintf(f, "src=\"%s\"/>\n", img->fName->c_str());
        delete img;
    }
    imgList.clear();

    for (HtmlString *tmp = yxStrings; tmp; tmp = tmp->yxNext) {
        if (tmp->htext) {
            if (!noRoundedCoordinates) {
                fprintf(f, "<text top=\"%d\" left=\"%d\" ", xoutRound(tmp->yMin), xoutRound(tmp->xMin));
                fprintf(f, "width=\"%d\" height=\"%d\" ", xoutRound(tmp->xMax - tmp->xMin), xoutRound(tmp->yMax - tmp->yMin));
            } else {
                fprintf(f, "<text top=\"%f\" left=\"%f\" ", tmp->yMin, tmp->xMin);
                fprintf(f, "width=\"%f\" height=\"%f\" ", tmp->xMax - tmp->xMin, tmp->yMax - tmp->yMin);
            }
            fprintf(f, "font=\"%d\">", tmp->fontpos);
            fputs(tmp->htext->c_str(), f);
            fputs("</text>\n", f);
        }
    }
    fputs("</page>\n", f);
}

static void printCSS(FILE *f)
{
    // Image flip/flop CSS
    // Source:
    // http://stackoverflow.com/questions/1309055/cross-browser-way-to-flip-html-image-via-javascript-css
    // tested in Chrome, Fx (Linux) and IE9 (W7)
    static const char css[] = "<style type=\"text/css\">"
                              "\n"
                              "<!--"
                              "\n"
                              ".xflip {"
                              "\n"
                              "    -moz-transform: scaleX(-1);"
                              "\n"
                              "    -webkit-transform: scaleX(-1);"
                              "\n"
                              "    -o-transform: scaleX(-1);"
                              "\n"
                              "    transform: scaleX(-1);"
                              "\n"
                              "    filter: fliph;"
                              "\n"
                              "}"
                              "\n"
                              ".yflip {"
                              "\n"
                              "    -moz-transform: scaleY(-1);"
                              "\n"
                              "    -webkit-transform: scaleY(-1);"
                              "\n"
                              "    -o-transform: scaleY(-1);"
                              "\n"
                              "    transform: scaleY(-1);"
                              "\n"
                              "    filter: flipv;"
                              "\n"
                              "}"
                              "\n"
                              ".xyflip {"
                              "\n"
                              "    -moz-transform: scaleX(-1) scaleY(-1);"
                              "\n"
                              "    -webkit-transform: scaleX(-1) scaleY(-1);"
                              "\n"
                              "    -o-transform: scaleX(-1) scaleY(-1);"
                              "\n"
                              "    transform: scaleX(-1) scaleY(-1);"
                              "\n"
                              "    filter: fliph + flipv;"
                              "\n"
                              "}"
                              "\n"
                              "-->"
                              "\n"
                              "</style>"
                              "\n";

    fwrite(css, sizeof(css) - 1, 1, f);
}

int HtmlPage::dumpComplexHeaders(FILE *const file, FILE *&pageFile, int page)
{

    if (!noframes) {
        const std::string pgNum = std::to_string(page);
        std::string pageFileName(DocName->toStr());
        if (!singleHtml) {
            pageFileName += '-' + pgNum + ".html";
            pageFile = fopen(pageFileName.c_str(), "w");
        } else {
            pageFileName += "-html.html";
            pageFile = fopen(pageFileName.c_str(), "a");
        }

        if (!pageFile) {
            error(errIO, -1, "Couldn't open html file '{0:s}'", pageFileName.c_str());
            return 1;
        }

        if (!singleHtml) {
            fprintf(pageFile, "%s\n<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"\" xml:lang=\"\">\n<head>\n<title>Page %d</title>\n\n", DOCTYPE, page);
        } else {
            fprintf(pageFile, "%s\n<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"\" xml:lang=\"\">\n<head>\n<title>%s</title>\n\n", DOCTYPE, pageFileName.c_str());
        }

        const std::string htmlEncoding = HtmlOutputDev::mapEncodingToHtml(globalParams->getTextEncodingName());
        if (!singleHtml) {
            fprintf(pageFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>\n", htmlEncoding.c_str());
        } else {
            fprintf(pageFile, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>\n <br/>\n", htmlEncoding.c_str());
        }
    } else {
        pageFile = file;
        fprintf(pageFile, "<!-- Page %d -->\n", page);
        fprintf(pageFile, "<a name=\"%d\"></a>\n", page);
    }

    return 0;
}

void HtmlPage::dumpComplex(FILE *file, int page, const std::vector<std::string> &backgroundImages)
{
    FILE *pageFile;

    if (firstPage == -1) {
        firstPage = page;
    }

    if (dumpComplexHeaders(file, pageFile, page)) {
        error(errIO, -1, "Couldn't write headers.");
        return;
    }

    fputs("<style type=\"text/css\">\n<!--\n", pageFile);
    fputs("\tp {margin: 0; padding: 0;}", pageFile);
    for (int i = fontsPageMarker; i != fonts->size(); i++) {
        GooString *fontCSStyle;
        if (!singleHtml) {
            fontCSStyle = fonts->CSStyle(i);
        } else {
            fontCSStyle = fonts->CSStyle(i, page);
        }
        fprintf(pageFile, "\t%s\n", fontCSStyle->c_str());
        delete fontCSStyle;
    }

    fputs("-->\n</style>\n", pageFile);

    if (!noframes) {
        fputs("</head>\n<body bgcolor=\"#A0A0A0\" vlink=\"blue\" link=\"blue\">\n", pageFile);
    }

    fprintf(pageFile, "<div id=\"page%d-div\" style=\"position:relative;width:%dpx;height:%dpx;\">\n", page, pageWidth, pageHeight);

    if (!ignore && (size_t)(page - firstPage) < backgroundImages.size()) {
        fprintf(pageFile, "<img width=\"%d\" height=\"%d\" src=\"%s\" alt=\"background image\"/>\n", pageWidth, pageHeight, backgroundImages[page - firstPage].c_str());
    }

    for (HtmlString *tmp1 = yxStrings; tmp1; tmp1 = tmp1->yxNext) {
        if (tmp1->htext) {
            fprintf(pageFile, "<p style=\"position:absolute;top:%dpx;left:%dpx;white-space:nowrap\" class=\"ft", xoutRound(tmp1->yMin), xoutRound(tmp1->xMin));
            if (!singleHtml) {
                fputc('0', pageFile);
            } else {
                fprintf(pageFile, "%d", page);
            }
            fprintf(pageFile, "%d\">", tmp1->fontpos);
            fputs(tmp1->htext->c_str(), pageFile);
            fputs("</p>\n", pageFile);
        }
    }

    fputs("</div>\n", pageFile);

    if (!noframes) {
        fputs("</body>\n</html>\n", pageFile);
        fclose(pageFile);
    }
}

void HtmlPage::dump(FILE *f, int pageNum, const std::vector<std::string> &backgroundImages)
{
    if (complexMode || singleHtml) {
        if (xml) {
            dumpAsXML(f, pageNum);
        }
        if (!xml) {
            dumpComplex(f, pageNum, backgroundImages);
        }
    } else {
        fprintf(f, "<a name=%d></a>", pageNum);
        // Loop over the list of image names on this page
        for (auto ptr : imgList) {
            auto img = static_cast<HtmlImage *>(ptr);

            // see printCSS() for class names
            const char *styles[4] = { "", " class=\"xflip\"", " class=\"yflip\"", " class=\"xyflip\"" };
            int style_index = 0;
            if (img->xMin > img->xMax) {
                style_index += 1; // xFlip
            }
            if (img->yMin > img->yMax) {
                style_index += 2; // yFlip
            }

            fprintf(f, "<img%s src=\"%s\"/><br/>\n", styles[style_index], img->fName->c_str());
            delete img;
        }
        imgList.clear();

        for (HtmlString *tmp = yxStrings; tmp; tmp = tmp->yxNext) {
            if (tmp->htext) {
                fputs(tmp->htext->c_str(), f);
                fputs("<br/>\n", f);
            }
        }
        fputs("<hr/>\n", f);
    }
}

void HtmlPage::clear()
{
    HtmlString *p1, *p2;

    if (curStr) {
        delete curStr;
        curStr = nullptr;
    }
    for (p1 = yxStrings; p1; p1 = p2) {
        p2 = p1->yxNext;
        delete p1;
    }
    yxStrings = nullptr;
    xyStrings = nullptr;
    yxCur1 = yxCur2 = nullptr;

    if (!noframes) {
        delete fonts;
        fonts = new HtmlFontAccu();
        fontsPageMarker = 0;
    } else {
        fontsPageMarker = fonts->size();
    }

    delete links;
    links = new HtmlLinks();
}

void HtmlPage::setDocName(const char *fname)
{
    DocName = new GooString(fname);
}

void HtmlPage::addImage(std::unique_ptr<GooString> &&fname, GfxState *state)
{
    HtmlImage *img = new HtmlImage(std::move(fname), state);
    imgList.push_back(img);
}

//------------------------------------------------------------------------
// HtmlMetaVar
//------------------------------------------------------------------------

HtmlMetaVar::HtmlMetaVar(const char *_name, const char *_content)
{
    name = new GooString(_name);
    content = new GooString(_content);
}

HtmlMetaVar::~HtmlMetaVar()
{
    delete name;
    delete content;
}

GooString *HtmlMetaVar::toString() const
{
    GooString *result = new GooString("<meta name=\"");
    result->append(name);
    result->append("\" content=\"");
    result->append(content);
    result->append("\"/>");
    return result;
}

//------------------------------------------------------------------------
// HtmlOutputDev
//------------------------------------------------------------------------

static const char *HtmlEncodings[][2] = { { "Latin1", "ISO-8859-1" }, { nullptr, nullptr } };

std::string HtmlOutputDev::mapEncodingToHtml(const std::string &encoding)
{
    for (int i = 0; HtmlEncodings[i][0] != nullptr; i++) {
        if (encoding == HtmlEncodings[i][0]) {
            return HtmlEncodings[i][1];
        }
    }
    return encoding;
}

void HtmlOutputDev::doFrame(int firstPage)
{
    GooString *fName = new GooString(Docname);
    fName->append(".html");

    if (!(fContentsFrame = fopen(fName->c_str(), "w"))) {
        error(errIO, -1, "Couldn't open html file '{0:t}'", fName);
        delete fName;
        return;
    }

    delete fName;

    const std::string baseName = gbasename(Docname->c_str());
    fputs(DOCTYPE, fContentsFrame);
    fputs("\n<html>", fContentsFrame);
    fputs("\n<head>", fContentsFrame);
    fprintf(fContentsFrame, "\n<title>%s</title>", docTitle->c_str());
    const std::string htmlEncoding = mapEncodingToHtml(globalParams->getTextEncodingName());
    fprintf(fContentsFrame, "\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>\n", htmlEncoding.c_str());
    dumpMetaVars(fContentsFrame);
    fprintf(fContentsFrame, "</head>\n");
    fputs("<frameset cols=\"100,*\">\n", fContentsFrame);
    fprintf(fContentsFrame, "<frame name=\"links\" src=\"%s_ind.html\"/>\n", baseName.c_str());
    fputs("<frame name=\"contents\" src=", fContentsFrame);
    if (complexMode) {
        fprintf(fContentsFrame, "\"%s-%d.html\"", baseName.c_str(), firstPage);
    } else {
        fprintf(fContentsFrame, "\"%ss.html\"", baseName.c_str());
    }

    fputs("/>\n</frameset>\n</html>\n", fContentsFrame);

    fclose(fContentsFrame);
}

HtmlOutputDev::HtmlOutputDev(Catalog *catalogA, const char *fileName, const char *title, const char *author, const char *keywords, const char *subject, const char *date, bool rawOrderA, int firstPage, bool outline)
{
    catalog = catalogA;
    fContentsFrame = nullptr;
    page = nullptr;
    docTitle = new GooString(title);
    pages = nullptr;
    dumpJPEG = true;
    // write = true;
    rawOrder = rawOrderA;
    this->doOutline = outline;
    ok = false;
    // this->firstPage = firstPage;
    // pageNum=firstPage;
    // open file
    needClose = false;
    pages = new HtmlPage(rawOrder);

    glMetaVars.push_back(new HtmlMetaVar("generator", "pdftohtml 0.36"));
    if (author) {
        glMetaVars.push_back(new HtmlMetaVar("author", author));
    }
    if (keywords) {
        glMetaVars.push_back(new HtmlMetaVar("keywords", keywords));
    }
    if (date) {
        glMetaVars.push_back(new HtmlMetaVar("date", date));
    }
    if (subject) {
        glMetaVars.push_back(new HtmlMetaVar("subject", subject));
    }

    maxPageWidth = 0;
    maxPageHeight = 0;

    pages->setDocName(fileName);
    Docname = new GooString(fileName);

    // for non-xml output (complex or simple) with frames generate the left frame
    if (!xml && !noframes) {
        if (!singleHtml) {
            GooString *left = new GooString(fileName);
            left->append("_ind.html");

            doFrame(firstPage);

            if (!(fContentsFrame = fopen(left->c_str(), "w"))) {
                error(errIO, -1, "Couldn't open html file '{0:t}'", left);
                delete left;
                return;
            }
            delete left;
            fputs(DOCTYPE, fContentsFrame);
            fputs("<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"\" xml:lang=\"\">\n<head>\n<title></title>\n</head>\n<body>\n", fContentsFrame);

            if (doOutline) {
                fprintf(fContentsFrame, "<a href=\"%s%s\" target=\"contents\">Outline</a><br/>", gbasename(Docname->c_str()).c_str(), complexMode ? "-outline.html" : "s.html#outline");
            }
        }
        if (!complexMode) { /* not in complex mode */

            GooString *right = new GooString(fileName);
            right->append("s.html");

            if (!(page = fopen(right->c_str(), "w"))) {
                error(errIO, -1, "Couldn't open html file '{0:t}'", right);
                delete right;
                return;
            }
            delete right;
            fputs(DOCTYPE, page);
            fputs("<html>\n<head>\n<title></title>\n", page);
            printCSS(page);
            fputs("</head>\n<body>\n", page);
        }
    }

    if (noframes) {
        if (stout) {
            page = stdout;
        } else {
            GooString *right = new GooString(fileName);
            if (!xml) {
                right->append(".html");
            }
            if (xml) {
                right->append(".xml");
            }
            if (!(page = fopen(right->c_str(), "w"))) {
                error(errIO, -1, "Couldn't open html file '{0:t}'", right);
                delete right;
                return;
            }
            delete right;
        }

        const std::string htmlEncoding = mapEncodingToHtml(globalParams->getTextEncodingName());
        if (xml) {
            fprintf(page, "<?xml version=\"1.0\" encoding=\"%s\"?>\n", htmlEncoding.c_str());
            fputs("<!DOCTYPE pdf2xml SYSTEM \"pdf2xml.dtd\">\n\n", page);
            fprintf(page, "<pdf2xml producer=\"%s\" version=\"%s\">\n", PACKAGE_NAME, PACKAGE_VERSION);
        } else {
            fprintf(page, "%s\n<html xmlns=\"http://www.w3.org/1999/xhtml\" lang=\"\" xml:lang=\"\">\n<head>\n<title>%s</title>\n", DOCTYPE, docTitle->c_str());

            fprintf(page, "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\"/>\n", htmlEncoding.c_str());

            dumpMetaVars(page);
            printCSS(page);
            fprintf(page, "</head>\n");
            fprintf(page, "<body bgcolor=\"#A0A0A0\" vlink=\"blue\" link=\"blue\">\n");
        }
    }
    ok = true;
}

HtmlOutputDev::~HtmlOutputDev()
{
    delete Docname;
    delete docTitle;

    for (auto entry : glMetaVars) {
        delete entry;
    }

    if (fContentsFrame) {
        fputs("</body>\n</html>\n", fContentsFrame);
        fclose(fContentsFrame);
    }
    if (page != nullptr) {
        if (xml) {
            fputs("</pdf2xml>\n", page);
            fclose(page);
        } else if (!complexMode || xml || noframes) {
            fputs("</body>\n</html>\n", page);
            fclose(page);
        }
    }
    if (pages) {
        delete pages;
    }
}

void HtmlOutputDev::startPage(int pageNumA, GfxState *state, XRef *xref)
{
#if 0
  if (mode&&!xml){
    if (write){
      write=false;
      GooString* fname=Dirname(Docname);
      fname->append("image.log");
      if((tin=fopen(getFileNameFromPath(fname->c_str(),fname->getLength()),"w"))==NULL){
	printf("Error : can not open %s",fname);
	exit(1);
      }
      delete fname;
    // if(state->getRotation()!=0) 
    //  fprintf(tin,"ROTATE=%d rotate %d neg %d neg translate\n",state->getRotation(),state->getX1(),-state->getY1());
    // else 
      fprintf(tin,"ROTATE=%d neg %d neg translate\n",state->getX1(),state->getY1());  
    }
  }
#endif

    pageNum = pageNumA;
    const std::string str = gbasename(Docname->c_str());
    pages->clear();
    if (!noframes) {
        if (fContentsFrame) {
            if (complexMode) {
                fprintf(fContentsFrame, "<a href=\"%s-%d.html\"", str.c_str(), pageNum);
            } else {
                fprintf(fContentsFrame, "<a href=\"%ss.html#%d\"", str.c_str(), pageNum);
            }
            fprintf(fContentsFrame, " target=\"contents\" >Page %d</a><br/>\n", pageNum);
        }
    }

    pages->pageWidth = static_cast<int>(state->getPageWidth());
    pages->pageHeight = static_cast<int>(state->getPageHeight());
}

void HtmlOutputDev::endPage()
{
    std::unique_ptr<Links> linksList = docPage->getLinks();
    for (AnnotLink *link : linksList->getLinks()) {
        doProcessLink(link);
    }

    pages->conv();
    pages->coalesce();
    pages->dump(page, pageNum, backgroundImages);

    // I don't yet know what to do in the case when there are pages of different
    // sizes and we want complex output: running ghostscript many times
    // seems very inefficient. So for now I'll just use last page's size
    maxPageWidth = pages->pageWidth;
    maxPageHeight = pages->pageHeight;

    // if(!noframes&&!xml) fputs("<br/>\n", fContentsFrame);
    if (!stout && !globalParams->getErrQuiet()) {
        printf("Page-%d\n", (pageNum));
    }
}

void HtmlOutputDev::addBackgroundImage(const std::string &img)
{
    backgroundImages.push_back(img);
}

void HtmlOutputDev::updateFont(GfxState *state)
{
    pages->updateFont(state);
}

void HtmlOutputDev::beginString(GfxState *state, const GooString *s)
{
    pages->beginString(state, s);
}

void HtmlOutputDev::endString(GfxState *state)
{
    pages->endString();
}

void HtmlOutputDev::drawChar(GfxState *state, double x, double y, double dx, double dy, double originX, double originY, CharCode code, int /*nBytes*/, const Unicode *u, int uLen)
{
    if (!showHidden && (state->getRender() & 3) == 3) {
        return;
    }
    pages->addChar(state, x, y, dx, dy, originX, originY, u, uLen);
}

void HtmlOutputDev::drawJpegImage(GfxState *state, Stream *str)
{
    InMemoryFile ims;
    FILE *f1 = nullptr;
    int c;

    // open the image file
    std::unique_ptr<GooString> fName = createImageFileName("jpg");
    f1 = dataUrls ? ims.open("wb") : fopen(fName->c_str(), "wb");
    if (!f1) {
        error(errIO, -1, "Couldn't open image file '{0:t}'", fName.get());
        return;
    }

    // initialize stream
    str = str->getNextStream();
    str->reset();

    // copy the stream
    while ((c = str->getChar()) != EOF) {
        fputc(c, f1);
    }

    fclose(f1);

    if (dataUrls) {
        fName = std::make_unique<GooString>(std::string("data:image/jpeg;base64,") + gbase64Encode(ims.getBuffer()));
    }
    pages->addImage(std::move(fName), state);
}

void HtmlOutputDev::drawPngImage(GfxState *state, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool isMask)
{
#ifdef ENABLE_LIBPNG
    FILE *f1;
    InMemoryFile ims;

    if (!colorMap && !isMask) {
        error(errInternal, -1, "Can't have color image without a color map");
        return;
    }

    // open the image file
    std::unique_ptr<GooString> fName = createImageFileName("png");
    f1 = dataUrls ? ims.open("wb") : fopen(fName->c_str(), "wb");
    if (!f1) {
        error(errIO, -1, "Couldn't open image file '{0:t}'", fName.get());
        return;
    }

    PNGWriter *writer = new PNGWriter(isMask ? PNGWriter::MONOCHROME : PNGWriter::RGB);
    // TODO can we calculate the resolution of the image?
    if (!writer->init(f1, width, height, 72, 72)) {
        error(errInternal, -1, "Can't init PNG for image '{0:t}'", fName.get());
        delete writer;
        fclose(f1);
        return;
    }

    if (!isMask) {
        unsigned char *p;
        GfxRGB rgb;
        unsigned char *row = (unsigned char *)gmalloc(3 * width); // 3 bytes/pixel: RGB
        unsigned char **row_pointer = &row;

        // Initialize the image stream
        ImageStream *imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(), colorMap->getBits());
        imgStr->reset();

        // For each line...
        for (int y = 0; y < height; y++) {

            // Convert into a PNG row
            p = imgStr->getLine();
            if (!p) {
                error(errIO, -1, "Failed to read PNG. '{0:t}' will be incorrect", fName.get());
                gfree(row);
                delete writer;
                delete imgStr;
                fclose(f1);
                return;
            }
            for (int x = 0; x < width; x++) {
                colorMap->getRGB(p, &rgb);
                // Write the RGB pixels into the row
                row[3 * x] = colToByte(rgb.r);
                row[3 * x + 1] = colToByte(rgb.g);
                row[3 * x + 2] = colToByte(rgb.b);
                p += colorMap->getNumPixelComps();
            }

            if (!writer->writeRow(row_pointer)) {
                error(errIO, -1, "Failed to write into PNG '{0:t}'", fName.get());
                delete writer;
                delete imgStr;
                fclose(f1);
                return;
            }
        }
        gfree(row);
        imgStr->close();
        delete imgStr;
    } else { // isMask == true
        int size = (width + 7) / 8;

        // PDF masks use 0 = draw current color, 1 = leave unchanged.
        // We invert this to provide the standard interpretation of alpha
        // (0 = transparent, 1 = opaque). If the colorMap already inverts
        // the mask we leave the data unchanged.
        int invert_bits = 0xff;
        if (colorMap) {
            GfxGray gray;
            unsigned char zero[gfxColorMaxComps];
            memset(zero, 0, sizeof(zero));
            colorMap->getGray(zero, &gray);
            if (colToByte(gray) == 0) {
                invert_bits = 0x00;
            }
        }

        str->reset();
        unsigned char *png_row = (unsigned char *)gmalloc(size);

        for (int ri = 0; ri < height; ++ri) {
            for (int i = 0; i < size; i++) {
                png_row[i] = str->getChar() ^ invert_bits;
            }

            if (!writer->writeRow(&png_row)) {
                error(errIO, -1, "Failed to write into PNG '{0:t}'", fName.get());
                delete writer;
                fclose(f1);
                gfree(png_row);
                return;
            }
        }
        str->close();
        gfree(png_row);
    }

    str->close();

    writer->close();
    delete writer;
    fclose(f1);

    if (dataUrls) {
        fName = std::make_unique<GooString>(std::string("data:image/png;base64,") + gbase64Encode(ims.getBuffer()));
    }
    pages->addImage(std::move(fName), state);
#else
    return;
#endif
}

std::unique_ptr<GooString> HtmlOutputDev::createImageFileName(const char *ext)
{
    return GooString::format("{0:s}-{1:d}_{2:d}.{3:s}", Docname->c_str(), pageNum, pages->getNumImages() + 1, ext);
}

void HtmlOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg)
{

    if (ignore || (complexMode && !xml)) {
        OutputDev::drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
        return;
    }

    // dump JPEG file
    if (dumpJPEG && str->getKind() == strDCT) {
        drawJpegImage(state, str);
    } else {
#ifdef ENABLE_LIBPNG
        drawPngImage(state, str, width, height, nullptr, true);
#else
        OutputDev::drawImageMask(state, ref, str, width, height, invert, interpolate, inlineImg);
#endif
    }
}

void HtmlOutputDev::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg)
{

    if (ignore || (complexMode && !xml)) {
        OutputDev::drawImage(state, ref, str, width, height, colorMap, interpolate, maskColors, inlineImg);
        return;
    }

    /*if( !globalParams->getErrQuiet() )
      printf("image stream of kind %d\n", str->getKind());*/
    // dump JPEG file
    if (dumpJPEG && str->getKind() == strDCT && (colorMap->getNumPixelComps() == 1 || colorMap->getNumPixelComps() == 3) && !inlineImg) {
        drawJpegImage(state, str);
    } else {
#ifdef ENABLE_LIBPNG
        drawPngImage(state, str, width, height, colorMap);
#else
        OutputDev::drawImage(state, ref, str, width, height, colorMap, interpolate, maskColors, inlineImg);
#endif
    }
}

void HtmlOutputDev::doProcessLink(AnnotLink *link)
{
    double _x1, _y1, _x2, _y2;
    int x1, y1, x2, y2;

    link->getRect(&_x1, &_y1, &_x2, &_y2);
    cvtUserToDev(_x1, _y1, &x1, &y1);

    cvtUserToDev(_x2, _y2, &x2, &y2);

    GooString *_dest = getLinkDest(link);
    HtmlLink t((double)x1, (double)y2, (double)x2, (double)y1, _dest);
    pages->AddLink(t);
    delete _dest;
}

GooString *HtmlOutputDev::getLinkDest(AnnotLink *link)
{
    if (!link->getAction()) {
        return new GooString();
    }
    switch (link->getAction()->getKind()) {
    case actionGoTo: {
        int destPage = 1;
        LinkGoTo *ha = (LinkGoTo *)link->getAction();
        std::unique_ptr<LinkDest> dest;
        if (ha->getDest() != nullptr) {
            dest = std::make_unique<LinkDest>(*ha->getDest());
        } else if (ha->getNamedDest() != nullptr) {
            dest = catalog->findDest(ha->getNamedDest());
        }

        if (dest) {
            GooString *file = new GooString(gbasename(Docname->c_str()));

            if (dest->isPageRef()) {
                const Ref pageref = dest->getPageRef();
                destPage = catalog->findPage(pageref);
            } else {
                destPage = dest->getPageNum();
            }

            /* 		complex 	simple
              frames		file-4.html	files.html#4
              noframes	file.html#4	file.html#4
             */
            if (noframes) {
                file->append(".html#");
                file->append(std::to_string(destPage));
            } else {
                if (complexMode) {
                    file->append("-");
                    file->append(std::to_string(destPage));
                    file->append(".html");
                } else {
                    file->append("s.html#");
                    file->append(std::to_string(destPage));
                }
            }

            if (printCommands) {
                printf(" link to page %d ", destPage);
            }
            return file;
        } else {
            return new GooString();
        }
    }
    case actionGoToR: {
        LinkGoToR *ha = (LinkGoToR *)link->getAction();
        LinkDest *dest = nullptr;
        int destPage = 1;
        GooString *file = new GooString();
        if (ha->getFileName()) {
            delete file;
            file = new GooString(ha->getFileName()->c_str());
        }
        if (ha->getDest() != nullptr) {
            dest = new LinkDest(*ha->getDest());
        }
        if (dest && file) {
            if (!(dest->isPageRef())) {
                destPage = dest->getPageNum();
            }
            delete dest;

            if (printCommands) {
                printf(" link to page %d ", destPage);
            }
            if (printHtml) {
                const char *p = file->c_str() + file->getLength() - 4;
                if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
                    file->del(file->getLength() - 4, 4);
                    file->append(".html");
                }
                file->append('#');
                file->append(std::to_string(destPage));
            }
        }
        if (printCommands && file) {
            printf("filename %s\n", file->c_str());
        }
        return file;
    }
    case actionURI: {
        LinkURI *ha = (LinkURI *)link->getAction();
        GooString *file = new GooString(ha->getURI());
        // printf("uri : %s\n",file->c_str());
        return file;
    }
    case actionLaunch:
        if (printHtml) {
            LinkLaunch *ha = (LinkLaunch *)link->getAction();
            GooString *file = new GooString(ha->getFileName()->c_str());
            const char *p = file->c_str() + file->getLength() - 4;
            if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
                file->del(file->getLength() - 4, 4);
                file->append(".html");
            }
            if (printCommands) {
                printf("filename %s", file->c_str());
            }

            return file;
        }
        // fallthrough
    default:
        return new GooString();
    }
}

void HtmlOutputDev::dumpMetaVars(FILE *file)
{
    GooString *var;

    for (const HtmlMetaVar *t : glMetaVars) {
        var = t->toString();
        fprintf(file, "%s\n", var->c_str());
        delete var;
    }
}

bool HtmlOutputDev::dumpDocOutline(PDFDoc *doc)
{
    FILE *output = nullptr;
    bool bClose = false;

    if (!ok) {
        return false;
    }

    Outline *outline = doc->getOutline();
    if (!outline) {
        return false;
    }

    const std::vector<OutlineItem *> *outlines = outline->getItems();
    if (!outlines) {
        return false;
    }

    if (!complexMode || xml) {
        output = page;
    } else if (complexMode && !xml) {
        if (noframes) {
            output = page;
            fputs("<hr/>\n", output);
        } else {
            GooString *str = Docname->copy();
            str->append("-outline.html");
            output = fopen(str->c_str(), "w");
            delete str;
            if (output == nullptr) {
                return false;
            }
            bClose = true;

            const std::string htmlEncoding = HtmlOutputDev::mapEncodingToHtml(globalParams->getTextEncodingName());

            fprintf(output,
                    "<html xmlns=\"http://www.w3.org/1999/xhtml\" "
                    "lang=\"\" xml:lang=\"\">\n"
                    "<head>\n"
                    "<title>Document Outline</title>\n"
                    "<meta http-equiv=\"Content-Type\" content=\"text/html; "
                    "charset=%s\"/>\n"
                    "</head>\n<body>\n",
                    htmlEncoding.c_str());
        }
    }

    if (!xml) {
        bool done = newHtmlOutlineLevel(output, outlines);
        if (done && !complexMode) {
            fputs("<hr/>\n", output);
        }

        if (bClose) {
            fputs("</body>\n</html>\n", output);
            fclose(output);
        }
    } else {
        newXmlOutlineLevel(output, outlines);
    }

    return true;
}

bool HtmlOutputDev::newHtmlOutlineLevel(FILE *output, const std::vector<OutlineItem *> *outlines, int level)
{
    bool atLeastOne = false;

    if (level == 1) {
        fputs("<a name=\"outline\"></a>", output);
        fputs("<h1>Document Outline</h1>\n", output);
    }
    fputs("<ul>\n", output);

    for (OutlineItem *item : *outlines) {
        GooString *titleStr = HtmlFont::HtmlFilter(item->getTitle(), item->getTitleLength());

        GooString *linkName = nullptr;
        ;
        const int itemPage = getOutlinePageNum(item);
        if (itemPage > 0) {
            /*		complex		simple
            frames		file-4.html	files.html#4
            noframes	file.html#4	file.html#4
            */
            linkName = new GooString(gbasename(Docname->c_str()));
            if (noframes) {
                linkName->append(".html#");
                linkName->append(std::to_string(itemPage));
            } else {
                if (complexMode) {
                    linkName->append("-");
                    linkName->append(std::to_string(itemPage));
                    linkName->append(".html");
                } else {
                    linkName->append("s.html#");
                    linkName->append(std::to_string(itemPage));
                }
            }
        }

        fputs("<li>", output);
        if (linkName) {
            fprintf(output, "<a href=\"%s\">", linkName->c_str());
        }
        fputs(titleStr->c_str(), output);
        if (linkName) {
            fputs("</a>", output);
            delete linkName;
        }
        delete titleStr;
        atLeastOne = true;

        item->open();
        if (item->hasKids() && item->getKids()) {
            fputs("\n", output);
            newHtmlOutlineLevel(output, item->getKids(), level + 1);
        }
        fputs("</li>\n", output);
    }
    fputs("</ul>\n", output);

    return atLeastOne;
}

void HtmlOutputDev::newXmlOutlineLevel(FILE *output, const std::vector<OutlineItem *> *outlines)
{
    fputs("<outline>\n", output);

    for (OutlineItem *item : *outlines) {
        GooString *titleStr = HtmlFont::HtmlFilter(item->getTitle(), item->getTitleLength());
        const int itemPage = getOutlinePageNum(item);
        if (itemPage > 0) {
            fprintf(output, "<item page=\"%d\">%s</item>\n", itemPage, titleStr->c_str());
        } else {
            fprintf(output, "<item>%s</item>\n", titleStr->c_str());
        }
        delete titleStr;

        item->open();
        if (item->hasKids() && item->getKids()) {
            newXmlOutlineLevel(output, item->getKids());
        }
    }

    fputs("</outline>\n", output);
}

int HtmlOutputDev::getOutlinePageNum(OutlineItem *item)
{
    const LinkAction *action = item->getAction();
    const LinkGoTo *link = nullptr;
    std::unique_ptr<LinkDest> linkdest;
    int pagenum = -1;

    if (!action || action->getKind() != actionGoTo) {
        return pagenum;
    }

    link = static_cast<const LinkGoTo *>(action);

    if (!link || !link->isOk()) {
        return pagenum;
    }

    if (link->getDest()) {
        linkdest = std::make_unique<LinkDest>(*link->getDest());
    } else if (link->getNamedDest()) {
        linkdest = catalog->findDest(link->getNamedDest());
    }

    if (!linkdest) {
        return pagenum;
    }

    if (linkdest->isPageRef()) {
        const Ref pageref = linkdest->getPageRef();
        pagenum = catalog->findPage(pageref);
    } else {
        pagenum = linkdest->getPageNum();
    }

    return pagenum;
}
