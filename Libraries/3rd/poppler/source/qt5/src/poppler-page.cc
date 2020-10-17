/* poppler-page.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2020, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2005, Stefan Kebekus <stefan.kebekus@math.uni-koeln.de>
 * Copyright (C) 2006-2011, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2009 Shawn Rutledge <shawn.t.rutledge@gmail.com>
 * Copyright (C) 2010, 2012, Guillermo Amaral <gamaral@kdab.com>
 * Copyright (C) 2010 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
 * Copyright (C) 2010 Matthias Fauconneau <matthias.fauconneau@gmail.com>
 * Copyright (C) 2010 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012 Tobias Koenig <tokoe@kdab.com>
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2012, 2015 Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
 * Copyright (C) 2015 William Bader <williambader@hotmail.com>
 * Copyright (C) 2016 Arseniy Lartsev <arseniy@alumni.chalmers.se>
 * Copyright (C) 2016, Hanno Meyer-Thurow <h.mth@web.de>
 * Copyright (C) 2017-2020, Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
 * Copyright (C) 2017, 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
 * Copyright (C) 2018 Intevation GmbH <intevation@intevation.de>
 * Copyright (C) 2018, Tobias Deiminger <haxtibal@posteo.de>
 * Copyright (C) 2018 Nelson Benítez León <nbenitezl@gmail.com>
 * Copyright (C) 2020 Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2020 Philipp Knechtges <philipp-dev@knechtges.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.
 */

#if defined(MIKTEX_WINDOWS)
#  define MIKTEX_UTF8_WRAP_ALL 1
#  include <miktex/utf8wrap.h>
#endif
#include <poppler-qt5.h>

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QVarLengthArray>
#include <QtGui/QImage>
#include <QtGui/QPainter>

#include <config.h>
#include <PDFDoc.h>
#include <Catalog.h>
#include <Form.h>
#include <ErrorCodes.h>
#include <TextOutputDev.h>
#include <Annot.h>
#include <Link.h>
#include <ArthurOutputDev.h>
#include <Rendition.h>
#if defined(HAVE_SPLASH)
#    include <SplashOutputDev.h>
#    include <splash/SplashBitmap.h>
#endif

#include "poppler-private.h"
#include "poppler-page-transition-private.h"
#include "poppler-page-private.h"
#include "poppler-link-extractor-private.h"
#include "poppler-link-private.h"
#include "poppler-annotation-private.h"
#include "poppler-form.h"
#include "poppler-media.h"

namespace Poppler {

class TextExtractionAbortHelper
{
public:
    TextExtractionAbortHelper(Page::ShouldAbortQueryFunc shouldAbortCallback, const QVariant &payloadA)
    {
        shouldAbortExtractionCallback = shouldAbortCallback;
        payload = payloadA;
    }

    Page::ShouldAbortQueryFunc shouldAbortExtractionCallback = nullptr;
    QVariant payload;
};

class OutputDevCallbackHelper
{
public:
    void setCallbacks(Page::RenderToImagePartialUpdateFunc callback, Page::ShouldRenderToImagePartialQueryFunc shouldDoCallback, Page::ShouldAbortQueryFunc shouldAbortCallback, const QVariant &payloadA)
    {
        partialUpdateCallback = callback;
        shouldDoPartialUpdateCallback = shouldDoCallback;
        shouldAbortRenderCallback = shouldAbortCallback;
        payload = payloadA;
    }

    Page::RenderToImagePartialUpdateFunc partialUpdateCallback = nullptr;
    Page::ShouldRenderToImagePartialQueryFunc shouldDoPartialUpdateCallback = nullptr;
    Page::ShouldAbortQueryFunc shouldAbortRenderCallback = nullptr;
    QVariant payload;
};

class Qt5SplashOutputDev : public SplashOutputDev, public OutputDevCallbackHelper
{
public:
    Qt5SplashOutputDev(SplashColorMode colorModeA, int bitmapRowPadA, bool reverseVideoA, bool ignorePaperColorA, SplashColorPtr paperColorA, bool bitmapTopDownA, SplashThinLineMode thinLineMode, bool overprintPreviewA)
        : SplashOutputDev(colorModeA, bitmapRowPadA, reverseVideoA, paperColorA, bitmapTopDownA, thinLineMode, overprintPreviewA), ignorePaperColor(ignorePaperColorA)
    {
    }

    void dump() override
    {
        if (partialUpdateCallback && shouldDoPartialUpdateCallback && shouldDoPartialUpdateCallback(payload)) {
            partialUpdateCallback(getXBGRImage(false /* takeImageData */), payload);
        }
    }

    QImage getXBGRImage(bool takeImageData)
    {
        SplashBitmap *b = getBitmap();

        const int bw = b->getWidth();
        const int bh = b->getHeight();
        const int brs = b->getRowSize();

        // If we use DeviceN8, convert to XBGR8.
        // If requested, also transfer Splash's internal alpha channel.
        const SplashBitmap::ConversionMode mode = ignorePaperColor ? SplashBitmap::conversionAlphaPremultiplied : SplashBitmap::conversionOpaque;

        const QImage::Format format = ignorePaperColor ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;

        if (b->convertToXBGR(mode)) {
            SplashColorPtr data = takeImageData ? b->takeData() : b->getDataPtr();

            if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                // Convert byte order from RGBX to XBGR.
                for (int i = 0; i < bh; ++i) {
                    for (int j = 0; j < bw; ++j) {
                        SplashColorPtr pixel = &data[i * brs + j];

                        qSwap(pixel[0], pixel[3]);
                        qSwap(pixel[1], pixel[2]);
                    }
                }
            }

            if (takeImageData) {
                // Construct a Qt image holding (and also owning) the raw bitmap data.
                return QImage(data, bw, bh, brs, format, gfree, data);
            } else {
                return QImage(data, bw, bh, brs, format).copy();
            }
        }

        return QImage();
    }

private:
    bool ignorePaperColor;
};

class QImageDumpingArthurOutputDev : public ArthurOutputDev, public OutputDevCallbackHelper
{
public:
    QImageDumpingArthurOutputDev(QPainter *painter, QImage *i) : ArthurOutputDev(painter), image(i) { }

    void dump() override
    {
        if (partialUpdateCallback && shouldDoPartialUpdateCallback && shouldDoPartialUpdateCallback(payload)) {
            partialUpdateCallback(*image, payload);
        }
    }

private:
    QImage *image;
};

Link *PageData::convertLinkActionToLink(::LinkAction *a, const QRectF &linkArea)
{
    return convertLinkActionToLink(a, parentDoc, linkArea);
}

Link *PageData::convertLinkActionToLink(::LinkAction *a, DocumentData *parentDoc, const QRectF &linkArea)
{
    if (!a)
        return nullptr;

    Link *popplerLink = nullptr;
    switch (a->getKind()) {
    case actionGoTo: {
        LinkGoTo *g = (LinkGoTo *)a;
        const LinkDestinationData ldd(g->getDest(), g->getNamedDest(), parentDoc, false);
        // create link: no ext file, namedDest, object pointer
        popplerLink = new LinkGoto(linkArea, QString(), LinkDestination(ldd));
    } break;

    case actionGoToR: {
        LinkGoToR *g = (LinkGoToR *)a;
        // copy link file
        const QString fileName = UnicodeParsedString(g->getFileName());
        const LinkDestinationData ldd(g->getDest(), g->getNamedDest(), parentDoc, !fileName.isEmpty());
        // create link: fileName, namedDest, object pointer
        popplerLink = new LinkGoto(linkArea, fileName, LinkDestination(ldd));
    } break;

    case actionLaunch: {
        LinkLaunch *e = (LinkLaunch *)a;
        const GooString *p = e->getParams();
        popplerLink = new LinkExecute(linkArea, e->getFileName()->c_str(), p ? p->c_str() : nullptr);
    } break;

    case actionNamed: {
        const std::string &name = ((LinkNamed *)a)->getName();
        if (name == "NextPage")
            popplerLink = new LinkAction(linkArea, LinkAction::PageNext);
        else if (name == "PrevPage")
            popplerLink = new LinkAction(linkArea, LinkAction::PagePrev);
        else if (name == "FirstPage")
            popplerLink = new LinkAction(linkArea, LinkAction::PageFirst);
        else if (name == "LastPage")
            popplerLink = new LinkAction(linkArea, LinkAction::PageLast);
        else if (name == "GoBack")
            popplerLink = new LinkAction(linkArea, LinkAction::HistoryBack);
        else if (name == "GoForward")
            popplerLink = new LinkAction(linkArea, LinkAction::HistoryForward);
        else if (name == "Quit")
            popplerLink = new LinkAction(linkArea, LinkAction::Quit);
        else if (name == "GoToPage")
            popplerLink = new LinkAction(linkArea, LinkAction::GoToPage);
        else if (name == "Find")
            popplerLink = new LinkAction(linkArea, LinkAction::Find);
        else if (name == "FullScreen")
            popplerLink = new LinkAction(linkArea, LinkAction::Presentation);
        else if (name == "Print")
            popplerLink = new LinkAction(linkArea, LinkAction::Print);
        else if (name == "Close") {
            // acroread closes the document always, doesnt care whether
            // its presentation mode or not
            // popplerLink = new LinkAction( linkArea, LinkAction::EndPresentation );
            popplerLink = new LinkAction(linkArea, LinkAction::Close);
        } else {
            // TODO
        }
    } break;

    case actionURI: {
        popplerLink = new LinkBrowse(linkArea, ((LinkURI *)a)->getURI().c_str());
    } break;

    case actionSound: {
        ::LinkSound *ls = (::LinkSound *)a;
        popplerLink = new LinkSound(linkArea, ls->getVolume(), ls->getSynchronous(), ls->getRepeat(), ls->getMix(), new SoundObject(ls->getSound()));
    } break;

    case actionJavaScript: {
        ::LinkJavaScript *ljs = (::LinkJavaScript *)a;
        popplerLink = new LinkJavaScript(linkArea, UnicodeParsedString(ljs->getScript()));
    } break;

    case actionMovie: {
        ::LinkMovie *lm = (::LinkMovie *)a;

        const QString title = (lm->hasAnnotTitle() ? UnicodeParsedString(lm->getAnnotTitle()) : QString());

        Ref reference = Ref::INVALID();
        if (lm->hasAnnotRef())
            reference = *lm->getAnnotRef();

        LinkMovie::Operation operation = LinkMovie::Play;
        switch (lm->getOperation()) {
        case ::LinkMovie::operationTypePlay:
            operation = LinkMovie::Play;
            break;
        case ::LinkMovie::operationTypePause:
            operation = LinkMovie::Pause;
            break;
        case ::LinkMovie::operationTypeResume:
            operation = LinkMovie::Resume;
            break;
        case ::LinkMovie::operationTypeStop:
            operation = LinkMovie::Stop;
            break;
        };

        popplerLink = new LinkMovie(linkArea, operation, title, reference);
    } break;

    case actionRendition: {
        ::LinkRendition *lrn = (::LinkRendition *)a;

        Ref reference = Ref::INVALID();
        if (lrn->hasScreenAnnot())
            reference = lrn->getScreenAnnot();

        popplerLink = new LinkRendition(linkArea, lrn->getMedia() ? lrn->getMedia()->copy() : nullptr, lrn->getOperation(), UnicodeParsedString(lrn->getScript()), reference);
    } break;

    case actionOCGState: {
        ::LinkOCGState *plocg = (::LinkOCGState *)a;

        LinkOCGStatePrivate *locgp = new LinkOCGStatePrivate(linkArea, plocg->getStateList(), plocg->getPreserveRB());
        popplerLink = new LinkOCGState(locgp);
    } break;

    case actionHide: {
        ::LinkHide *lh = (::LinkHide *)a;

        LinkHidePrivate *lhp = new LinkHidePrivate(linkArea, lh->hasTargetName() ? UnicodeParsedString(lh->getTargetName()) : QString(), lh->isShowAction());
        popplerLink = new LinkHide(lhp);
    } break;

    case actionResetForm:
        // Not handled in Qt5 front-end yet
        break;

    case actionUnknown:
        break;
    }

    if (popplerLink) {
        QVector<Link *> links;
        for (const std::unique_ptr<::LinkAction> &nextAction : a->nextActions()) {
            links << convertLinkActionToLink(nextAction.get(), parentDoc, linkArea);
        }
        LinkPrivate::get(popplerLink)->nextLinks = links;
    }

    return popplerLink;
}

inline TextPage *PageData::prepareTextSearch(const QString &text, Page::Rotation rotate, QVector<Unicode> *u)
{
    *u = text.toUcs4();

    const int rotation = (int)rotate * 90;

    // fetch ourselves a textpage
    TextOutputDev td(nullptr, true, 0, false, false);
    parentDoc->doc->displayPage(&td, index + 1, 72, 72, rotation, false, true, false, nullptr, nullptr, nullptr, nullptr, true);
    TextPage *textPage = td.takeText();

    return textPage;
}

inline bool PageData::performSingleTextSearch(TextPage *textPage, QVector<Unicode> &u, double &sLeft, double &sTop, double &sRight, double &sBottom, Page::SearchDirection direction, bool sCase, bool sWords, bool sDiacritics = false)
{
    if (direction == Page::FromTop)
        return textPage->findText(u.data(), u.size(), true, true, false, false, sCase, sDiacritics, false, sWords, &sLeft, &sTop, &sRight, &sBottom);
    else if (direction == Page::NextResult)
        return textPage->findText(u.data(), u.size(), false, true, true, false, sCase, sDiacritics, false, sWords, &sLeft, &sTop, &sRight, &sBottom);
    else if (direction == Page::PreviousResult)
        return textPage->findText(u.data(), u.size(), false, true, true, false, sCase, sDiacritics, true, sWords, &sLeft, &sTop, &sRight, &sBottom);

    return false;
}

inline QList<QRectF> PageData::performMultipleTextSearch(TextPage *textPage, QVector<Unicode> &u, bool sCase, bool sWords, bool sDiacritics = false)
{
    QList<QRectF> results;
    double sLeft = 0.0, sTop = 0.0, sRight = 0.0, sBottom = 0.0;

    while (textPage->findText(u.data(), u.size(), false, true, true, false, sCase, sDiacritics, false, sWords, &sLeft, &sTop, &sRight, &sBottom)) {
        QRectF result;

        result.setLeft(sLeft);
        result.setTop(sTop);
        result.setRight(sRight);
        result.setBottom(sBottom);

        results.append(result);
    }

    return results;
}

Page::Page(DocumentData *doc, int index)
{
    m_page = new PageData();
    m_page->index = index;
    m_page->parentDoc = doc;
    m_page->page = doc->doc->getPage(m_page->index + 1);
    m_page->transition = nullptr;
}

Page::~Page()
{
    delete m_page->transition;
    delete m_page;
}

// Callback that filters out everything but form fields
static auto annotDisplayDecideCbk = [](Annot *annot, void *user_data) {
    // Hide everything but forms
    return (annot->getType() == Annot::typeWidget);
};

// A nullptr, but with the type of a function pointer
// Needed to make the ternary operator happy.
static bool (*nullAnnotCallBack)(Annot *annot, void *user_data) = nullptr;

static auto shouldAbortRenderInternalCallback = [](void *user_data) {
    OutputDevCallbackHelper *helper = reinterpret_cast<OutputDevCallbackHelper *>(user_data);
    return helper->shouldAbortRenderCallback(helper->payload);
};

static auto shouldAbortExtractionInternalCallback = [](void *user_data) {
    TextExtractionAbortHelper *helper = reinterpret_cast<TextExtractionAbortHelper *>(user_data);
    return helper->shouldAbortExtractionCallback(helper->payload);
};

// A nullptr, but with the type of a function pointer
// Needed to make the ternary operator happy.
static bool (*nullAbortCallBack)(void *user_data) = nullptr;

static bool renderToArthur(QImageDumpingArthurOutputDev *arthur_output, QPainter *painter, PageData *page, double xres, double yres, int x, int y, int w, int h, Page::Rotation rotate, Page::PainterFlags flags)
{
    const bool savePainter = !(flags & Page::DontSaveAndRestore);
    if (savePainter)
        painter->save();
    if (page->parentDoc->m_hints & Document::Antialiasing)
        painter->setRenderHint(QPainter::Antialiasing);
    if (page->parentDoc->m_hints & Document::TextAntialiasing)
        painter->setRenderHint(QPainter::TextAntialiasing);
    painter->translate(x == -1 ? 0 : -x, y == -1 ? 0 : -y);

    arthur_output->startDoc(page->parentDoc->doc);

    const bool hideAnnotations = page->parentDoc->m_hints & Document::HideAnnotations;

    OutputDevCallbackHelper *abortHelper = arthur_output;
    page->parentDoc->doc->displayPageSlice(arthur_output, page->index + 1, xres, yres, (int)rotate * 90, false, true, false, x, y, w, h, abortHelper->shouldAbortRenderCallback ? shouldAbortRenderInternalCallback : nullAbortCallBack,
                                           abortHelper, (hideAnnotations) ? annotDisplayDecideCbk : nullAnnotCallBack, nullptr, true);
    if (savePainter)
        painter->restore();
    return true;
}

QImage Page::renderToImage(double xres, double yres, int x, int y, int w, int h, Rotation rotate) const
{
    return renderToImage(xres, yres, x, y, w, h, rotate, nullptr, nullptr, QVariant());
}

QImage Page::renderToImage(double xres, double yres, int x, int y, int w, int h, Rotation rotate, RenderToImagePartialUpdateFunc partialUpdateCallback, ShouldRenderToImagePartialQueryFunc shouldDoPartialUpdateCallback,
                           const QVariant &payload) const
{
    return renderToImage(xres, yres, x, y, w, h, rotate, partialUpdateCallback, shouldDoPartialUpdateCallback, nullptr, payload);
}

// Translate the text hinting settings from poppler-speak to Qt-speak
static QFont::HintingPreference QFontHintingFromPopplerHinting(int renderHints)
{
    QFont::HintingPreference result = QFont::PreferNoHinting;

    if (renderHints & Document::TextHinting) {
        result = (renderHints & Document::TextSlightHinting) ? QFont::PreferVerticalHinting : QFont::PreferFullHinting;
    }

    return result;
}

QImage Page::renderToImage(double xres, double yres, int xPos, int yPos, int w, int h, Rotation rotate, RenderToImagePartialUpdateFunc partialUpdateCallback, ShouldRenderToImagePartialQueryFunc shouldDoPartialUpdateCallback,
                           ShouldAbortQueryFunc shouldAbortRenderCallback, const QVariant &payload) const
{
    int rotation = (int)rotate * 90;
    QImage img;
    switch (m_page->parentDoc->m_backend) {
    case Poppler::Document::SplashBackend: {
#if defined(HAVE_SPLASH)
        SplashColor bgColor;
        const bool overprintPreview = m_page->parentDoc->m_hints & Document::OverprintPreview ? true : false;
        if (overprintPreview) {
            unsigned char c, m, y, k;

            c = 255 - m_page->parentDoc->paperColor.blue();
            m = 255 - m_page->parentDoc->paperColor.red();
            y = 255 - m_page->parentDoc->paperColor.green();
            k = c;
            if (m < k) {
                k = m;
            }
            if (y < k) {
                k = y;
            }
            bgColor[0] = c - k;
            bgColor[1] = m - k;
            bgColor[2] = y - k;
            bgColor[3] = k;
            for (int i = 4; i < SPOT_NCOMPS + 4; i++) {
                bgColor[i] = 0;
            }
        } else {
            bgColor[0] = m_page->parentDoc->paperColor.blue();
            bgColor[1] = m_page->parentDoc->paperColor.green();
            bgColor[2] = m_page->parentDoc->paperColor.red();
        }

        const SplashColorMode colorMode = overprintPreview ? splashModeDeviceN8 : splashModeXBGR8;

        SplashThinLineMode thinLineMode = splashThinLineDefault;
        if (m_page->parentDoc->m_hints & Document::ThinLineShape)
            thinLineMode = splashThinLineShape;
        if (m_page->parentDoc->m_hints & Document::ThinLineSolid)
            thinLineMode = splashThinLineSolid;

        const bool ignorePaperColor = m_page->parentDoc->m_hints & Document::IgnorePaperColor;

        Qt5SplashOutputDev splash_output(colorMode, 4, false, ignorePaperColor, ignorePaperColor ? nullptr : bgColor, true, thinLineMode, overprintPreview);

        splash_output.setCallbacks(partialUpdateCallback, shouldDoPartialUpdateCallback, shouldAbortRenderCallback, payload);

        splash_output.setFontAntialias(m_page->parentDoc->m_hints & Document::TextAntialiasing ? true : false);
        splash_output.setVectorAntialias(m_page->parentDoc->m_hints & Document::Antialiasing ? true : false);
        splash_output.setFreeTypeHinting(m_page->parentDoc->m_hints & Document::TextHinting ? true : false, m_page->parentDoc->m_hints & Document::TextSlightHinting ? true : false);

#    ifdef USE_CMS
        splash_output.setDisplayProfile(m_page->parentDoc->m_displayProfile);
#    endif

        splash_output.startDoc(m_page->parentDoc->doc);

        const bool hideAnnotations = m_page->parentDoc->m_hints & Document::HideAnnotations;

        OutputDevCallbackHelper *abortHelper = &splash_output;
        m_page->parentDoc->doc->displayPageSlice(&splash_output, m_page->index + 1, xres, yres, rotation, false, true, false, xPos, yPos, w, h, shouldAbortRenderCallback ? shouldAbortRenderInternalCallback : nullAbortCallBack, abortHelper,
                                                 (hideAnnotations) ? annotDisplayDecideCbk : nullAnnotCallBack, nullptr, true);

        img = splash_output.getXBGRImage(true /* takeImageData */);
#endif
        break;
    }
    case Poppler::Document::ArthurBackend: {
        QSize size = pageSize();
        QImage tmpimg(w == -1 ? qRound(size.width() * xres / 72.0) : w, h == -1 ? qRound(size.height() * yres / 72.0) : h, QImage::Format_ARGB32);

        QColor bgColor(m_page->parentDoc->paperColor.red(), m_page->parentDoc->paperColor.green(), m_page->parentDoc->paperColor.blue(), m_page->parentDoc->paperColor.alpha());

        tmpimg.fill(bgColor);

        QPainter painter(&tmpimg);
        QImageDumpingArthurOutputDev arthur_output(&painter, &tmpimg);

        arthur_output.setHintingPreference(QFontHintingFromPopplerHinting(m_page->parentDoc->m_hints));

#ifdef USE_CMS
        arthur_output.setDisplayProfile(m_page->parentDoc->m_displayProfile);
#endif

        arthur_output.setCallbacks(partialUpdateCallback, shouldDoPartialUpdateCallback, shouldAbortRenderCallback, payload);
        renderToArthur(&arthur_output, &painter, m_page, xres, yres, xPos, yPos, w, h, rotate, DontSaveAndRestore);
        painter.end();
        img = tmpimg;
        break;
    }
    }

    if (shouldAbortRenderCallback && shouldAbortRenderCallback(payload))
        return QImage();

    return img;
}

bool Page::renderToPainter(QPainter *painter, double xres, double yres, int x, int y, int w, int h, Rotation rotate, PainterFlags flags) const
{
    if (!painter)
        return false;

    switch (m_page->parentDoc->m_backend) {
    case Poppler::Document::SplashBackend:
        return false;
    case Poppler::Document::ArthurBackend: {
        QImageDumpingArthurOutputDev arthur_output(painter, nullptr);

        arthur_output.setHintingPreference(QFontHintingFromPopplerHinting(m_page->parentDoc->m_hints));

        return renderToArthur(&arthur_output, painter, m_page, xres, yres, x, y, w, h, rotate, flags);
    }
    }
    return false;
}

QImage Page::thumbnail() const
{
    unsigned char *data = nullptr;
    int w = 0;
    int h = 0;
    int rowstride = 0;
    bool r = m_page->page->loadThumb(&data, &w, &h, &rowstride);
    QImage ret;
    if (r) {
        // first construct a temporary image with the data got,
        // then force a copy of it so we can free the raw thumbnail data
        ret = QImage(data, w, h, rowstride, QImage::Format_RGB888).copy();
        gfree(data);
    }
    return ret;
}

QString Page::text(const QRectF &r, TextLayout textLayout) const
{
    TextOutputDev *output_dev;
    GooString *s;
    QString result;

    const bool rawOrder = textLayout == RawOrderLayout;
    output_dev = new TextOutputDev(nullptr, false, 0, rawOrder, false);
    m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72, 0, false, true, false, -1, -1, -1, -1, nullptr, nullptr, nullptr, nullptr, true);
    if (r.isNull()) {
        const PDFRectangle *rect = m_page->page->getCropBox();
        s = output_dev->getText(rect->x1, rect->y1, rect->x2, rect->y2);
    } else {
        s = output_dev->getText(r.left(), r.top(), r.right(), r.bottom());
    }

    result = QString::fromUtf8(s->c_str());

    delete output_dev;
    delete s;
    return result;
}

QString Page::text(const QRectF &r) const
{
    return text(r, PhysicalLayout);
}

bool Page::search(const QString &text, double &sLeft, double &sTop, double &sRight, double &sBottom, SearchDirection direction, SearchMode caseSensitive, Rotation rotate) const
{
    const bool sCase = caseSensitive == Page::CaseSensitive ? true : false;

    QVector<Unicode> u;
    TextPage *textPage = m_page->prepareTextSearch(text, rotate, &u);

    const bool found = m_page->performSingleTextSearch(textPage, u, sLeft, sTop, sRight, sBottom, direction, sCase, false);

    textPage->decRefCnt();

    return found;
}

bool Page::search(const QString &text, double &sLeft, double &sTop, double &sRight, double &sBottom, SearchDirection direction, SearchFlags flags, Rotation rotate) const
{
    const bool sCase = flags.testFlag(IgnoreCase) ? false : true;
    const bool sWords = flags.testFlag(WholeWords) ? true : false;
    const bool sDiacritics = flags.testFlag(IgnoreDiacritics) ? true : false;

    QVector<Unicode> u;
    TextPage *textPage = m_page->prepareTextSearch(text, rotate, &u);

    const bool found = m_page->performSingleTextSearch(textPage, u, sLeft, sTop, sRight, sBottom, direction, sCase, sWords, sDiacritics);

    textPage->decRefCnt();

    return found;
}

QList<QRectF> Page::search(const QString &text, SearchMode caseSensitive, Rotation rotate) const
{
    const bool sCase = caseSensitive == Page::CaseSensitive ? true : false;

    QVector<Unicode> u;
    TextPage *textPage = m_page->prepareTextSearch(text, rotate, &u);

    const QList<QRectF> results = m_page->performMultipleTextSearch(textPage, u, sCase, false);

    textPage->decRefCnt();

    return results;
}

QList<QRectF> Page::search(const QString &text, SearchFlags flags, Rotation rotate) const
{
    const bool sCase = flags.testFlag(IgnoreCase) ? false : true;
    const bool sWords = flags.testFlag(WholeWords) ? true : false;
    const bool sDiacritics = flags.testFlag(IgnoreDiacritics) ? true : false;

    QVector<Unicode> u;
    TextPage *textPage = m_page->prepareTextSearch(text, rotate, &u);

    const QList<QRectF> results = m_page->performMultipleTextSearch(textPage, u, sCase, sWords, sDiacritics);

    textPage->decRefCnt();

    return results;
}

QList<TextBox *> Page::textList(Rotation rotate) const
{
    return textList(rotate, nullptr, QVariant());
}

QList<TextBox *> Page::textList(Rotation rotate, ShouldAbortQueryFunc shouldAbortExtractionCallback, const QVariant &closure) const
{
    TextOutputDev *output_dev;

    QList<TextBox *> output_list;

    output_dev = new TextOutputDev(nullptr, false, 0, false, false);

    int rotation = (int)rotate * 90;

    TextExtractionAbortHelper abortHelper(shouldAbortExtractionCallback, closure);
    m_page->parentDoc->doc->displayPageSlice(output_dev, m_page->index + 1, 72, 72, rotation, false, false, false, -1, -1, -1, -1, shouldAbortExtractionCallback ? shouldAbortExtractionInternalCallback : nullAbortCallBack, &abortHelper,
                                             nullptr, nullptr, true);

    TextWordList *word_list = output_dev->makeWordList();

    if (!word_list || (shouldAbortExtractionCallback && shouldAbortExtractionCallback(closure))) {
        delete word_list;
        delete output_dev;
        return output_list;
    }

    QHash<const TextWord *, TextBox *> wordBoxMap;

    output_list.reserve(word_list->getLength());
    for (int i = 0; i < word_list->getLength(); i++) {
        TextWord *word = word_list->get(i);
        GooString *gooWord = word->getText();
        QString string = QString::fromUtf8(gooWord->c_str());
        delete gooWord;
        double xMin, yMin, xMax, yMax;
        word->getBBox(&xMin, &yMin, &xMax, &yMax);

        TextBox *text_box = new TextBox(string, QRectF(xMin, yMin, xMax - xMin, yMax - yMin));
        text_box->m_data->hasSpaceAfter = word->hasSpaceAfter() == true;
        text_box->m_data->charBBoxes.reserve(word->getLength());
        for (int j = 0; j < word->getLength(); ++j) {
            word->getCharBBox(j, &xMin, &yMin, &xMax, &yMax);
            text_box->m_data->charBBoxes.append(QRectF(xMin, yMin, xMax - xMin, yMax - yMin));
        }

        wordBoxMap.insert(word, text_box);

        output_list.append(text_box);
    }

    for (int i = 0; i < word_list->getLength(); i++) {
        TextWord *word = word_list->get(i);
        TextBox *text_box = wordBoxMap.value(word);
        text_box->m_data->nextWord = wordBoxMap.value(word->nextWord());
    }

    delete word_list;
    delete output_dev;

    return output_list;
}

PageTransition *Page::transition() const
{
    if (!m_page->transition) {
        Object o = m_page->page->getTrans();
        PageTransitionParams params;
        params.dictObj = &o;
        if (params.dictObj->isDict())
            m_page->transition = new PageTransition(params);
    }
    return m_page->transition;
}

Link *Page::action(PageAction act) const
{
    if (act == Page::Opening || act == Page::Closing) {
        Object o = m_page->page->getActions();
        if (!o.isDict()) {
            return nullptr;
        }
        Dict *dict = o.getDict();
        const char *key = act == Page::Opening ? "O" : "C";
        Object o2 = dict->lookup((char *)key);
        std::unique_ptr<::LinkAction> lact = ::LinkAction::parseAction(&o2, m_page->parentDoc->doc->getCatalog()->getBaseURI());
        Link *popplerLink = nullptr;
        if (lact != nullptr) {
            popplerLink = m_page->convertLinkActionToLink(lact.get(), QRectF());
        }
        return popplerLink;
    }
    return nullptr;
}

QSizeF Page::pageSizeF() const
{
    Page::Orientation orient = orientation();
    if ((Page::Landscape == orient) || (Page::Seascape == orient)) {
        return QSizeF(m_page->page->getCropHeight(), m_page->page->getCropWidth());
    } else {
        return QSizeF(m_page->page->getCropWidth(), m_page->page->getCropHeight());
    }
}

QSize Page::pageSize() const
{
    return pageSizeF().toSize();
}

Page::Orientation Page::orientation() const
{
    const int rotation = m_page->page->getRotate();
    switch (rotation) {
    case 90:
        return Page::Landscape;
        break;
    case 180:
        return Page::UpsideDown;
        break;
    case 270:
        return Page::Seascape;
        break;
    default:
        return Page::Portrait;
    }
}

void Page::defaultCTM(double *CTM, double dpiX, double dpiY, int rotate, bool upsideDown)
{
    m_page->page->getDefaultCTM(CTM, dpiX, dpiY, rotate, false, upsideDown);
}

QList<Link *> Page::links() const
{
    LinkExtractorOutputDev link_dev(m_page);
    m_page->parentDoc->doc->processLinks(&link_dev, m_page->index + 1);
    QList<Link *> popplerLinks = link_dev.links();

    return popplerLinks;
}

QList<Annotation *> Page::annotations() const
{
    return AnnotationPrivate::findAnnotations(m_page->page, m_page->parentDoc, QSet<Annotation::SubType>());
}

QList<Annotation *> Page::annotations(const QSet<Annotation::SubType> &subtypes) const
{
    return AnnotationPrivate::findAnnotations(m_page->page, m_page->parentDoc, subtypes);
}

void Page::addAnnotation(const Annotation *ann)
{
    AnnotationPrivate::addAnnotationToPage(m_page->page, m_page->parentDoc, ann);
}

void Page::removeAnnotation(const Annotation *ann)
{
    AnnotationPrivate::removeAnnotationFromPage(m_page->page, ann);
}

QList<FormField *> Page::formFields() const
{
    QList<FormField *> fields;
    ::Page *p = m_page->page;
    ::FormPageWidgets *form = p->getFormWidgets();
    int formcount = form->getNumWidgets();
    for (int i = 0; i < formcount; ++i) {
        ::FormWidget *fm = form->getWidget(i);
        FormField *ff = nullptr;
        switch (fm->getType()) {
        case formButton: {
            ff = new FormFieldButton(m_page->parentDoc, p, static_cast<FormWidgetButton *>(fm));
        } break;

        case formText: {
            ff = new FormFieldText(m_page->parentDoc, p, static_cast<FormWidgetText *>(fm));
        } break;

        case formChoice: {
            ff = new FormFieldChoice(m_page->parentDoc, p, static_cast<FormWidgetChoice *>(fm));
        } break;

        case formSignature: {
            ff = new FormFieldSignature(m_page->parentDoc, p, static_cast<FormWidgetSignature *>(fm));
        } break;

        default:;
        }

        if (ff)
            fields.append(ff);
    }

    delete form;

    return fields;
}

double Page::duration() const
{
    return m_page->page->getDuration();
}

QString Page::label() const
{
    GooString goo;
    if (!m_page->parentDoc->doc->getCatalog()->indexToLabel(m_page->index, &goo))
        return QString();

    return UnicodeParsedString(&goo);
}

int Page::index() const
{
    return m_page->index;
}

}
