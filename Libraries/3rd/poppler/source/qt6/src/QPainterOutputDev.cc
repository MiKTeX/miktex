//========================================================================
//
// QPainterOutputDev.cc
//
// Copyright 2003 Glyph & Cog, LLC
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005 Brad Hards <bradh@frogmouth.net>
// Copyright (C) 2005-2009, 2011, 2012, 2014, 2015, 2018, 2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2008, 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2009 Petr Gajdos <pgajdos@novell.com>
// Copyright (C) 2010 Matthias Fauconneau <matthias.fauconneau@gmail.com>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Dominik Haumann <dhaumann@kde.org>
// Copyright (C) 2013 Mihai Niculescu <q.quark@gmail.com>
// Copyright (C) 2017, 2018, 2020-2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2017, 2022 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstring>
#include <cmath>

#include <array>

#include "goo/ft_utils.h"
#include "goo/gfile.h"
#include "GlobalParams.h"
#include "Error.h"
#include "Object.h"
#include "GfxState.h"
#include "GfxFont.h"
#include "Link.h"
#include "FontEncodingTables.h"
#include <fofi/FoFiTrueType.h>
#include <fofi/FoFiType1C.h>
#include "QPainterOutputDev.h"
#include "Page.h"
#include "Gfx.h"
#include "PDFDoc.h"

#include <QtCore/QtDebug>
#include <QRawFont>
#include <QGlyphRun>
#include <QtGui/QPainterPath>
#include <QPicture>

class QPainterOutputDevType3Font
{
public:
    QPainterOutputDevType3Font(PDFDoc *doc, const std::shared_ptr<Gfx8BitFont> &font);

    const QPicture &getGlyph(int gid) const;

private:
    PDFDoc *m_doc;
    std::shared_ptr<Gfx8BitFont> m_font;

    mutable std::vector<std::unique_ptr<QPicture>> glyphs;

public:
    std::vector<int> codeToGID;
};

QPainterOutputDevType3Font::QPainterOutputDevType3Font(PDFDoc *doc, const std::shared_ptr<Gfx8BitFont> &font) : m_doc(doc), m_font(font)
{
    char *name;
    const Dict *charProcs = font->getCharProcs();

    // Storage for the rendered glyphs
    glyphs.resize(charProcs->getLength());

    // Compute the code-to-GID map
    char **enc = font->getEncoding();

    codeToGID.resize(256);

    for (int i = 0; i < 256; ++i) {
        codeToGID[i] = 0;
        if (charProcs && (name = enc[i])) {
            for (int j = 0; j < charProcs->getLength(); j++) {
                if (strcmp(name, charProcs->getKey(j)) == 0) {
                    codeToGID[i] = j;
                }
            }
        }
    }
}

const QPicture &QPainterOutputDevType3Font::getGlyph(int gid) const
{
    if (!glyphs[gid]) {

        // Glyph has not been rendered before: render it now

        // Smallest box that contains all the glyphs from this font
        const double *fontBBox = m_font->getFontBBox();
        PDFRectangle box(fontBBox[0], fontBBox[1], fontBBox[2], fontBBox[3]);

        Dict *resDict = m_font->getResources();

        QPainter glyphPainter;
        glyphs[gid] = std::make_unique<QPicture>();
        glyphPainter.begin(glyphs[gid].get());
        auto output_dev = std::make_unique<QPainterOutputDev>(&glyphPainter);

        auto gfx = std::make_unique<Gfx>(m_doc, output_dev.get(), resDict,
                                         &box, // pagebox
                                         nullptr // cropBox
        );

        output_dev->startDoc(m_doc);

        output_dev->startPage(1, gfx->getState(), gfx->getXRef());

        const Dict *charProcs = m_font->getCharProcs();
        Object charProc = charProcs->getVal(gid);
        gfx->display(&charProc);

        glyphPainter.end();
    }

    return *glyphs[gid];
}

//------------------------------------------------------------------------
// QPainterOutputDev
//------------------------------------------------------------------------

QPainterOutputDev::QPainterOutputDev(QPainter *painter) : m_lastTransparencyGroupPicture(nullptr), m_hintingPreference(QFont::PreferDefaultHinting)
{
    m_painter.push(painter);
    m_currentBrush = QBrush(Qt::SolidPattern);

    auto error = FT_Init_FreeType(&m_ftLibrary);
    if (error) {
        qCritical() << "An error occurred will initializing the FreeType library";
    }

    // as of FT 2.1.8, CID fonts are indexed by CID instead of GID
    FT_Int major, minor, patch;
    FT_Library_Version(m_ftLibrary, &major, &minor, &patch);
    m_useCIDs = major > 2 || (major == 2 && (minor > 1 || (minor == 1 && patch > 7)));
}

QPainterOutputDev::~QPainterOutputDev()
{
    for (auto &codeToGID : m_codeToGIDCache) {
        gfree(const_cast<int *>(codeToGID.second));
    }

    FT_Done_FreeType(m_ftLibrary);
}

void QPainterOutputDev::startDoc(PDFDoc *doc)
{
    xref = doc->getXRef();
    m_doc = doc;

    for (auto &codeToGID : m_codeToGIDCache) {
        gfree(const_cast<int *>(codeToGID.second));
    }
    m_codeToGIDCache.clear();
}

void QPainterOutputDev::startPage(int pageNum, GfxState *state, XRef *) { }

void QPainterOutputDev::endPage() { }

void QPainterOutputDev::saveState(GfxState *state)
{
    m_currentPenStack.push(m_currentPen);
    m_currentBrushStack.push(m_currentBrush);
    m_rawFontStack.push(m_rawFont);
    m_type3FontStack.push(m_currentType3Font);
    m_codeToGIDStack.push(m_codeToGID);

    m_painter.top()->save();
}

void QPainterOutputDev::restoreState(GfxState *state)
{
    m_painter.top()->restore();

    m_codeToGID = m_codeToGIDStack.top();
    m_codeToGIDStack.pop();
    m_rawFont = m_rawFontStack.top();
    m_rawFontStack.pop();
    m_currentType3Font = m_type3FontStack.top();
    m_type3FontStack.pop();
    m_currentBrush = m_currentBrushStack.top();
    m_currentBrushStack.pop();
    m_currentPen = m_currentPenStack.top();
    m_currentPenStack.pop();
}

void QPainterOutputDev::updateAll(GfxState *state)
{
    OutputDev::updateAll(state);
    m_needFontUpdate = true;
}

// Set CTM (Current Transformation Matrix) to a fixed matrix
void QPainterOutputDev::setDefaultCTM(const double *ctm)
{
    m_painter.top()->setTransform(QTransform(ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]));
}

// Update the CTM (Current Transformation Matrix), i.e., compose the old
// CTM with a new matrix.
void QPainterOutputDev::updateCTM(GfxState *state, double m11, double m12, double m21, double m22, double m31, double m32)
{
    updateLineDash(state);
    updateLineJoin(state);
    updateLineCap(state);
    updateLineWidth(state);

    QTransform update(m11, m12, m21, m22, m31, m32);

    // We could also set (rather than update) the painter transformation to state->getCMT();
    m_painter.top()->setTransform(update, true);
}

void QPainterOutputDev::updateLineDash(GfxState *state)
{
    double dashStart;
    const std::vector<double> &dashPattern = state->getLineDash(&dashStart);

    // Special handling for zero-length patterns, i.e., solid lines.
    // Simply calling QPen::setDashPattern with an empty pattern does *not*
    // result in a solid line.  Rather, the current pattern is unchanged.
    // See the implementation of the setDashPattern method in the file qpen.cpp.
    if (dashPattern.empty()) {
        m_currentPen.setStyle(Qt::SolidLine);
        m_painter.top()->setPen(m_currentPen);
        return;
    }

    QVector<qreal> pattern(dashPattern.size());
    double scaling = state->getLineWidth();

    //  Negative line widths are not allowed, width 0 counts as 'one pixel width'.
    if (scaling <= 0) {
        scaling = 1.0;
    }

    for (std::vector<double>::size_type i = 0; i < dashPattern.size(); ++i) {
        // pdf measures the dash pattern in dots, but Qt uses the
        // line width as the unit.
        pattern[i] = dashPattern[i] / scaling;
    }
    m_currentPen.setDashPattern(pattern);
    m_currentPen.setDashOffset(dashStart);
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateFlatness(GfxState *state)
{
    // qDebug() << "updateFlatness";
}

void QPainterOutputDev::updateLineJoin(GfxState *state)
{
    switch (state->getLineJoin()) {
    case 0:
        // The correct style here is Qt::SvgMiterJoin, *not* Qt::MiterJoin.
        // The two differ in what to do if the miter limit is exceeded.
        // See https://bugs.freedesktop.org/show_bug.cgi?id=102356
        m_currentPen.setJoinStyle(Qt::SvgMiterJoin);
        break;
    case 1:
        m_currentPen.setJoinStyle(Qt::RoundJoin);
        break;
    case 2:
        m_currentPen.setJoinStyle(Qt::BevelJoin);
        break;
    }
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateLineCap(GfxState *state)
{
    switch (state->getLineCap()) {
    case 0:
        m_currentPen.setCapStyle(Qt::FlatCap);
        break;
    case 1:
        m_currentPen.setCapStyle(Qt::RoundCap);
        break;
    case 2:
        m_currentPen.setCapStyle(Qt::SquareCap);
        break;
    }
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateMiterLimit(GfxState *state)
{
    m_currentPen.setMiterLimit(state->getMiterLimit());
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateLineWidth(GfxState *state)
{
    m_currentPen.setWidthF(state->getLineWidth());
    m_painter.top()->setPen(m_currentPen);
    // The updateLineDash method needs to know the line width, but it is sometimes
    // called before the updateLineWidth method.  To make sure that the last call
    // to updateLineDash before a drawing operation is always with the correct line
    // width, we call it here, right after a change to the line width.
    updateLineDash(state);
}

void QPainterOutputDev::updateFillColor(GfxState *state)
{
    GfxRGB rgb;
    QColor brushColour = m_currentBrush.color();
    state->getFillRGB(&rgb);
    brushColour.setRgbF(colToDbl(rgb.r), colToDbl(rgb.g), colToDbl(rgb.b), brushColour.alphaF());
    m_currentBrush.setColor(brushColour);
}

void QPainterOutputDev::updateStrokeColor(GfxState *state)
{
    GfxRGB rgb;
    QColor penColour = m_currentPen.color();
    state->getStrokeRGB(&rgb);
    penColour.setRgbF(colToDbl(rgb.r), colToDbl(rgb.g), colToDbl(rgb.b), penColour.alphaF());
    m_currentPen.setColor(penColour);
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateBlendMode(GfxState *state)
{
    GfxBlendMode blendMode = state->getBlendMode();

    // missing composition modes in QPainter:
    // - CompositionMode_Hue
    // - CompositionMode_Color
    // - CompositionMode_Luminosity
    // - CompositionMode_Saturation

    switch (blendMode) {
    case gfxBlendMultiply:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Multiply);
        break;
    case gfxBlendScreen:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Screen);
        break;
    case gfxBlendDarken:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Darken);
        break;
    case gfxBlendLighten:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Lighten);
        break;
    case gfxBlendColorDodge:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_ColorDodge);
        break;
    case gfxBlendColorBurn:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_ColorBurn);
        break;
    case gfxBlendHardLight:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_HardLight);
        break;
    case gfxBlendSoftLight:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_SoftLight);
        break;
    case gfxBlendDifference:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Difference);
        break;
    case gfxBlendExclusion:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Exclusion);
        break;
    case gfxBlendColor:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_Plus);
        break;
    default:
        qDebug() << "Unsupported blend mode, falling back to CompositionMode_SourceOver";
        [[fallthrough]];
    case gfxBlendNormal:
        m_painter.top()->setCompositionMode(QPainter::CompositionMode_SourceOver);
        break;
    }
}

void QPainterOutputDev::updateFillOpacity(GfxState *state)
{
    QColor brushColour = m_currentBrush.color();
    brushColour.setAlphaF(state->getFillOpacity());
    m_currentBrush.setColor(brushColour);
}

void QPainterOutputDev::updateStrokeOpacity(GfxState *state)
{
    QColor penColour = m_currentPen.color();
    penColour.setAlphaF(state->getStrokeOpacity());
    m_currentPen.setColor(penColour);
    m_painter.top()->setPen(m_currentPen);
}

void QPainterOutputDev::updateFont(GfxState *state)
{
    const std::shared_ptr<GfxFont> &gfxFont = state->getFont();
    if (!gfxFont) {
        return;
    }

    // The key to look in the font caches
    QPainterFontID fontID = { *gfxFont->getID(), state->getFontSize() };

    // Current font is a type3 font
    if (gfxFont->getType() == fontType3) {
        auto cacheEntry = m_type3FontCache.find(fontID);

        if (cacheEntry != m_type3FontCache.end()) {

            // Take the font from the cache
            m_currentType3Font = cacheEntry->second.get();

        } else {

            m_currentType3Font = new QPainterOutputDevType3Font(m_doc, std::static_pointer_cast<Gfx8BitFont>(gfxFont));
            m_type3FontCache.insert(std::make_pair(fontID, std::unique_ptr<QPainterOutputDevType3Font>(m_currentType3Font)));
        }

        return;
    }

    // Non-type3: is the font in the cache?
    auto cacheEntry = m_rawFontCache.find(fontID);

    if (cacheEntry != m_rawFontCache.end()) {

        // Take the font from the cache
        m_rawFont = cacheEntry->second.get();

    } else {

        // New font: load it into the cache
        float fontSize = state->getFontSize();

        std::optional<GfxFontLoc> fontLoc = gfxFont->locateFont(xref, nullptr);

        if (fontLoc) {
            // load the font from respective location
            switch (fontLoc->locType) {
            case gfxFontLocEmbedded: { // if there is an embedded font, read it to memory
                const std::optional<std::vector<unsigned char>> fontData = gfxFont->readEmbFontFile(xref);

                // fontData gets copied in the QByteArray constructor
                m_rawFont = new QRawFont(QByteArray(fontData ? (const char *)fontData->data() : nullptr, fontData ? fontData->size() : 0), fontSize, m_hintingPreference);
                m_rawFontCache.insert(std::make_pair(fontID, std::unique_ptr<QRawFont>(m_rawFont)));

                break;
            }
            case gfxFontLocExternal: { // font is in an external font file
                QString fontFile(fontLoc->path.c_str());
                m_rawFont = new QRawFont(fontFile, fontSize, m_hintingPreference);
                m_rawFontCache.insert(std::make_pair(fontID, std::unique_ptr<QRawFont>(m_rawFont)));
                break;
            }
            case gfxFontLocResident: { // font resides in a PS printer
                qDebug() << "Resident Font Resident not implemented yet!";

                break;
            }
            } // end switch

        } else {
            qDebug() << "Font location not found!";
            return;
        }
    }

    if (!m_rawFont->isValid()) {
        qDebug() << "RawFont is not valid";
    }

    // *****************************************************************************
    //  We have now successfully loaded the font into a QRawFont object.  This
    //  allows us to draw all the glyphs in the font.  However, what is missing is
    //  the charcode-to-glyph-index mapping.  Apparently, Qt does not provide this
    //  information at all.  Therefore, we need to figure it ourselves, using
    //  FoFi and FreeType.
    // *****************************************************************************

    m_needFontUpdate = false;

    GfxFontType fontType = gfxFont->getType();

    // Default: no codeToGID table
    m_codeToGID = nullptr;

    // check the font file cache
    Ref id = *gfxFont->getID();

    auto codeToGIDIt = m_codeToGIDCache.find(id);

    if (codeToGIDIt != m_codeToGIDCache.end()) {

        m_codeToGID = codeToGIDIt->second;

    } else {

        std::optional<std::vector<unsigned char>> fontBuffer;

        std::optional<GfxFontLoc> fontLoc = gfxFont->locateFont(xref, nullptr);
        if (!fontLoc) {
            error(errSyntaxError, -1, "Couldn't find a font for '{0:s}'", gfxFont->getName() ? gfxFont->getName()->c_str() : "(unnamed)");
            return;
        }

        // embedded font
        if (fontLoc->locType == gfxFontLocEmbedded) {
            // if there is an embedded font, read it to memory
            fontBuffer = gfxFont->readEmbFontFile(xref);
            if (!fontBuffer) {
                return;
            }

            // external font
        } else { // gfxFontLocExternal
            // Hmm, fontType has already been set to gfxFont->getType() above.
            // Can it really assume a different value here?
            fontType = fontLoc->fontType;
        }

        switch (fontType) {
        case fontType1:
        case fontType1C:
        case fontType1COT: {
            // Load the font face using FreeType
            const int faceIndex = 0; // We always load the zero-th face from a font
            FT_Face freeTypeFace;

            if (fontLoc->locType != gfxFontLocEmbedded) {
                if (ft_new_face_from_file(m_ftLibrary, fontLoc->path.c_str(), faceIndex, &freeTypeFace)) {
                    error(errSyntaxError, -1, "Couldn't create a FreeType face for '{0:s}'", gfxFont->getName() ? gfxFont->getName()->c_str() : "(unnamed)");
                    return;
                }
            } else {
                if (FT_New_Memory_Face(m_ftLibrary, (const FT_Byte *)fontBuffer->data(), fontBuffer->size(), faceIndex, &freeTypeFace)) {
                    error(errSyntaxError, -1, "Couldn't create a FreeType face for '{0:s}'", gfxFont->getName() ? gfxFont->getName()->c_str() : "(unnamed)");
                    return;
                }
            }

            const char *name;

            int *codeToGID = (int *)gmallocn(256, sizeof(int));
            for (int i = 0; i < 256; ++i) {
                codeToGID[i] = 0;
                if ((name = ((const char **)((Gfx8BitFont *)gfxFont.get())->getEncoding())[i])) {
                    codeToGID[i] = (int)FT_Get_Name_Index(freeTypeFace, (char *)name);
                    if (codeToGID[i] == 0) {
                        name = GfxFont::getAlternateName(name);
                        if (name) {
                            codeToGID[i] = FT_Get_Name_Index(freeTypeFace, (char *)name);
                        }
                    }
                }
            }

            FT_Done_Face(freeTypeFace);

            m_codeToGIDCache[id] = codeToGID;

            break;
        }
        case fontTrueType:
        case fontTrueTypeOT: {
            auto ff = (fontLoc->locType != gfxFontLocEmbedded) ? FoFiTrueType::load(fontLoc->path.c_str()) : FoFiTrueType::make(fontBuffer->data(), fontBuffer->size());

            m_codeToGIDCache[id] = (ff) ? ((Gfx8BitFont *)gfxFont.get())->getCodeToGIDMap(ff.get()) : nullptr;

            break;
        }
        case fontCIDType0:
        case fontCIDType0C: {
            int *cidToGIDMap = nullptr;
            int nCIDs = 0;

            // check for a CFF font
            if (!m_useCIDs) {
                auto ff = (fontLoc->locType != gfxFontLocEmbedded) ? std::unique_ptr<FoFiType1C>(FoFiType1C::load(fontLoc->path.c_str())) : std::unique_ptr<FoFiType1C>(FoFiType1C::make(fontBuffer->data(), fontBuffer->size()));

                cidToGIDMap = (ff) ? ff->getCIDToGIDMap(&nCIDs) : nullptr;
            }

            m_codeToGIDCache[id] = cidToGIDMap;

            break;
        }
        case fontCIDType0COT: {
            int *codeToGID = nullptr;

            if (((GfxCIDFont *)gfxFont.get())->getCIDToGID()) {
                int codeToGIDLen = ((GfxCIDFont *)gfxFont.get())->getCIDToGIDLen();
                codeToGID = (int *)gmallocn(codeToGIDLen, sizeof(int));
                memcpy(codeToGID, ((GfxCIDFont *)gfxFont.get())->getCIDToGID(), codeToGIDLen * sizeof(int));
            }

            int *cidToGIDMap = nullptr;
            int nCIDs = 0;

            if (!codeToGID && !m_useCIDs) {
                auto ff = (fontLoc->locType != gfxFontLocEmbedded) ? FoFiTrueType::load(fontLoc->path.c_str()) : FoFiTrueType::make(fontBuffer->data(), fontBuffer->size());

                if (ff && ff->isOpenTypeCFF()) {
                    cidToGIDMap = ff->getCIDToGIDMap(&nCIDs);
                }
            }

            m_codeToGIDCache[id] = codeToGID ? codeToGID : cidToGIDMap;

            break;
        }
        case fontCIDType2:
        case fontCIDType2OT: {
            int *codeToGID = nullptr;
            int codeToGIDLen = 0;
            if (((GfxCIDFont *)gfxFont.get())->getCIDToGID()) {
                codeToGIDLen = ((GfxCIDFont *)gfxFont.get())->getCIDToGIDLen();
                if (codeToGIDLen) {
                    codeToGID = (int *)gmallocn(codeToGIDLen, sizeof(int));
                    memcpy(codeToGID, ((GfxCIDFont *)gfxFont.get())->getCIDToGID(), codeToGIDLen * sizeof(int));
                }
            } else {
                auto ff = (fontLoc->locType != gfxFontLocEmbedded) ? FoFiTrueType::load(fontLoc->path.c_str()) : FoFiTrueType::make(fontBuffer->data(), fontBuffer->size());
                if (!ff) {
                    return;
                }
                codeToGID = ((GfxCIDFont *)gfxFont.get())->getCodeToGIDMap(ff.get(), &codeToGIDLen);
            }

            m_codeToGIDCache[id] = codeToGID;

            break;
        }
        default:
            // this shouldn't happen
            return;
        }

        m_codeToGID = m_codeToGIDCache[id];
    }
}

static QPainterPath convertPath(GfxState *state, const GfxPath *path, Qt::FillRule fillRule)
{
    int i, j;

    QPainterPath qPath;
    qPath.setFillRule(fillRule);
    for (i = 0; i < path->getNumSubpaths(); ++i) {
        const GfxSubpath *subpath = path->getSubpath(i);
        if (subpath->getNumPoints() > 0) {
            qPath.moveTo(subpath->getX(0), subpath->getY(0));
            j = 1;
            while (j < subpath->getNumPoints()) {
                if (subpath->getCurve(j)) {
                    qPath.cubicTo(subpath->getX(j), subpath->getY(j), subpath->getX(j + 1), subpath->getY(j + 1), subpath->getX(j + 2), subpath->getY(j + 2));
                    j += 3;
                } else {
                    qPath.lineTo(subpath->getX(j), subpath->getY(j));
                    ++j;
                }
            }
            if (subpath->isClosed()) {
                qPath.closeSubpath();
            }
        }
    }
    return qPath;
}

void QPainterOutputDev::stroke(GfxState *state)
{
    m_painter.top()->strokePath(convertPath(state, state->getPath(), Qt::OddEvenFill), m_currentPen);
}

void QPainterOutputDev::fill(GfxState *state)
{
    m_painter.top()->fillPath(convertPath(state, state->getPath(), Qt::WindingFill), m_currentBrush);
}

void QPainterOutputDev::eoFill(GfxState *state)
{
    m_painter.top()->fillPath(convertPath(state, state->getPath(), Qt::OddEvenFill), m_currentBrush);
}

bool QPainterOutputDev::axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax)
{
    double x0, y0, x1, y1;
    shading->getCoords(&x0, &y0, &x1, &y1);

    // get the clip region bbox
    double xMin, yMin, xMax, yMax;
    state->getUserClipBBox(&xMin, &yMin, &xMax, &yMax);

    // get the function domain
    double t0 = shading->getDomain0();
    double t1 = shading->getDomain1();

    // Max number of splits along the t axis
    constexpr int maxSplits = 256;

    // Max delta allowed in any color component
    const double colorDelta = (dblToCol(1 / 256.0));

    // Number of color space components
    auto nComps = shading->getColorSpace()->getNComps();
    // If the clipping region is a stroke, then the current operation counts as a stroke
    // rather than as a fill, and the opacity has to be set accordingly.
    // See https://gitlab.freedesktop.org/poppler/poppler/-/issues/178
    auto opacity = (state->getStrokePattern()) ? state->getStrokeOpacity() : state->getFillOpacity();

    // Helper function to test two color objects for 'almost-equality'
    auto isSameGfxColor = [&nComps, &colorDelta](const GfxColor &colorA, const GfxColor &colorB) {
        for (int k = 0; k < nComps; ++k) {
            if (abs(colorA.c[k] - colorB.c[k]) > colorDelta) {
                return false;
            }
        }
        return true;
    };

    // Helper function: project a number into an interval
    // With C++17 this is part of the standard library
    auto clamp = [](double v, double lo, double hi) { return std::min(std::max(v, lo), hi); };

    // ta stores all parameter values where we evaluate the input shading function.
    // In between, QLinearGradient will interpolate linearly.
    // We set up the array with three values.
    std::array<double, maxSplits + 1> ta;
    ta[0] = tMin;
    std::array<int, maxSplits + 1> next;
    next[0] = maxSplits / 2;
    ta[maxSplits / 2] = 0.5 * (tMin + tMax);
    next[maxSplits / 2] = maxSplits;
    ta[maxSplits] = tMax;

    // compute the color at t = tMin
    double tt = clamp(t0 + (t1 - t0) * tMin, t0, t1);

    GfxColor color0, color1;
    shading->getColor(tt, &color0);

    // Construct a gradient object and set its color at one parameter end
    QLinearGradient gradient(QPointF(x0 + tMin * (x1 - x0), y0 + tMin * (y1 - y0)), QPointF(x0 + tMax * (x1 - x0), y0 + tMax * (y1 - y0)));

    GfxRGB rgb;
    shading->getColorSpace()->getRGB(&color0, &rgb);
    QColor qColor(colToByte(rgb.r), colToByte(rgb.g), colToByte(rgb.b), dblToByte(opacity));
    gradient.setColorAt(0, qColor);

    // Look for more relevant parameter values by bisection
    int i = 0;
    while (i < maxSplits) {

        int j = next[i];
        while (j > i + 1) {

            // Next parameter value to try
            tt = clamp(t0 + (t1 - t0) * ta[j], t0, t1);
            shading->getColor(tt, &color1);

            // j is a good next color stop if the input shading can be approximated well
            // on the interval (ta[i], ta[j]) by a linear interpolation.
            // We test this by comparing the real color in the middle between ta[i] and ta[j]
            // with the linear interpolant there.
            auto midPoint = 0.5 * (ta[i] + ta[j]);
            GfxColor colorAtMidPoint;
            shading->getColor(midPoint, &colorAtMidPoint);

            GfxColor linearlyInterpolatedColor;
            for (int ii = 0; ii < nComps; ii++) {
                linearlyInterpolatedColor.c[ii] = 0.5 * (color0.c[ii] + color1.c[ii]);
            }

            // If the two colors are equal, ta[j] is a good place for the next color stop; take it!
            if (isSameGfxColor(colorAtMidPoint, linearlyInterpolatedColor)) {
                break;
            }

            // Otherwise: bisect further
            int k = (i + j) / 2;
            ta[k] = midPoint;
            next[i] = k;
            next[k] = j;
            j = k;
        }

        // set the color
        shading->getColorSpace()->getRGB(&color1, &rgb);
        qColor.setRgb(colToByte(rgb.r), colToByte(rgb.g), colToByte(rgb.b), dblToByte(opacity));
        gradient.setColorAt((ta[j] - tMin) / (tMax - tMin), qColor);

        // Move to the next parameter region
        color0 = color1;
        i = next[i];
    }

    state->moveTo(xMin, yMin);
    state->lineTo(xMin, yMax);
    state->lineTo(xMax, yMax);
    state->lineTo(xMax, yMin);
    state->closePath();

    // Actually paint the shaded region
    QBrush newBrush(gradient);
    m_painter.top()->fillPath(convertPath(state, state->getPath(), Qt::WindingFill), newBrush);

    state->clearPath();

    // True means: The shaded region has been painted
    return true;
}

void QPainterOutputDev::clip(GfxState *state)
{
    m_painter.top()->setClipPath(convertPath(state, state->getPath(), Qt::WindingFill), Qt::IntersectClip);
}

void QPainterOutputDev::eoClip(GfxState *state)
{
    m_painter.top()->setClipPath(convertPath(state, state->getPath(), Qt::OddEvenFill), Qt::IntersectClip);
}

void QPainterOutputDev::clipToStrokePath(GfxState *state)
{
    QPainterPath clipPath = convertPath(state, state->getPath(), Qt::WindingFill);

    // Get the outline of 'clipPath' as a separate path
    QPainterPathStroker stroker;
    stroker.setWidth(state->getLineWidth());
    stroker.setCapStyle(m_currentPen.capStyle());
    stroker.setJoinStyle(m_currentPen.joinStyle());
    stroker.setMiterLimit(state->getMiterLimit());
    stroker.setDashPattern(m_currentPen.dashPattern());
    stroker.setDashOffset(m_currentPen.dashOffset());
    QPainterPath clipPathOutline = stroker.createStroke(clipPath);

    // The interior of the outline is the desired clipping region
    m_painter.top()->setClipPath(clipPathOutline, Qt::IntersectClip);
}

void QPainterOutputDev::drawChar(GfxState *state, double x, double y, double dx, double dy, double originX, double originY, CharCode code, int nBytes, const Unicode *u, int uLen)
{

    // First handle type3 fonts
    const std::shared_ptr<GfxFont> &gfxFont = state->getFont();

    GfxFontType fontType = gfxFont->getType();
    if (fontType == fontType3) {

        /////////////////////////////////////////////////////////////////////
        //  Draw the QPicture that contains the glyph onto the page
        /////////////////////////////////////////////////////////////////////

        // Store the QPainter state; we need to modify it temporarily
        m_painter.top()->save();

        // Make the glyph position the coordinate origin -- that's our center of scaling
        m_painter.top()->translate(QPointF(x - originX, y - originY));

        const double *mat = gfxFont->getFontMatrix();
        QTransform fontMatrix(mat[0], mat[1], mat[2], mat[3], mat[4], mat[5]);

        // Scale with the font size
        fontMatrix.scale(state->getFontSize(), state->getFontSize());
        m_painter.top()->setTransform(fontMatrix, true);

        // Apply the text matrix on top
        const double *textMat = state->getTextMat();

        QTransform textTransform(textMat[0] * state->getHorizScaling(), textMat[1] * state->getHorizScaling(), textMat[2], textMat[3], 0, 0);

        m_painter.top()->setTransform(textTransform, true);

        // Actually draw the glyph
        int gid = m_currentType3Font->codeToGID[code];
        m_painter.top()->drawPicture(QPointF(0, 0), m_currentType3Font->getGlyph(gid));

        // Restore transformation
        m_painter.top()->restore();

        return;
    }

    // check for invisible text -- this is used by Acrobat Capture
    int render = state->getRender();
    if (render == 3 || !m_rawFont) {
        qDebug() << "Invisible text found!";
        return;
    }

    if (!(render & 1)) {
        quint32 glyphIndex = (m_codeToGID) ? m_codeToGID[code] : code;
        QPointF glyphPosition = QPointF(x - originX, y - originY);

        // QGlyphRun objects can hold an entire sequence of glyphs, and it would possibly
        // be more efficient to simply note the glyph and glyph position here and then
        // draw several glyphs at once in the endString method.  What keeps us from doing
        // that is the transformation below: each glyph needs to be drawn upside down,
        // i.e., reflected at its own baseline.  Since we have no guarantee that this
        // baseline is the same for all glyphs in a string we have to do it one by one.
        QGlyphRun glyphRun;
        glyphRun.setRawData(&glyphIndex, &glyphPosition, 1);
        glyphRun.setRawFont(*m_rawFont);

        // Store the QPainter state; we need to modify it temporarily
        m_painter.top()->save();

        // Apply the text matrix to the glyph.  The glyph is not scaled by the font size,
        // because the font in m_rawFont already has the correct size.
        // Additionally, the CTM is upside down, i.e., it contains a negative Y-scaling
        // entry.  Therefore, Qt will paint the glyphs upside down.  We need to temporarily
        // reflect the page at glyphPosition.y().

        // Make the glyph position the coordinate origin -- that's our center of scaling
        const double *textMat = state->getTextMat();

        m_painter.top()->translate(QPointF(glyphPosition.x(), glyphPosition.y()));

        QTransform textTransform(textMat[0] * state->getHorizScaling(), textMat[1] * state->getHorizScaling(),
                                 -textMat[2], // reflect at the horizontal axis,
                                 -textMat[3], // because CTM is upside-down.
                                 0, 0);

        m_painter.top()->setTransform(textTransform, true);

        // We are painting a filled glyph here.  But QPainter uses the pen to draw even filled text,
        // not the brush.  (see, e.g.,  http://doc.qt.io/qt-5/qpainter.html#setPen )
        // Therefore we have to temporarily overwrite the pen color.

        // Since we are drawing a filled glyph, one would really expect to have m_currentBrush
        // have the correct color.  However, somehow state->getFillRGB can change without
        // updateFillColor getting called.  Then m_currentBrush may not contain the correct color.
        GfxRGB rgb;
        state->getFillRGB(&rgb);
        QColor fontColor;
        fontColor.setRgbF(colToDbl(rgb.r), colToDbl(rgb.g), colToDbl(rgb.b), state->getFillOpacity());
        m_painter.top()->setPen(fontColor);

        // Actually draw the glyph
        m_painter.top()->drawGlyphRun(QPointF(-glyphPosition.x(), -glyphPosition.y()), glyphRun);

        // Restore transformation and pen color
        m_painter.top()->restore();
    }
}

void QPainterOutputDev::type3D0(GfxState *state, double wx, double wy) { }

void QPainterOutputDev::type3D1(GfxState *state, double wx, double wy, double llx, double lly, double urx, double ury) { }

void QPainterOutputDev::endTextObject(GfxState *state) { }

void QPainterOutputDev::drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg)
{
    auto imgStr = std::make_unique<ImageStream>(str, width,
                                                1, // numPixelComps
                                                1 // getBits
    );
    imgStr->reset();

    // TODO: Would using QImage::Format_Mono be more efficient here?
    QImage image(width, height, QImage::Format_ARGB32);
    unsigned int *data = reinterpret_cast<unsigned int *>(image.bits());
    int stride = image.bytesPerLine() / 4;

    QRgb fillColor = m_currentBrush.color().rgb();

    for (int y = 0; y < height; y++) {

        unsigned char *pix = imgStr->getLine();

        // Invert the vertical coordinate: y is increasing from top to bottom
        // on the page, but y is increasing bottom to top in the picture.
        unsigned int *dest = data + (height - 1 - y) * stride;

        for (int x = 0; x < width; x++) {

            bool opaque = ((bool)pix[x]) == invert;
            dest[x] = (opaque) ? fillColor : 0;
        }
    }

    // At this point, the QPainter coordinate transformation (CTM) is such
    // that QRect(0,0,1,1) is exactly the area of the image.
    m_painter.top()->drawImage(QRect(0, 0, 1, 1), image);
    imgStr->close();
}

// TODO: lots more work here.
void QPainterOutputDev::drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg)
{
    unsigned int *data;
    unsigned int *line;
    int x, y;
    unsigned char *pix;
    int i;
    QImage image;
    int stride;

    /* TODO: Do we want to cache these? */
    auto imgStr = std::make_unique<ImageStream>(str, width, colorMap->getNumPixelComps(), colorMap->getBits());
    imgStr->reset();

    image = QImage(width, height, QImage::Format_ARGB32);
    data = reinterpret_cast<unsigned int *>(image.bits());
    stride = image.bytesPerLine() / 4;
    for (y = 0; y < height; y++) {
        pix = imgStr->getLine();
        // Invert the vertical coordinate: y is increasing from top to bottom
        // on the page, but y is increasing bottom to top in the picture.
        line = data + (height - 1 - y) * stride;
        colorMap->getRGBLine(pix, line, width);

        if (maskColors) {
            for (x = 0; x < width; x++) {
                for (i = 0; i < colorMap->getNumPixelComps(); ++i) {
                    if (pix[i] < maskColors[2 * i] * 255 || pix[i] > maskColors[2 * i + 1] * 255) {
                        *line = *line | 0xff000000;
                        break;
                    }
                }
                pix += colorMap->getNumPixelComps();
                line++;
            }
        } else {
            for (x = 0; x < width; x++) {
                *line = *line | 0xff000000;
                line++;
            }
        }
    }

    // At this point, the QPainter coordinate transformation (CTM) is such
    // that QRect(0,0,1,1) is exactly the area of the image.
    m_painter.top()->drawImage(QRect(0, 0, 1, 1), image);
}

void QPainterOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap,
                                            bool maskInterpolate)
{
    // Bail out if the image size doesn't match the mask size.  I don't know
    // what to do in this case.
    if (width != maskWidth || height != maskHeight) {
        qDebug() << "Soft mask size does not match image size!";
        drawImage(state, ref, str, width, height, colorMap, interpolate, nullptr, false);
        return;
    }

    // Bail out if the mask isn't a single channel.  I don't know
    // what to do in this case.
    if (maskColorMap->getColorSpace()->getNComps() != 1) {
        qDebug() << "Soft mask is not a single 8-bit channel!";
        drawImage(state, ref, str, width, height, colorMap, interpolate, nullptr, false);
        return;
    }

    /* TODO: Do we want to cache these? */
    auto imgStr = std::make_unique<ImageStream>(str, width, colorMap->getNumPixelComps(), colorMap->getBits());
    imgStr->reset();

    auto maskImageStr = std::make_unique<ImageStream>(maskStr, maskWidth, maskColorMap->getNumPixelComps(), maskColorMap->getBits());
    maskImageStr->reset();

    QImage image(width, height, QImage::Format_ARGB32);
    unsigned int *data = reinterpret_cast<unsigned int *>(image.bits());
    int stride = image.bytesPerLine() / 4;

    std::vector<unsigned char> maskLine(maskWidth);

    for (int y = 0; y < height; y++) {

        unsigned char *pix = imgStr->getLine();
        unsigned char *maskPix = maskImageStr->getLine();

        // Invert the vertical coordinate: y is increasing from top to bottom
        // on the page, but y is increasing bottom to top in the picture.
        unsigned int *line = data + (height - 1 - y) * stride;
        colorMap->getRGBLine(pix, line, width);

        // Apply the mask values to the image alpha channel
        maskColorMap->getGrayLine(maskPix, maskLine.data(), width);
        for (int x = 0; x < width; x++) {
            *line = *line | (maskLine[x] << 24);
            line++;
        }
    }

    // At this point, the QPainter coordinate transformation (CTM) is such
    // that QRect(0,0,1,1) is exactly the area of the image.
    m_painter.top()->drawImage(QRect(0, 0, 1, 1), image);
}

void QPainterOutputDev::beginTransparencyGroup(GfxState * /*state*/, const double * /*bbox*/, GfxColorSpace * /*blendingColorSpace*/, bool /*isolated*/, bool /*knockout*/, bool /*forSoftMask*/)
{
    // The entire transparency group will be painted into a
    // freshly created QPicture object.  Since an existing painter
    // cannot change its paint device, we need to construct a
    // new QPainter object as well.
    m_qpictures.push(new QPicture);
    m_painter.push(new QPainter(m_qpictures.top()));
}

void QPainterOutputDev::endTransparencyGroup(GfxState * /*state*/)
{
    // Stop painting into the group
    m_painter.top()->end();

    // Kill the painter that has been used for the transparency group
    delete (m_painter.top());
    m_painter.pop();

    // Store the QPicture object that holds the result of the transparency group
    // painting.  It will be painted and deleted in the method paintTransparencyGroup.
    if (m_lastTransparencyGroupPicture) {
        qDebug() << "Found a transparency group that has not been painted";
        delete (m_lastTransparencyGroupPicture);
    }
    m_lastTransparencyGroupPicture = m_qpictures.top();
    m_qpictures.pop();
}

void QPainterOutputDev::paintTransparencyGroup(GfxState * /*state*/, const double * /*bbox*/)
{
    // Actually draw the transparency group
    m_painter.top()->drawPicture(0, 0, *m_lastTransparencyGroupPicture);

    // And delete it
    delete (m_lastTransparencyGroupPicture);
    m_lastTransparencyGroupPicture = nullptr;
}
