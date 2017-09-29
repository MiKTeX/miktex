//========================================================================
//
// SignatureHandler.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2017 Albert Astals Cid <aacid@kde.org>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
//
//========================================================================

#ifndef SIGNATURE_HANDLER_H
#define SIGNATURE_HANDLER_H

#include "goo/GooString.h"
#include "SignatureInfo.h"

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
  char * getSignerName();
  const char * getSignerSubjectDN();
  HASH_HashType getHashAlgorithm();
  void setSignature(unsigned char *, int);
  void updateHash(unsigned char * data_block, int data_len);
  NSSCMSVerificationStatus validateSignature();
  // Use -1 as validation_time for now
  SECErrorCodes validateCertificate(time_t validation_time);

  //Translate NSS error codes
  static SignatureValidationStatus NSS_SigTranslate(NSSCMSVerificationStatus nss_code);
  static CertificateValidationStatus NSS_CertTranslate(SECErrorCodes nss_code);

private:
  SignatureHandler(const SignatureHandler &);
  SignatureHandler& operator=(const SignatureHandler &);

  void init_nss();

  GooString * getDefaultFirefoxCertDB_Linux();
  unsigned int digestLength(SECOidTag digestAlgId);
  NSSCMSMessage *CMS_MessageCreate(SECItem * cms_item);
  NSSCMSSignedData *CMS_SignedDataCreate(NSSCMSMessage * cms_msg);
  NSSCMSSignerInfo *CMS_SignerInfoCreate(NSSCMSSignedData * cms_sig_data);
  HASHContext * initHashContext();

  unsigned int hash_length;
  SECItem CMSitem;
  HASHContext *hash_context;
  NSSCMSMessage *CMSMessage;
  NSSCMSSignedData *CMSSignedData;
  NSSCMSSignerInfo *CMSSignerInfo;
  CERTCertificate **temp_certs;
};

#endif
