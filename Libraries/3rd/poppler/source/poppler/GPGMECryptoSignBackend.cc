//========================================================================
//
// GPGMECryptoSignBackend.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//========================================================================
#include "config.h"
#include "GPGMECryptoSignBackend.h"
#include "DistinguishedNameParser.h"
#include <gpgme.h>
#include <gpgme++/key.h>
#include <gpgme++/gpgmepp_version.h>
#include <gpgme++/signingresult.h>
#include <gpgme++/engineinfo.h>

bool GpgSignatureBackend::hasSufficientVersion()
{
    // gpg 2.4.0 does not support padded signatures.
    // Most gpg signatures are padded. This is fixed for 2.4.1
    // gpg 2.4.0 does not support generating signatures
    // with definite lengths. This is also fixed for 2.4.1.
    // this has also been fixed in 2.2.42 in the 2.2 branch
    auto version = GpgME::engineInfo(GpgME::GpgSMEngine).engineVersion();
    if (version > "2.4.0") {
        return true;
    }
    if (version >= "2.3.0") { // development branch for 2.4 releases; no more releases here
        return false;
    }
    return version >= "2.2.42";
}

/// GPGME helper methods

// gpgme++'s string-like functions returns char pointers that can be nullptr
// Creating std::string from nullptr is, depending on c++ standards versions
// either undefined behavior or illegal, so we need a helper.

static std::string fromCharPtr(const char *data)
{
    if (data) {
        return std::string { data };
    }
    return {};
}

static bool isSuccess(const GpgME::Error &err)
{
    if (err) {
        return false;
    }
    if (err.isCanceled()) {
        return false;
    }
    return true;
}

template<typename Result>
static bool isValidResult(const Result &result)
{
    return isSuccess(result.error());
}

template<typename Result>
static bool hasValidResult(const std::optional<Result> &result)
{
    if (!result) {
        return false;
    }
    return isValidResult(result.value());
}

static std::optional<GpgME::Signature> getSignature(const GpgME::VerificationResult &result, size_t signatureNumber)
{
    if (result.numSignatures() > signatureNumber) {
        return result.signature(signatureNumber);
    }
    return std::nullopt;
}

static X509CertificateInfo::Validity getValidityFromSubkey(const GpgME::Subkey &key)
{
    X509CertificateInfo::Validity validity;
    validity.notBefore = key.creationTime();
    validity.notAfter = key.expirationTime();
    return validity;
}

static X509CertificateInfo::EntityInfo getEntityInfoFromKey(std::string_view dnString)
{
    const auto dn = DN::parseString(dnString);
    X509CertificateInfo::EntityInfo info;
    info.commonName = DN::FindFirstValue(dn, "CN").value_or(std::string {});
    info.organization = DN::FindFirstValue(dn, "O").value_or(std::string {});
    info.email = DN::FindFirstValue(dn, "EMAIL").value_or(std::string {});
    info.distinguishedName = std::string { dnString };
    return info;
}

static std::unique_ptr<X509CertificateInfo> getCertificateInfoFromKey(const GpgME::Key &key)
{
    auto certificateInfo = std::make_unique<X509CertificateInfo>();
    certificateInfo->setIssuerInfo(getEntityInfoFromKey(fromCharPtr(key.issuerName())));
    certificateInfo->setSerialNumber(GooString { DN::detail::parseHexString(fromCharPtr(key.issuerSerial())).value_or("") });
    auto subjectInfo = getEntityInfoFromKey(fromCharPtr(key.userID(0).id()));
    if (subjectInfo.email.empty()) {
        subjectInfo.email = fromCharPtr(key.userID(1).email());
    }
    certificateInfo->setSubjectInfo(std::move(subjectInfo));
    certificateInfo->setValidity(getValidityFromSubkey(key.subkey(0)));
    certificateInfo->setNickName(GooString(fromCharPtr(key.primaryFingerprint())));
    X509CertificateInfo::PublicKeyInfo pkInfo;
    pkInfo.publicKeyStrength = key.subkey(0).length();
    switch (key.subkey(0).publicKeyAlgorithm()) {
    case GpgME::Subkey::AlgoDSA:
        pkInfo.publicKeyType = DSAKEY;
        break;
    case GpgME::Subkey::AlgoECC:
    case GpgME::Subkey::AlgoECDH:
    case GpgME::Subkey::AlgoECDSA:
    case GpgME::Subkey::AlgoEDDSA:
        pkInfo.publicKeyType = ECKEY;
        break;
    case GpgME::Subkey::AlgoRSA:
    case GpgME::Subkey::AlgoRSA_E:
    case GpgME::Subkey::AlgoRSA_S:
        pkInfo.publicKeyType = RSAKEY;
        break;
    case GpgME::Subkey::AlgoELG:
    case GpgME::Subkey::AlgoELG_E:
    case GpgME::Subkey::AlgoMax:
    case GpgME::Subkey::AlgoUnknown:
        pkInfo.publicKeyType = OTHERKEY;
    }
    {
        auto ctx = GpgME::Context::create(GpgME::CMS);
        GpgME::Data pubkeydata;
        const auto err = ctx->exportPublicKeys(key.primaryFingerprint(), pubkeydata);
        if (isSuccess(err)) {
            certificateInfo->setCertificateDER(GooString(pubkeydata.toString()));
        }
    }

    certificateInfo->setPublicKeyInfo(std::move(pkInfo));

    int kue = 0;
    // this block is kind of a hack. GPGSM collapses multiple
    // into one bit, so trying to match it back can never be good
    if (key.canSign()) {
        kue |= KU_NON_REPUDIATION;
        kue |= KU_DIGITAL_SIGNATURE;
    }
    if (key.canEncrypt()) {
        kue |= KU_KEY_ENCIPHERMENT;
        kue |= KU_DATA_ENCIPHERMENT;
    }
    if (key.canCertify()) {
        kue |= KU_KEY_CERT_SIGN;
    }
    certificateInfo->setKeyUsageExtensions(kue);

    auto subkey = key.subkey(0);
    if (subkey.isCardKey()) {
        certificateInfo->setKeyLocation(KeyLocation::HardwareToken);
    } else if (subkey.isSecret()) {
        certificateInfo->setKeyLocation(KeyLocation::Computer);
    }

    return certificateInfo;
}

/// implementation of header file

GpgSignatureBackend::GpgSignatureBackend()
{
    GpgME::initializeLibrary();
}

std::unique_ptr<CryptoSign::SigningInterface> GpgSignatureBackend::createSigningHandler(const std::string &certID, HashAlgorithm /*digestAlgTag*/)
{
    return std::make_unique<GpgSignatureCreation>(certID);
}

std::unique_ptr<CryptoSign::VerificationInterface> GpgSignatureBackend::createVerificationHandler(std::vector<unsigned char> &&pkcs7)
{
    return std::make_unique<GpgSignatureVerification>(std::move(pkcs7));
}

std::vector<std::unique_ptr<X509CertificateInfo>> GpgSignatureBackend::getAvailableSigningCertificates()
{
    std::vector<std::unique_ptr<X509CertificateInfo>> certificates;
    const auto context = GpgME::Context::create(GpgME::CMS);
    auto err = context->startKeyListing(static_cast<const char *>(nullptr), true /*secretOnly*/);
    while (isSuccess(err)) {
        const auto key = context->nextKey(err);
        if (!key.isNull() && isSuccess(err)) {
            if (key.isBad()) {
                continue;
            }
            if (!key.canSign()) {
                continue;
            }
            certificates.push_back(getCertificateInfoFromKey(key));
        } else {
            break;
        }
    }
    return certificates;
}

GpgSignatureCreation::GpgSignatureCreation(const std::string &certId) : gpgContext { GpgME::Context::create(GpgME::CMS) }
{
    GpgME::Error error;
    const auto signingKey = gpgContext->key(certId.c_str(), error, true);
    if (isSuccess(error)) {
        gpgContext->addSigningKey(signingKey);
        key = signingKey;
    }
}

void GpgSignatureCreation::addData(unsigned char *dataBlock, int dataLen)
{
    gpgData.write(dataBlock, dataLen);
}
std::optional<GooString> GpgSignatureCreation::signDetached(const std::string &password)
{
    if (!key) {
        return {};
    }
    gpgData.rewind();
    GpgME::Data signatureData;
    const auto signingResult = gpgContext->sign(gpgData, signatureData, GpgME::SignatureMode::Detached);
    if (!isValidResult(signingResult)) {
        return {};
    }

    const auto signatureString = signatureData.toString();
    return GooString(std::move(signatureString));
}

std::unique_ptr<X509CertificateInfo> GpgSignatureCreation::getCertificateInfo() const
{
    if (!key) {
        return nullptr;
    }
    return getCertificateInfoFromKey(*key);
}

GpgSignatureVerification::GpgSignatureVerification(const std::vector<unsigned char> &p7data) : gpgContext { GpgME::Context::create(GpgME::CMS) }, signatureData(reinterpret_cast<const char *>(p7data.data()), p7data.size())
{
    gpgContext->setOffline(true);
    signatureData.setEncoding(GpgME::Data::BinaryEncoding);
}

void GpgSignatureVerification::addData(unsigned char *dataBlock, int dataLen)
{
    signedData.write(dataBlock, dataLen);
}

std::unique_ptr<X509CertificateInfo> GpgSignatureVerification::getCertificateInfo() const
{
    if (!hasValidResult(gpgResult)) {
        return nullptr;
    }
    auto signature = getSignature(gpgResult.value(), 0);
    if (!signature) {
        return nullptr;
    }
    auto gpgInfo = getCertificateInfoFromKey(signature->key(true, false));
    return gpgInfo;
}

HashAlgorithm GpgSignatureVerification::getHashAlgorithm() const
{
    if (gpgResult) {
        const auto signature = getSignature(gpgResult.value(), 0);
        if (!signature) {
            return HashAlgorithm::Unknown;
        }
        switch (signature->hashAlgorithm()) {
        case GPGME_MD_MD5:
            return HashAlgorithm::Md5;
        case GPGME_MD_SHA1:
            return HashAlgorithm::Sha1;
        case GPGME_MD_MD2:
            return HashAlgorithm::Md2;
        case GPGME_MD_SHA256:
            return HashAlgorithm::Sha256;
        case GPGME_MD_SHA384:
            return HashAlgorithm::Sha384;
        case GPGME_MD_SHA512:
            return HashAlgorithm::Sha512;
        case GPGME_MD_SHA224:
            return HashAlgorithm::Sha224;
        case GPGME_MD_NONE:
        case GPGME_MD_RMD160:
        case GPGME_MD_TIGER:
        case GPGME_MD_HAVAL:
        case GPGME_MD_MD4:
        case GPGME_MD_CRC32:
        case GPGME_MD_CRC32_RFC1510:
        case GPGME_MD_CRC24_RFC2440:
        default:
            return HashAlgorithm::Unknown;
        }
    }
    return HashAlgorithm::Unknown;
}

std::string GpgSignatureVerification::getSignerName() const
{
    if (!hasValidResult(gpgResult)) {
        return {};
    }

    const auto signature = getSignature(gpgResult.value(), 0);
    if (!signature) {
        return {};
    }
    const auto dn = DN::parseString(fromCharPtr(signature->key(true, false).userID(0).id()));
    return DN::FindFirstValue(dn, "CN").value_or("");
}

std::string GpgSignatureVerification::getSignerSubjectDN() const
{
    if (!hasValidResult(gpgResult)) {
        return {};
    }
    const auto signature = getSignature(gpgResult.value(), 0);
    if (!signature) {
        return {};
    }
    return fromCharPtr(signature->key(true, false).userID(0).id());
}

std::chrono::system_clock::time_point GpgSignatureVerification::getSigningTime() const
{
    if (!hasValidResult(gpgResult)) {
        return {};
    }
    const auto signature = getSignature(gpgResult.value(), 0);
    if (!signature) {
        return {};
    }
    return std::chrono::system_clock::from_time_t(signature->creationTime());
}

CertificateValidationStatus GpgSignatureVerification::validateCertificate(std::chrono::system_clock::time_point validation_time, bool ocspRevocationCheck, bool useAIACertFetch)
{
    if (!gpgResult) {
        return CERTIFICATE_NOT_VERIFIED;
    }
    if (gpgResult->error()) {
        return CERTIFICATE_GENERIC_ERROR;
    }
    const auto signature = getSignature(gpgResult.value(), 0);
    if (!signature) {
        return CERTIFICATE_GENERIC_ERROR;
    }
    const auto offline = gpgContext->offline();
    gpgContext->setOffline((!ocspRevocationCheck) || useAIACertFetch);
    const auto key = signature->key(true, true);
    gpgContext->setOffline(offline);
    if (key.isExpired()) {
        return CERTIFICATE_EXPIRED;
    }
    if (key.isRevoked()) {
        return CERTIFICATE_REVOKED;
    }
    if (key.isBad()) {
        return CERTIFICATE_NOT_VERIFIED;
    }
    return CERTIFICATE_TRUSTED;
}

SignatureValidationStatus GpgSignatureVerification::validateSignature()
{
    signedData.rewind();
    const auto result = gpgContext->verifyDetachedSignature(signatureData, signedData);
    gpgResult = result;

    if (!isValidResult(result)) {
        return SIGNATURE_DECODING_ERROR;
    }
    const auto signature = getSignature(result, 0);
    if (!signature) {
        return SIGNATURE_DECODING_ERROR;
    }
    // Ensure key is actually available
    signature->key(true, true);
    const auto summary = signature->summary();

    using Summary = GpgME::Signature::Summary;
    if (summary & Summary::Red) {
        return SIGNATURE_INVALID;
    }
    if (summary & Summary::Green || summary & Summary::Valid) {
        return SIGNATURE_VALID;
    }
    return SIGNATURE_GENERIC_ERROR;
}
