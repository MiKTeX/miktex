//========================================================================
//
// SignatureInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2017-2020 Albert Astals Cid <aacid@kde.org>
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
// Copyright 2021 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2021 Marek Kasik <mkasik@redhat.com>
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
//========================================================================

#include <config.h>

#include "SignatureInfo.h"
#include "CertificateInfo.h"
#include "goo/gmem.h"
#include <cstdlib>
#include <cstring>

/* Constructor & Destructor */

SignatureInfo::~SignatureInfo() = default;

/* GETTERS */

SignatureValidationStatus SignatureInfo::getSignatureValStatus() const
{
    return sig_status;
}

CertificateValidationStatus SignatureInfo::getCertificateValStatus() const
{
    return cert_status;
}

std::string SignatureInfo::getSignerName() const
{
    return signer_name;
}

std::string SignatureInfo::getSubjectDN() const
{
    return subject_dn;
}

const GooString &SignatureInfo::getLocation() const
{
    return location;
}

const GooString &SignatureInfo::getReason() const
{
    return reason;
}

HashAlgorithm SignatureInfo::getHashAlgorithm() const
{
    return hash_type;
}

time_t SignatureInfo::getSigningTime() const
{
    return signing_time;
}

const X509CertificateInfo *SignatureInfo::getCertificateInfo() const
{
    return cert_info.get();
}

/* SETTERS */

void SignatureInfo::setSignatureValStatus(enum SignatureValidationStatus sig_val_status)
{
    sig_status = sig_val_status;
}

void SignatureInfo::setCertificateValStatus(enum CertificateValidationStatus cert_val_status)
{
    cert_status = cert_val_status;
}

void SignatureInfo::setSignerName(const std::string &signerName)
{
    signer_name = signerName;
}

void SignatureInfo::setSubjectDN(const std::string &subjectDN)
{
    subject_dn = subjectDN;
}

void SignatureInfo::setLocation(const GooString *loc)
{
    location = GooString(loc);
}

void SignatureInfo::setReason(const GooString *signingReason)
{
    reason = GooString(signingReason);
}

void SignatureInfo::setHashAlgorithm(HashAlgorithm type)
{
    hash_type = type;
}

void SignatureInfo::setSigningTime(time_t signingTime)
{
    signing_time = signingTime;
}

void SignatureInfo::setCertificateInfo(std::unique_ptr<X509CertificateInfo> certInfo)
{
    cert_info = std::move(certInfo);
}
