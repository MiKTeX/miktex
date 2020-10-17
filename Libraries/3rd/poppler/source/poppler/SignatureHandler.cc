//========================================================================
//
// SignatureHandler.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015, 2016 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2016, 2018, 2019 Albert Astals Cid <aacid@kde.org>
// Copyright 2015 Markus Kilås <digital@markuspage.com>
// Copyright 2017 Sebastian Rasmussen <sebras@gmail.com>
// Copyright 2017 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2018 Oliver Sander <oliver.sander@tu-dresden.de>
//
//========================================================================

#include <config.h>

#include "SignatureHandler.h"
#include "goo/gmem.h"
#include <secmod.h>
#include <keyhi.h>
#include <secder.h>

#include <dirent.h>
#include <Error.h>

static void shutdownNss()
{
    if (NSS_Shutdown() != SECSuccess) {
        fprintf(stderr, "NSS_Shutdown failed: %s\n", PR_ErrorToString(PORT_GetError(), PR_LANGUAGE_I_DEFAULT));
    }
}

unsigned int SignatureHandler::digestLength(SECOidTag digestAlgId)
{
    switch (digestAlgId) {
    case SEC_OID_SHA1:
        return 20;
    case SEC_OID_SHA256:
        return 32;
    case SEC_OID_SHA384:
        return 48;
    case SEC_OID_SHA512:
        return 64;
    default:
        printf("ERROR: Unrecognized Hash ID\n");
        return 0;
    }
}

char *SignatureHandler::getSignerName()
{
    if (!CMSSignerInfo || !NSS_IsInitialized())
        return nullptr;

    CERTCertificate *cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB());

    if (!cert)
        return nullptr;

    return CERT_GetCommonName(&cert->subject);
}

const char *SignatureHandler::getSignerSubjectDN()
{
    if (!CMSSignerInfo)
        return nullptr;

    CERTCertificate *cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB());
    if (!cert)
        return nullptr;
    return cert->subjectName;
}

HASH_HashType SignatureHandler::getHashAlgorithm()
{
    if (hash_context && hash_context->hashobj) {
        return hash_context->hashobj->type;
    }
    return HASH_AlgNULL;
}

time_t SignatureHandler::getSigningTime()
{
    PRTime sTime; // time in microseconds since the epoch

    if (NSS_CMSSignerInfo_GetSigningTime(CMSSignerInfo, &sTime) != SECSuccess)
        return 0;

    return static_cast<time_t>(sTime / 1000000);
}

X509CertificateInfo::EntityInfo SignatureHandler::getEntityInfo(CERTName *entityName) const
{
    X509CertificateInfo::EntityInfo info;

    if (!entityName)
        return info;

    char *dn = CERT_NameToAscii(entityName);
    if (dn) {
        info.distinguishedName = dn;
        PORT_Free(dn);
    }

    char *cn = CERT_GetCommonName(entityName);
    if (cn) {
        info.commonName = cn;
        PORT_Free(cn);
    }

    char *email = CERT_GetCertEmailAddress(entityName);
    if (email) {
        info.email = email;
        PORT_Free(email);
    }

    char *org = CERT_GetOrgName(entityName);
    if (org) {
        info.organization = org;
        PORT_Free(org);
    }

    return info;
}

static GooString SECItemToGooString(const SECItem &secItem)
{
    // TODO do we need to handle secItem.type;
    return GooString((const char *)secItem.data, secItem.len);
}

std::unique_ptr<X509CertificateInfo> SignatureHandler::getCertificateInfo() const
{
    if (!CMSSignerInfo)
        return nullptr;

    CERTCertificate *cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB());
    if (!cert)
        return nullptr;

    auto certInfo = std::make_unique<X509CertificateInfo>();

    certInfo->setVersion(DER_GetInteger(&cert->version) + 1);
    certInfo->setSerialNumber(SECItemToGooString(cert->serialNumber));

    // issuer info
    certInfo->setIssuerInfo(getEntityInfo(&cert->issuer));

    // validity
    PRTime notBefore, notAfter;
    CERT_GetCertTimes(cert, &notBefore, &notAfter);
    X509CertificateInfo::Validity certValidity;
    certValidity.notBefore = static_cast<time_t>(notBefore / 1000000);
    certValidity.notAfter = static_cast<time_t>(notAfter / 1000000);
    certInfo->setValidity(certValidity);

    // subject info
    certInfo->setSubjectInfo(getEntityInfo(&cert->subject));

    // public key info
    X509CertificateInfo::PublicKeyInfo pkInfo;
    SECKEYPublicKey *pk = CERT_ExtractPublicKey(cert);
    switch (pk->keyType) {
    case rsaKey:
        pkInfo.publicKey = SECItemToGooString(pk->u.rsa.modulus);
        pkInfo.publicKeyType = RSAKEY;
        break;
    case dsaKey:
        pkInfo.publicKey = SECItemToGooString(pk->u.dsa.publicValue);
        pkInfo.publicKeyType = DSAKEY;
        break;
    case ecKey:
        pkInfo.publicKey = SECItemToGooString(pk->u.ec.publicValue);
        pkInfo.publicKeyType = ECKEY;
        break;
    default:
        pkInfo.publicKey = SECItemToGooString(cert->subjectPublicKeyInfo.subjectPublicKey);
        pkInfo.publicKeyType = OTHERKEY;
        break;
    }
    pkInfo.publicKeyStrength = SECKEY_PublicKeyStrengthInBits(pk);
    certInfo->setPublicKeyInfo(std::move(pkInfo));

    certInfo->setKeyUsageExtensions(cert->keyUsage);
    certInfo->setCertificateDER(SECItemToGooString(cert->derCert));
    certInfo->setIsSelfSigned(CERT_CompareName(&cert->subject, &cert->issuer) == SECEqual);

    SECKEY_DestroyPublicKey(pk);

    return certInfo;
}

static GooString *getDefaultFirefoxCertDB_Linux()
{
    GooString *finalPath = nullptr;
    DIR *toSearchIn;
    struct dirent *subFolder;

    GooString *homePath = new GooString(getenv("HOME"));
    homePath = homePath->append("/.mozilla/firefox/");

    if ((toSearchIn = opendir(homePath->c_str())) == nullptr) {
        error(errInternal, 0, "couldn't find default Firefox Folder");
        delete homePath;
        return nullptr;
    }
    do {
        if ((subFolder = readdir(toSearchIn)) != nullptr) {
            if (strstr(subFolder->d_name, "default") != nullptr) {
                finalPath = homePath->append(subFolder->d_name);
                closedir(toSearchIn);
                return finalPath;
            }
        }
    } while (subFolder != nullptr);

    closedir(toSearchIn);
    delete homePath;
    return nullptr;
}

/**
 * Initialise NSS
 */
void SignatureHandler::setNSSDir(const GooString &nssDir)
{
    static bool setNssDirCalled = false;

    if (NSS_IsInitialized() && nssDir.getLength() > 0) {
        error(errInternal, 0, "You need to call setNSSDir before signature validation related operations happen");
        return;
    }

    if (setNssDirCalled)
        return;

    setNssDirCalled = true;

    atexit(shutdownNss);

    bool initSuccess = false;
    if (nssDir.getLength() > 0) {
        initSuccess = (NSS_Init(nssDir.c_str()) == SECSuccess);
    } else {
        GooString *certDBPath = getDefaultFirefoxCertDB_Linux();
        if (certDBPath == nullptr) {
            initSuccess = (NSS_Init("sql:/etc/pki/nssdb") == SECSuccess);
        } else {
            initSuccess = (NSS_Init(certDBPath->c_str()) == SECSuccess);
        }
        if (!initSuccess) {
            GooString homeNssDb(getenv("HOME"));
            homeNssDb.append("/.pki/nssdb");
            initSuccess = (NSS_Init(homeNssDb.c_str()) == SECSuccess);
            if (!initSuccess) {
                NSS_NoDB_Init(nullptr);
            }
        }
        delete certDBPath;
    }

    if (initSuccess) {
        // Make sure NSS root certificates module is loaded
        SECMOD_AddNewModule("Root Certs", "libnssckbi.so", 0, 0);
    }
}

SignatureHandler::SignatureHandler(unsigned char *p7, int p7_length) : hash_context(nullptr), CMSMessage(nullptr), CMSSignedData(nullptr), CMSSignerInfo(nullptr), temp_certs(nullptr)
{
    setNSSDir({});
    CMSitem.data = p7;
    CMSitem.len = p7_length;
    CMSMessage = CMS_MessageCreate(&CMSitem);
    CMSSignedData = CMS_SignedDataCreate(CMSMessage);
    if (CMSSignedData) {
        CMSSignerInfo = CMS_SignerInfoCreate(CMSSignedData);
        hash_context = initHashContext();
    }
}

HASHContext *SignatureHandler::initHashContext()
{

    SECItem usedAlgorithm = NSS_CMSSignedData_GetDigestAlgs(CMSSignedData)[0]->algorithm;
    hash_length = digestLength(SECOID_FindOIDTag(&usedAlgorithm));
    HASH_HashType hashType;
    hashType = HASH_GetHashTypeByOidTag(SECOID_FindOIDTag(&usedAlgorithm));
    return HASH_Create(hashType);
}

void SignatureHandler::updateHash(unsigned char *data_block, int data_len)
{
    if (hash_context) {
        HASH_Update(hash_context, data_block, data_len);
    }
}

SignatureHandler::~SignatureHandler()
{
    SECITEM_FreeItem(&CMSitem, PR_FALSE);
    if (CMSMessage)
        NSS_CMSMessage_Destroy(CMSMessage);

    if (hash_context)
        HASH_Destroy(hash_context);

    free(temp_certs);
}

NSSCMSMessage *SignatureHandler::CMS_MessageCreate(SECItem *cms_item)
{
    if (cms_item->data) {
        return NSS_CMSMessage_CreateFromDER(cms_item, nullptr, nullptr /* Content callback */
                                            ,
                                            nullptr, nullptr /*Password callback*/
                                            ,
                                            nullptr, nullptr /*Decrypt callback*/);
    } else {
        return nullptr;
    }
}

NSSCMSSignedData *SignatureHandler::CMS_SignedDataCreate(NSSCMSMessage *cms_msg)
{
    if (!NSS_CMSMessage_IsSigned(cms_msg)) {
        error(errInternal, 0, "Input couldn't be parsed as a CMS signature");
        return nullptr;
    }

    NSSCMSContentInfo *cinfo = NSS_CMSMessage_ContentLevel(cms_msg, 0);
    if (!cinfo) {
        error(errInternal, 0, "Error in NSS_CMSMessage_ContentLevel");
        return nullptr;
    }

    NSSCMSSignedData *signedData = (NSSCMSSignedData *)NSS_CMSContentInfo_GetContent(cinfo);
    if (!signedData) {
        error(errInternal, 0, "CError in NSS_CMSContentInfo_GetContent()");
        return nullptr;
    }

    if (signedData->rawCerts) {
        size_t i;
        for (i = 0; signedData->rawCerts[i]; ++i) { } // just count the length of the certificate chain

        // tempCerts field needs to be filled for complete memory release by NSSCMSSignedData_Destroy
        signedData->tempCerts = (CERTCertificate **)gmallocn(i + 1, sizeof(CERTCertificate *));
        memset(signedData->tempCerts, 0, (i + 1) * sizeof(CERTCertificate *));
        // store the addresses of these temporary certificates for future release
        for (i = 0; signedData->rawCerts[i]; ++i)
            signedData->tempCerts[i] = CERT_NewTempCertificate(CERT_GetDefaultCertDB(), signedData->rawCerts[i], nullptr, 0, 0);

        temp_certs = signedData->tempCerts;
        return signedData;
    } else {
        return nullptr;
    }
}

NSSCMSSignerInfo *SignatureHandler::CMS_SignerInfoCreate(NSSCMSSignedData *cms_sig_data)
{
    NSSCMSSignerInfo *signerInfo = NSS_CMSSignedData_GetSignerInfo(cms_sig_data, 0);
    if (!signerInfo) {
        printf("Error in NSS_CMSSignedData_GetSignerInfo()\n");
        return nullptr;
    } else {
        return signerInfo;
    }
}

static SignatureValidationStatus NSS_SigTranslate(NSSCMSVerificationStatus nss_code)
{
    switch (nss_code) {
    case NSSCMSVS_GoodSignature:
        return SIGNATURE_VALID;

    case NSSCMSVS_BadSignature:
        return SIGNATURE_INVALID;

    case NSSCMSVS_DigestMismatch:
        return SIGNATURE_DIGEST_MISMATCH;

    case NSSCMSVS_ProcessingError:
        return SIGNATURE_DECODING_ERROR;

    default:
        return SIGNATURE_GENERIC_ERROR;
    }
}

SignatureValidationStatus SignatureHandler::validateSignature()
{
    unsigned char *digest_buffer = nullptr;

    if (!CMSSignedData)
        return SIGNATURE_GENERIC_ERROR;

    if (!NSS_IsInitialized())
        return SIGNATURE_GENERIC_ERROR;

    if (!hash_context)
        return SIGNATURE_GENERIC_ERROR;

    digest_buffer = (unsigned char *)PORT_Alloc(hash_length);
    unsigned int result_len = 0;

    HASH_End(hash_context, digest_buffer, &result_len, hash_length);

    SECItem digest;
    digest.data = digest_buffer;
    digest.len = hash_length;

    if ((NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB())) == nullptr)
        CMSSignerInfo->verificationStatus = NSSCMSVS_SigningCertNotFound;

    SECItem *content_info_data = CMSSignedData->contentInfo.content.data;
    if (content_info_data != nullptr && content_info_data->data != nullptr) {
        /*
          This means it's not a detached type signature
          so the digest is contained in SignedData->contentInfo
        */
        if (memcmp(digest.data, content_info_data->data, hash_length) == 0 && digest.len == content_info_data->len) {
            PORT_Free(digest_buffer);
            return SIGNATURE_VALID;
        } else {
            PORT_Free(digest_buffer);
            return SIGNATURE_DIGEST_MISMATCH;
        }

    } else if (NSS_CMSSignerInfo_Verify(CMSSignerInfo, &digest, nullptr) != SECSuccess) {

        PORT_Free(digest_buffer);
        return NSS_SigTranslate(CMSSignerInfo->verificationStatus);
    } else {
        PORT_Free(digest_buffer);
        return SIGNATURE_VALID;
    }
}

CertificateValidationStatus SignatureHandler::validateCertificate(time_t validation_time)
{
    CERTCertificate *cert;

    if (!CMSSignerInfo)
        return CERTIFICATE_GENERIC_ERROR;

    if ((cert = NSS_CMSSignerInfo_GetSigningCertificate(CMSSignerInfo, CERT_GetDefaultCertDB())) == nullptr)
        CMSSignerInfo->verificationStatus = NSSCMSVS_SigningCertNotFound;

    PRTime vTime = 0; // time in microseconds since the epoch, special value 0 means now
    if (validation_time > 0)
        vTime = 1000000 * (PRTime)validation_time;
    CERTValInParam inParams[3];
    inParams[0].type = cert_pi_revocationFlags;
    inParams[0].value.pointer.revocation = CERT_GetClassicOCSPEnabledSoftFailurePolicy();
    inParams[1].type = cert_pi_date;
    inParams[1].value.scalar.time = vTime;
    inParams[2].type = cert_pi_end;

    CERT_PKIXVerifyCert(cert, certificateUsageEmailSigner, inParams, nullptr, CMSSignerInfo->cmsg->pwfn_arg);

    switch (PORT_GetError()) {
    // 0 not defined in SECErrorCodes, it means success for this purpose.
    case 0:
        return CERTIFICATE_TRUSTED;

    case SEC_ERROR_UNKNOWN_ISSUER:
        return CERTIFICATE_UNKNOWN_ISSUER;

    case SEC_ERROR_UNTRUSTED_ISSUER:
        return CERTIFICATE_UNTRUSTED_ISSUER;

    case SEC_ERROR_REVOKED_CERTIFICATE:
        return CERTIFICATE_REVOKED;

    case SEC_ERROR_EXPIRED_CERTIFICATE:
        return CERTIFICATE_EXPIRED;
    }

    return CERTIFICATE_GENERIC_ERROR;
}
