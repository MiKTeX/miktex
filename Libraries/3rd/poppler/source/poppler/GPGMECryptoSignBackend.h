//========================================================================
//
// GPGMECryptoSignBackend.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//========================================================================
#include "CryptoSignBackend.h"

#include <gpgme++/data.h>
#include <gpgme++/context.h>
#include <optional>

class GpgSignatureBackend : public CryptoSign::Backend
{
public:
    GpgSignatureBackend();
    std::unique_ptr<CryptoSign::VerificationInterface> createVerificationHandler(std::vector<unsigned char> &&pkcs7) final;
    std::unique_ptr<CryptoSign::SigningInterface> createSigningHandler(const std::string &certID, HashAlgorithm digestAlgTag) final;
    std::vector<std::unique_ptr<X509CertificateInfo>> getAvailableSigningCertificates() final;
    static bool hasSufficientVersion();
};

class GpgSignatureCreation : public CryptoSign::SigningInterface
{
public:
    GpgSignatureCreation(const std::string &certId);
    void addData(unsigned char *dataBlock, int dataLen) final;
    std::unique_ptr<X509CertificateInfo> getCertificateInfo() const final;
    std::optional<GooString> signDetached(const std::string &password) final;

private:
    std::unique_ptr<GpgME::Context> gpgContext;
    GpgME::Data gpgData;
    std::optional<GpgME::Key> key;
};

class GpgSignatureVerification : public CryptoSign::VerificationInterface
{
public:
    explicit GpgSignatureVerification(const std::vector<unsigned char> &pkcs7data);
    SignatureValidationStatus validateSignature() final;
    void addData(unsigned char *dataBlock, int dataLen) final;
    std::chrono::system_clock::time_point getSigningTime() const final;
    std::string getSignerName() const final;
    std::string getSignerSubjectDN() const final;
    HashAlgorithm getHashAlgorithm() const final;
    CertificateValidationStatus validateCertificate(std::chrono::system_clock::time_point validation_time, bool ocspRevocationCheck, bool useAIACertFetch) final;
    std::unique_ptr<X509CertificateInfo> getCertificateInfo() const final;

private:
    std::unique_ptr<GpgME::Context> gpgContext;
    GpgME::Data signatureData;
    GpgME::Data signedData;
    std::optional<GpgME::VerificationResult> gpgResult;
};
