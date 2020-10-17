//========================================================================
//
// CertificateInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@gmail.com>
// Copyright 2018, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#include "CertificateInfo.h"

#include <cstring>
#include <cstdlib>

X509CertificateInfo::PublicKeyInfo::PublicKeyInfo() : publicKeyType(OTHERKEY), publicKeyStrength(0) { }

X509CertificateInfo::PublicKeyInfo::PublicKeyInfo(X509CertificateInfo::PublicKeyInfo &&other) noexcept
{
    publicKey = std::move(other.publicKey);
    publicKeyType = other.publicKeyType;
    publicKeyStrength = other.publicKeyStrength;
}

X509CertificateInfo::PublicKeyInfo &X509CertificateInfo::PublicKeyInfo::operator=(X509CertificateInfo::PublicKeyInfo &&other) noexcept
{
    publicKey = std::move(other.publicKey);
    publicKeyType = other.publicKeyType;
    publicKeyStrength = other.publicKeyStrength;
    return *this;
}

X509CertificateInfo::EntityInfo::EntityInfo() = default;

X509CertificateInfo::EntityInfo::~EntityInfo() = default;

X509CertificateInfo::EntityInfo::EntityInfo(X509CertificateInfo::EntityInfo &&other) noexcept = default;

// TODO when we stop supporting gcc 5.4 use this instead of the manually defined one
// X509CertificateInfo::EntityInfo &X509CertificateInfo::EntityInfo::operator=(X509CertificateInfo::EntityInfo &&other) noexcept = default;
X509CertificateInfo::EntityInfo &X509CertificateInfo::EntityInfo::operator=(X509CertificateInfo::EntityInfo &&other) noexcept
{
    commonName = std::move(other.commonName);
    distinguishedName = std::move(other.distinguishedName);
    email = std::move(other.email);
    organization = std::move(other.organization);
    return *this;
}

X509CertificateInfo::X509CertificateInfo() : ku_extensions(KU_NONE), cert_version(-1), is_self_signed(false) { }

X509CertificateInfo::~X509CertificateInfo() = default;

int X509CertificateInfo::getVersion() const
{
    return cert_version;
}

const GooString &X509CertificateInfo::getSerialNumber() const
{
    return cert_serial;
}

const X509CertificateInfo::EntityInfo &X509CertificateInfo::getIssuerInfo() const
{
    return issuer_info;
}

const X509CertificateInfo::Validity &X509CertificateInfo::getValidity() const
{
    return cert_validity;
}

const X509CertificateInfo::EntityInfo &X509CertificateInfo::getSubjectInfo() const
{
    return subject_info;
}

const X509CertificateInfo::PublicKeyInfo &X509CertificateInfo::getPublicKeyInfo() const
{
    return public_key_info;
}

unsigned int X509CertificateInfo::getKeyUsageExtensions() const
{
    return ku_extensions;
}

const GooString &X509CertificateInfo::getCertificateDER() const
{
    return cert_der;
}

bool X509CertificateInfo::getIsSelfSigned() const
{
    return is_self_signed;
}

void X509CertificateInfo::setVersion(int version)
{
    cert_version = version;
}

void X509CertificateInfo::setSerialNumber(const GooString &serialNumber)
{
    cert_serial.Set(&serialNumber);
}

void X509CertificateInfo::setIssuerInfo(EntityInfo &&issuerInfo)
{
    issuer_info = std::move(issuerInfo);
}

void X509CertificateInfo::setValidity(Validity validity)
{
    cert_validity = validity;
}

void X509CertificateInfo::setSubjectInfo(EntityInfo &&subjectInfo)
{
    subject_info = std::move(subjectInfo);
}

void X509CertificateInfo::setPublicKeyInfo(PublicKeyInfo &&pkInfo)
{
    public_key_info = std::move(pkInfo);
}

void X509CertificateInfo::setKeyUsageExtensions(unsigned int keyUsages)
{
    ku_extensions = keyUsages;
}

void X509CertificateInfo::setCertificateDER(const GooString &certDer)
{
    cert_der.Set(&certDer);
}

void X509CertificateInfo::setIsSelfSigned(bool isSelfSigned)
{
    is_self_signed = isSelfSigned;
}
