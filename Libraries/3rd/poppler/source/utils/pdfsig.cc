//========================================================================
//
// pdfsig.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015, 2017-2023 Albert Astals Cid <aacid@kde.org>
// Copyright 2016 Markus Kilås <digital@markuspage.com>
// Copyright 2017, 2019 Hans-Ulrich Jüttner <huj@froreich-bioscientia.de>
// Copyright 2017, 2019 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2018 Chinmoy Ranjan Pradhan <chinmoyrp65@protonmail.com>
// Copyright 2019 Alexey Pavlov <alexpux@gmail.com>
// Copyright 2019. 2023 Oliver Sander <oliver.sander@tu-dresden.de>
// Copyright 2019 Nelson Efrain A. Cruz <neac03@gmail.com>
// Copyright 2021 Georgiy Sgibnev <georgiy@sgibnev.com>. Work sponsored by lab50.net.
// Copyright 2021 Theofilos Intzoglou <int.teo@gmail.com>
// Copyright 2022 Felix Jung <fxjung@posteo.de>
// Copyright 2022 Erich E. Hoover <erich.e.hoover@gmail.com>
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <fstream>
#include <random>
#include "parseargs.h"
#include "Object.h"
#include "Array.h"
#include "goo/gbasename.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Error.h"
#include "GlobalParams.h"
#ifdef ENABLE_NSS3
#    include "NSSCryptoSignBackend.h"
#endif
#include "CryptoSignBackend.h"
#include "SignatureInfo.h"
#include "Win32Console.h"
#include "numberofcharacters.h"
#include "UTF.h"
#if __has_include(<libgen.h>)
#    include <libgen.h>
#endif

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
    const std::unique_ptr<GooString> format = GooString::format("{{0:s}}.sig{{1:{0:d}d}}", sigCountLength);
    const std::unique_ptr<GooString> path = GooString::format(format->c_str(), gbasename(filename).c_str(), sig_num);
    printf("Signature #%d (%u bytes) => %s\n", sig_num, signature->getLength(), path->c_str());
    std::ofstream outfile(path->c_str(), std::ofstream::binary);
    outfile.write(signature->c_str(), signature->getLength());
    outfile.close();

    return true;
}

static GooString nssDir;
static GooString nssPassword;
static char ownerPassword[33] = "\001";
static char userPassword[33] = "\001";
static bool printVersion = false;
static bool printHelp = false;
static bool printCryptoSignBackends = false;
static bool dontVerifyCert = false;
static bool noOCSPRevocationCheck = false;
static bool dumpSignatures = false;
static bool etsiCAdESdetached = false;
static char backendString[256] = "";
static char signatureName[256] = "";
static char certNickname[256] = "";
static char password[256] = "";
static char digestName[256] = "SHA256";
static GooString reason;
static bool listNicknames = false;
static bool addNewSignature = false;
static bool useAIACertFetch = false;
static GooString newSignatureFieldName;

static const ArgDesc argDesc[] = { { "-nssdir", argGooString, &nssDir, 0, "path to directory of libnss3 database" },
                                   { "-nss-pwd", argGooString, &nssPassword, 0, "password to access the NSS database (if any)" },
                                   { "-nocert", argFlag, &dontVerifyCert, 0, "don't perform certificate validation" },
                                   { "-no-ocsp", argFlag, &noOCSPRevocationCheck, 0, "don't perform online OCSP certificate revocation check" },
                                   { "-aia", argFlag, &useAIACertFetch, 0, "use Authority Information Access (AIA) extension for certificate fetching" },
                                   { "-dump", argFlag, &dumpSignatures, 0, "dump all signatures into current directory" },
                                   { "-add-signature", argFlag, &addNewSignature, 0, "adds a new signature to the document" },
                                   { "-new-signature-field-name", argGooString, &newSignatureFieldName, 0, "field name used for the newly added signature. A random ID will be used if empty" },
                                   { "-sign", argString, &signatureName, 256, "sign the document in the given signature field (by name or number)" },
                                   { "-etsi", argFlag, &etsiCAdESdetached, 0, "create a signature of type ETSI.CAdES.detached instead of adbe.pkcs7.detached" },
                                   { "-backend", argString, &backendString, 256, "use given backend for signing/verification" },
                                   { "-nick", argString, &certNickname, 256, "use the certificate with the given nickname/fingerprint for signing" },
                                   { "-kpw", argString, &password, 256, "password for the signing key (might be missing if the key isn't password protected)" },
                                   { "-digest", argString, &digestName, 256, "name of the digest algorithm (default: SHA256)" },
                                   { "-reason", argGooString, &reason, 0, "reason for signing (default: no reason given)" },
                                   { "-list-nicks", argFlag, &listNicknames, 0, "list available nicknames in the NSS database" },
                                   { "-list-backends", argFlag, &printCryptoSignBackends, 0, "print cryptographic signature backends" },
                                   { "-opw", argString, ownerPassword, sizeof(ownerPassword), "owner password (for encrypted files)" },
                                   { "-upw", argString, userPassword, sizeof(userPassword), "user password (for encrypted files)" },
                                   { "-v", argFlag, &printVersion, 0, "print copyright and version info" },
                                   { "-h", argFlag, &printHelp, 0, "print usage information" },
                                   { "-help", argFlag, &printHelp, 0, "print usage information" },
                                   { "--help", argFlag, &printHelp, 0, "print usage information" },
                                   { "-?", argFlag, &printHelp, 0, "print usage information" },
                                   {} };

static void print_version_usage(bool usage)
{
    fprintf(stderr, "pdfsig version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (usage) {
        printUsage("pdfsig", "<PDF-file> [<output-file>]", argDesc);
    }
}

static void print_backends()
{
    fprintf(stderr, "pdfsig backends:\n");
    for (const auto &backend : CryptoSign::Factory::getAvailable()) {
        switch (backend) {
        case CryptoSign::Backend::Type::NSS3:
            fprintf(stderr, "NSS");
            break;
        case CryptoSign::Backend::Type::GPGME:
            fprintf(stderr, "GPG");
            break;
        }
        if (backend == CryptoSign::Factory::getActive()) {
            fprintf(stderr, " (active)\n");
        } else {
            fprintf(stderr, "\n");
        }
    }
}

static std::vector<std::unique_ptr<X509CertificateInfo>> getAvailableSigningCertificates(bool *error)
{
#ifdef ENABLE_NSS3
    bool wrongPassword = false;
    bool passwordNeeded = false;
    auto passwordCallback = [&passwordNeeded, &wrongPassword](const char *) -> char * {
        static bool firstTime = true;
        if (!firstTime) {
            wrongPassword = true;
            return nullptr;
        }
        firstTime = false;
        if (nssPassword.getLength() > 0) {
            return strdup(nssPassword.c_str());
        } else {
            passwordNeeded = true;
            return nullptr;
        }
    };
    NSSSignatureConfiguration::setNSSPasswordCallback(passwordCallback);
#endif
    auto backend = CryptoSign::Factory::createActive();
    if (!backend) {
        *error = true;
        printf("No backends for cryptographic signatures available");
        return {};
    }
    std::vector<std::unique_ptr<X509CertificateInfo>> vCerts = backend->getAvailableSigningCertificates();
#ifdef ENABLE_NSS3
    NSSSignatureConfiguration::setNSSPasswordCallback({});
    if (passwordNeeded) {
        *error = true;
        printf("Password is needed to access the NSS database.\n");
        printf("\tPlease provide one with -nss-pwd.\n");
        return {};
    }
    if (wrongPassword) {
        *error = true;
        printf("Password was not accepted to open the NSS database.\n");
        printf("\tPlease provide the correct one with -nss-pwd.\n");
        return {};
    }

#endif
    *error = false;
    return vCerts;
}

static std::string locationToString(KeyLocation location)
{
    switch (location) {
    case KeyLocation::Unknown:
        return {};
    case KeyLocation::Other:
        return "(Other)";
    case KeyLocation::Computer:
        return "(Computer)";
    case KeyLocation::HardwareToken:
        return "(Hardware Token)";
    }
    return {};
}

static std::string TextStringToUTF8(const std::string &str)
{
    const UnicodeMap *utf8Map = globalParams->getUtf8Map();

    Unicode *u;
    const int len = TextStringToUCS4(str, &u);

    std::string convertedStr;
    for (int i = 0; i < len; ++i) {
        char buf[8];
        const int n = utf8Map->mapUnicode(u[i], buf, sizeof(buf));
        convertedStr.append(buf, n);
    }
    gfree(u);

    return convertedStr;
}

int main(int argc, char *argv[])
{
    char *time_str = nullptr;
    globalParams = std::make_unique<GlobalParams>();

    Win32Console win32Console(&argc, &argv);

    const bool ok = parseArgs(argDesc, &argc, argv);

    if (!ok) {
        print_version_usage(true);
        return 99;
    }

    if (printVersion) {
        print_version_usage(false);
        return 0;
    }

    if (printHelp) {
        print_version_usage(true);
        return 0;
    }

    if (strlen(backendString) > 0) {
        auto backend = CryptoSign::Factory::typeFromString(backendString);
        if (backend) {
            CryptoSign::Factory::setPreferredBackend(backend.value());
        } else {
            fprintf(stderr, "Unsupported backend\n");
            return 98;
        }
    }

    if (printCryptoSignBackends) {
        print_backends();
        return 0;
    }

#ifdef ENABLE_NSS3
    NSSSignatureConfiguration::setNSSDir(nssDir);
#endif

    if (listNicknames) {
        bool getCertsError;
        const std::vector<std::unique_ptr<X509CertificateInfo>> vCerts = getAvailableSigningCertificates(&getCertsError);
        if (getCertsError) {
            return 2;
        } else {
            if (vCerts.empty()) {
                printf("There are no certificates available.\n");
            } else {
                printf("Certificate nicknames available:\n");
                for (auto &cert : vCerts) {
                    const GooString &nick = cert->getNickName();
                    const auto location = locationToString(cert->getKeyLocation());
                    printf("%s %s\n", nick.c_str(), location.c_str());
                }
            }
        }
        return 0;
    }

    if (argc < 2) {
        // no filename was given
        print_version_usage(true);
        return 99;
    }

    std::unique_ptr<GooString> fileName = std::make_unique<GooString>(argv[1]);

    std::optional<GooString> ownerPW, userPW;
    if (ownerPassword[0] != '\001') {
        ownerPW = GooString(ownerPassword);
    }
    if (userPassword[0] != '\001') {
        userPW = GooString(userPassword);
    }
    // open PDF file
    std::unique_ptr<PDFDoc> doc(PDFDocFactory().createPDFDoc(*fileName, ownerPW, userPW));

    if (!doc->isOk()) {
        return 1;
    }

    int signatureNumber;
    if (strlen(signatureName) > 0) {
        signatureNumber = atoi(signatureName);
        if (signatureNumber == 0) {
            signatureNumber = -1;
        }
    } else {
        signatureNumber = 0;
    }

    if (addNewSignature && signatureNumber > 0) {
        // incompatible options
        print_version_usage(true);
        return 99;
    }

    if (addNewSignature) {
        if (argc == 2) {
            fprintf(stderr, "An output filename for the signed document must be given\n");
            return 2;
        }

        if (strlen(certNickname) == 0) {
            printf("A nickname of the signing certificate must be given\n");
            return 2;
        }

        if (etsiCAdESdetached) {
            printf("-etsi is not supported yet with -add-signature\n");
            printf("Please file a bug report if this is important for you\n");
            return 2;
        }

        if (digestName != std::string("SHA256")) {
            printf("Only digest SHA256 is supported at the moment with -add-signature\n");
            printf("Please file a bug report if this is important for you\n");
            return 2;
        }

        if (doc->getPage(1) == nullptr) {
            printf("Error getting first page of the document.\n");
            return 2;
        }

        bool getCertsError;
        // We need to call this otherwise NSS spins forever
        getAvailableSigningCertificates(&getCertsError);
        if (getCertsError) {
            return 2;
        }

        const auto rs = std::unique_ptr<GooString>(reason.toStr().empty() ? nullptr : std::make_unique<GooString>(utf8ToUtf16WithBom(reason.toStr())));

        if (newSignatureFieldName.getLength() == 0) {
            // Create a random field name, it could be anything but 32 hex numbers should
            // hopefully give us something that is not already in the document
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(1, 15);
            for (int i = 0; i < 32; ++i) {
                const int value = distrib(gen);
                newSignatureFieldName.append(value < 10 ? 48 + value : 65 + (value - 10));
            }
        }

        // We don't provide a way to customize the UI from pdfsig for now
        const bool success = doc->sign(std::string { argv[2] }, std::string { certNickname }, std::string { password }, newSignatureFieldName.copy(), /*page*/ 1,
                                       /*rect */ { 0, 0, 0, 0 }, /*signatureText*/ {}, /*signatureTextLeft*/ {}, /*fontSize */ 0, /*leftFontSize*/ 0,
                                       /*fontColor*/ {}, /*borderWidth*/ 0, /*borderColor*/ {}, /*backgroundColor*/ {}, rs.get(), /* location */ nullptr, /* image path */ "", ownerPW, userPW);
        return success ? 0 : 3;
    }

    const std::vector<FormFieldSignature *> signatures = doc->getSignatureFields();
    const unsigned int sigCount = signatures.size();

    if (signatureNumber == -1) {
        for (unsigned int i = 0; i < sigCount; i++) {
            const GooString *goo = signatures.at(i)->getCreateWidget()->getField()->getFullyQualifiedName();
            if (!goo) {
                continue;
            }

            const std::string name = TextStringToUTF8(goo->toStr());
            if (name == signatureName) {
                signatureNumber = i + 1;
                break;
            }
        }

        if (signatureNumber == -1) {
            fprintf(stderr, "Signature field not found by name\n");
            return 2;
        }
    }

    if (signatureNumber > 0) {
        // We are signing an existing signature field
        if (argc == 2) {
            fprintf(stderr, "An output filename for the signed document must be given\n");
            return 2;
        }

        if (signatureNumber > static_cast<int>(sigCount)) {
            printf("File '%s' does not contain a signature with number %d\n", fileName->c_str(), signatureNumber);
            return 2;
        }

        if (strlen(certNickname) == 0) {
            printf("A nickname of the signing certificate must be given\n");
            return 2;
        }

        if (digestName != std::string("SHA256")) {
            printf("Only digest SHA256 is supported at the moment\n");
            printf("Please file a bug report if this is important for you\n");
            return 2;
        }

        bool getCertsError;
        // We need to call this otherwise NSS spins forever
        getAvailableSigningCertificates(&getCertsError);
        if (getCertsError) {
            return 2;
        }

        FormFieldSignature *ffs = signatures.at(signatureNumber - 1);
        Goffset file_size = 0;
        const std::optional<GooString> sig = ffs->getCheckedSignature(&file_size);
        if (sig) {
            printf("Signature number %d is already signed\n", signatureNumber);
            return 2;
        }
        if (etsiCAdESdetached) {
            ffs->setSignatureType(ETSI_CAdES_detached);
        }
        const auto rs = std::unique_ptr<GooString>(reason.toStr().empty() ? nullptr : std::make_unique<GooString>(utf8ToUtf16WithBom(reason.toStr())));
        if (ffs->getNumWidgets() != 1) {
            printf("Unexpected number of widgets for the signature: %d\n", ffs->getNumWidgets());
            return 2;
        }
        FormWidgetSignature *fws = static_cast<FormWidgetSignature *>(ffs->getWidget(0));
        const bool success = fws->signDocument(std::string { argv[2] }, std::string { certNickname }, std::string { password }, rs.get());
        return success ? 0 : 3;
    }

    if (argc > 2) {
        // We are not signing and more than 1 filename was given
        print_version_usage(true);
        return 99;
    }

    if (sigCount >= 1) {
        if (dumpSignatures) {
            printf("Dumping Signatures: %u\n", sigCount);
            for (unsigned int i = 0; i < sigCount; i++) {
                const bool dumpingOk = dumpSignature(i, sigCount, signatures.at(i), fileName->c_str());
                if (!dumpingOk) {
                    // for now, do nothing. We have logged a message
                    // to the user before returning false in dumpSignature
                    // and it is possible to have "holes" in the signatures
                    continue;
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
        FormFieldSignature *ffs = signatures.at(i);
        printf("Signature #%u:\n", i + 1);
        const GooString *goo = ffs->getCreateWidget()->getField()->getFullyQualifiedName();
        if (goo) {
            const std::string name = TextStringToUTF8(goo->toStr());
            printf("  - Signature Field Name: %s\n", name.c_str());
        }

        if (ffs->getSignatureType() == unsigned_signature_field) {
            printf("  The signature form field is not signed.\n");
            continue;
        }

        const SignatureInfo *sig_info = ffs->validateSignature(!dontVerifyCert, false, -1 /* now */, !noOCSPRevocationCheck, useAIACertFetch);
        printf("  - Signer Certificate Common Name: %s\n", sig_info->getSignerName().c_str());
        printf("  - Signer full Distinguished Name: %s\n", sig_info->getSubjectDN().c_str());
        printf("  - Signing Time: %s\n", time_str = getReadableTime(sig_info->getSigningTime()));
        printf("  - Signing Hash Algorithm: ");
        switch (sig_info->getHashAlgorithm()) {
        case HashAlgorithm::Md2:
            printf("MD2\n");
            break;
        case HashAlgorithm::Md5:
            printf("MD5\n");
            break;
        case HashAlgorithm::Sha1:
            printf("SHA1\n");
            break;
        case HashAlgorithm::Sha256:
            printf("SHA-256\n");
            break;
        case HashAlgorithm::Sha384:
            printf("SHA-384\n");
            break;
        case HashAlgorithm::Sha512:
            printf("SHA-512\n");
            break;
        case HashAlgorithm::Sha224:
            printf("SHA-224\n");
            break;
        default:
            printf("unknown\n");
        }
        printf("  - Signature Type: ");
        switch (ffs->getSignatureType()) {
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
        const std::vector<Goffset> ranges = ffs->getSignedRangeBounds();
        if (ranges.size() == 4) {
            printf("  - Signed Ranges: [%lld - %lld], [%lld - %lld]\n", ranges[0], ranges[1], ranges[2], ranges[3]);
            Goffset checked_file_size;
            const std::optional<GooString> signature = signatures.at(i)->getCheckedSignature(&checked_file_size);
            if (signature && checked_file_size == ranges[3]) {
                printf("  - Total document signed\n");
            } else {
                printf("  - Not total document signed\n");
            }
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
