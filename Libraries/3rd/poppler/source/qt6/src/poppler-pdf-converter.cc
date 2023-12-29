/* poppler-pdf-converter.cc: qt interface to poppler
 * Copyright (C) 2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, 2009, 2020-2022, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2020, Thorsten Behrens <Thorsten.Behrens@CIB.de>
 * Copyright (C) 2020, Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
 * Copyright (C) 2021, Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>.
 * Copyright (C) 2021, Zachary Travis <ztravis@everlaw.com>
 * Copyright (C) 2021, Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
 * Copyright (C) 2022, Martin <martinbts@gmx.net>
 * Copyright (C) 2022, Felix Jung <fxjung@posteo.de>
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
#define MIKTEX_UTF8_WRAP_ALL 1
#include <miktex/utf8wrap.h>
#endif
#include "poppler-qt6.h"

#include "poppler-annotation-helper.h"
#include "poppler-annotation-private.h"
#include "poppler-private.h"
#include "poppler-converter-private.h"
#include "poppler-qiodeviceoutstream-private.h"

#include <QFile>
#include <QUuid>

#include "Array.h"
#include "Form.h"
#include <ErrorCodes.h>

namespace Poppler {

class PDFConverterPrivate : public BaseConverterPrivate
{
public:
    PDFConverterPrivate();
    ~PDFConverterPrivate() override;

    PDFConverter::PDFOptions opts;
};

PDFConverterPrivate::PDFConverterPrivate() : BaseConverterPrivate() { }

PDFConverterPrivate::~PDFConverterPrivate() = default;

PDFConverter::PDFConverter(DocumentData *document) : BaseConverter(*new PDFConverterPrivate())
{
    Q_D(PDFConverter);
    d->document = document;
}

PDFConverter::~PDFConverter() { }

void PDFConverter::setPDFOptions(PDFConverter::PDFOptions options)
{
    Q_D(PDFConverter);
    d->opts = options;
}

PDFConverter::PDFOptions PDFConverter::pdfOptions() const
{
    Q_D(const PDFConverter);
    return d->opts;
}

bool PDFConverter::convert()
{
    Q_D(PDFConverter);
    d->lastError = NoError;

    if (d->document->locked) {
        d->lastError = FileLockedError;
        return false;
    }

    QIODevice *dev = d->openDevice();
    if (!dev) {
        d->lastError = OpenOutputError;
        return false;
    }

    bool deleteFile = false;
    if (QFile *file = qobject_cast<QFile *>(dev)) {
        deleteFile = !file->exists();
    }

    int errorCode = errNone;
    QIODeviceOutStream stream(dev);
    if (d->opts & WithChanges) {
        errorCode = d->document->doc->saveAs(&stream);
    } else {
        errorCode = d->document->doc->saveWithoutChangesAs(&stream);
    }
    d->closeDevice();
    if (errorCode != errNone) {
        if (deleteFile) {
            qobject_cast<QFile *>(dev)->remove();
        }
        if (errorCode == errOpenFile) {
            d->lastError = OpenOutputError;
        } else {
            d->lastError = NotSupportedInputFileError;
        }
    }

    return (errorCode == errNone);
}

bool PDFConverter::sign(const NewSignatureData &data)
{
    Q_D(PDFConverter);
    d->lastError = NoError;

    if (d->document->locked) {
        d->lastError = FileLockedError;
        return false;
    }

    if (data.signatureText().isEmpty()) {
        qWarning() << "No signature text given";
        return false;
    }

    ::PDFDoc *doc = d->document->doc;
    ::Page *destPage = doc->getPage(data.page() + 1);
    std::unique_ptr<GooString> gSignatureText = std::unique_ptr<GooString>(QStringToUnicodeGooString(data.signatureText()));
    std::unique_ptr<GooString> gSignatureLeftText = std::unique_ptr<GooString>(QStringToUnicodeGooString(data.signatureLeftText()));
    const auto reason = std::unique_ptr<GooString>(data.reason().isEmpty() ? nullptr : QStringToUnicodeGooString(data.reason()));
    const auto location = std::unique_ptr<GooString>(data.location().isEmpty() ? nullptr : QStringToUnicodeGooString(data.location()));
    const auto ownerPwd = std::optional<GooString>(data.documentOwnerPassword().constData());
    const auto userPwd = std::optional<GooString>(data.documentUserPassword().constData());
    return doc->sign(d->outputFileName.toUtf8().constData(), data.certNickname().toUtf8().constData(), data.password().toUtf8().constData(), QStringToGooString(data.fieldPartialName()), data.page() + 1,
                     boundaryToPdfRectangle(destPage, data.boundingRectangle(), Annotation::FixedRotation), *gSignatureText, *gSignatureLeftText, data.fontSize(), data.leftFontSize(), convertQColor(data.fontColor()), data.borderWidth(),
                     convertQColor(data.borderColor()), convertQColor(data.backgroundColor()), reason.get(), location.get(), data.imagePath().toStdString(), ownerPwd, userPwd);
}

struct PDFConverter::NewSignatureData::NewSignatureDataPrivate
{
    NewSignatureDataPrivate() = default;

    QString certNickname;
    QString password;
    int page;
    QRectF boundingRectangle;
    QString signatureText;
    QString signatureLeftText;
    QString reason;
    QString location;
    double fontSize = 10.0;
    double leftFontSize = 20.0;
    QColor fontColor = Qt::red;
    QColor borderColor = Qt::red;
    double borderWidth = 1.5;
    QColor backgroundColor = QColor(240, 240, 240);

    QString partialName = QUuid::createUuid().toString();

    QByteArray documentOwnerPassword;
    QByteArray documentUserPassword;

    QString imagePath;
};

PDFConverter::NewSignatureData::NewSignatureData() : d(new NewSignatureDataPrivate()) { }

PDFConverter::NewSignatureData::~NewSignatureData()
{
    delete d;
}

QString PDFConverter::NewSignatureData::certNickname() const
{
    return d->certNickname;
}

void PDFConverter::NewSignatureData::setCertNickname(const QString &certNickname)
{
    d->certNickname = certNickname;
}

QString PDFConverter::NewSignatureData::password() const
{
    return d->password;
}

void PDFConverter::NewSignatureData::setPassword(const QString &password)
{
    d->password = password;
}

int PDFConverter::NewSignatureData::page() const
{
    return d->page;
}

void PDFConverter::NewSignatureData::setPage(int page)
{
    d->page = page;
}

QRectF PDFConverter::NewSignatureData::boundingRectangle() const
{
    return d->boundingRectangle;
}

void PDFConverter::NewSignatureData::setBoundingRectangle(const QRectF &rect)
{
    d->boundingRectangle = rect;
}

QString PDFConverter::NewSignatureData::signatureText() const
{
    return d->signatureText;
}

void PDFConverter::NewSignatureData::setSignatureText(const QString &text)
{
    d->signatureText = text;
}

QString PDFConverter::NewSignatureData::signatureLeftText() const
{
    return d->signatureLeftText;
}

void PDFConverter::NewSignatureData::setSignatureLeftText(const QString &text)
{
    d->signatureLeftText = text;
}

QString PDFConverter::NewSignatureData::reason() const
{
    return d->reason;
}

void PDFConverter::NewSignatureData::setReason(const QString &reason)
{
    d->reason = reason;
}

QString PDFConverter::NewSignatureData::location() const
{
    return d->location;
}

void PDFConverter::NewSignatureData::setLocation(const QString &location)
{
    d->location = location;
}

double PDFConverter::NewSignatureData::fontSize() const
{
    return d->fontSize;
}

void PDFConverter::NewSignatureData::setFontSize(double fontSize)
{
    d->fontSize = fontSize;
}

double PDFConverter::NewSignatureData::leftFontSize() const
{
    return d->leftFontSize;
}

void PDFConverter::NewSignatureData::setLeftFontSize(double fontSize)
{
    d->leftFontSize = fontSize;
}

QColor PDFConverter::NewSignatureData::fontColor() const
{
    return d->fontColor;
}

void PDFConverter::NewSignatureData::setFontColor(const QColor &color)
{
    d->fontColor = color;
}

QColor PDFConverter::NewSignatureData::borderColor() const
{
    return d->borderColor;
}

void PDFConverter::NewSignatureData::setBorderColor(const QColor &color)
{
    d->borderColor = color;
}

QColor PDFConverter::NewSignatureData::backgroundColor() const
{
    return d->backgroundColor;
}

double PDFConverter::NewSignatureData::borderWidth() const
{
    return d->borderWidth;
}

void PDFConverter::NewSignatureData::setBorderWidth(double width)
{
    d->borderWidth = width;
}

void PDFConverter::NewSignatureData::setBackgroundColor(const QColor &color)
{
    d->backgroundColor = color;
}

QString PDFConverter::NewSignatureData::fieldPartialName() const
{
    return d->partialName;
}

void PDFConverter::NewSignatureData::setFieldPartialName(const QString &name)
{
    d->partialName = name;
}

QByteArray PDFConverter::NewSignatureData::documentOwnerPassword() const
{
    return d->documentOwnerPassword;
}

void PDFConverter::NewSignatureData::setDocumentOwnerPassword(const QByteArray &password)
{
    d->documentOwnerPassword = password;
}

QByteArray PDFConverter::NewSignatureData::documentUserPassword() const
{
    return d->documentUserPassword;
}

void PDFConverter::NewSignatureData::setDocumentUserPassword(const QByteArray &password)
{
    d->documentUserPassword = password;
}

QString PDFConverter::NewSignatureData::imagePath() const
{
    return d->imagePath;
}

void PDFConverter::NewSignatureData::setImagePath(const QString &path)
{
    d->imagePath = path;
}
}
