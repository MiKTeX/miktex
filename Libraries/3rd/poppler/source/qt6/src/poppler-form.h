/* poppler-form.h: qt interface to poppler
 * Copyright (C) 2007-2008, Pino Toscano <pino@kde.org>
 * Copyright (C) 2008, 2011, 2016, 2017, 2019-2022, Albert Astals Cid <aacid@kde.org>
 * Copyright (C) 2012, Adam Reichold <adamreichold@myopera.com>
 * Copyright (C) 2016, Hanno Meyer-Thurow <h.mth@web.de>
 * Copyright (C) 2017, Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
 * Copyright (C) 2017, Tobias C. Berner <tcberner@freebsd.org>
 * Copyright (C) 2018, Andre Heinecke <aheinecke@intevation.de>
 * Copyright (C) 2018, Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
 * Copyright (C) 2018, 2021 Oliver Sander <oliver.sander@tu-dresden.de>
 * Copyright (C) 2019 João Netto <joaonetto901@gmail.com>
 * Copyright (C) 2019, Adrian Johnson <ajohnson@redneon.com>
 * Copyright (C) 2020, Thorsten Behrens <Thorsten.Behrens@CIB.de>
 * Copyright (C) 2020, Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by Technische Universität Dresden
 * Copyright (C) 2021, Theofilos Intzoglou <int.teo@gmail.com>
 * Copyright (C) 2023, g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
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

#ifndef _POPPLER_QT6_FORM_H_
#define _POPPLER_QT6_FORM_H_

#include <functional>
#include <memory>
#include <ctime>
#include <optional>
#include <QtCore/QDateTime>
#include <QtCore/QVector>
#include <QtCore/QList>
#include <QtCore/QRectF>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include "poppler-export.h"
#include "poppler-annotation.h"
#include "poppler-qt6.h"

class Object;
class Page;
class FormWidget;
class FormWidgetButton;
class FormWidgetText;
class FormWidgetChoice;
class FormWidgetSignature;

namespace Poppler {

class DocumentData;
class Link;

class FormFieldData;
class FormFieldIconData;

/**
     The class containing the appearance information
 */

class POPPLER_QT6_EXPORT FormFieldIcon
{

    friend class FormFieldIconData;

public:
    explicit FormFieldIcon(FormFieldIconData *data);
    FormFieldIcon(const FormFieldIcon &ffIcon);
    ~FormFieldIcon();

    FormFieldIcon &operator=(const FormFieldIcon &ffIcon);

private:
    FormFieldIconData *d_ptr;
};
/**
  The base class representing a form field.
 */
class POPPLER_QT6_EXPORT FormField
{

    friend class FormFieldData;

public:
    /**
       The different types of form field.
    */
    enum FormType
    {
        FormButton, ///< A button field. See \ref Poppler::FormFieldButton::ButtonType "ButtonType"
        FormText, ///< A text field. See \ref Poppler::FormFieldText::TextType "TextType"
        FormChoice, ///< A single choice field. See \ref Poppler::FormFieldChoice::ChoiceType "ChoiceType"
        FormSignature ///< A signature field.
    };

    virtual ~FormField();

    /**
      The type of the field.
     */
    virtual FormType type() const = 0;

    /**
       \return The size of the field, in normalized coordinates, i.e.
       [0..1] with regard to the dimensions (cropbox) of the page
    */
    QRectF rect() const;

    /**
      The ID of the field.
     */
    int id() const;

    /**
      The internal name (T) of the field.
     */
    QString name() const;

    /**
      Sets the internal name (T) of the field.
     */
    void setName(const QString &name) const;

    /**
      The internal fully qualified name of the field.
     */
    QString fullyQualifiedName() const;

    /**
      The name of the field to be used in user interface (eg messages to
      the user).
     */
    QString uiName() const;

    /**
      Whether this form field is read-only.
     */
    bool isReadOnly() const;

    /**
      Set whether this form field is read-only.
     */
    void setReadOnly(bool value);

    /**
      Whether this form field is visible.
     */
    bool isVisible() const;

    /**
      Set whether this form field is visible.
     */
    void setVisible(bool value);

    /**
      Whether this field is printable.
     */
    bool isPrintable() const;

    /**
      Set whether this field is printable.
     */
    void setPrintable(bool value);

    /**
      The activation action of this form field.

      \note It may be null.
     */
    std::unique_ptr<Link> activationAction() const;

    /**
     * Describes the flags from the form 'AA' dictionary.
     */
    enum AdditionalActionType
    {
        FieldModified, ///< A JavaScript action to be performed when the user modifies the field
        FormatField, ///< A JavaScript action to be performed before the field is formatted to display its value
        ValidateField, ///< A JavaScript action to be performed when the field value changes
        CalculateField, ///< A JavaScript action to be performed when the field needs to be recalculated
    };
    /**
     * Returns a given form additional action
     */
    std::unique_ptr<Link> additionalAction(AdditionalActionType type) const;

    /**
     * Returns a given widget annotation additional action
     */
    std::unique_ptr<Link> additionalAction(Annotation::AdditionalActionType type) const;

protected:
    /// \cond PRIVATE
    explicit FormField(std::unique_ptr<FormFieldData> dd);

    std::unique_ptr<FormFieldData> m_formData;
    /// \endcond

private:
    Q_DISABLE_COPY(FormField)
};

/**
  A form field that represents a "button".
 */
class POPPLER_QT6_EXPORT FormFieldButton : public FormField
{
public:
    /**
     * The types of button field.
     */
    enum ButtonType
    {
        Push, ///< A simple push button.
        CheckBox, ///< A check box.
        Radio ///< A radio button.
    };

    /// \cond PRIVATE
    FormFieldButton(DocumentData *doc, ::Page *p, ::FormWidgetButton *w);
    /// \endcond
    ~FormFieldButton() override;

    FormType type() const override;

    /**
      The particular type of the button field.
     */
    ButtonType buttonType() const;

    /**
     * The caption to be used for the button.
     */
    QString caption() const;

    /**
     * Gets the icon used by the button
     */
    FormFieldIcon icon() const;

    /**
     * Sets a new icon for the button, it has to be a icon
     * returned by FormFieldButton::icon.
     */
    void setIcon(const FormFieldIcon &icon);

    /**
      The state of the button.
     */
    bool state() const;

    /**
      Sets the state of the button to the new \p state .
     */
    void setState(bool state);

    /**
      The list with the IDs of siblings (ie, buttons belonging to the same
      group as the current one.

      Valid only for \ref Radio buttons, an empty list otherwise.
     */
    QList<int> siblings() const;

private:
    Q_DISABLE_COPY(FormFieldButton)
};

/**
  A form field that represents a text input.
 */
class POPPLER_QT6_EXPORT FormFieldText : public FormField
{
public:
    /**
       The particular type of this text field.
    */
    enum TextType
    {
        Normal, ///< A simple singleline text field.
        Multiline, ///< A multiline text field.
        FileSelect ///< An input field to select the path of a file on disk.
    };

    /// \cond PRIVATE
    FormFieldText(DocumentData *doc, ::Page *p, ::FormWidgetText *w);
    /// \endcond
    ~FormFieldText() override;

    FormType type() const override;

    /**
      The text type of the text field.
     */
    TextType textType() const;

    /**
      The text associated with the text field.
     */
    QString text() const;

    /**
      Sets the text associated with the text field to the specified
      \p text.
     */
    void setText(const QString &text);

    /**
      Sets the text inside the Appearance Stream to the specified
      \p text
     */
    void setAppearanceText(const QString &text);

    /**
      Whether this text field is a password input, eg its text \b must be
      replaced with asterisks.

      Always false for \ref FileSelect text fields.
     */
    bool isPassword() const;

    /**
      Whether this text field should allow rich text.
     */
    bool isRichText() const;

    /**
      The maximum length for the text of this field, or -1 if not set.
     */
    int maximumLength() const;

    /**
      The horizontal alignment for the text of this text field.
     */
    Qt::Alignment textAlignment() const;

    /**
      Whether the text inserted manually in the field (where possible)
      can be spell-checked.
     */
    bool canBeSpellChecked() const;

    /**
      The font size of the text in the form field
     */
    double getFontSize() const;

    /**
      Set the font size of the text in the form field (currently only as integer)
     */
    void setFontSize(int fontSize);

private:
    Q_DISABLE_COPY(FormFieldText)
};

/**
  A form field that represents a choice field.
 */
class POPPLER_QT6_EXPORT FormFieldChoice : public FormField
{
public:
    /**
       The particular type of this choice field.
    */
    enum ChoiceType
    {
        ComboBox, ///< A simple singleline text field.
        ListBox ///< A multiline text field.
    };

    /// \cond PRIVATE
    FormFieldChoice(DocumentData *doc, ::Page *p, ::FormWidgetChoice *w);
    /// \endcond
    ~FormFieldChoice() override;

    FormType type() const override;

    /**
      The choice type of the choice field.
     */
    ChoiceType choiceType() const;

    /**
      The possible choices of the choice field.
     */
    QStringList choices() const;

    /**
      The possible choices of the choice field.
      The first value of the pair is the display name of the choice,
      The second value is the export value (i.e. for use in javascript, etc) of the choice
     */
    QVector<QPair<QString, QString>> choicesWithExportValues() const;

    /**
      Whether this FormFieldChoice::ComboBox is editable, i.e. the user
      can type in a custom value.

      Always false for the other types of choices.
     */
    bool isEditable() const;

    /**
      Whether more than one choice of this FormFieldChoice::ListBox
      can be selected at the same time.

      Always false for the other types of choices.
     */
    bool multiSelect() const;

    /**
      The currently selected choices.
     */
    QList<int> currentChoices() const;

    /**
      Sets the selected choices to \p choice.
     */
    void setCurrentChoices(const QList<int> &choice);

    /**
      The text entered into an editable combo box choice field. Otherwise a null string.
    */
    QString editChoice() const;

    /**
      Sets the text entered into an editable combo box choice field. Otherwise does nothing.
    */
    void setEditChoice(const QString &text);

    /**
      The horizontal alignment for the text of this text field.
     */
    Qt::Alignment textAlignment() const;

    /**
      Whether the text inserted manually in the field (where possible)
      can be spell-checked.

      Returns false if the field is not an editable text field.
     */
    bool canBeSpellChecked() const;

private:
    Q_DISABLE_COPY(FormFieldChoice)
};

/**
  A helper class to store x509 certificate information.
 */
class CertificateInfoPrivate;
class POPPLER_QT6_EXPORT CertificateInfo
{
public:
    /**
      The algorithm of public key.
     */
    enum PublicKeyType
    {
        RsaKey,
        DsaKey,
        EcKey,
        OtherKey
    };

    /**
      Certificate key usage extensions.
     */
    enum KeyUsageExtension
    {
        KuDigitalSignature = 0x80,
        KuNonRepudiation = 0x40,
        KuKeyEncipherment = 0x20,
        KuDataEncipherment = 0x10,
        KuKeyAgreement = 0x08,
        KuKeyCertSign = 0x04,
        KuClrSign = 0x02,
        KuEncipherOnly = 0x01,
        KuNone = 0x00
    };
    Q_DECLARE_FLAGS(KeyUsageExtensions, KeyUsageExtension)

    /**
      Predefined keys for elements in an entity's distinguished name.
     */
    enum EntityInfoKey
    {
        CommonName,
        DistinguishedName,
        EmailAddress,
        Organization,
    };

    /** A signing key can be located in different places
     sometimes. For the user, it might be easier to pick
     the key located on a card if it has some visual
     indicator that it is somehow removable.

     \note a keylocation for a certificate without a private
     key (cannot be used for signing) will likely be "Unknown"

     \since 23.09
     */
    enum class KeyLocation
    {
        Unknown, /** We don't know the location */
        Other, /** We know the location, but it is somehow not covered by this enum */
        Computer, /** The key is on this computer */
        HardwareToken /** The key is on a dedicated hardware token, either a smartcard or a dedicated usb token (e.g. gnuk, nitrokey or yubikey) */
    };

    CertificateInfo();
    explicit CertificateInfo(CertificateInfoPrivate *priv);
    ~CertificateInfo();

    /**
      Returns true if certificate has no contents; otherwise returns false
     */
    bool isNull() const;

    /**
      The certificate version string.
     */
    int version() const;

    /**
      The certificate serial number.
     */
    QByteArray serialNumber() const;

    /**
      Information about the issuer.
     */
    QString issuerInfo(EntityInfoKey key) const;

    /**
      Information about the subject
     */
    QString subjectInfo(EntityInfoKey key) const;

    /**
      The certificate internal database nickname

      \since 21.01
     */
    QString nickName() const;

    /**
      The date-time when certificate becomes valid.
     */
    QDateTime validityStart() const;

    /**
      The date-time when certificate expires.
     */
    QDateTime validityEnd() const;

    /**
      The uses allowed for the certificate.
     */
    KeyUsageExtensions keyUsageExtensions() const;

    /**
      The public key value.
     */
    QByteArray publicKey() const;

    /**
      The public key type.
     */
    PublicKeyType publicKeyType() const;

    /**
      The strength of public key in bits.
     */
    int publicKeyStrength() const;

    /**
      Returns true if certificate is self-signed otherwise returns false.
     */
    bool isSelfSigned() const;

    /**
      The DER encoded certificate.
     */
    QByteArray certificateData() const;

    /**
      Checks if the given password is the correct one for this certificate

      \since 21.01
     */
    bool checkPassword(const QString &password) const;

    /**
     The storage location for this key

     \since 23.09
     */
    KeyLocation keyLocation() const;

    CertificateInfo(const CertificateInfo &other);
    CertificateInfo &operator=(const CertificateInfo &other);

private:
    Q_DECLARE_PRIVATE(CertificateInfo)

    QSharedPointer<CertificateInfoPrivate> d_ptr;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(CertificateInfo::KeyUsageExtensions)

/**
  A signature validation info helper class.
 */
class SignatureValidationInfoPrivate;
class POPPLER_QT6_EXPORT SignatureValidationInfo
{
public:
    /**
       The verification result of the signature.
    */
    enum SignatureStatus
    {
        SignatureValid, ///< The signature is cryptographically valid.
        SignatureInvalid, ///< The signature is cryptographically invalid.
        SignatureDigestMismatch, ///< The document content was changed after the signature was applied.
        SignatureDecodingError, ///< The signature CMS/PKCS7 structure is malformed.
        SignatureGenericError, ///< The signature could not be verified.
        SignatureNotFound, ///< The requested signature is not present in the document.
        SignatureNotVerified ///< The signature is not yet verified.
    };

    /**
       The verification result of the certificate.
    */
    enum CertificateStatus
    {
        CertificateTrusted, ///< The certificate is considered trusted.
        CertificateUntrustedIssuer, ///< The issuer of this certificate has been marked as untrusted by the user.
        CertificateUnknownIssuer, ///< The certificate trust chain has not finished in a trusted root certificate.
        CertificateRevoked, ///< The certificate was revoked by the issuing certificate authority.
        CertificateExpired, ///< The signing time is outside the validity bounds of this certificate.
        CertificateGenericError, ///< The certificate could not be verified.
        CertificateNotVerified ///< The certificate is not yet verified.
    };

    /**
        The hash algorithm of the signature
     */
    enum HashAlgorithm
    {
        HashAlgorithmUnknown,
        HashAlgorithmMd2,
        HashAlgorithmMd5,
        HashAlgorithmSha1,
        HashAlgorithmSha256,
        HashAlgorithmSha384,
        HashAlgorithmSha512,
        HashAlgorithmSha224
    };

    /// \cond PRIVATE
    explicit SignatureValidationInfo(SignatureValidationInfoPrivate *priv);
    /// \endcond
    ~SignatureValidationInfo();

    /**
      The signature status of the signature.
     */
    SignatureStatus signatureStatus() const;

    /**
      The certificate status of the signature.
     */
    CertificateStatus certificateStatus() const;

    /**
      The signer name associated with the signature.
     */
    QString signerName() const;

    /**
      The signer subject distinguished name associated with the signature.
     */
    QString signerSubjectDN() const;

    /**
      Get signing location.
    */
    QString location() const;

    /**
      Get signing reason.
    */
    QString reason() const;

    /**
      The hash algorithm used for the signature.
     */
    HashAlgorithm hashAlgorithm() const;

    /**
      The signing time associated with the signature.
     */
    time_t signingTime() const;

    /**
      Get the signature binary data.
     */
    QByteArray signature() const;

    /**
      Get the bounds of the ranges of the document which are signed.
     */
    QList<qint64> signedRangeBounds() const;

    /**
      Checks whether the signature authenticates the total document
      except for the signature itself.
     */
    bool signsTotalDocument() const;

    /**
      The signer certificate info.
    */
    CertificateInfo certificateInfo() const;

    SignatureValidationInfo(const SignatureValidationInfo &other);
    SignatureValidationInfo &operator=(const SignatureValidationInfo &other);

private:
    Q_DECLARE_PRIVATE(SignatureValidationInfo)

    QSharedPointer<SignatureValidationInfoPrivate> d_ptr;
};

/**
  A form field that represents a signature.
 */
class POPPLER_QT6_EXPORT FormFieldSignature : public FormField
{
public:
    /**
        The types of signature fields.
    */
    enum SignatureType
    {
        UnknownSignatureType,
        AdbePkcs7sha1,
        AdbePkcs7detached,
        EtsiCAdESdetached,
        UnsignedSignature ///< \since 22.02
    };

    /**
       The validation options of this signature.
    */
    enum ValidateOptions
    {
        ValidateVerifyCertificate = 1, ///< Validate the certificate.
        ValidateForceRevalidation = 2, ///< Force revalidation of the certificate.
        ValidateWithoutOCSPRevocationCheck = 4, ///< Do not contact OCSP servers to check for certificate revocation status \since 21.10
        ValidateUseAIACertFetch = 8 ///< Use the AIA extension for certificate fetching \since 21.10
    };

    /// \cond PRIVATE
    FormFieldSignature(DocumentData *doc, ::Page *p, ::FormWidgetSignature *w);
    /// \endcond
    ~FormFieldSignature() override;

    FormType type() const override;

    /**
        The signature type
    */
    SignatureType signatureType() const;

    /**
      Validate the signature with now as validation time.

      Reset signature validatation info of scoped instance.

      \note depending on the backend, some options are only
      partially respected. In case of the NSS backend, the two options
      requiring network access, AIAFetch and OCSP,
      can be toggled individually. In case of the GPG backend, if either
      OCSP is used or AIAFetch is used, the other one is also used.
     */
    SignatureValidationInfo validate(ValidateOptions opt) const;

    /**
      Validate the signature with @p validationTime as validation time.

      Reset signature validatation info of scoped instance.

      \note depending on the backend, some options are only
      partially respected. In case of the NSS backend, the two options
      requiring network access, AIAFetch and OCSP,
      can be toggled individually. In case of the GPG backend, if either
      OCSP is used or AIAFetch is used, the other one is also used.
     */
    SignatureValidationInfo validate(int opt, const QDateTime &validationTime) const;

    /**
     * \since 22.02
     */
    enum SigningResult
    {
        FieldAlreadySigned, ///< Trying to sign a field that is already signed
        GenericSigningError,
        SigningSuccess
    };

    /**
      Signs a field of UnsignedSignature type.

      Ignores data.page(), data.fieldPartialName() and data.boundingRectangle()

      \since 22.02
     */
    SigningResult sign(const QString &outputFileName, const PDFConverter::NewSignatureData &data) const;

private:
    Q_DISABLE_COPY(FormFieldSignature)
};
/**
 * Possible compiled in backends for signature handling
 *
 * \since 23.06
 */
enum class CryptoSignBackend
{
    NSS,
    GPG
};

/**
 * The available compiled-in backends
 *
 * \since 23.06
 */
QVector<CryptoSignBackend> POPPLER_QT6_EXPORT availableCryptoSignBackends();

/**
 * Returns current active backend or nullopt if none is active
 *
 * \note there will always be an active backend if there is available backends
 *
 * \since 23.06
 */
std::optional<CryptoSignBackend> POPPLER_QT6_EXPORT activeCryptoSignBackend();

/**
 * Sets active backend
 *
 * \return true on success
 *
 * \since 23.06
 */
bool POPPLER_QT6_EXPORT setActiveCryptoSignBackend(CryptoSignBackend backend);

enum class CryptoSignBackendFeature
{
    /// If the backend itself out of band requests passwords
    /// or if the host applicaion somehow must do it
    BackendAsksPassphrase
};

/**
 * Queries if a backend supports or not supports a given feature.
 *
 * \since 23.06
 */
bool POPPLER_QT6_EXPORT hasCryptoSignBackendFeature(CryptoSignBackend, CryptoSignBackendFeature);

/**
  Returns is poppler was compiled with NSS support

  \deprecated Use availableBackends instead

  \since 21.01
*/
bool POPPLER_QT6_DEPRECATED POPPLER_QT6_EXPORT hasNSSSupport();

/**
  Return vector of suitable signing certificates

  \since 21.01
*/
QVector<CertificateInfo> POPPLER_QT6_EXPORT getAvailableSigningCertificates();

/**
  Gets the current NSS CertDB directory

  \since 21.01
*/
QString POPPLER_QT6_EXPORT getNSSDir();

/**
  Set a custom NSS CertDB directory. Needs to be called before doing any other signature operation

  \since 21.01
*/
void POPPLER_QT6_EXPORT setNSSDir(const QString &pathURL);

/**
  Sets the callback for NSS password requests

  \since 21.01
*/
void POPPLER_QT6_EXPORT setNSSPasswordCallback(const std::function<char *(const char *)> &f);
}

#endif
