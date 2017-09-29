//========================================================================
//
// SignatureInfo.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2017 Albert Astals Cid <aacid@kde.org>
//
//========================================================================

#include <config.h>

#include "SignatureInfo.h"
#include "goo/gmem.h"
#include <stdlib.h>
#include <string.h>

#ifdef ENABLE_NSS3
    #include <hasht.h>
#else
    static const int HASH_AlgNULL = -1;
#endif

/* Constructor & Destructor */

SignatureInfo::SignatureInfo()
{
  sig_status = SIGNATURE_NOT_VERIFIED;
  cert_status = CERTIFICATE_NOT_VERIFIED;
  signer_name = nullptr;
  subject_dn = nullptr;
  hash_type = HASH_AlgNULL;
  signing_time = 0;
  sig_subfilter_supported = false;
}

SignatureInfo::SignatureInfo(SignatureValidationStatus sig_val_status, CertificateValidationStatus cert_val_status)
{
  sig_status = sig_val_status;
  cert_status = cert_val_status;
  signer_name = nullptr;
  subject_dn = nullptr;
  hash_type = HASH_AlgNULL;
  signing_time = 0;
  sig_subfilter_supported = false;
}

SignatureInfo::~SignatureInfo()
{
  free(signer_name);
}

/* GETTERS */

SignatureValidationStatus SignatureInfo::getSignatureValStatus()
{
  return sig_status;
}

CertificateValidationStatus SignatureInfo::getCertificateValStatus()
{
  return cert_status;
}

const char *SignatureInfo::getSignerName()
{
  return signer_name;
}

const char *SignatureInfo::getSubjectDN()
{
  return subject_dn;
}

int SignatureInfo::getHashAlgorithm()
{
  return hash_type;
}

time_t SignatureInfo::getSigningTime()
{
  return signing_time;
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

void SignatureInfo::setSignerName(char *signerName)
{
  free(signer_name);
  signer_name = signerName;
}

void SignatureInfo::setSubjectDN(const char *subjectDN)
{
  subject_dn = subjectDN;
}

void SignatureInfo::setHashAlgorithm(int type)
{
  hash_type = type;
}

void SignatureInfo::setSigningTime(time_t signingTime)
{
  signing_time = signingTime;
}
