/* poppler-private.h: qt interface to poppler
 * Copyright (C) 2005, Net Integration Technologies, Inc.
 * Copyright (C) 2005, 2008, Brad Hards <bradh@frogmouth.net>
 * Copyright (C) 2006-2009, 2011, 2012, 2017-2020 by Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2007-2009, 2011, 2014 by Pino Toscano <pino@kde.org>
 * Copyright (C) 2011 Andreas Hartmetz <ahartmetz@gmail.com>
 * Copyright (C) 2011 Hib Eris <hib@hiberis.nl>
 * Copyright (C) 2012, 2013 Thomas Freitag <Thomas.Freitag@alfa.de>
 * Copyright (C) 2013 Anthony Granger <grangeranthony@gmail.com>
 * Copyright (C) 2014 Bogdan Cristea <cristeab@gmail.com>
 * Copyright (C) 2014 Aki Koskinen <freedesktop@akikoskinen.info>
 * Copyright (C) 2016 Jakub Alba <jakubalba@gmail.com>
 * Copyright (C) 2017 Christoph Cullmann <cullmann@kde.org>
 * Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
 * Copyright (C) 2018, 2020 Adam Reichold <adam.reichold@t-online.de>
 * Copyright (C) 2019, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2019 João Netto <joaonetto901@gmail.com>
 * Copyright (C) 2019 Jan Grulich <jgrulich@redhat.com>
 * Copyright (C) 2019 Alexander Volkov <a.volkov@rusbitech.ru>
 * Copyright (C) 2020 Philipp Knechtges <philipp-dev@knechtges.com>
 * Inspired on code by
 * Copyright (C) 2004 by Albert Astals Cid <tsdgeos@terra.es>
 * Copyright (C) 2004 by Enrico Ros <eros.kde@email.it>
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

#ifndef _POPPLER_PRIVATE_H_
#define _POPPLER_PRIVATE_H_

#include <QtCore/QFile>
#include <QtCore/QMutex>
#include <QtCore/QPointer>
#include <QtCore/QVector>

#include <config.h>
#include <GfxState.h>
#include <GlobalParams.h>
#include <Form.h>
#include <PDFDoc.h>
#include <FontInfo.h>
#include <OutputDev.h>
#include <Error.h>
#if defined(HAVE_SPLASH)
#    include <SplashOutputDev.h>
#endif

#include "poppler-qt5.h"
#include "poppler-embeddedfile-private.h"
#include "poppler-qiodeviceinstream-private.h"

class LinkDest;
class FormWidget;
class OutlineItem;

namespace Poppler {

/* borrowed from kpdf */
POPPLER_QT5_EXPORT QString unicodeToQString(const Unicode *u, int len);

POPPLER_QT5_EXPORT QString UnicodeParsedString(const GooString *s1);

POPPLER_QT5_EXPORT QString UnicodeParsedString(const std::string &s1);

POPPLER_QT5_EXPORT GooString *QStringToUnicodeGooString(const QString &s);

POPPLER_QT5_EXPORT GooString *QStringToGooString(const QString &s);

GooString *QDateTimeToUnicodeGooString(const QDateTime &dt);

void qt5ErrorFunction(ErrorCategory /*category*/, Goffset pos, const char *msg);

Annot::AdditionalActionsType toPopplerAdditionalActionType(Annotation::AdditionalActionType type);

class LinkDestinationData
{
public:
    LinkDestinationData(const LinkDest *l, const GooString *nd, Poppler::DocumentData *pdfdoc, bool external) : ld(l), namedDest(nd), doc(pdfdoc), externalDest(external) { }

    const LinkDest *ld;
    const GooString *namedDest;
    Poppler::DocumentData *doc;
    bool externalDest;
};

class DocumentData : private GlobalParamsIniter
{
public:
    DocumentData(const QString &filePath, GooString *ownerPassword, GooString *userPassword) : GlobalParamsIniter(qt5ErrorFunction)
    {
        init();
        m_device = nullptr;
        m_filePath = filePath;

#ifdef _WIN32
        doc = new PDFDoc((wchar_t *)filePath.utf16(), filePath.length(), ownerPassword, userPassword);
#else
        GooString *fileName = new GooString(QFile::encodeName(filePath).constData());
        doc = new PDFDoc(fileName, ownerPassword, userPassword);
#endif

        delete ownerPassword;
        delete userPassword;
    }

    DocumentData(QIODevice *device, GooString *ownerPassword, GooString *userPassword) : GlobalParamsIniter(qt5ErrorFunction)
    {
        m_device = device;
        QIODeviceInStream *str = new QIODeviceInStream(device, 0, false, device->size(), Object(objNull));
        init();
        doc = new PDFDoc(str, ownerPassword, userPassword);
        delete ownerPassword;
        delete userPassword;
    }

    DocumentData(const QByteArray &data, GooString *ownerPassword, GooString *userPassword) : GlobalParamsIniter(qt5ErrorFunction)
    {
        m_device = nullptr;
        fileContents = data;
        MemStream *str = new MemStream((char *)fileContents.data(), 0, fileContents.length(), Object(objNull));
        init();
        doc = new PDFDoc(str, ownerPassword, userPassword);
        delete ownerPassword;
        delete userPassword;
    }

    void init();

    ~DocumentData();

    DocumentData(const DocumentData &) = delete;
    DocumentData &operator=(const DocumentData &) = delete;

    void addTocChildren(QDomDocument *docSyn, QDomNode *parent, const std::vector<::OutlineItem *> *items);

    void setPaperColor(const QColor &color) { paperColor = color; }

    void fillMembers()
    {
        int numEmb = doc->getCatalog()->numEmbeddedFiles();
        if (!(0 == numEmb)) {
            // we have some embedded documents, build the list
            for (int yalv = 0; yalv < numEmb; ++yalv) {
                FileSpec *fs = doc->getCatalog()->embeddedFile(yalv);
                m_embeddedFiles.append(new EmbeddedFile(*new EmbeddedFileData(fs)));
            }
        }
    }

    static Document *checkDocument(DocumentData *doc);

    PDFDoc *doc;
    QString m_filePath;
    QIODevice *m_device;
    QByteArray fileContents;
    bool locked;
    Document::RenderBackend m_backend;
    QList<EmbeddedFile *> m_embeddedFiles;
    QPointer<OptContentModel> m_optContentModel;
    QColor paperColor;
    int m_hints;
#ifdef USE_CMS
    GfxLCMSProfilePtr m_sRGBProfile;
    GfxLCMSProfilePtr m_displayProfile;
#endif
};

class FontInfoData
{
public:
    FontInfoData()
    {
        isEmbedded = false;
        isSubset = false;
        type = FontInfo::unknown;
    }

    FontInfoData(::FontInfo *fi)
    {
        if (fi->getName())
            fontName = fi->getName()->c_str();
        if (fi->getFile())
            fontFile = fi->getFile()->c_str();
        if (fi->getSubstituteName())
            fontSubstituteName = fi->getSubstituteName()->c_str();
        isEmbedded = fi->getEmbedded();
        isSubset = fi->getSubset();
        type = (Poppler::FontInfo::Type)fi->getType();
        embRef = fi->getEmbRef();
    }

    FontInfoData(const FontInfoData &fid) = default;
    FontInfoData &operator=(const FontInfoData &) = default;

    QString fontName;
    QString fontSubstituteName;
    QString fontFile;
    bool isEmbedded : 1;
    bool isSubset : 1;
    FontInfo::Type type;
    Ref embRef;
};

class FontIteratorData
{
public:
    FontIteratorData(int startPage, DocumentData *dd) : fontInfoScanner(dd->doc, startPage), totalPages(dd->doc->getNumPages()), currentPage(qMax(startPage, 0) - 1) { }

    ~FontIteratorData() { }

    FontInfoScanner fontInfoScanner;
    int totalPages;
    int currentPage;
};

class TextBoxData
{
public:
    TextBoxData() : nextWord(nullptr), hasSpaceAfter(false) { }

    QString text;
    QRectF bBox;
    TextBox *nextWord;
    QVector<QRectF> charBBoxes; // the boundingRect of each character
    bool hasSpaceAfter;
};

class FormFieldData
{
public:
    FormFieldData(DocumentData *_doc, ::Page *p, ::FormWidget *w) : doc(_doc), page(p), fm(w) { }

    DocumentData *doc;
    ::Page *page; // Note for some signatures it can be null since there's signatures that don't belong to a given page
    ::FormWidget *fm;
    QRectF box;
    static POPPLER_QT5_EXPORT ::FormWidget *getFormWidget(const FormField *f);
};

class FormFieldIcon;
class FormFieldIconData
{
public:
    static POPPLER_QT5_EXPORT FormFieldIconData *getData(const FormFieldIcon &f);
    Dict *icon;
};

}

#endif
