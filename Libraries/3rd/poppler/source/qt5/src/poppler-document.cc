/* poppler-document.cc: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, 2008, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2005-2010, 2012, 2013, 2015, 2017-2020, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2006-2010, Pino Toscano <pino@kde.org>
 * Copyright (C) 2010, 2011 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012 Koji Otani <sho@bbr.jp>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
 * Copyright (C) 2012 Fabio D'Urso <fabiodurso@hotmail.it>
 * Copyright (C) 2014, 2018, 2020 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2015 William Bader <williambader@hotmail.com>
 * Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
 * Copyright (C) 2017 Adrian Johnson <ajohnson@redneon.com>
 * Copyright (C) 2017 Suzuki Toshiya <mpsuzuki@hiroshima-u.ac.jp>
 * Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
 * Copyright (C) 2019 Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2019 Alexander Volkov <a.volkov@rusbitech.ru>
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

#include "poppler-qt5.h"

#include <config.h>
#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <Outline.h>
#include <PDFDoc.h>
#include <Stream.h>
#include <Catalog.h>
#include <ViewerPreferences.h>
#include <DateInfo.h>
#include <GfxState.h>

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include "poppler-form.h"
#include "poppler-private.h"
#include "poppler-page-private.h"
#include "poppler-outline-private.h"

#if defined(USE_CMS)
#    include <lcms2.h>
#endif

namespace Poppler {

Document *Document::load(const QString &filePath, const QByteArray &ownerPassword, const QByteArray &userPassword)
{
    DocumentData *doc = new DocumentData(filePath, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
    return DocumentData::checkDocument(doc);
}

Document *Document::load(QIODevice *device, const QByteArray &ownerPassword, const QByteArray &userPassword)
{
    DocumentData *doc = new DocumentData(device, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
    return DocumentData::checkDocument(doc);
}

Document *Document::loadFromData(const QByteArray &fileContents, const QByteArray &ownerPassword, const QByteArray &userPassword)
{
    // create stream
    DocumentData *doc = new DocumentData(fileContents, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
    return DocumentData::checkDocument(doc);
}

Document *DocumentData::checkDocument(DocumentData *doc)
{
    Document *pdoc;
    if (doc->doc->isOk() || doc->doc->getErrorCode() == errEncrypted) {
        pdoc = new Document(doc);
        if (doc->doc->getErrorCode() == errEncrypted)
            pdoc->m_doc->locked = true;
        else {
            pdoc->m_doc->locked = false;
            pdoc->m_doc->fillMembers();
        }
        return pdoc;
    } else {
        delete doc;
    }
    return nullptr;
}

Document::Document(DocumentData *dataA)
{
    m_doc = dataA;
}

Document::~Document()
{
    delete m_doc;
}

Page *Document::page(int index) const
{
    Page *page = new Page(m_doc, index);
    if (page->m_page->page == nullptr) {
        delete page;
        return nullptr;
    }

    return page;
}

bool Document::isLocked() const
{
    return m_doc->locked;
}

bool Document::unlock(const QByteArray &ownerPassword, const QByteArray &userPassword)
{
    if (m_doc->locked) {
        /* racier then it needs to be */
        DocumentData *doc2;
        if (!m_doc->fileContents.isEmpty()) {
            doc2 = new DocumentData(m_doc->fileContents, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
        } else if (m_doc->m_device) {
            doc2 = new DocumentData(m_doc->m_device, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
        } else {
            doc2 = new DocumentData(m_doc->m_filePath, new GooString(ownerPassword.data()), new GooString(userPassword.data()));
        }
        if (!doc2->doc->isOk()) {
            delete doc2;
        } else {
            delete m_doc;
            m_doc = doc2;
            m_doc->locked = false;
            m_doc->fillMembers();
        }
    }
    return m_doc->locked;
}

Document::PageMode Document::pageMode() const
{
    switch (m_doc->doc->getCatalog()->getPageMode()) {
    case Catalog::pageModeNone:
        return UseNone;
    case Catalog::pageModeOutlines:
        return UseOutlines;
    case Catalog::pageModeThumbs:
        return UseThumbs;
    case Catalog::pageModeFullScreen:
        return FullScreen;
    case Catalog::pageModeOC:
        return UseOC;
    case Catalog::pageModeAttach:
        return UseAttach;
    default:
        return UseNone;
    }
}

Document::PageLayout Document::pageLayout() const
{
    switch (m_doc->doc->getCatalog()->getPageLayout()) {
    case Catalog::pageLayoutNone:
        return NoLayout;
    case Catalog::pageLayoutSinglePage:
        return SinglePage;
    case Catalog::pageLayoutOneColumn:
        return OneColumn;
    case Catalog::pageLayoutTwoColumnLeft:
        return TwoColumnLeft;
    case Catalog::pageLayoutTwoColumnRight:
        return TwoColumnRight;
    case Catalog::pageLayoutTwoPageLeft:
        return TwoPageLeft;
    case Catalog::pageLayoutTwoPageRight:
        return TwoPageRight;
    default:
        return NoLayout;
    }
}

Qt::LayoutDirection Document::textDirection() const
{
    if (!m_doc->doc->getCatalog()->getViewerPreferences())
        return Qt::LayoutDirectionAuto;

    switch (m_doc->doc->getCatalog()->getViewerPreferences()->getDirection()) {
    case ViewerPreferences::directionL2R:
        return Qt::LeftToRight;
    case ViewerPreferences::directionR2L:
        return Qt::RightToLeft;
    default:
        return Qt::LayoutDirectionAuto;
    }
}

int Document::numPages() const
{
    return m_doc->doc->getNumPages();
}

QList<FontInfo> Document::fonts() const
{
    QList<FontInfo> ourList;
    FontIterator it(0, m_doc);
    while (it.hasNext()) {
        ourList += it.next();
    }
    return ourList;
}

QList<EmbeddedFile *> Document::embeddedFiles() const
{
    return m_doc->m_embeddedFiles;
}

FontIterator *Document::newFontIterator(int startPage) const
{
    return new FontIterator(startPage, m_doc);
}

QByteArray Document::fontData(const FontInfo &fi) const
{
    QByteArray result;
    if (fi.isEmbedded()) {
        XRef *xref = m_doc->doc->getXRef()->copy();

        Object refObj(fi.m_data->embRef);
        Object strObj = refObj.fetch(xref);
        if (strObj.isStream()) {
            int c;
            strObj.streamReset();
            while ((c = strObj.streamGetChar()) != EOF) {
                result.append((char)c);
            }
            strObj.streamClose();
        }
        delete xref;
    }
    return result;
}

QString Document::info(const QString &type) const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoStringEntry(type.toLatin1().constData()));
    return UnicodeParsedString(goo.data());
}

bool Document::setInfo(const QString &key, const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    GooString *goo = QStringToUnicodeGooString(val);
    m_doc->doc->setDocInfoStringEntry(key.toLatin1().constData(), goo);
    return true;
}

QString Document::title() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoTitle());
    return UnicodeParsedString(goo.data());
}

bool Document::setTitle(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoTitle(QStringToUnicodeGooString(val));
    return true;
}

QString Document::author() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoAuthor());
    return UnicodeParsedString(goo.data());
}

bool Document::setAuthor(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoAuthor(QStringToUnicodeGooString(val));
    return true;
}

QString Document::subject() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoSubject());
    return UnicodeParsedString(goo.data());
}

bool Document::setSubject(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoSubject(QStringToUnicodeGooString(val));
    return true;
}

QString Document::keywords() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoKeywords());
    return UnicodeParsedString(goo.data());
}

bool Document::setKeywords(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoKeywords(QStringToUnicodeGooString(val));
    return true;
}

QString Document::creator() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoCreator());
    return UnicodeParsedString(goo.data());
}

bool Document::setCreator(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoCreator(QStringToUnicodeGooString(val));
    return true;
}

QString Document::producer() const
{
    if (m_doc->locked) {
        return QString();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoProducer());
    return UnicodeParsedString(goo.data());
}

bool Document::setProducer(const QString &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoProducer(QStringToUnicodeGooString(val));
    return true;
}

bool Document::removeInfo()
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->removeDocInfo();
    return true;
}

QStringList Document::infoKeys() const
{
    QStringList keys;

    if (m_doc->locked)
        return QStringList();

    QScopedPointer<XRef> xref(m_doc->doc->getXRef()->copy());
    if (!xref)
        return QStringList();
    Object info = xref->getDocInfo();
    if (!info.isDict())
        return QStringList();

    Dict *infoDict = info.getDict();
    // somehow iterate over keys in infoDict
    keys.reserve(infoDict->getLength());
    for (int i = 0; i < infoDict->getLength(); ++i) {
        keys.append(QString::fromLatin1(infoDict->getKey(i)));
    }

    return keys;
}

QDateTime Document::date(const QString &type) const
{
    if (m_doc->locked) {
        return QDateTime();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoStringEntry(type.toLatin1().constData()));
    QString str = UnicodeParsedString(goo.data());
    return Poppler::convertDate(str.toLatin1().constData());
}

bool Document::setDate(const QString &key, const QDateTime &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoStringEntry(key.toLatin1().constData(), QDateTimeToUnicodeGooString(val));
    return true;
}

QDateTime Document::creationDate() const
{
    if (m_doc->locked) {
        return QDateTime();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoCreatDate());
    QString str = UnicodeParsedString(goo.data());
    return Poppler::convertDate(str.toLatin1().constData());
}

bool Document::setCreationDate(const QDateTime &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoCreatDate(QDateTimeToUnicodeGooString(val));
    return true;
}

QDateTime Document::modificationDate() const
{
    if (m_doc->locked) {
        return QDateTime();
    }

    QScopedPointer<GooString> goo(m_doc->doc->getDocInfoModDate());
    QString str = UnicodeParsedString(goo.data());
    return Poppler::convertDate(str.toLatin1().constData());
}

bool Document::setModificationDate(const QDateTime &val)
{
    if (m_doc->locked) {
        return false;
    }

    m_doc->doc->setDocInfoModDate(QDateTimeToUnicodeGooString(val));
    return true;
}

bool Document::isEncrypted() const
{
    return m_doc->doc->isEncrypted();
}

bool Document::isLinearized() const
{
    return m_doc->doc->isLinearized();
}

bool Document::okToPrint() const
{
    return m_doc->doc->okToPrint();
}

bool Document::okToPrintHighRes() const
{
    return m_doc->doc->okToPrintHighRes();
}

bool Document::okToChange() const
{
    return m_doc->doc->okToChange();
}

bool Document::okToCopy() const
{
    return m_doc->doc->okToCopy();
}

bool Document::okToAddNotes() const
{
    return m_doc->doc->okToAddNotes();
}

bool Document::okToFillForm() const
{
    return m_doc->doc->okToFillForm();
}

bool Document::okToCreateFormFields() const
{
    return (okToFillForm() && okToChange());
}

bool Document::okToExtractForAccessibility() const
{
    return m_doc->doc->okToAccessibility();
}

bool Document::okToAssemble() const
{
    return m_doc->doc->okToAssemble();
}

void Document::getPdfVersion(int *major, int *minor) const
{
    if (major)
        *major = m_doc->doc->getPDFMajorVersion();
    if (minor)
        *minor = m_doc->doc->getPDFMinorVersion();
}

Page *Document::page(const QString &label) const
{
    GooString label_g(label.toLatin1().data());
    int index;

    if (!m_doc->doc->getCatalog()->labelToIndex(&label_g, &index)) {
        std::unique_ptr<GooString> label_ug(QStringToUnicodeGooString(label));
        if (!m_doc->doc->getCatalog()->labelToIndex(label_ug.get(), &index)) {
            return nullptr;
        }
    }

    return page(index);
}

bool Document::hasEmbeddedFiles() const
{
    return (!(0 == m_doc->doc->getCatalog()->numEmbeddedFiles()));
}

QDomDocument *Document::toc() const
{
    Outline *outline = m_doc->doc->getOutline();
    if (!outline)
        return nullptr;

    const std::vector<::OutlineItem *> *items = outline->getItems();
    if (!items || items->size() < 1)
        return nullptr;

    QDomDocument *toc = new QDomDocument();
    if (items->size() > 0)
        m_doc->addTocChildren(toc, toc, items);

    return toc;
}

QVector<OutlineItem> Document::outline() const
{
    QVector<OutlineItem> result;

    if (::Outline *outline = m_doc->doc->getOutline()) {
        if (const std::vector<::OutlineItem *> *items = outline->getItems()) {
            for (void *item : *items) {
                result.push_back(OutlineItem { new OutlineItemData { static_cast<::OutlineItem *>(item), m_doc } });
            }
        }
    }

    return result;
}

LinkDestination *Document::linkDestination(const QString &name)
{
    GooString *namedDest = QStringToGooString(name);
    LinkDestinationData ldd(nullptr, namedDest, m_doc, false);
    LinkDestination *ld = new LinkDestination(ldd);
    delete namedDest;
    return ld;
}

void Document::setPaperColor(const QColor &color)
{
    m_doc->setPaperColor(color);
}

void Document::setColorDisplayProfile(void *outputProfileA)
{
#if defined(USE_CMS)
    if (m_doc->m_sRGBProfile && m_doc->m_sRGBProfile.get() == outputProfileA) {
        // Catch the special case that the user passes the sRGB profile
        m_doc->m_displayProfile = m_doc->m_sRGBProfile;
        return;
    }
    if (m_doc->m_displayProfile && m_doc->m_displayProfile.get() == outputProfileA) {
        // Catch the special case that the user passes the display profile
        return;
    }
    m_doc->m_displayProfile = make_GfxLCMSProfilePtr(outputProfileA);
#else
    Q_UNUSED(outputProfileA);
#endif
}

void Document::setColorDisplayProfileName(const QString &name)
{
#if defined(USE_CMS)
    void *rawprofile = cmsOpenProfileFromFile(name.toLocal8Bit().constData(), "r");
    m_doc->m_displayProfile = make_GfxLCMSProfilePtr(rawprofile);
#else
    Q_UNUSED(name);
#endif
}

void *Document::colorRgbProfile() const
{
#if defined(USE_CMS)
    if (!m_doc->m_sRGBProfile) {
        m_doc->m_sRGBProfile = make_GfxLCMSProfilePtr(cmsCreate_sRGBProfile());
    }
    return m_doc->m_sRGBProfile.get();
#else
    return nullptr;
#endif
}

void *Document::colorDisplayProfile() const
{
#if defined(USE_CMS)
    return m_doc->m_displayProfile.get();
#else
    return nullptr;
#endif
}

QColor Document::paperColor() const
{
    return m_doc->paperColor;
}

void Document::setRenderBackend(Document::RenderBackend backend)
{
    // no need to delete the outputdev as for the moment we always create a splash one
    // as the arthur one does not allow "precaching" due to it's signature
    // delete m_doc->m_outputDev;
    // m_doc->m_outputDev = NULL;
    m_doc->m_backend = backend;
}

Document::RenderBackend Document::renderBackend() const
{
    return m_doc->m_backend;
}

QSet<Document::RenderBackend> Document::availableRenderBackends()
{
    QSet<Document::RenderBackend> ret;
#if defined(HAVE_SPLASH)
    ret << Document::SplashBackend;
#endif
    ret << Document::ArthurBackend;
    return ret;
}

void Document::setRenderHint(Document::RenderHint hint, bool on)
{
    const bool touchesOverprinting = hint & Document::OverprintPreview;

    int hintForOperation = hint;
    if (touchesOverprinting && !isOverprintPreviewAvailable())
        hintForOperation = hintForOperation & ~(int)Document::OverprintPreview;

    if (on)
        m_doc->m_hints |= hintForOperation;
    else
        m_doc->m_hints &= ~hintForOperation;
}

Document::RenderHints Document::renderHints() const
{
    return Document::RenderHints(m_doc->m_hints);
}

PSConverter *Document::psConverter() const
{
    return new PSConverter(m_doc);
}

PDFConverter *Document::pdfConverter() const
{
    return new PDFConverter(m_doc);
}

QString Document::metadata() const
{
    QString result;
    Catalog *catalog = m_doc->doc->getCatalog();
    if (catalog && catalog->isOk()) {
        GooString *s = catalog->readMetadata();
        if (s)
            result = UnicodeParsedString(s);
        delete s;
    }
    return result;
}

bool Document::hasOptionalContent() const
{
    return (m_doc->doc->getOptContentConfig() && m_doc->doc->getOptContentConfig()->hasOCGs());
}

OptContentModel *Document::optionalContentModel()
{
    if (m_doc->m_optContentModel.isNull()) {
        m_doc->m_optContentModel = new OptContentModel(m_doc->doc->getOptContentConfig(), nullptr);
    }
    return (OptContentModel *)m_doc->m_optContentModel;
}

QStringList Document::scripts() const
{
    Catalog *catalog = m_doc->doc->getCatalog();
    const int numScripts = catalog->numJS();
    QStringList scripts;
    for (int i = 0; i < numScripts; ++i) {
        GooString *s = catalog->getJS(i);
        if (s) {
            scripts.append(UnicodeParsedString(s));
            delete s;
        }
    }
    return scripts;
}

bool Document::getPdfId(QByteArray *permanentId, QByteArray *updateId) const
{
    GooString gooPermanentId;
    GooString gooUpdateId;

    if (!m_doc->doc->getID(permanentId ? &gooPermanentId : nullptr, updateId ? &gooUpdateId : nullptr))
        return false;

    if (permanentId)
        *permanentId = gooPermanentId.c_str();
    if (updateId)
        *updateId = gooUpdateId.c_str();

    return true;
}

Document::FormType Document::formType() const
{
    switch (m_doc->doc->getCatalog()->getFormType()) {
    case Catalog::NoForm:
        return Document::NoForm;
    case Catalog::AcroForm:
        return Document::AcroForm;
    case Catalog::XfaForm:
        return Document::XfaForm;
    }

    return Document::NoForm; // make gcc happy
}

QVector<int> Document::formCalculateOrder() const
{
    QVector<int> result;

    Form *form = m_doc->doc->getCatalog()->getForm();
    const std::vector<Ref> &calculateOrder = form->getCalculateOrder();
    for (Ref r : calculateOrder) {
        FormWidget *w = form->findWidgetByRef(r);
        if (w) {
            result << w->getID();
        }
    }

    return result;
}

QVector<FormFieldSignature *> Document::signatures() const
{
    QVector<FormFieldSignature *> result;

    const std::vector<::FormFieldSignature *> pSignatures = m_doc->doc->getSignatureFields();

    for (::FormFieldSignature *pSignature : pSignatures) {
        ::FormWidget *fw = pSignature->getWidget(0);
        ::Page *p = m_doc->doc->getPage(fw->getWidgetAnnotation()->getPageNum());
        result.append(new FormFieldSignature(m_doc, p, static_cast<FormWidgetSignature *>(fw)));
    }

    return result;
}

QDateTime convertDate(const char *dateString)
{
    int year, mon, day, hour, min, sec, tzHours, tzMins;
    char tz;

    if (parseDateString(dateString, &year, &mon, &day, &hour, &min, &sec, &tz, &tzHours, &tzMins)) {
        QDate d(year, mon, day);
        QTime t(hour, min, sec);
        if (d.isValid() && t.isValid()) {
            QDateTime dt(d, t, Qt::UTC);
            if (tz) {
                // then we have some form of timezone
                if ('Z' == tz) {
                    // We are already at UTC
                } else if ('+' == tz) {
                    // local time is ahead of UTC
                    dt = dt.addSecs(-1 * ((tzHours * 60) + tzMins) * 60);
                } else if ('-' == tz) {
                    // local time is behind UTC
                    dt = dt.addSecs(((tzHours * 60) + tzMins) * 60);
                } else {
                    qWarning("unexpected tz val");
                }
            }
            return dt;
        }
    }
    return QDateTime();
}

QDateTime convertDate(char *dateString)
{
    return convertDate((const char *)dateString);
}

bool isCmsAvailable()
{
#if defined(USE_CMS)
    return true;
#else
    return false;
#endif
}

bool isOverprintPreviewAvailable()
{
    return true;
}

}
