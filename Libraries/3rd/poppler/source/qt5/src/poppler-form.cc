/* poppler-form.h: qt interface to poppler
 * Copyright (C) 2007-2008, 2011, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, 2011, 2012, 2015-2017 Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2011 Carlos Garcia Campos <carlosgc@gnome.org>
 * Copyright (C) 2012, Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2016, Hanno Meyer-Thurow <h.mth@web.de>
 * Copyright (C) 2017, Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
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

#if defined(MIKTEX)
#  include <config.h>
#endif
#include "poppler-qt5.h"

#include <QtCore/QSizeF>

#include <Form.h>
#include <Object.h>
#include <Link.h>
#include <SignatureInfo.h>

#include "poppler-form.h"
#include "poppler-page-private.h"
#include "poppler-private.h"
#include "poppler-annotation-helper.h"

#include <math.h>
#include <ctype.h>

#ifdef ENABLE_NSS3
  #include <hasht.h>
#endif

namespace {

Qt::Alignment formTextAlignment(::FormWidget *fm)
{
  Qt::Alignment qtalign = Qt::AlignLeft;
  switch (fm->getField()->getTextQuadding())
  {
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

FormField::FormField(FormFieldData &dd)
  : m_formData(&dd)
{
  const int rotation = m_formData->page->getRotate();
  // reading the coords
  double left, top, right, bottom;
  m_formData->fm->getRect(&left, &bottom, &right, &top);
  // build a normalized transform matrix for this page at 100% scale
  GfxState gfxState( 72.0, 72.0, m_formData->page->getCropBox(), rotation, gTrue );
  double * gfxCTM = gfxState.getCTM();
  double MTX[6];
  double pageWidth = m_formData->page->getCropWidth();
  double pageHeight = m_formData->page->getCropHeight();
  // landscape and seascape page rotation: be sure to use the correct (== rotated) page size
  if (((rotation / 90) % 2) == 1)
    qSwap(pageWidth, pageHeight);
  for ( int i = 0; i < 6; i+=2 )
  {
    MTX[i] = gfxCTM[i] / pageWidth;
    MTX[i+1] = gfxCTM[i+1] / pageHeight;
  }
  QPointF topLeft;
  XPDFReader::transform( MTX, qMin( left, right ), qMax( top, bottom ), topLeft );
  QPointF bottomRight;
  XPDFReader::transform( MTX, qMax( left, right ), qMin( top, bottom ), bottomRight );
  m_formData->box = QRectF(topLeft, QSizeF(bottomRight.x() - topLeft.x(), bottomRight.y() - topLeft.y()));
}

FormField::~FormField()
{
  delete m_formData;
  m_formData = 0;
}

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
  if (GooString *goo = m_formData->fm->getPartialName())
  {
    name = QString::fromLatin1(goo->getCString());
  }
  return name;
}

void FormField::setName(const QString &name) const
{
  GooString * goo = QStringToGooString( name );
  m_formData->fm->setPartialName(*goo);
  delete goo;
}

QString FormField::fullyQualifiedName() const
{
  QString name;
  if (GooString *goo = m_formData->fm->getFullyQualifiedName())
  {
    name = UnicodeParsedString(goo);
  }
  return name;
}

QString FormField::uiName() const
{
  QString name;
  if (GooString *goo = m_formData->fm->getAlternateUiName())
  {
    name = QString::fromLatin1(goo->getCString());
  }
  return name;
}

bool FormField::isReadOnly() const
{
  return m_formData->fm->isReadOnly();
}

bool FormField::isVisible() const
{
  return !(m_formData->fm->getWidgetAnnotation()->getFlags() & Annot::flagHidden);
}

Link* FormField::activationAction() const
{
  Link* action = 0;
  if (::LinkAction *act = m_formData->fm->getActivationAction())
  {
    action = PageData::convertLinkActionToLink(act, m_formData->doc, QRectF());
  }
  return action;
}

Link *FormField::additionalAction(AdditionalActionType type) const
{
  Annot::FormAdditionalActionsType actionType = Annot::actionFieldModified;
  switch ( type )
  {
      case FieldModified:  actionType = Annot::actionFieldModified; break;
      case FormatField:    actionType = Annot::actionFormatField; break;
      case ValidateField:  actionType = Annot::actionValidateField; break;
      case CalculateField: actionType = Annot::actionCalculateField; break;

  }

  Link* action = 0;
  if (::LinkAction *act = m_formData->fm->getAdditionalAction(actionType))
  {
    action = PageData::convertLinkActionToLink(act, m_formData->doc, QRectF());
  }
  return action;
}


FormFieldButton::FormFieldButton(DocumentData *doc, ::Page *p, ::FormWidgetButton *w)
  : FormField(*new FormFieldData(doc, p, w))
{
}

FormFieldButton::~FormFieldButton()
{
}

FormFieldButton::FormType FormFieldButton::type() const
{
  return FormField::FormButton;
}

FormFieldButton::ButtonType FormFieldButton::buttonType() const
{
  FormWidgetButton* fwb = static_cast<FormWidgetButton*>(m_formData->fm);
  switch (fwb->getButtonType())
  {
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
  FormWidgetButton* fwb = static_cast<FormWidgetButton*>(m_formData->fm);
  QString ret;
  if (fwb->getButtonType() == formButtonPush)
  {
    Dict *dict = m_formData->fm->getObj()->getDict();
    Object obj1 = dict->lookup("MK");
    if (obj1.isDict())
    {
      AnnotAppearanceCharacs appearCharacs(obj1.getDict());
      if (appearCharacs.getNormalCaption())
      {
        ret = UnicodeParsedString(appearCharacs.getNormalCaption());
      }
    }
  }
  else
  {
    if (const char *goo = fwb->getOnStr())
    {
      ret = QString::fromUtf8(goo);
    }
  }
  return ret;
}

bool FormFieldButton::state() const
{
  FormWidgetButton* fwb = static_cast<FormWidgetButton*>(m_formData->fm);
  return fwb->getState();
}

void FormFieldButton::setState( bool state )
{
  FormWidgetButton* fwb = static_cast<FormWidgetButton*>(m_formData->fm);
  fwb->setState((GBool)state);
}

QList<int> FormFieldButton::siblings() const
{
  FormWidgetButton* fwb = static_cast<FormWidgetButton*>(m_formData->fm);
  ::FormFieldButton* ffb = static_cast< ::FormFieldButton* >(fwb->getField());
  if (fwb->getButtonType() == formButtonPush)
    return QList<int>();

  QList<int> ret;
  for (int i = 0; i < ffb->getNumSiblings(); ++i)
  {
    ::FormFieldButton* sibling = static_cast< ::FormFieldButton* >(ffb->getSibling(i));
    for (int j = 0; j < sibling->getNumWidgets(); ++j)
    {
        FormWidget *w = sibling->getWidget(j);
        if (w) ret.append(w->getID());
    }
  }

  return ret;
}


FormFieldText::FormFieldText(DocumentData *doc, ::Page *p, ::FormWidgetText *w)
  : FormField(*new FormFieldData(doc, p, w))
{
}

FormFieldText::~FormFieldText()
{
}

FormField::FormType FormFieldText::type() const
{
  return FormField::FormText;
}

FormFieldText::TextType FormFieldText::textType() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  if (fwt->isFileSelect())
    return FormFieldText::FileSelect;
  else if (fwt->isMultiline())
    return FormFieldText::Multiline;
  return FormFieldText::Normal;
}

QString FormFieldText::text() const
{
  GooString *goo = static_cast<FormWidgetText*>(m_formData->fm)->getContent();
  return UnicodeParsedString(goo);
}

void FormFieldText::setText( const QString& text )
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  GooString * goo = QStringToUnicodeGooString( text );
  fwt->setContent( goo );
  delete goo;
}

bool FormFieldText::isPassword() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return fwt->isPassword();
}

bool FormFieldText::isRichText() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return fwt->isRichText();
}

int FormFieldText::maximumLength() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  const int maxlen = fwt->getMaxLen();
  return maxlen > 0 ? maxlen : -1;
}

Qt::Alignment FormFieldText::textAlignment() const
{
  return formTextAlignment(m_formData->fm);
}

bool FormFieldText::canBeSpellChecked() const
{
  FormWidgetText* fwt = static_cast<FormWidgetText*>(m_formData->fm);
  return !fwt->noSpellCheck();
}


FormFieldChoice::FormFieldChoice(DocumentData *doc, ::Page *p, ::FormWidgetChoice *w)
  : FormField(*new FormFieldData(doc, p, w))
{
}

FormFieldChoice::~FormFieldChoice()
{
}

FormFieldChoice::FormType FormFieldChoice::type() const
{
  return FormField::FormChoice;
}

FormFieldChoice::ChoiceType FormFieldChoice::choiceType() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  if (fwc->isCombo())
    return FormFieldChoice::ComboBox;
  return FormFieldChoice::ListBox;
}

QStringList FormFieldChoice::choices() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  QStringList ret;
  int num = fwc->getNumChoices();
  ret.reserve(num);
  for (int i = 0; i < num; ++i)
  {
    ret.append(UnicodeParsedString(fwc->getChoice(i)));
  }
  return ret;
}

bool FormFieldChoice::isEditable() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return fwc->isCombo() ? fwc->hasEdit() : false;
}

bool FormFieldChoice::multiSelect() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return !fwc->isCombo() ? fwc->isMultiSelect() : false;
}

QList<int> FormFieldChoice::currentChoices() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  int num = fwc->getNumChoices();
  QList<int> choices;
  for ( int i = 0; i < num; ++i )
    if ( fwc->isSelected( i ) )
      choices.append( i );
  return choices;
}

void FormFieldChoice::setCurrentChoices( const QList<int> &choice )
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  fwc->deselectAll();
  for ( int i = 0; i < choice.count(); ++i )
    fwc->select( choice.at( i ) );
}

QString FormFieldChoice::editChoice() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  
  if ( fwc->isCombo() && fwc->hasEdit() )
    return UnicodeParsedString(fwc->getEditChoice());
  else
    return QString();
}

void FormFieldChoice::setEditChoice(const QString& text)
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  
  if ( fwc->isCombo() && fwc->hasEdit() )
  {
    GooString* goo = QStringToUnicodeGooString( text );
    fwc->setEditChoice( goo );
    delete goo;
  }
}

Qt::Alignment FormFieldChoice::textAlignment() const
{
  return formTextAlignment(m_formData->fm);
}

bool FormFieldChoice::canBeSpellChecked() const
{
  FormWidgetChoice* fwc = static_cast<FormWidgetChoice*>(m_formData->fm);
  return !fwc->noSpellCheck();
}


struct SignatureValidationInfoPrivate {
	SignatureValidationInfo::SignatureStatus signature_status;
	SignatureValidationInfo::CertificateStatus certificate_status;

	QByteArray signature;
	QString signer_name;
	QString signer_subject_dn;
	int hash_algorithm;
	time_t signing_time;
	QList<qint64> range_bounds;
	qint64 docLength;
};


SignatureValidationInfo::SignatureValidationInfo(SignatureValidationInfoPrivate* priv)
  : d_ptr(priv)
{
}

SignatureValidationInfo::SignatureValidationInfo(const SignatureValidationInfo &other)
 : d_ptr( other.d_ptr )
{
}

SignatureValidationInfo::~SignatureValidationInfo()
{
}

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

SignatureValidationInfo::HashAlgorithm SignatureValidationInfo::hashAlgorithm() const
{
  Q_D(const SignatureValidationInfo);

#ifdef ENABLE_NSS3
  switch (d->hash_algorithm)
  {
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
  if (d->range_bounds.size() == 4 && d->range_bounds.value(0) == 0 &&
      d->range_bounds.value(1) >= 0 &&
      d->range_bounds.value(2) > d->range_bounds.value(1) &&
      d->range_bounds.value(3) >= d->range_bounds.value(2))
  {
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

SignatureValidationInfo &SignatureValidationInfo::operator=(const SignatureValidationInfo &other)
{
  if ( this != &other )
    d_ptr = other.d_ptr;

  return *this;
}

FormFieldSignature::FormFieldSignature(DocumentData *doc, ::Page *p, ::FormWidgetSignature *w)
  : FormField(*new FormFieldData(doc, p, w))
{
}

FormFieldSignature::~FormFieldSignature()
{
}

FormField::FormType FormFieldSignature::type() const
{
  return FormField::FormSignature;
}

FormFieldSignature::SignatureType FormFieldSignature::signatureType() const
{
  SignatureType sigType = AdbePkcs7detached;
  FormWidgetSignature* fws = static_cast<FormWidgetSignature*>(m_formData->fm);
  switch (fws->signatureType())
  {
    case adbe_pkcs7_sha1:
      sigType = AdbePkcs7sha1;
      break;
    case adbe_pkcs7_detached:
      sigType = AdbePkcs7detached;
      break;
    case ETSI_CAdES_detached:
      sigType = EtsiCAdESdetached;
      break;
  }
  return sigType;
}

SignatureValidationInfo FormFieldSignature::validate(ValidateOptions opt) const
{
  return validate(opt, QDateTime());
}

SignatureValidationInfo FormFieldSignature::validate(int opt, const QDateTime& validationTime) const
{
  FormWidgetSignature* fws = static_cast<FormWidgetSignature*>(m_formData->fm);
  const time_t validationTimeT = validationTime.isValid() ? validationTime.toTime_t() : -1;
  SignatureInfo* si = fws->validateSignature(opt & ValidateVerifyCertificate, opt & ValidateForceRevalidation, validationTimeT);
  SignatureValidationInfoPrivate* priv = new SignatureValidationInfoPrivate;
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

  priv->signing_time = si->getSigningTime();
  const std::vector<Goffset> ranges = fws->getSignedRangeBounds();
  if (!ranges.empty())
  {
    for (Goffset bound : ranges)
    {
      priv->range_bounds.append(bound);
    }
  }
  GooString* checkedSignature = fws->getCheckedSignature(&priv->docLength);
  if (priv->range_bounds.size() == 4 && checkedSignature)
  {
    priv->signature = QByteArray::fromHex(checkedSignature->getCString());
  }
  delete checkedSignature;

  return SignatureValidationInfo(priv);
}

}
