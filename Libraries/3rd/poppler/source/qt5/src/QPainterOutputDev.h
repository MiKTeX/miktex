//========================================================================
//
// QPainterOutputDev.h
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
// Copyright (C) 2005, 2018, 2019, 2021 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2009, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2010 Pino Toscano <pino@kde.org>
// Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2013 Mihai Niculescu <q.quark@gmail.com>
// Copyright (C) 2017, 2018, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#ifndef QPAINTEROUTPUTDEV_H
#define QPAINTEROUTPUTDEV_H

#include <memory>
#include <map>
#include <stack>

#include "OutputDev.h"
#include "GfxState.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <QtGui/QPainter>

class GfxState;
class PDFDoc;

class QRawFont;

class QPainterOutputDevType3Font;

//------------------------------------------------------------------------
// QPainterOutputDev - Qt 5 QPainter renderer
//------------------------------------------------------------------------

class QPainterOutputDev : public OutputDev
{
public:
    // Constructor.
    explicit QPainterOutputDev(QPainter *painter);

    // Destructor.
    ~QPainterOutputDev() override;

    void setHintingPreference(QFont::HintingPreference hintingPreference) { m_hintingPreference = hintingPreference; }

    //----- get info about output device

    // Does this device use upside-down coordinates?
    // (Upside-down means (0,0) is the top left corner of the page.)
    bool upsideDown() override { return true; }

    // Does this device use drawChar() or drawString()?
    bool useDrawChar() override { return true; }

    // Does this device implement shaded fills (aka gradients) natively?
    // If this returns false, these shaded fills
    // will be reduced to a series of other drawing operations.
    // type==2 is 'axial shading'
    bool useShadedFills(int type) override { return type == 2; }

    // Does this device use beginType3Char/endType3Char?  Otherwise,
    // text in Type 3 fonts will be drawn with drawChar/drawString.
    bool interpretType3Chars() override { return false; }

    //----- initialization and control

    // Set Current Transformation Matrix to a fixed matrix given in ctm[0],...,ctm[5]
    void setDefaultCTM(const double *ctm) override;

    // Start a page.
    void startPage(int pageNum, GfxState *state, XRef *xref) override;

    // End a page.
    void endPage() override;

    //----- save/restore graphics state
    void saveState(GfxState *state) override;
    void restoreState(GfxState *state) override;

    //----- update graphics state
    void updateAll(GfxState *state) override;
    void updateCTM(GfxState *state, double m11, double m12, double m21, double m22, double m31, double m32) override;
    void updateLineDash(GfxState *state) override;
    void updateFlatness(GfxState *state) override;
    void updateLineJoin(GfxState *state) override;
    void updateLineCap(GfxState *state) override;
    void updateMiterLimit(GfxState *state) override;
    void updateLineWidth(GfxState *state) override;
    void updateFillColor(GfxState *state) override;
    void updateStrokeColor(GfxState *state) override;
    void updateBlendMode(GfxState *state) override;
    void updateFillOpacity(GfxState *state) override;
    void updateStrokeOpacity(GfxState *state) override;

    //----- update text state
    void updateFont(GfxState *state) override;

    //----- path painting
    void stroke(GfxState *state) override;
    void fill(GfxState *state) override;
    void eoFill(GfxState *state) override;
    bool axialShadedFill(GfxState *state, GfxAxialShading *shading, double tMin, double tMax) override;

    //----- path clipping
    void clip(GfxState *state) override;
    void eoClip(GfxState *state) override;
    void clipToStrokePath(GfxState *state) override;

    //----- text drawing
    //   virtual void drawString(GfxState *state, GooString *s);
    void drawChar(GfxState *state, double x, double y, double dx, double dy, double originX, double originY, CharCode code, int nBytes, const Unicode *u, int uLen) override;
    void endTextObject(GfxState *state) override;

    //----- image drawing
    void drawImageMask(GfxState *state, Object *ref, Stream *str, int width, int height, bool invert, bool interpolate, bool inlineImg) override;
    void drawImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, const int *maskColors, bool inlineImg) override;

    void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str, int width, int height, GfxImageColorMap *colorMap, bool interpolate, Stream *maskStr, int maskWidth, int maskHeight, GfxImageColorMap *maskColorMap,
                             bool maskInterpolate) override;

    //----- Type 3 font operators
    void type3D0(GfxState *state, double wx, double wy) override;
    void type3D1(GfxState *state, double wx, double wy, double llx, double lly, double urx, double ury) override;

    //----- transparency groups and soft masks
    void beginTransparencyGroup(GfxState *state, const double *bbox, GfxColorSpace *blendingColorSpace, bool isolated, bool knockout, bool forSoftMask) override;
    void endTransparencyGroup(GfxState *state) override;
    void paintTransparencyGroup(GfxState *state, const double *bbox) override;

    //----- special access

    // Called to indicate that a new PDF document has been loaded.
    void startDoc(PDFDoc *doc);

    bool isReverseVideo() { return false; }

private:
    // The stack of QPainters is used to implement transparency groups.  When such a group
    // is opened, annew Painter that paints onto a QPicture is pushed onto the stack.
    // It is popped again when the transparency group ends.
    std::stack<QPainter *> m_painter;

    // This is the corresponding stack of QPicture objects
    std::stack<QPicture *> m_qpictures;

    // endTransparencyGroup removes a QPicture from the stack, but stores
    // it here for later use in paintTransparencyGroup.
    QPicture *m_lastTransparencyGroupPicture;

    QFont::HintingPreference m_hintingPreference;

    QPen m_currentPen;
    // The various stacks are used to implement the 'saveState' and 'restoreState' methods
    std::stack<QPen> m_currentPenStack;

    QBrush m_currentBrush;
    std::stack<QBrush> m_currentBrushStack;

    bool m_needFontUpdate; // set when the font needs to be updated
    PDFDoc *m_doc;
    XRef *xref; // xref table for current document

    // The current font in use
    QRawFont *m_rawFont;
    std::stack<QRawFont *> m_rawFontStack;

    QPainterOutputDevType3Font *m_currentType3Font;
    std::stack<QPainterOutputDevType3Font *> m_type3FontStack;

    // Cache all fonts by their Ref and font size
    using QPainterFontID = std::pair<Ref, double>;
    std::map<QPainterFontID, std::unique_ptr<QRawFont>> m_rawFontCache;
    std::map<QPainterFontID, std::unique_ptr<QPainterOutputDevType3Font>> m_type3FontCache;
    std::map<Ref, const int *> m_codeToGIDCache;

    // The table that maps character codes to glyph indexes
    const int *m_codeToGID;
    std::stack<const int *> m_codeToGIDStack;

    FT_Library m_ftLibrary;
    // as of FT 2.1.8, CID fonts are indexed by CID instead of GID
    bool m_useCIDs;
};

#endif
