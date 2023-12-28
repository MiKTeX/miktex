//========================================================================
//
// CairoFontEngine.cc
//
// Copyright 2003 Glyph & Cog, LLC
// Copyright 2004 Red Hat, Inc
//
//========================================================================

//========================================================================
//
// Modified under the Poppler project - http://poppler.freedesktop.org
//
// All changes made under the Poppler project to this file are licensed
// under GPL version 2 or later
//
// Copyright (C) 2005-2007 Jeff Muizelaar <jeff@infidigm.net>
// Copyright (C) 2005, 2006 Kristian Høgsberg <krh@redhat.com>
// Copyright (C) 2005 Martin Kretzschmar <martink@gnome.org>
// Copyright (C) 2005, 2009, 2012, 2013, 2015, 2017-2019, 2021, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright (C) 2006, 2007, 2010, 2011 Carlos Garcia Campos <carlosgc@gnome.org>
// Copyright (C) 2007 Koji Otani <sho@bbr.jp>
// Copyright (C) 2008, 2009 Chris Wilson <chris@chris-wilson.co.uk>
// Copyright (C) 2008, 2012, 2014, 2016, 2017, 2022, 2023 Adrian Johnson <ajohnson@redneon.com>
// Copyright (C) 2009 Darren Kenny <darren.kenny@sun.com>
// Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
// Copyright (C) 2010 Jan Kümmel <jan+freedesktop@snorc.org>
// Copyright (C) 2012 Hib Eris <hib@hiberis.nl>
// Copyright (C) 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
// Copyright (C) 2015, 2016 Jason Crain <jason@aquaticape.us>
// Copyright (C) 2018 Adam Reichold <adam.reichold@t-online.de>
// Copyright (C) 2019 Christian Persch <chpe@src.gnome.org>
// Copyright (C) 2020 Michal <sudolskym@gmail.com>
// Copyright (C) 2021, 2022 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright (C) 2022 Marcel Fabian Krüger <tex@2krueger.de>
// Copyright (C) 2023 Pablo Correa Gómez <ablocorrea@hotmail.com>
// Copyright (C) 2023 Frederic Germain <frederic.germain@gmail.com>
// Copyright (C) 2023 Ilia Kats <ilia-kats@gmx.net>
//
// To see a description of the changes please see the Changelog file that
// came with your tarball or type make ChangeLog if you are building from git
//
//========================================================================

#include <config.h>

#include <cstring>
#include <fstream>
#include "CairoFontEngine.h"
#include "CairoOutputDev.h"
#include "GlobalParams.h"
#include <fofi/FoFiTrueType.h>
#include <fofi/FoFiType1C.h>
#include "goo/ft_utils.h"
#include "goo/gfile.h"
#include "Error.h"
#include "XRef.h"
#include "Gfx.h"
#include "Page.h"

//------------------------------------------------------------------------
// CairoFont
//------------------------------------------------------------------------

CairoFont::CairoFont(Ref refA, cairo_font_face_t *cairo_font_faceA, std::vector<int> &&codeToGIDA, bool substituteA, bool printingA) : ref(refA), cairo_font_face(cairo_font_faceA), substitute(substituteA), printing(printingA)
{
    codeToGID = std::move(codeToGIDA);
}

CairoFont::~CairoFont()
{
    cairo_font_face_destroy(cairo_font_face);
}

bool CairoFont::matches(Ref &other, bool printingA)
{
    return (other == ref);
}

cairo_font_face_t *CairoFont::getFontFace()
{
    return cairo_font_face;
}

unsigned long CairoFont::getGlyph(CharCode code, const Unicode *u, int uLen)
{
    FT_UInt gid;

    if (code < codeToGID.size()) {
        gid = (FT_UInt)codeToGID[code];
    } else {
        gid = (FT_UInt)code;
    }
    return gid;
}

double CairoFont::getSubstitutionCorrection(const std::shared_ptr<GfxFont> &gfxFont)
{
    double w1, w2, w3;
    CharCode code;
    const char *name;

    // for substituted fonts: adjust the font matrix -- compare the
    // width of 'm' in the original font and the substituted font
    if (isSubstitute() && !gfxFont->isCIDFont()) {
        for (code = 0; code < 256; ++code) {
            if ((name = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getCharName(code)) && name[0] == 'm' && name[1] == '\0') {
                break;
            }
        }
        if (code < 256) {
            w1 = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getWidth(code);
            {
                cairo_matrix_t m;
                cairo_matrix_init_identity(&m);
                cairo_font_options_t *options = cairo_font_options_create();
                cairo_font_options_set_hint_style(options, CAIRO_HINT_STYLE_NONE);
                cairo_font_options_set_hint_metrics(options, CAIRO_HINT_METRICS_OFF);
                cairo_scaled_font_t *scaled_font = cairo_scaled_font_create(cairo_font_face, &m, &m, options);

                cairo_text_extents_t extents;
                cairo_scaled_font_text_extents(scaled_font, "m", &extents);

                cairo_scaled_font_destroy(scaled_font);
                cairo_font_options_destroy(options);
                w2 = extents.x_advance;
            }
            w3 = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getWidth(0);
            if (!gfxFont->isSymbolic() && w2 > 0 && w1 > w3) {
                // if real font is substantially narrower than substituted
                // font, reduce the font size accordingly
                if (w1 > 0.01 && w1 < 0.9 * w2) {
                    w1 /= w2;
                    return w1;
                }
            }
        }
    }
    return 1.0;
}

//------------------------------------------------------------------------
// CairoFreeTypeFont
//------------------------------------------------------------------------

static cairo_user_data_key_t ft_cairo_key;

// Font resources to be freed when cairo_font_face_t is destroyed
struct FreeTypeFontResource
{
    FT_Face face;
    std::vector<unsigned char> font_data;
};

// cairo callback for when cairo_font_face_t is destroyed
static void _ft_done_face(void *closure)
{
    FreeTypeFontResource *resource = (FreeTypeFontResource *)closure;

    FT_Done_Face(resource->face);
    delete resource;
}

CairoFreeTypeFont::CairoFreeTypeFont(Ref refA, cairo_font_face_t *cairo_font_faceA, std::vector<int> &&codeToGIDA, bool substituteA) : CairoFont(refA, cairo_font_faceA, std::move(codeToGIDA), substituteA, true) { }

CairoFreeTypeFont::~CairoFreeTypeFont() { }

// Create a cairo_font_face_t for the given font filename OR font data.
static std::optional<FreeTypeFontFace> createFreeTypeFontFace(FT_Library lib, const std::string &filename, std::vector<unsigned char> &&font_data)
{
    FreeTypeFontResource *resource = new FreeTypeFontResource;
    FreeTypeFontFace font_face;

    if (font_data.empty()) {
        FT_Error err = ft_new_face_from_file(lib, filename.c_str(), 0, &resource->face);
        if (err) {
            delete resource;
            return {};
        }
    } else {
        resource->font_data = std::move(font_data);
        FT_Error err = FT_New_Memory_Face(lib, (FT_Byte *)resource->font_data.data(), resource->font_data.size(), 0, &resource->face);
        if (err) {
            delete resource;
            return {};
        }
    }

    font_face.cairo_font_face = cairo_ft_font_face_create_for_ft_face(resource->face, FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP);
    if (cairo_font_face_set_user_data(font_face.cairo_font_face, &ft_cairo_key, resource, _ft_done_face)) {
        cairo_font_face_destroy(font_face.cairo_font_face);
        _ft_done_face(resource);
        return {};
    }

    font_face.face = resource->face;
    return font_face;
}

// Create a cairo_font_face_t for the given font filename OR font data. First checks if external font
// is in the cache.
std::optional<FreeTypeFontFace> CairoFreeTypeFont::getFreeTypeFontFace(CairoFontEngine *fontEngine, FT_Library lib, const std::string &filename, std::vector<unsigned char> &&font_data)
{
    if (font_data.empty()) {
        return fontEngine->getExternalFontFace(lib, filename);
    }

    return createFreeTypeFontFace(lib, filename, std::move(font_data));
}

CairoFreeTypeFont *CairoFreeTypeFont::create(const std::shared_ptr<GfxFont> &gfxFont, XRef *xref, FT_Library lib, CairoFontEngine *fontEngine, bool useCIDs)
{
    std::string fileName;
    std::vector<unsigned char> font_data;
    int i, n;
    std::optional<GfxFontLoc> fontLoc;
    char **enc;
    const char *name;
    FoFiType1C *ff1c;
    std::optional<FreeTypeFontFace> font_face;
    std::vector<int> codeToGID;
    bool substitute = false;

    Ref ref = *gfxFont->getID();
    Ref embFontID = Ref::INVALID();
    gfxFont->getEmbeddedFontID(&embFontID);
    GfxFontType fontType = gfxFont->getType();

    if (!(fontLoc = gfxFont->locateFont(xref, nullptr))) {
        error(errSyntaxError, -1, "Couldn't find a font for '{0:s}'", gfxFont->getName() ? gfxFont->getName()->c_str() : "(unnamed)");
        goto err2;
    }

    // embedded font
    if (fontLoc->locType == gfxFontLocEmbedded) {
        auto fd = gfxFont->readEmbFontFile(xref);
        if (!fd || fd->empty()) {
            goto err2;
        }
        font_data = std::move(fd.value());

        // external font
    } else { // gfxFontLocExternal
        fileName = fontLoc->path;
        fontType = fontLoc->fontType;
        substitute = true;
    }

    switch (fontType) {
    case fontType1:
    case fontType1C:
    case fontType1COT:
        font_face = getFreeTypeFontFace(fontEngine, lib, fileName, std::move(font_data));
        if (!font_face) {
            error(errSyntaxError, -1, "could not create type1 face");
            goto err2;
        }

        enc = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getEncoding();

        codeToGID.resize(256);
        for (i = 0; i < 256; ++i) {
            codeToGID[i] = 0;
            if ((name = enc[i])) {
                codeToGID[i] = FT_Get_Name_Index(font_face->face, (char *)name);
                if (codeToGID[i] == 0) {
                    Unicode u;
                    u = globalParams->mapNameToUnicodeText(name);
                    codeToGID[i] = FT_Get_Char_Index(font_face->face, u);
                }
                if (codeToGID[i] == 0) {
                    name = GfxFont::getAlternateName(name);
                    if (name) {
                        codeToGID[i] = FT_Get_Name_Index(font_face->face, (char *)name);
                    }
                }
            }
        }
        break;
    case fontCIDType2:
    case fontCIDType2OT:
        if (std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGID()) {
            n = std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGIDLen();
            if (n) {
                const int *src = std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGID();
                codeToGID.reserve(n);
                codeToGID.insert(codeToGID.begin(), src, src + n);
            }
        } else {
            std::unique_ptr<FoFiTrueType> ff;
            if (!font_data.empty()) {
                ff = FoFiTrueType::make(font_data.data(), font_data.size());
            } else {
                ff = FoFiTrueType::load(fileName.c_str());
            }
            if (!ff) {
                goto err2;
            }
            int *src = std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCodeToGIDMap(ff.get(), &n);
            codeToGID.reserve(n);
            codeToGID.insert(codeToGID.begin(), src, src + n);
            gfree(src);
        }
        /* Fall through */
    case fontTrueType:
    case fontTrueTypeOT: {
        std::unique_ptr<FoFiTrueType> ff;
        if (!font_data.empty()) {
            ff = FoFiTrueType::make(font_data.data(), font_data.size());
        } else {
            ff = FoFiTrueType::load(fileName.c_str());
        }
        if (!ff) {
            error(errSyntaxError, -1, "failed to load truetype font\n");
            goto err2;
        }
        /* This might be set already for the CIDType2 case */
        if (fontType == fontTrueType || fontType == fontTrueTypeOT) {
            int *src = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getCodeToGIDMap(ff.get());
            codeToGID.reserve(256);
            codeToGID.insert(codeToGID.begin(), src, src + 256);
            gfree(src);
        }
        font_face = getFreeTypeFontFace(fontEngine, lib, fileName, std::move(font_data));
        if (!font_face) {
            error(errSyntaxError, -1, "could not create truetype face\n");
            goto err2;
        }
        break;
    }
    case fontCIDType0:
    case fontCIDType0C:
        if (!useCIDs) {
            if (!font_data.empty()) {
                ff1c = FoFiType1C::make(font_data.data(), font_data.size());
            } else {
                ff1c = FoFiType1C::load(fileName.c_str());
            }
            if (ff1c) {
                int *src = ff1c->getCIDToGIDMap(&n);
                codeToGID.reserve(n);
                codeToGID.insert(codeToGID.begin(), src, src + n);
                gfree(src);
                delete ff1c;
            }
        }

        font_face = getFreeTypeFontFace(fontEngine, lib, fileName, std::move(font_data));
        if (!font_face) {
            error(errSyntaxError, -1, "could not create cid face\n");
            goto err2;
        }
        break;

    case fontCIDType0COT:
        if (std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGID()) {
            n = std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGIDLen();
            if (n) {
                const int *src = std::static_pointer_cast<GfxCIDFont>(gfxFont)->getCIDToGID();
                codeToGID.reserve(n);
                codeToGID.insert(codeToGID.begin(), src, src + n);
            }
        }

        if (codeToGID.empty()) {
            if (!useCIDs) {
                std::unique_ptr<FoFiTrueType> ff;
                if (!font_data.empty()) {
                    ff = FoFiTrueType::make(font_data.data(), font_data.size());
                } else {
                    ff = FoFiTrueType::load(fileName.c_str());
                }
                if (ff) {
                    if (ff->isOpenTypeCFF()) {
                        int *src = ff->getCIDToGIDMap(&n);
                        codeToGID.reserve(n);
                        codeToGID.insert(codeToGID.begin(), src, src + n);
                        gfree(src);
                    }
                }
            }
        }
        font_face = getFreeTypeFontFace(fontEngine, lib, fileName, std::move(font_data));
        if (!font_face) {
            error(errSyntaxError, -1, "could not create cid (OT) face\n");
            goto err2;
        }
        break;

    default:
        fprintf(stderr, "font type %d not handled\n", (int)fontType);
        goto err2;
        break;
    }

    return new CairoFreeTypeFont(ref, font_face->cairo_font_face, std::move(codeToGID), substitute);

err2:
    fprintf(stderr, "some font thing failed\n");
    return nullptr;
}

//------------------------------------------------------------------------
// CairoType3Font
//------------------------------------------------------------------------

static const cairo_user_data_key_t type3_font_key = { 0 };

typedef struct _type3_font_info
{
    _type3_font_info(const std::shared_ptr<GfxFont> &fontA, PDFDoc *docA, CairoFontEngine *fontEngineA, CairoOutputDev *outputDevA, Gfx *gfxA) : font(fontA), doc(docA), fontEngine(fontEngineA), outputDev(outputDevA), gfx(gfxA) { }

    std::shared_ptr<GfxFont> font;
    PDFDoc *doc;
    CairoFontEngine *fontEngine;
    CairoOutputDev *outputDev;
    Gfx *gfx;
} type3_font_info_t;

static void _free_type3_font_info(void *closure)
{
    type3_font_info_t *info = (type3_font_info_t *)closure;
    delete info->gfx;
    delete info->outputDev;
    delete info;
}

static cairo_status_t _init_type3_glyph(cairo_scaled_font_t *scaled_font, cairo_t *cr, cairo_font_extents_t *extents)
{
    type3_font_info_t *info;

    info = (type3_font_info_t *)cairo_font_face_get_user_data(cairo_scaled_font_get_font_face(scaled_font), &type3_font_key);
    const double *mat = info->font->getFontBBox();
    extents->ascent = mat[3]; /* y2 */
    extents->descent = -mat[3]; /* -y1 */
    extents->height = extents->ascent + extents->descent;
    extents->max_x_advance = mat[2] - mat[1]; /* x2 - x1 */
    extents->max_y_advance = 0;

    return CAIRO_STATUS_SUCCESS;
}

static cairo_status_t _render_type3_glyph(cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics, bool color)
{
    Dict *charProcs;
    Object charProc;
    cairo_matrix_t matrix, invert_y_axis;
    const double *mat;
    double wx, wy;
    type3_font_info_t *info;
    Gfx *gfx;
    cairo_status_t status;

    info = (type3_font_info_t *)cairo_font_face_get_user_data(cairo_scaled_font_get_font_face(scaled_font), &type3_font_key);

    charProcs = std::static_pointer_cast<Gfx8BitFont>(info->font)->getCharProcs();
    if (!charProcs) {
        return CAIRO_STATUS_USER_FONT_ERROR;
    }

    if ((int)glyph >= charProcs->getLength()) {
        return CAIRO_STATUS_USER_FONT_ERROR;
    }

    mat = info->font->getFontMatrix();
    matrix.xx = mat[0];
    matrix.yx = mat[1];
    matrix.xy = mat[2];
    matrix.yy = mat[3];
    matrix.x0 = mat[4];
    matrix.y0 = mat[5];
    cairo_matrix_init_scale(&invert_y_axis, 1, -1);
    cairo_matrix_multiply(&matrix, &matrix, &invert_y_axis);
    cairo_transform(cr, &matrix);

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 18, 0)
    cairo_set_source(cr, cairo_user_scaled_font_get_foreground_marker(scaled_font));
#endif

    CairoOutputDev *output_dev = info->outputDev;
    output_dev->setCairo(cr);

    gfx = info->gfx;
    gfx->saveState();

    output_dev->startDoc(info->doc, info->fontEngine);
    output_dev->startType3Render(gfx->getState(), gfx->getXRef());
    output_dev->setType3RenderType(color ? CairoOutputDev::Type3RenderColor : CairoOutputDev::Type3RenderMask);
    charProc = charProcs->getVal(glyph);
    if (!charProc.isStream()) {
        return CAIRO_STATUS_USER_FONT_ERROR;
    }
    Object charProcResObject = charProc.streamGetDict()->lookup("Resources");
    if (charProcResObject.isDict()) {
        gfx->pushResources(charProcResObject.getDict());
    }
    gfx->display(&charProc);
    if (charProcResObject.isDict()) {
        gfx->popResources();
    }

    output_dev->getType3GlyphWidth(&wx, &wy);
    cairo_matrix_transform_distance(&matrix, &wx, &wy);
    metrics->x_advance = wx;
    metrics->y_advance = wy;
    if (output_dev->hasType3GlyphBBox()) {
        double *bbox = output_dev->getType3GlyphBBox();

        cairo_matrix_transform_point(&matrix, &bbox[0], &bbox[1]);
        cairo_matrix_transform_point(&matrix, &bbox[2], &bbox[3]);
        metrics->x_bearing = bbox[0];
        metrics->y_bearing = bbox[1];
        metrics->width = bbox[2] - bbox[0];
        metrics->height = bbox[3] - bbox[1];
    }

    status = CAIRO_STATUS_SUCCESS;

    // If this is a render color glyph callback but the Type 3 glyph
    // specified non-color, return NOT_IMPLEMENTED. Cairo will then
    // call the render non-color glyph callback.
    if (color && !output_dev->type3GlyphHasColor()) {
        status = CAIRO_STATUS_USER_FONT_NOT_IMPLEMENTED;
    }

    return status;
}

#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 18, 0)
static cairo_status_t _render_type3_color_glyph(cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics)
{
    return _render_type3_glyph(scaled_font, glyph, cr, metrics, true);
}
#endif

static cairo_status_t _render_type3_noncolor_glyph(cairo_scaled_font_t *scaled_font, unsigned long glyph, cairo_t *cr, cairo_text_extents_t *metrics)
{
    return _render_type3_glyph(scaled_font, glyph, cr, metrics, false);
}

CairoType3Font *CairoType3Font::create(const std::shared_ptr<GfxFont> &gfxFont, PDFDoc *doc, CairoFontEngine *fontEngine, bool printing, XRef *xref)
{
    std::vector<int> codeToGID;
    char *name;
    const double *mat;

    Dict *charProcs = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getCharProcs();
    Ref ref = *gfxFont->getID();
    cairo_font_face_t *font_face = cairo_user_font_face_create();
    cairo_user_font_face_set_init_func(font_face, _init_type3_glyph);
#if CAIRO_VERSION >= CAIRO_VERSION_ENCODE(1, 18, 0)
    // When both callbacks are set, Cairo will call the color glyph
    // callback first.  If that returns NOT_IMPLEMENTED, Cairo will
    // then call the non-color glyph callback.
    cairo_user_font_face_set_render_color_glyph_func(font_face, _render_type3_color_glyph);
#endif
    cairo_user_font_face_set_render_glyph_func(font_face, _render_type3_noncolor_glyph);

    CairoOutputDev *output_dev = new CairoOutputDev();
    output_dev->setPrinting(printing);

    Dict *resDict = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getResources();
    mat = gfxFont->getFontBBox();
    PDFRectangle box;
    box.x1 = mat[0];
    box.y1 = mat[1];
    box.x2 = mat[2];
    box.y2 = mat[3];
    Gfx *gfx = new Gfx(doc, output_dev, resDict, &box, nullptr);

    type3_font_info_t *info = new type3_font_info_t(gfxFont, doc, fontEngine, output_dev, gfx);
    cairo_font_face_set_user_data(font_face, &type3_font_key, (void *)info, _free_type3_font_info);

    char **enc = std::static_pointer_cast<Gfx8BitFont>(gfxFont)->getEncoding();
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

    return new CairoType3Font(ref, font_face, std::move(codeToGID), printing, xref);
}

CairoType3Font::CairoType3Font(Ref refA, cairo_font_face_t *cairo_font_faceA, std::vector<int> &&codeToGIDA, bool printingA, XRef *xref) : CairoFont(refA, cairo_font_faceA, std::move(codeToGIDA), false, printingA) { }

CairoType3Font::~CairoType3Font() { }

bool CairoType3Font::matches(Ref &other, bool printingA)
{
    return (other == ref && printing == printingA);
}

//------------------------------------------------------------------------
// CairoFontEngine
//------------------------------------------------------------------------

std::unordered_map<std::string, FreeTypeFontFace> CairoFontEngine::fontFileCache;
std::recursive_mutex CairoFontEngine::fontFileCacheMutex;

CairoFontEngine::CairoFontEngine(FT_Library libA)
{
    lib = libA;
    fontCache.reserve(cairoFontCacheSize);

    FT_Int major, minor, patch;
    // as of FT 2.1.8, CID fonts are indexed by CID instead of GID
    FT_Library_Version(lib, &major, &minor, &patch);
    useCIDs = major > 2 || (major == 2 && (minor > 1 || (minor == 1 && patch > 7)));
}

CairoFontEngine::~CairoFontEngine() { }

std::shared_ptr<CairoFont> CairoFontEngine::getFont(const std::shared_ptr<GfxFont> &gfxFont, PDFDoc *doc, bool printing, XRef *xref)
{
    std::scoped_lock lock(mutex);
    Ref ref = *gfxFont->getID();
    std::shared_ptr<CairoFont> font;

    // Check if font is in the MRU cache, and move it to the end if it is.
    for (auto it = fontCache.rbegin(); it != fontCache.rend(); ++it) {
        if ((*it)->matches(ref, printing)) {
            font = *it;
            // move it to the end
            if (it != fontCache.rbegin()) {
                // https://stackoverflow.com/questions/1830158/how-to-call-erase-with-a-reverse-iterator
                fontCache.erase(std::next(it).base());
                fontCache.push_back(font);
            }
            return font;
        }
    }

    GfxFontType fontType = gfxFont->getType();
    if (fontType == fontType3) {
        font = std::shared_ptr<CairoFont>(CairoType3Font::create(gfxFont, doc, this, printing, xref));
    } else {
        font = std::shared_ptr<CairoFont>(CairoFreeTypeFont::create(gfxFont, xref, lib, this, useCIDs));
    }

    if (font) {
        if (fontCache.size() == cairoFontCacheSize) {
            fontCache.erase(fontCache.begin());
        }
        fontCache.push_back(font);
    }
    return font;
}

std::optional<FreeTypeFontFace> CairoFontEngine::getExternalFontFace(FT_Library ftlib, const std::string &filename)
{
    std::scoped_lock lock(fontFileCacheMutex);

    auto it = fontFileCache.find(filename);
    if (it != fontFileCache.end()) {
        FreeTypeFontFace font = it->second;
        cairo_font_face_reference(font.cairo_font_face);
        return font;
    }

    std::optional<FreeTypeFontFace> font_face = createFreeTypeFontFace(ftlib, filename, {});
    if (font_face) {
        cairo_font_face_reference(font_face->cairo_font_face);
        fontFileCache[filename] = *font_face;
    }

    it = fontFileCache.begin();
    while (it != fontFileCache.end()) {
        if (cairo_font_face_get_reference_count(it->second.cairo_font_face) == 1) {
            cairo_font_face_destroy(it->second.cairo_font_face);
            it = fontFileCache.erase(it);
        } else {
            ++it;
        }
    }

    return font_face;
}
