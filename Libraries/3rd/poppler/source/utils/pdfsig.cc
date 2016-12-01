//========================================================================
//
// pdfsig.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2015 André Guerreiro <aguerreiro1985@gmail.com>
// Copyright 2015 André Esser <bepandre@hotmail.com>
// Copyright 2015 Albert Astals Cid <aacid@kde.org>
// Copyright 2016 Markus Kilås <digital@markuspage.com>
//
//========================================================================

#include "config.h"
#include <poppler-config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include "parseargs.h"
#include "Object.h"
#include "Array.h"
#include "Page.h"
#include "PDFDoc.h"
#include "PDFDocFactory.h"
#include "Error.h"
#include "GlobalParams.h"
#include "SignatureInfo.h"


const char * getReadableSigState(SignatureValidationStatus sig_vs)
{
  switch(sig_vs) {
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

const char * getReadableCertState(CertificateValidationStatus cert_vs)
{
  switch(cert_vs) {
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

char *getReadableTime(time_t unix_time)
{
  char * time_str = (char *) gmalloc(64);
  strftime(time_str, 64, "%b %d %Y %H:%M:%S", localtime(&unix_time));
  return time_str;
}

static GBool printVersion = gFalse;
static GBool printHelp = gFalse;
static GBool dontVerifyCert = gFalse;

static const ArgDesc argDesc[] = {
  {"-nocert", argFlag,     &dontVerifyCert,     0,
   "don't perform certificate validation"},

  {"-v",      argFlag,     &printVersion,  0,
   "print copyright and version info"},
  {"-h",      argFlag,     &printHelp,     0,
   "print usage information"},
  {"-help",   argFlag,     &printHelp,     0,
   "print usage information"},
  {"-?",      argFlag,     &printHelp,     0,
   "print usage information"},
  {NULL}
};


#if defined(MIKTEX)
int Main(int argc, char** argv)
#else
int main(int argc, char *argv[])
#endif
{
  PDFDoc *doc = NULL;
  unsigned int sigCount;
  GooString * fileName = NULL;
  SignatureInfo *sig_info = NULL;
  char *time_str = NULL;
  std::vector<FormWidgetSignature*> sig_widgets;
  globalParams = new GlobalParams();

  int exitCode = 99;
  GBool ok;

  ok = parseArgs(argDesc, &argc, argv);

  if (!ok || argc != 2 || printVersion || printHelp) {
    fprintf(stderr, "pdfsig version %s\n", PACKAGE_VERSION);
    fprintf(stderr, "%s\n", popplerCopyright);
    fprintf(stderr, "%s\n", xpdfCopyright);
    if (!printVersion) {
      printUsage("pdfsig", "<PDF-file>", argDesc);
    }
    if (printVersion || printHelp)
      exitCode = 0;
    goto end;
  }

  fileName = new GooString(argv[argc - 1]);

  // open PDF file
  doc = PDFDocFactory().createPDFDoc(*fileName, NULL, NULL);

  if (!doc->isOk()) {
    exitCode = 1;
    goto end;
  }

  sig_widgets = doc->getSignatureWidgets();
  sigCount = sig_widgets.size();

  if (sigCount >= 1) {
    printf("Digital Signature Info of: %s\n", fileName->getCString());
  } else {
    printf("File '%s' does not contain any signatures\n", fileName->getCString());
    exitCode = 2;
    goto end;
  }

  for (unsigned int i = 0; i < sigCount; i++) {
    sig_info = sig_widgets.at(i)->validateSignature(!dontVerifyCert, false);
    printf("Signature #%u:\n", i+1);
    printf("  - Signer Certificate Common Name: %s\n", sig_info->getSignerName());
    printf("  - Signing Time: %s\n", time_str = getReadableTime(sig_info->getSigningTime()));
    printf("  - Signature Validation: %s\n", getReadableSigState(sig_info->getSignatureValStatus()));
    gfree(time_str);
    if (sig_info->getSignatureValStatus() != SIGNATURE_VALID || dontVerifyCert) {
      continue;
    }
    printf("  - Certificate Validation: %s\n", getReadableCertState(sig_info->getCertificateValStatus()));
  }

  exitCode = 0;

end:
  delete globalParams;
  delete fileName;
  delete doc;

  return exitCode;
}
