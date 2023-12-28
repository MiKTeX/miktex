//========================================================================
//
// CryptoSignBackend.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//========================================================================

#ifndef SIGNATUREBACKEND_H
#define SIGNATUREBACKEND_H

#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include "HashAlgorithm.h"
#include "CertificateInfo.h"
#include "SignatureInfo.h"
#include "goo/GooString.h"
#include "poppler_private_export.h"

namespace CryptoSign {

// experiments seems to say that this is a bit above
// what we have seen in the wild, and much larger than
// what we have managed to get nss and gpgme to create.
static const int maxSupportedSignatureSize = 10000;

// Classes to help manage signature backends

class VerificationInterface
{
public:
    virtual void addData(unsigned char *data_block, int data_len) = 0;
    virtual SignatureValidationStatus validateSignature() = 0;
    virtual std::chrono::system_clock::time_point getSigningTime() const = 0;
    virtual std::string getSignerName() const = 0;
    virtual std::string getSignerSubjectDN() const = 0;
    virtual HashAlgorithm getHashAlgorithm() const = 0;
    virtual CertificateValidationStatus validateCertificate(std::chrono::system_clock::time_point validation_time, bool ocspRevocationCheck, bool useAIACertFetch) = 0;
    virtual std::unique_ptr<X509CertificateInfo> getCertificateInfo() const = 0;
    virtual ~VerificationInterface();
    VerificationInterface() = default;
    VerificationInterface(const VerificationInterface &other) = delete;
    VerificationInterface &operator=(const VerificationInterface &other) = delete;
};

class SigningInterface
{
public:
    virtual void addData(unsigned char *data_block, int data_len) = 0;
    virtual std::unique_ptr<X509CertificateInfo> getCertificateInfo() const = 0;
    virtual std::optional<GooString> signDetached(const std::string &password) = 0;
    virtual ~SigningInterface();
    SigningInterface() = default;
    SigningInterface(const SigningInterface &other) = delete;
    SigningInterface &operator=(const SigningInterface &other) = delete;
};

class Backend
{
public:
    enum class Type
    {
        NSS3,
        GPGME
    };
    virtual std::unique_ptr<VerificationInterface> createVerificationHandler(std::vector<unsigned char> &&pkcs7) = 0;
    virtual std::unique_ptr<SigningInterface> createSigningHandler(const std::string &certID, HashAlgorithm digestAlgTag) = 0;
    virtual std::vector<std::unique_ptr<X509CertificateInfo>> getAvailableSigningCertificates() = 0;
    virtual ~Backend();
    Backend() = default;
    Backend(const Backend &other) = delete;
    Backend &operator=(const Backend &other) = delete;
};

class POPPLER_PRIVATE_EXPORT Factory
{
public:
    // Sets the user preferred backend
    static void setPreferredBackend(Backend::Type backend);
    // Gets the current active backend
    // prioritized from 1) setPreferredBackend,
    //                  2) POPPLER_SIGNATURE_BACKEND
    //                  3) Compiled in default
    static std::optional<Backend::Type> getActive();
    static std::vector<Backend::Type> getAvailable();
    static std::unique_ptr<Backend> createActive();
    static std::unique_ptr<Backend> create(Backend::Type);
    static std::optional<Backend::Type> typeFromString(std::string_view string);
    Factory() = delete;
    /// backend specific settings

private:
    static std::optional<Backend::Type> preferredBackend;
};

}

#endif // SIGNATUREBACKEND_H
