//========================================================================
//
// CertificateInfo.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@gmail.com>
// Copyright 2018, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#ifndef CERTIFICATEINFO_H
#define CERTIFICATEINFO_H

#include <memory>
#include <ctime>
#include "goo/GooString.h"

enum CertificateKeyUsageExtension
{
    KU_DIGITAL_SIGNATURE = 0x80,
    KU_NON_REPUDIATION = 0x40,
    KU_KEY_ENCIPHERMENT = 0x20,
    KU_DATA_ENCIPHERMENT = 0x10,
    KU_KEY_AGREEMENT = 0x08,
    KU_KEY_CERT_SIGN = 0x04,
    KU_CRL_SIGN = 0x02,
    KU_ENCIPHER_ONLY = 0x01,
    KU_NONE = 0x00
};

enum PublicKeyType
{
    RSAKEY,
    DSAKEY,
    ECKEY,
    OTHERKEY
};

class X509CertificateInfo
{
public:
    X509CertificateInfo();
    ~X509CertificateInfo();

    X509CertificateInfo(const X509CertificateInfo &) = delete;
    X509CertificateInfo &operator=(const X509CertificateInfo &) = delete;

    struct PublicKeyInfo
    {
        PublicKeyInfo();

        PublicKeyInfo(PublicKeyInfo &&) noexcept;
        PublicKeyInfo &operator=(PublicKeyInfo &&) noexcept;

        PublicKeyInfo(const PublicKeyInfo &) = delete;
        PublicKeyInfo &operator=(const PublicKeyInfo &) = delete;

        GooString publicKey;
        PublicKeyType publicKeyType;
        unsigned int publicKeyStrength; // in bits
    };

    struct EntityInfo
    {
        EntityInfo();
        ~EntityInfo();

        EntityInfo(EntityInfo &&) noexcept;
        EntityInfo &operator=(EntityInfo &&) noexcept;

        EntityInfo(const EntityInfo &) = delete;
        EntityInfo &operator=(const EntityInfo &) = delete;

        std::string commonName;
        std::string distinguishedName;
        std::string email;
        std::string organization;
    };

    struct Validity
    {
        Validity() : notBefore(0), notAfter(0) { }

        time_t notBefore;
        time_t notAfter;
    };

    /* GETTERS */
    int getVersion() const;
    const GooString &getSerialNumber() const;
    const EntityInfo &getIssuerInfo() const;
    const Validity &getValidity() const;
    const EntityInfo &getSubjectInfo() const;
    const PublicKeyInfo &getPublicKeyInfo() const;
    unsigned int getKeyUsageExtensions() const;
    const GooString &getCertificateDER() const;
    bool getIsSelfSigned() const;

    /* SETTERS */
    void setVersion(int);
    void setSerialNumber(const GooString &);
    void setIssuerInfo(EntityInfo &&);
    void setValidity(Validity);
    void setSubjectInfo(EntityInfo &&);
    void setPublicKeyInfo(PublicKeyInfo &&);
    void setKeyUsageExtensions(unsigned int);
    void setCertificateDER(const GooString &);
    void setIsSelfSigned(bool);

private:
    EntityInfo issuer_info;
    EntityInfo subject_info;
    PublicKeyInfo public_key_info;
    Validity cert_validity;
    GooString cert_serial;
    GooString cert_der;
    unsigned int ku_extensions;
    int cert_version;
    bool is_self_signed;
};

#endif
