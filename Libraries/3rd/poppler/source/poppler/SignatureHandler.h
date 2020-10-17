//========================================================================
//
// SignatureHandler.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2017, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#ifndef SIGNATURE_HANDLER_H
#define SIGNATURE_HANDLER_H

#include "goo/GooString.h"
#include "SignatureInfo.h"
#include "CertificateInfo.h"

/* NSPR Headers */
#include <nspr.h>

/* NSS headers */
#include <cms.h>
#include <nss.h>
#include <cert.h>
#include <cryptohi.h>
#include <secerr.h>
#include <secoid.h>
#include <secmodt.h>
#include <sechash.h>

class SignatureHandler
{
public:
    SignatureHandler(unsigned char *p7, int p7_length);
    ~SignatureHandler();
    time_t getSigningTime();
    char *getSignerName();
    const char *getSignerSubjectDN();
    HASH_HashType getHashAlgorithm();
    void setSignature(unsigned char *, int);
    void updateHash(unsigned char *data_block, int data_len);
    SignatureValidationStatus validateSignature();
    // Use -1 as validation_time for now
    CertificateValidationStatus validateCertificate(time_t validation_time);
    std::unique_ptr<X509CertificateInfo> getCertificateInfo() const;

    // Initializes the NSS dir with the custom given directory
    // calling it with an empty string means use the default firefox db, /etc/pki/nssdb, ~/.pki/nssdb
    // If you don't want a custom NSS dir and the default entries are fine for you, not calling this function is fine
    // If wanted, this has to be called before doing signature validation calls
    static void setNSSDir(const GooString &nssDir);

private:
    SignatureHandler(const SignatureHandler &);
    SignatureHandler &operator=(const SignatureHandler &);

    unsigned int digestLength(SECOidTag digestAlgId);
    NSSCMSMessage *CMS_MessageCreate(SECItem *cms_item);
    NSSCMSSignedData *CMS_SignedDataCreate(NSSCMSMessage *cms_msg);
    NSSCMSSignerInfo *CMS_SignerInfoCreate(NSSCMSSignedData *cms_sig_data);
    HASHContext *initHashContext();
    X509CertificateInfo::EntityInfo getEntityInfo(CERTName *entityName) const;

    unsigned int hash_length;
    SECItem CMSitem;
    HASHContext *hash_context;
    NSSCMSMessage *CMSMessage;
    NSSCMSSignedData *CMSSignedData;
    NSSCMSSignerInfo *CMSSignerInfo;
    CERTCertificate **temp_certs;
};

#endif
