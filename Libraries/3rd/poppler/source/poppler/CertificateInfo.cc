//========================================================================
//
// CertificateInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@gmail.com>
// Copyright 2018, 2019, 2022 Albert Astals Cid <aacid@kde.org>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2020 Thorsten Behrens <Thorsten.Behrens@CIB.de>
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
//========================================================================

#include "CertificateInfo.h"

#include <cstring>
#include <cstdlib>

X509CertificateInfo::X509CertificateInfo() : ku_extensions(KU_NONE), cert_version(-1), is_self_signed(false), keyLocation(KeyLocation::Unknown) { }

X509CertificateInfo::~X509CertificateInfo() = default;

int X509CertificateInfo::getVersion() const
{
    return cert_version;
}

const GooString &X509CertificateInfo::getSerialNumber() const
{
    return cert_serial;
}

const GooString &X509CertificateInfo::getNickName() const
{
    return cert_nick;
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

void X509CertificateInfo::setNickName(const GooString &nickName)
{
    cert_nick.Set(&nickName);
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
KeyLocation X509CertificateInfo::getKeyLocation() const
{
    return keyLocation;
}

void X509CertificateInfo::setKeyLocation(KeyLocation location)
{
    keyLocation = location;
}
