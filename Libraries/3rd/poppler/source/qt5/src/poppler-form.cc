/* poppler-form.h: qt interface to poppler
 * Copyright (C) 2007-2008, 2011, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, 2011, 2012, 2015-2020 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2012, Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2016, Hanno Meyer-Thurow <h.mth@web.de>
 * Copyright (C) 2017, Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
 * Copyright (C) 2018, Andre Heinecke <aheinecke@intevation.de>
 * Copyright (C) 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
 * Copyright (C) 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
 * Copyright (C) 2018, 2020 Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2019 João Netto <joaonetto901@gmail.com>
 * Copyright (C) 2020 David García Garzón <voki@canvoki.net>
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

#include <QtCore/QSizeF>

#include <Form.h>
#include <Object.h>
#include <Link.h>
#include <SignatureInfo.h>
#include <CertificateInfo.h>

#include "poppler-form.h"
#include "poppler-page-private.h"
#include "poppler-private.h"
#include "poppler-annotation-helper.h"

#include <cmath>
#include <cctype>

#ifdef ENABLE_NSS3
#    include <hasht.h>
#endif

namespace {

Qt::Alignment formTextAlignment(::FormWidget *fm)
{
    Qt::Alignment qtalign = Qt::AlignLeft;
    switch (fm->getField()->getTextQuadding()) {
    case quaddingCentered:
        qtalign = Qt::AlignHCenter;
        break;
    case quaddingRightJustified:
        qtalign = Qt::AlignRight;
        break;
    case quaddingLeftJustified:
        qtalign = Qt::AlignLeft;
    }
    return qtalign;
}

}

namespace Poppler {

FormFieldIcon::FormFieldIcon(FormFieldIconData *data) : d_ptr(data) { }

FormFieldIcon::FormFieldIcon(const FormFieldIcon &ffIcon)
{
    d_ptr = new FormFieldIconData;
    d_ptr->icon = ffIcon.d_ptr->icon;
}

FormFieldIcon &FormFieldIcon::operator=(const FormFieldIcon &ffIcon)
{
    if (this != &ffIcon) {
        delete d_ptr;
        d_ptr = nullptr;

        d_ptr = new FormFieldIconData;
        *d_ptr = *ffIcon.d_ptr;
    }

    return *this;
}

FormFieldIcon::~FormFieldIcon()
{
    delete d_ptr;
}

FormField::FormField(std::unique_ptr<FormFieldData> dd) : m_formData(std::move(dd))
{
    if (m_formData->page) {
        const int rotation = m_formData->page->getRotate();
        // reading the coords
        double left, top, right, bottom;
        m_formData->fm->getRect(&left, &bottom, &right, &top);
        // build a normalized transform matrix for this page at 100% scale
        GfxState gfxState(72.0, 72.0, m_formData->page->getCropBox(), rotation, true);
        const double *gfxCTM = gfxState.getCTM();
        double MTX[6];
        double pageWidth = m_formData->page->getCropWidth();
        double pageHeight = m_formData->page->getCropHeight();
        // landscape and seascape page rotation: be sure to use the correct (== rotated) page size
        if (((rotation / 90) % 2) == 1)
            qSwap(pageWidth, pageHeight);
        for (int i = 0; i < 6; i += 2) {
            MTX[i] = gfxCTM[i] / pageWidth;
            MTX[i + 1] = gfxCTM[i + 1] / pageHeight;
        }
        QPointF topLeft;
        XPDFReader::transform(MTX, qMin(left, right), qMax(top, bottom), topLeft);
        QPointF bottomRight;
        XPDFReader::transform(MTX, qMax(left, right), qMin(top, bottom), bottomRight);
        m_formData->box = QRectF(topLeft, QSizeF(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y()));
    }
}

FormField::~FormField() = default;

QRectF FormField::rect() const
{
    return m_formData->box;
}

int FormField::id() const
{
    return m_formData->fm->getID();
}

QString FormField::name() const
{
    QString name;
    if (const GooString *goo = m_formData->fm->getPartialName()) {
        name = UnicodeParsedString(goo);
    }
    return name;
}

void FormField::setName(const QString &name) const
{
    GooString *goo = QStringToGooString(name);
    m_formData->fm->setPartialName(*goo);
    delete goo;
}

QString FormField::fullyQualifiedName() const
{
    QString name;
    if (GooString *goo = m_formData->fm->getFullyQualifiedName()) {
        name = UnicodeParsedString(goo);
    }
    return name;
}

QString FormField::uiName() const
{
    QString name;
    if (const GooString *goo = m_formData->fm->getAlternateUiName()) {
        name = UnicodeParsedString(goo);
    }
    return name;
}

bool FormField::isReadOnly() const
{
    return m_formData->fm->isReadOnly();
}

void FormField::setReadOnly(bool value)
{
    m_formData->fm->setReadOnly(value);
}

bool FormField::isVisible() const
{
    return !(m_formData->fm->getWidgetAnnotation()->getFlags() & Annot::flagHidden);
}

void FormField::setVisible(bool value)
{
    unsigned int flags = m_formData->fm->getWidgetAnnotation()->getFlags();
    if (value) {
        flags &= ~Annot::flagHidden;
    } else {
        flags |= Annot::flagHidden;
    }
    m_formData->fm->getWidgetAnnotation()->setFlags(flags);
}

bool FormField::isPrintable() const
{
    return (m_formData->fm->getWidgetAnnotation()->getFlags() & Annot::flagPrint);
}

void FormField::setPrintable(bool value)
{
    unsigned int flags = m_formData->fm->getWidgetAnnotation()->getFlags();
    if (value) {
        flags |= Annot::flagPrint;
    } else {
        flags &= ~Annot::flagPrint;
    }
    m_formData->fm->getWidgetAnnotation()->setFlags(flags);
}

Link *FormField::activationAction() const
{
    Link *action = nullptr;
    if (::LinkAction *act = m_formData->fm->getActivationAction()) {
        action = PageData::convertLinkActionToLink(act, m_formData->doc, QRectF());
    }
    return action;
}

Link *FormField::additionalAction(AdditionalActionType type) const
{
    Annot::FormAdditionalActionsType actionType = Annot::actionFieldModified;
    switch (type) {
    case FieldModified:
        actionType = Annot::actionFieldModified;
        break;
    case FormatField:
        actionType = Annot::actionFormatField;
        break;
    case ValidateField:
        actionType = Annot::actionValidateField;
        break;
    case CalculateField:
        actionType = Annot::actionCalculateField;
        break;
    }

    Link *action = nullptr;
    if (std::unique_ptr<::LinkAction> act = m_formData->fm->getAdditionalAction(actionType)) {
        action = PageData::convertLinkActionToLink(act.get(), m_formData->doc, QRectF());
    }
    return action;
}

Link *FormField::additionalAction(Annotation::AdditionalActionType type) const
{
    ::AnnotWidget *w = m_formData->fm->getWidgetAnnotation();
    if (!w) {
        return nullptr;
    }

    const Annot::AdditionalActionsType actionType = toPopplerAdditionalActionType(type);

    Link *action = nullptr;
    if (std::unique_ptr<::LinkAction> act = w->getAdditionalAction(actionType)) {
        action = PageData::convertLinkActionToLink(act.get(), m_formData->doc, QRectF());
    }
    return action;
}

FormFieldButton::FormFieldButton(DocumentData *doc, ::Page *p, ::FormWidgetButton *w) : FormField(std::make_unique<FormFieldData>(doc, p, w)) { }

FormFieldButton::~FormFieldButton() { }

FormFieldButton::FormType FormFieldButton::type() const
{
    return FormField::FormButton;
}

FormFieldButton::ButtonType FormFieldButton::buttonType() const
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    switch (fwb->getButtonType()) {
    case formButtonCheck:
        return FormFieldButton::CheckBox;
        break;
    case formButtonPush:
        return FormFieldButton::Push;
        break;
    case formButtonRadio:
        return FormFieldButton::Radio;
        break;
    }
    return FormFieldButton::CheckBox;
}

QString FormFieldButton::caption() const
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    QString ret;
    if (fwb->getButtonType() == formButtonPush) {
        Dict *dict = m_formData->fm->getObj()->getDict();
        Object obj1 = dict->lookup("MK");
        if (obj1.isDict()) {
            AnnotAppearanceCharacs appearCharacs(obj1.getDict());
            if (appearCharacs.getNormalCaption()) {
                ret = UnicodeParsedString(appearCharacs.getNormalCaption());
            }
        }
    } else {
        if (const char *goo = fwb->getOnStr()) {
            ret = QString::fromUtf8(goo);
        }
    }
    return ret;
}

FormFieldIcon FormFieldButton::icon() const
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    if (fwb->getButtonType() == formButtonPush) {
        Dict *dict = m_formData->fm->getObj()->getDict();
        FormFieldIconData *data = new FormFieldIconData;
        data->icon = dict;
        return FormFieldIcon(data);
    }
    return FormFieldIcon(nullptr);
}

void FormFieldButton::setIcon(const FormFieldIcon &icon)
{
    if (FormFieldIconData::getData(icon) == nullptr)
        return;

    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    if (fwb->getButtonType() == formButtonPush) {
        ::AnnotWidget *w = m_formData->fm->getWidgetAnnotation();
        FormFieldIconData *data = FormFieldIconData::getData(icon);
        if (data->icon != nullptr)
            w->setNewAppearance(data->icon->lookup("AP"));
    }
}

bool FormFieldButton::state() const
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    return fwb->getState();
}

void FormFieldButton::setState(bool state)
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    fwb->setState((bool)state);
}

QList<int> FormFieldButton::siblings() const
{
    FormWidgetButton *fwb = static_cast<FormWidgetButton *>(m_formData->fm);
    ::FormFieldButton *ffb = static_cast<::FormFieldButton *>(fwb->getField());
    if (fwb->getButtonType() == formButtonPush)
        return QList<int>();

    QList<int> ret;
    for (int i = 0; i < ffb->getNumSiblings(); ++i) {
        ::FormFieldButton *sibling = static_cast<::FormFieldButton *>(ffb->getSibling(i));
        for (int j = 0; j < sibling->getNumWidgets(); ++j) {
            FormWidget *w = sibling->getWidget(j);
            if (w)
                ret.append(w->getID());
        }
    }

    return ret;
}

FormFieldText::FormFieldText(DocumentData *doc, ::Page *p, ::FormWidgetText *w) : FormField(std::make_unique<FormFieldData>(doc, p, w)) { }

FormFieldText::~FormFieldText() { }

FormField::FormType FormFieldText::type() const
{
    return FormField::FormText;
}

FormFieldText::TextType FormFieldText::textType() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    if (fwt->isFileSelect())
        return FormFieldText::FileSelect;
    else if (fwt->isMultiline())
        return FormFieldText::Multiline;
    return FormFieldText::Normal;
}

QString FormFieldText::text() const
{
    const GooString *goo = static_cast<FormWidgetText *>(m_formData->fm)->getContent();
    return UnicodeParsedString(goo);
}

void FormFieldText::setText(const QString &text)
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    GooString *goo = QStringToUnicodeGooString(text);
    fwt->setContent(goo);
    delete goo;
}

void FormFieldText::setAppearanceText(const QString &text)
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    GooString *goo = QStringToUnicodeGooString(text);
    fwt->setAppearanceContent(goo);
    delete goo;
}

bool FormFieldText::isPassword() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    return fwt->isPassword();
}

bool FormFieldText::isRichText() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    return fwt->isRichText();
}

int FormFieldText::maximumLength() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    const int maxlen = fwt->getMaxLen();
    return maxlen > 0 ? maxlen : -1;
}

Qt::Alignment FormFieldText::textAlignment() const
{
    return formTextAlignment(m_formData->fm);
}

bool FormFieldText::canBeSpellChecked() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    return !fwt->noSpellCheck();
}

double FormFieldText::getFontSize() const
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    return fwt->getTextFontSize();
}

void FormFieldText::setFontSize(int fontSize)
{
    FormWidgetText *fwt = static_cast<FormWidgetText *>(m_formData->fm);
    fwt->setTextFontSize(fontSize);
}

FormFieldChoice::FormFieldChoice(DocumentData *doc, ::Page *p, ::FormWidgetChoice *w) : FormField(std::make_unique<FormFieldData>(doc, p, w)) { }

FormFieldChoice::~FormFieldChoice() { }

FormFieldChoice::FormType FormFieldChoice::type() const
{
    return FormField::FormChoice;
}

FormFieldChoice::ChoiceType FormFieldChoice::choiceType() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    if (fwc->isCombo())
        return FormFieldChoice::ComboBox;
    return FormFieldChoice::ListBox;
}

QStringList FormFieldChoice::choices() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    QStringList ret;
    int num = fwc->getNumChoices();
    ret.reserve(num);
    for (int i = 0; i < num; ++i) {
        ret.append(UnicodeParsedString(fwc->getChoice(i)));
    }
    return ret;
}

QVector<QPair<QString, QString>> FormFieldChoice::choicesWithExportValues() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    QVector<QPair<QString, QString>> ret;
    const int num = fwc->getNumChoices();
    ret.reserve(num);
    for (int i = 0; i < num; ++i) {
        const QString display = UnicodeParsedString(fwc->getChoice(i));
        const GooString *exportValueG = fwc->getExportVal(i);
        const QString exportValue = exportValueG ? UnicodeParsedString(exportValueG) : display;
        ret.append({ display, exportValue });
    }
    return ret;
}

bool FormFieldChoice::isEditable() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    return fwc->isCombo() ? fwc->hasEdit() : false;
}

bool FormFieldChoice::multiSelect() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    return !fwc->isCombo() ? fwc->isMultiSelect() : false;
}

QList<int> FormFieldChoice::currentChoices() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    int num = fwc->getNumChoices();
    QList<int> choices;
    for (int i = 0; i < num; ++i)
        if (fwc->isSelected(i))
            choices.append(i);
    return choices;
}

void FormFieldChoice::setCurrentChoices(const QList<int> &choice)
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    fwc->deselectAll();
    for (int i = 0; i < choice.count(); ++i)
        fwc->select(choice.at(i));
}

QString FormFieldChoice::editChoice() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);

    if (fwc->isCombo() && fwc->hasEdit())
        return UnicodeParsedString(fwc->getEditChoice());
    else
        return QString();
}

void FormFieldChoice::setEditChoice(const QString &text)
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);

    if (fwc->isCombo() && fwc->hasEdit()) {
        GooString *goo = QStringToUnicodeGooString(text);
        fwc->setEditChoice(goo);
        delete goo;
    }
}

Qt::Alignment FormFieldChoice::textAlignment() const
{
    return formTextAlignment(m_formData->fm);
}

bool FormFieldChoice::canBeSpellChecked() const
{
    FormWidgetChoice *fwc = static_cast<FormWidgetChoice *>(m_formData->fm);
    return !fwc->noSpellCheck();
}

class CertificateInfoPrivate
{
public:
    struct EntityInfo
    {
        QString common_name;
        QString email_address;
        QString org_name;
        QString distinguished_name;
    };

    EntityInfo issuer_info;
    EntityInfo subject_info;
    QByteArray certificate_der;
    QByteArray serial_number;
    QByteArray public_key;
    QDateTime validity_start;
    QDateTime validity_end;
    int public_key_type;
    int public_key_strength;
    int ku_extensions;
    int version;
    bool is_self_signed;
    bool is_null;
};

CertificateInfo::CertificateInfo(CertificateInfoPrivate *priv) : d_ptr(priv) { }

CertificateInfo::CertificateInfo(const CertificateInfo &other) : d_ptr(other.d_ptr) { }

CertificateInfo::~CertificateInfo() = default;

CertificateInfo &CertificateInfo::operator=(const CertificateInfo &other)
{
    if (this != &other)
        d_ptr = other.d_ptr;

    return *this;
}

bool CertificateInfo::isNull() const
{
    Q_D(const CertificateInfo);
    return d->is_null;
}

int CertificateInfo::version() const
{
    Q_D(const CertificateInfo);
    return d->version;
}

QByteArray CertificateInfo::serialNumber() const
{
    Q_D(const CertificateInfo);
    return d->serial_number;
}

QString CertificateInfo::issuerInfo(EntityInfoKey key) const
{
    Q_D(const CertificateInfo);
    switch (key) {
    case CommonName:
        return d->issuer_info.common_name;
    case DistinguishedName:
        return d->issuer_info.distinguished_name;
    case EmailAddress:
        return d->issuer_info.email_address;
    case Organization:
        return d->issuer_info.org_name;
    default:
        return QString();
    }
}

QString CertificateInfo::subjectInfo(EntityInfoKey key) const
{
    Q_D(const CertificateInfo);
    switch (key) {
    case CommonName:
        return d->subject_info.common_name;
    case DistinguishedName:
        return d->subject_info.distinguished_name;
    case EmailAddress:
        return d->subject_info.email_address;
    case Organization:
        return d->subject_info.org_name;
    default:
        return QString();
    }
}

QDateTime CertificateInfo::validityStart() const
{
    Q_D(const CertificateInfo);
    return d->validity_start;
}

QDateTime CertificateInfo::validityEnd() const
{
    Q_D(const CertificateInfo);
    return d->validity_end;
}

CertificateInfo::KeyUsageExtensions CertificateInfo::keyUsageExtensions() const
{
    Q_D(const CertificateInfo);

    KeyUsageExtensions kuExtensions = KuNone;
    if (d->ku_extensions & KU_DIGITAL_SIGNATURE)
        kuExtensions |= KuDigitalSignature;
    if (d->ku_extensions & KU_NON_REPUDIATION)
        kuExtensions |= KuNonRepudiation;
    if (d->ku_extensions & KU_KEY_ENCIPHERMENT)
        kuExtensions |= KuKeyEncipherment;
    if (d->ku_extensions & KU_DATA_ENCIPHERMENT)
        kuExtensions |= KuDataEncipherment;
    if (d->ku_extensions & KU_KEY_AGREEMENT)
        kuExtensions |= KuKeyAgreement;
    if (d->ku_extensions & KU_KEY_CERT_SIGN)
        kuExtensions |= KuKeyCertSign;
    if (d->ku_extensions & KU_CRL_SIGN)
        kuExtensions |= KuClrSign;
    if (d->ku_extensions & KU_ENCIPHER_ONLY)
        kuExtensions |= KuEncipherOnly;

    return kuExtensions;
}

QByteArray CertificateInfo::publicKey() const
{
    Q_D(const CertificateInfo);
    return d->public_key;
}

CertificateInfo::PublicKeyType CertificateInfo::publicKeyType() const
{
    Q_D(const CertificateInfo);
    switch (d->public_key_type) {
    case RSAKEY:
        return RsaKey;
    case DSAKEY:
        return DsaKey;
    case ECKEY:
        return EcKey;
    default:
        return OtherKey;
    }
}

int CertificateInfo::publicKeyStrength() const
{
    Q_D(const CertificateInfo);
    return d->public_key_strength;
}

bool CertificateInfo::isSelfSigned() const
{
    Q_D(const CertificateInfo);
    return d->is_self_signed;
}

QByteArray CertificateInfo::certificateData() const
{
    Q_D(const CertificateInfo);
    return d->certificate_der;
}

class SignatureValidationInfoPrivate
{
public:
    SignatureValidationInfoPrivate(CertificateInfo &&ci) : cert_info(ci) { }

    SignatureValidationInfo::SignatureStatus signature_status;
    SignatureValidationInfo::CertificateStatus certificate_status;
    CertificateInfo cert_info;

    QByteArray signature;
    QString signer_name;
    QString signer_subject_dn;
    QString location;
    QString reason;
    int hash_algorithm;
    time_t signing_time;
    QList<qint64> range_bounds;
    qint64 docLength;
};

SignatureValidationInfo::SignatureValidationInfo(SignatureValidationInfoPrivate *priv) : d_ptr(priv) { }

SignatureValidationInfo::SignatureValidationInfo(const SignatureValidationInfo &other) : d_ptr(other.d_ptr) { }

SignatureValidationInfo::~SignatureValidationInfo() { }

SignatureValidationInfo::SignatureStatus SignatureValidationInfo::signatureStatus() const
{
    Q_D(const SignatureValidationInfo);
    return d->signature_status;
}

SignatureValidationInfo::CertificateStatus SignatureValidationInfo::certificateStatus() const
{
    Q_D(const SignatureValidationInfo);
    return d->certificate_status;
}

QString SignatureValidationInfo::signerName() const
{
    Q_D(const SignatureValidationInfo);
    return d->signer_name;
}

QString SignatureValidationInfo::signerSubjectDN() const
{
    Q_D(const SignatureValidationInfo);
    return d->signer_subject_dn;
}

QString SignatureValidationInfo::location() const
{
    Q_D(const SignatureValidationInfo);
    return d->location;
}

QString SignatureValidationInfo::reason() const
{
    Q_D(const SignatureValidationInfo);
    return d->reason;
}

SignatureValidationInfo::HashAlgorithm SignatureValidationInfo::hashAlgorithm() const
{
#ifdef ENABLE_NSS3
    Q_D(const SignatureValidationInfo);

    switch (d->hash_algorithm) {
    case HASH_AlgMD2:
        return HashAlgorithmMd2;
    case HASH_AlgMD5:
        return HashAlgorithmMd5;
    case HASH_AlgSHA1:
        return HashAlgorithmSha1;
    case HASH_AlgSHA256:
        return HashAlgorithmSha256;
    case HASH_AlgSHA384:
        return HashAlgorithmSha384;
    case HASH_AlgSHA512:
        return HashAlgorithmSha512;
    case HASH_AlgSHA224:
        return HashAlgorithmSha224;
    }
#endif
    return HashAlgorithmUnknown;
}

time_t SignatureValidationInfo::signingTime() const
{
    Q_D(const SignatureValidationInfo);
    return d->signing_time;
}

QByteArray SignatureValidationInfo::signature() const
{
    Q_D(const SignatureValidationInfo);
    return d->signature;
}

QList<qint64> SignatureValidationInfo::signedRangeBounds() const
{
    Q_D(const SignatureValidationInfo);
    return d->range_bounds;
}

bool SignatureValidationInfo::signsTotalDocument() const
{
    Q_D(const SignatureValidationInfo);
    if (d->range_bounds.size() == 4 && d->range_bounds.value(0) == 0 && d->range_bounds.value(1) >= 0 && d->range_bounds.value(2) > d->range_bounds.value(1) && d->range_bounds.value(3) >= d->range_bounds.value(2)) {
        // The range from d->range_bounds.value(1) to d->range_bounds.value(2) is
        // not authenticated by the signature and should only contain the signature
        // itself padded with 0 bytes. This has been checked in readSignature().
        // If it failed, d->signature is empty.
        // A potential range after d->range_bounds.value(3) would be also not
        // authenticated. Therefore d->range_bounds.value(3) should coincide with
        // the end of the document.
        if (d->docLength == d->range_bounds.value(3) && !d->signature.isEmpty())
            return true;
    }
    return false;
}

CertificateInfo SignatureValidationInfo::certificateInfo() const
{
    Q_D(const SignatureValidationInfo);
    return d->cert_info;
}

SignatureValidationInfo &SignatureValidationInfo::operator=(const SignatureValidationInfo &other)
{
    if (this != &other)
        d_ptr = other.d_ptr;

    return *this;
}

FormFieldSignature::FormFieldSignature(DocumentData *doc, ::Page *p, ::FormWidgetSignature *w) : FormField(std::make_unique<FormFieldData>(doc, p, w)) { }

FormFieldSignature::~FormFieldSignature() { }

FormField::FormType FormFieldSignature::type() const
{
    return FormField::FormSignature;
}

FormFieldSignature::SignatureType FormFieldSignature::signatureType() const
{
    SignatureType sigType = AdbePkcs7detached;
    FormWidgetSignature *fws = static_cast<FormWidgetSignature *>(m_formData->fm);
    switch (fws->signatureType()) {
    case adbe_pkcs7_sha1:
        sigType = AdbePkcs7sha1;
        break;
    case adbe_pkcs7_detached:
        sigType = AdbePkcs7detached;
        break;
    case ETSI_CAdES_detached:
        sigType = EtsiCAdESdetached;
        break;
    case unknown_signature_type:
        sigType = UnknownSignatureType;
        break;
    }
    return sigType;
}

SignatureValidationInfo FormFieldSignature::validate(ValidateOptions opt) const
{
    return validate(opt, QDateTime());
}

SignatureValidationInfo FormFieldSignature::validate(int opt, const QDateTime &validationTime) const
{
    FormWidgetSignature *fws = static_cast<FormWidgetSignature *>(m_formData->fm);
    const time_t validationTimeT = validationTime.isValid() ? validationTime.toTime_t() : -1;
    SignatureInfo *si = fws->validateSignature(opt & ValidateVerifyCertificate, opt & ValidateForceRevalidation, validationTimeT);

    // get certificate info
    const X509CertificateInfo *ci = si->getCertificateInfo();
    CertificateInfoPrivate *certPriv = new CertificateInfoPrivate;
    certPriv->is_null = true;
    if (ci) {
        certPriv->version = ci->getVersion();
        certPriv->ku_extensions = ci->getKeyUsageExtensions();

        const GooString &certSerial = ci->getSerialNumber();
        certPriv->serial_number = QByteArray(certSerial.c_str(), certSerial.getLength());

        const X509CertificateInfo::EntityInfo &issuerInfo = ci->getIssuerInfo();
        certPriv->issuer_info.common_name = issuerInfo.commonName.c_str();
        certPriv->issuer_info.distinguished_name = issuerInfo.distinguishedName.c_str();
        certPriv->issuer_info.email_address = issuerInfo.email.c_str();
        certPriv->issuer_info.org_name = issuerInfo.organization.c_str();

        const X509CertificateInfo::EntityInfo &subjectInfo = ci->getSubjectInfo();
        certPriv->subject_info.common_name = subjectInfo.commonName.c_str();
        certPriv->subject_info.distinguished_name = subjectInfo.distinguishedName.c_str();
        certPriv->subject_info.email_address = subjectInfo.email.c_str();
        certPriv->subject_info.org_name = subjectInfo.organization.c_str();

        X509CertificateInfo::Validity certValidity = ci->getValidity();
        certPriv->validity_start = QDateTime::fromTime_t(certValidity.notBefore, Qt::UTC);
        certPriv->validity_end = QDateTime::fromTime_t(certValidity.notAfter, Qt::UTC);

        const X509CertificateInfo::PublicKeyInfo &pkInfo = ci->getPublicKeyInfo();
        certPriv->public_key = QByteArray(pkInfo.publicKey.c_str(), pkInfo.publicKey.getLength());
        certPriv->public_key_type = static_cast<int>(pkInfo.publicKeyType);
        certPriv->public_key_strength = pkInfo.publicKeyStrength;

        const GooString &certDer = ci->getCertificateDER();
        certPriv->certificate_der = QByteArray(certDer.c_str(), certDer.getLength());

        certPriv->is_null = false;
    }

    SignatureValidationInfoPrivate *priv = new SignatureValidationInfoPrivate(CertificateInfo(certPriv));
    switch (si->getSignatureValStatus()) {
    case SIGNATURE_VALID:
        priv->signature_status = SignatureValidationInfo::SignatureValid;
        break;
    case SIGNATURE_INVALID:
        priv->signature_status = SignatureValidationInfo::SignatureInvalid;
        break;
    case SIGNATURE_DIGEST_MISMATCH:
        priv->signature_status = SignatureValidationInfo::SignatureDigestMismatch;
        break;
    case SIGNATURE_DECODING_ERROR:
        priv->signature_status = SignatureValidationInfo::SignatureDecodingError;
        break;
    default:
    case SIGNATURE_GENERIC_ERROR:
        priv->signature_status = SignatureValidationInfo::SignatureGenericError;
        break;
    case SIGNATURE_NOT_FOUND:
        priv->signature_status = SignatureValidationInfo::SignatureNotFound;
        break;
    case SIGNATURE_NOT_VERIFIED:
        priv->signature_status = SignatureValidationInfo::SignatureNotVerified;
        break;
    }
    switch (si->getCertificateValStatus()) {
    case CERTIFICATE_TRUSTED:
        priv->certificate_status = SignatureValidationInfo::CertificateTrusted;
        break;
    case CERTIFICATE_UNTRUSTED_ISSUER:
        priv->certificate_status = SignatureValidationInfo::CertificateUntrustedIssuer;
        break;
    case CERTIFICATE_UNKNOWN_ISSUER:
        priv->certificate_status = SignatureValidationInfo::CertificateUnknownIssuer;
        break;
    case CERTIFICATE_REVOKED:
        priv->certificate_status = SignatureValidationInfo::CertificateRevoked;
        break;
    case CERTIFICATE_EXPIRED:
        priv->certificate_status = SignatureValidationInfo::CertificateExpired;
        break;
    default:
    case CERTIFICATE_GENERIC_ERROR:
        priv->certificate_status = SignatureValidationInfo::CertificateGenericError;
        break;
    case CERTIFICATE_NOT_VERIFIED:
        priv->certificate_status = SignatureValidationInfo::CertificateNotVerified;
        break;
    }
    priv->signer_name = si->getSignerName();
    priv->signer_subject_dn = si->getSubjectDN();
    priv->hash_algorithm = si->getHashAlgorithm();
    priv->location = si->getLocation();
    priv->reason = si->getReason();

    priv->signing_time = si->getSigningTime();
    const std::vector<Goffset> ranges = fws->getSignedRangeBounds();
    if (!ranges.empty()) {
        for (Goffset bound : ranges) {
            priv->range_bounds.append(bound);
        }
    }
    GooString *checkedSignature = fws->getCheckedSignature(&priv->docLength);
    if (priv->range_bounds.size() == 4 && checkedSignature) {
        priv->signature = QByteArray::fromHex(checkedSignature->c_str());
    }
    delete checkedSignature;

    return SignatureValidationInfo(priv);
}

}
