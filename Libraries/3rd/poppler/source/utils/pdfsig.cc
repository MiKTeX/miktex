//========================================================================
//
// pdfsig.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2017-2020 Albert Astals Cid <aacid@kde.org>
// Copyright 2016 Markus Kilås <digital@markuspage.com>
// Copyright 2017, 2019 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2017, 2019 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2019 Alexey Pavlov <alexpux@gmail.com>
// Copyright 2019 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2019 Nelson Efrain A. Cruz <neac03@gmail.com>
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <hasht.h>
#include <fstream>
#include "parseargs.h"
#include "Object.h"
#include "Array.h"
#include "goo/gbasename.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Error.h"
#include "GlobalParams.h"
#include "SignatureHandler.h"
#include "SignatureInfo.h"
#include "Win32Console.h"
#include "numberofcharacters.h"
#include <libgen.h>

static const char *getReadableSigState(SignatureValidationStatus sig_vs)
{
    switch (sig_vs) {
    case SIGNATURE_VALID:
        return "Signature is Valid.";

    case SIGNATURE_INVALID:
        return "Signature is Invalid.";

    case SIGNATURE_DIGEST_MISMATCH:
        return "Digest Mismatch.";

    case SIGNATURE_DECODING_ERROR:
        return "Document isn't signed or corrupted data.";

    case SIGNATURE_NOT_VERIFIED:
        return "Signature has not yet been verified.";

    default:
        return "Unknown Validation Failure.";
    }
}

static const char *getReadableCertState(CertificateValidationStatus cert_vs)
{
    switch (cert_vs) {
    case CERTIFICATE_TRUSTED:
        return "Certificate is Trusted.";

    case CERTIFICATE_UNTRUSTED_ISSUER:
        return "Certificate issuer isn't Trusted.";

    case CERTIFICATE_UNKNOWN_ISSUER:
        return "Certificate issuer is unknown.";

    case CERTIFICATE_REVOKED:
        return "Certificate has been Revoked.";

    case CERTIFICATE_EXPIRED:
        return "Certificate has Expired";

    case CERTIFICATE_NOT_VERIFIED:
        return "Certificate has not yet been verified.";

    default:
        return "Unknown issue with Certificate or corrupted data.";
    }
}

static char *getReadableTime(time_t unix_time)
{
    char *time_str = (char *)gmalloc(64);
    strftime(time_str, 64, "%b %d %Y %H:%M:%S", localtime(&unix_time));
    return time_str;
}

static bool dumpSignature(int sig_num, int sigCount, FormFieldSignature *s, const char *filename)
{
    const GooString *signature = s->getSignature();
    if (!signature) {
        printf("Cannot dump signature #%d\n", sig_num);
        return false;
    }

    const int sigCountLength = numberOfCharacters(sigCount);
    // We want format to be {0:s}.sig{1:Xd} where X is sigCountLength
    // since { is the magic character to replace things we need to put it twice where
    // we don't want it to be replaced
    GooString *format = GooString::format("{{0:s}}.sig{{1:{0:d}d}}", sigCountLength);
    GooString *path = GooString::format(format->c_str(), gbasename(filename).c_str(), sig_num);
    printf("Signature #%d (%u bytes) => %s\n", sig_num, signature->getLength(), path->c_str());
    std::ofstream outfile(path->c_str(), std::ofstream::binary);
    outfile.write(signature->c_str(), signature->getLength());
    outfile.close();
    delete format;
    delete path;

    return true;
}

static GooString nssDir;
static bool printVersion = false;
static bool printHelp = false;
static bool dontVerifyCert = false;
static bool dumpSignatures = false;

static const ArgDesc argDesc[] = { { "-nssdir", argGooString, &nssDir, 0, "path to directory of libnss3 database" },
                                   { "-nocert", argFlag, &dontVerifyCert, 0, "don't perform certificate validation" },
                                   { "-dump", argFlag, &dumpSignatures, 0, "dump all signatures into current directory" },

                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

int main(int argc, char *argv[])
{
    char *time_str = nullptr;
    globalParams = std::make_unique<GlobalParams>();

    Win32Console win32Console(&argc, &argv);

    const bool ok = parseArgs(argDesc, &argc, argv);

    if (!ok || argc != 2 || printVersion || printHelp) {
        fprintf(stderr, "pdfsig version %s\n", PACKAGE_VERSION);
        fprintf(stderr, "%s\n", popplerCopyright);
        fprintf(stderr, "%s\n", xpdfCopyright);
        if (!printVersion) {
            printUsage("pdfsig", "<PDF-file>", argDesc);
        }
        if (printVersion || printHelp)
            return 0;
        return 99;
    }

    std::unique_ptr<GooString> fileName = std::make_unique<GooString>(argv[argc - 1]);

    SignatureHandler::setNSSDir(nssDir);

    // open PDF file
    std::unique_ptr<PDFDoc> doc(PDFDocFactory().createPDFDoc(*fileName, nullptr, nullptr));

    if (!doc->isOk()) {
        return 1;
    }

    const std::vector<FormFieldSignature *> signatures = doc->getSignatureFields();
    const unsigned int sigCount = signatures.size();

    if (sigCount >= 1) {
        if (dumpSignatures) {
            printf("Dumping Signatures: %u\n", sigCount);
            for (unsigned int i = 0; i < sigCount; i++) {
                const bool dumpingOk = dumpSignature(i, sigCount, signatures.at(i), fileName->c_str());
                if (!dumpingOk) {
                    return 3;
                }
            }
            return 0;
        } else {
            printf("Digital Signature Info of: %s\n", fileName->c_str());
        }
    } else {
        printf("File '%s' does not contain any signatures\n", fileName->c_str());
        return 2;
    }

    for (unsigned int i = 0; i < sigCount; i++) {
        const SignatureInfo *sig_info = signatures.at(i)->validateSignature(!dontVerifyCert, false, -1 /* now */);
        printf("Signature #%u:\n", i + 1);
        printf("  - Signer Certificate Common Name: %s\n", sig_info->getSignerName());
        printf("  - Signer full Distinguished Name: %s\n", sig_info->getSubjectDN());
        printf("  - Signing Time: %s\n", time_str = getReadableTime(sig_info->getSigningTime()));
        printf("  - Signing Hash Algorithm: ");
        switch (sig_info->getHashAlgorithm()) {
        case HASH_AlgMD2:
            printf("MD2\n");
            break;
        case HASH_AlgMD5:
            printf("MD5\n");
            break;
        case HASH_AlgSHA1:
            printf("SHA1\n");
            break;
        case HASH_AlgSHA256:
            printf("SHA-256\n");
            break;
        case HASH_AlgSHA384:
            printf("SHA-384\n");
            break;
        case HASH_AlgSHA512:
            printf("SHA-512\n");
            break;
        case HASH_AlgSHA224:
            printf("SHA-224\n");
            break;
        default:
            printf("unknown\n");
        }
        printf("  - Signature Type: ");
        switch (signatures.at(i)->getSignatureType()) {
        case adbe_pkcs7_sha1:
            printf("adbe.pkcs7.sha1\n");
            break;
        case adbe_pkcs7_detached:
            printf("adbe.pkcs7.detached\n");
            break;
        case ETSI_CAdES_detached:
            printf("ETSI.CAdES.detached\n");
            break;
        default:
            printf("unknown\n");
        }
        std::vector<Goffset> ranges = signatures.at(i)->getSignedRangeBounds();
        if (ranges.size() == 4) {
            printf("  - Signed Ranges: [%lld - %lld], [%lld - %lld]\n", ranges[0], ranges[1], ranges[2], ranges[3]);
            Goffset checked_file_size;
            GooString *signature = signatures.at(i)->getCheckedSignature(&checked_file_size);
            if (signature && checked_file_size == ranges[3]) {
                printf("  - Total document signed\n");
            } else {
                printf("  - Not total document signed\n");
            }
            delete signature;
        }
        printf("  - Signature Validation: %s\n", getReadableSigState(sig_info->getSignatureValStatus()));
        gfree(time_str);
        if (sig_info->getSignatureValStatus() != SIGNATURE_VALID || dontVerifyCert) {
            continue;
        }
        printf("  - Certificate Validation: %s\n", getReadableCertState(sig_info->getCertificateValStatus()));
    }

    return 0;
}
